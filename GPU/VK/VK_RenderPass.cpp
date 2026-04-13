#include "VK_RenderPass.h"

#include "VK_wrappar.h"
#include "VK_Backend.h"

#include <array>

namespace GPU
{

	void VK_RenderPass::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		const bool IsDynamic = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;

		if (!IsDynamic)
		{
			vkDestroyRenderPass(pDevice, pRenderPass, NULL);

			for (uint32_t i = 0; i < pFramebuffers.size(); i++)
				vkDestroyFramebuffer(pDevice, pFramebuffers[i], NULL);
		}
	}

	void VK_RenderPass::Begin(RHI::GPU_Clear pClear)
	{
		VK_Backend::Get()->GetCurrentRenderPassInfo() = pRenderPassInfo;
		VK_Backend::Get()->GetCurrentRenderPass()	  = pRenderPass;
		pClearValue = pClear;

		VK_Backend::Get()->GetDrawCmdsArray()->push_back(
			[&](VkCommandBuffer Cmd, uint32_t ImageIndex)
			{
				StartPass(Cmd, ImageIndex);
			}
		);
	}

	void VK_RenderPass::End()
	{
		VK_Backend::Get()->GetDrawCmdsArray()->push_back(
			[&](VkCommandBuffer Cmd, uint32_t ImageIndex)
			{
				EndPass(Cmd, ImageIndex);
			}
		);

		RHI::GPU_RenderPassInfo pNullDrawInfo{};
		VK_Backend::Get()->GetCurrentRenderPassInfo() = pNullDrawInfo;
		VK_Backend::Get()->GetCurrentRenderPass() = VK_NULL_HANDLE;
	}

	void VK_RenderPass::StartPass(const VkCommandBuffer& pCmdBuf, uint32_t ImageIndex)
	{
		const bool IsDynamic = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;

		// # Begin with dynamic 
		if (IsDynamic)
		{
			if (pRenderPassInfo.pEnableColor)
			{
				for (uint32_t i = 0; i < pImageFrames[ImageIndex].pColorImages.size(); i++)
				{
					ImageMemBarrier(
						pCmdBuf, pImageFrames[ImageIndex].pColorImages[i], pImageFrames[ImageIndex].pColorImageFormats[i],
						VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1
					);
				}
			}

			auto pC = pClearValue.pColor;
			auto pD = pClearValue.pDepth;
			VkClearValue clearColor = { .color = { pC.r, pC.g, pC.b, pC.a } };
			VkClearValue clearDepth = { .depthStencil{.depth = pD.depth, .stencil = pD.stencil} };

			BeginDynamicRendering(
				pCmdBuf,
				pImageFrames[ImageIndex].pColorImageViews.data(), (uint32_t)pImageFrames[ImageIndex].pColorImageViews.size(),
				&pImageFrames[ImageIndex].pDepthImageView,
				&clearColor, &clearDepth,
				pRenderPassInfo.pEnableColor, pRenderPassInfo.pEnableDepth,
				pRenderPassInfo.pRenderArea
			);
		}
		// # Begin with render pass
		else
		{
			// # Clear components
			uint32_t pClearValueCount = 0;
			pClearValueCount += pRenderPassInfo.pEnableColor ? 1 : 0;
			pClearValueCount += pRenderPassInfo.pEnableDepth ? 1 : 0;

			std::array<VkClearValue, 2> pClearValues;
			if (pRenderPassInfo.pEnableColor)
			{
				auto pC = pClearValue.pColor;
				pClearValues[0].color = { pC.r, pC.g, pC.b, pC.a };

				if (pRenderPassInfo.pEnableDepth)
				{
					auto pD = pClearValue.pDepth;
					pClearValues[1].depthStencil = { .depth = pD.depth, .stencil = pD.stencil };
				}
			}
			else if (pRenderPassInfo.pEnableDepth)
			{
				auto pD = pClearValue.pDepth;
				pClearValues[0].depthStencil = { .depth = pD.depth, .stencil = pD.stencil };
			}
			
			// # Render pass begin info
			VkRenderPassBeginInfo BeginInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = NULL,
				.renderPass = pRenderPass,
				.framebuffer = pFramebuffers[ImageIndex],
				.renderArea = {
					.offset = { .x = (int32_t)pRenderPassInfo.pRenderArea.pOffset.x, .y = (int32_t)pRenderPassInfo.pRenderArea.pOffset.y },
					.extent = {.width = pRenderPassInfo.pRenderArea.pExtent.width, .height = pRenderPassInfo.pRenderArea.pExtent.height }
				},
				.clearValueCount = pClearValueCount,
				.pClearValues = &pClearValues[0],
			};

			// # Begin the render pass
			vkCmdBeginRenderPass(pCmdBuf, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	void VK_RenderPass::EndPass(const VkCommandBuffer& pCmdBuf, uint32_t ImageIndex)
	{
		const bool IsDynamic = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;

		// # End wit dynamic
		if (IsDynamic)
		{
			vkCmdEndRendering(pCmdBuf);

			if (pRenderPassInfo.pEnableColor)
			{
				for (uint32_t i = 0; i < pImageFrames[ImageIndex].pColorImages.size(); i++)
				{
					ImageMemBarrier(
						pCmdBuf, pImageFrames[ImageIndex].pColorImages[i], pImageFrames[ImageIndex].pColorImageFormats[i],
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1
					);
				}
			}
		}
		// # End with render pass
		else
		{
			vkCmdEndRenderPass(pCmdBuf);
		}
	}

	void VK_RenderPass::Create(const RHI::GPU_RenderPassInfo& pInfo)
	{
		pRenderPassInfo = pInfo;

		// # Create image frames
		for (uint32_t i = 0; i < VK_Backend::Get()->GetSwapChain().GetImageCount(); i++)
		{
			Frame_Data pFrameData;

			if (pInfo.pEnableColor)
			{
				for (uint32_t j = 0; j < pInfo.pColorTexCount; j++)
				{
					const VK_Texture* pTex = reinterpret_cast<const VK_Texture*>(&pInfo.pColorTextures[j]);
					// # Get color images & image views
					pFrameData.pColorImages.push_back(
						pTex->GetImage(i)
					);

					pFrameData.pColorImageViews.push_back(
						pTex->GetView(i)
					);

					pFrameData.pColorImageFormats.push_back(
						pTex->GetFormat()
					);

				}
			}

			// # Get depth image & image view
			if (pInfo.pEnableDepth)
			{
				const VK_Texture* pTexDepth = reinterpret_cast<const VK_Texture*>(pInfo.pDepthTexture);

				pFrameData.pDepthImage = pTexDepth->GetImage(i);
				pFrameData.pDepthImageView = pTexDepth->GetView(i);
				pFrameData.pDepthFormat = pTexDepth->GetFormat();
			}

			// # Add the frame 
			pImageFrames.push_back(pFrameData);
		}

		const bool pIsDynamic = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;

		if (!pIsDynamic)
		{
			CreateRenderPass();
			CreateFramebuffers();
		}
	}

	void VK_RenderPass::CreateRenderPass()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		// # Color attachments
		std::vector<VkAttachmentDescription> pColorAttachDescs;
		std::vector<VkAttachmentReference> pColorAttachRefs;

		if (pRenderPassInfo.pEnableColor)
		{
			for (uint32_t i = 0; i < pRenderPassInfo.pColorTexCount; i++)
			{
				const VK_Texture* pTexColor = reinterpret_cast<const VK_Texture*>(&pRenderPassInfo.pColorTextures[i]);

				VkAttachmentDescription ColorAttachDesc = {
					.flags = 0,
					.format = pTexColor->GetFormat(),
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				};
				pColorAttachDescs.push_back(ColorAttachDesc);

				VkAttachmentReference ColorAttachRef = {
					.attachment = i,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				};
				pColorAttachRefs.push_back(ColorAttachRef);
			}
		}

		// # Depth attachment
		VkAttachmentDescription DepthAttachDesc;
		VkAttachmentReference DepthAttachRef;

		if (pRenderPassInfo.pEnableDepth)
		{
			const VK_Texture* pTexDepth = reinterpret_cast<const VK_Texture*>(pRenderPassInfo.pDepthTexture);

			DepthAttachDesc = {
				.flags = 0,
				.format = pTexDepth->GetFormat(),
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			};

			DepthAttachRef = {
				.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};
		}

		// # Subpass description
		VkSubpassDescription SubpassDesc = {
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = pRenderPassInfo.pEnableColor ? pRenderPassInfo.pColorTexCount : 0,
			.pColorAttachments = pRenderPassInfo.pEnableColor ? pColorAttachRefs.data() : NULL,
			.pResolveAttachments = NULL,
			.pDepthStencilAttachment = pRenderPassInfo.pEnableDepth ? &DepthAttachRef : NULL,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL
		};

		std::vector<VkAttachmentDescription> Attachments;
		
		// # Color attachments
		if (pRenderPassInfo.pEnableColor)
		{
			for (uint32_t i = 0; i < pRenderPassInfo.pColorTexCount; i++)
			{
				Attachments.push_back(pColorAttachDescs[i]);
			}
		}
		// # Depth attachment
		if (pRenderPassInfo.pEnableDepth)
		{
			Attachments.push_back(DepthAttachDesc);
		}

		// # Render pass create info
		VkRenderPassCreateInfo RenderPassCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.attachmentCount = (uint32_t)Attachments.size(),
			.pAttachments = Attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &SubpassDesc,
			.dependencyCount = 0,
			.pDependencies = NULL
		};

		VkResult res = vkCreateRenderPass(
			pDevice, &RenderPassCreateInfo, NULL, &pRenderPass
		);
		VK_CHECK("vkCreateRenderPass: VK_RenderPass", res);
	}

	void VK_RenderPass::CreateFramebuffers()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		const uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();
		pFramebuffers.resize(NumImages);

		for (uint32_t i = 0; i < NumImages; i++)
		{
			std::vector<VkImageView> Attachments;

			// # Color image views
			if (pRenderPassInfo.pEnableColor)
			{
				for (uint32_t j = 0; j < pImageFrames[i].pColorImages.size(); j++)
				{
					Attachments.push_back(pImageFrames[i].pColorImageViews[j]);
				}
			}
			// # Depth image view
			if (pRenderPassInfo.pEnableDepth)
			{
				Attachments.push_back(pImageFrames[i].pDepthImageView);
			}

			auto ImageSize = reinterpret_cast<const VK_Texture*>(&pRenderPassInfo.pColorTextures[i])->GetImageSize();

			VkFramebufferCreateInfo CreateInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = pRenderPass,
				.attachmentCount = (uint32_t)Attachments.size(),
				.pAttachments = Attachments.data(),
				.width = pRenderPassInfo.pRenderArea.pExtent.width,
				.height = pRenderPassInfo.pRenderArea.pExtent.height,
				.layers = 1
			};

			VkResult res = vkCreateFramebuffer(
				pDevice, &CreateInfo, NULL, &pFramebuffers[i]
			);
			VK_CHECK("vkCreateFramebuffer: VK_RenderPass", res);
		}

	}

}