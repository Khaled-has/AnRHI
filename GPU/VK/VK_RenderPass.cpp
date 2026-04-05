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

		for (uint32_t i = 0; i < pColorImages.pImages.size(); i++)
		{
			if (!IsDynamic)
			{
				vkDestroyFramebuffer(pDevice, pFramebuffers[i], NULL);
			}

			if (pDrawInfo.pEnableColor)
			{
				vkFreeMemory(pDevice, pColorImages.pMemories[i], NULL);
				vkDestroySampler(pDevice, pColorImages.pSamplers[i], NULL);
				vkDestroyImageView(pDevice, pColorImages.pImageViews[i], NULL);
				vkDestroyImage(pDevice, pColorImages.pImages[i], NULL);
			}
			if (pDrawInfo.pEnableDepth)
			{
				vkFreeMemory(pDevice, pDepthImages.pMemories[i], NULL);
				vkDestroySampler(pDevice, pDepthImages.pSamplers[i], NULL);
				vkDestroyImageView(pDevice, pDepthImages.pImageViews[i], NULL);
				vkDestroyImage(pDevice, pDepthImages.pImages[i], NULL);
			}
		}

		if (!IsDynamic)
		{
			vkDestroyRenderPass(pDevice, pRenderPass, NULL);
		}
	}

	void VK_RenderPass::Begin(RHI::GPU_Clear pClear)
	{
		VK_Backend::Get()->GetCurrentDrawInfo() = pDrawInfo;
		VK_Backend::Get()->GetCurrentRenderPass() = pRenderPass;
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

		VK_RenderPassDrawInfo pNullDrawInfo{};
		VK_Backend::Get()->GetCurrentDrawInfo() = pNullDrawInfo;
		VK_Backend::Get()->GetCurrentRenderPass() = VK_NULL_HANDLE;
	}

	void VK_RenderPass::StartPass(const VkCommandBuffer& pCmdBuf, uint32_t ImageIndex)
	{
		const bool IsDynamic = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;

		// # Begin with dynamic 
		if (IsDynamic)
		{
			if (pDrawInfo.pEnableColor)
			{
				ImageMemBarrier(
					pCmdBuf, pColorImages.pImages[ImageIndex], pDrawInfo.pColorFormats[0],
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1
				);
			}

			auto pC = pClearValue.pColor;
			auto pD = pClearValue.pDepth;
			VkClearValue clearColor = { .color = { pC.r, pC.g, pC.b, pC.a } };
			VkClearValue clearDepth = { .depthStencil{.depth = pD.depth, .stencil = pD.stencil} };

			BeginDynamicRendering(
				pCmdBuf, 
				pDrawInfo.pEnableColor ? &pColorImages.pImageViews[ImageIndex] : NULL, 
				pDrawInfo.pEnableDepth ? &pDepthImages.pImageViews[ImageIndex] : NULL, 
				ImageIndex, 
				&clearColor, &clearDepth, 
				pDrawInfo.pEnableColor, pDrawInfo.pEnableDepth,
				pDrawInfo.pWidth, pDrawInfo.pHeight
			);
		}
		// # Begin with render pass
		else
		{
			// # Clear components
			uint32_t pClearValueCount = 0;
			pClearValueCount += pDrawInfo.pEnableColor ? 1 : 0;
			pClearValueCount += pDrawInfo.pEnableDepth ? 1 : 0;

			std::array<VkClearValue, 2> pClearValues;
			if (pDrawInfo.pEnableColor)
			{
				auto pC = pClearValue.pColor;
				pClearValues[0].color = { pC.r, pC.g, pC.b, pC.a };

				if (pDrawInfo.pEnableDepth)
				{
					auto pD = pClearValue.pDepth;
					pClearValues[1].depthStencil = { .depth = pD.depth, .stencil = pD.stencil };
				}
			}
			else if (pDrawInfo.pEnableDepth)
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
					.offset = {.x = 0, .y = 0}, // It's hard code for now
					.extent = {.width = pDrawInfo.pWidth, .height = pDrawInfo.pHeight }
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

			if (pDrawInfo.pEnableColor)
			{
				ImageMemBarrier(
					pCmdBuf, pColorImages.pImages[ImageIndex], pDrawInfo.pColorFormats[0],
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1
				);
			}
		}
		// # End with render pass
		else
		{
			vkCmdEndRenderPass(pCmdBuf);
		}
	}

	void VK_RenderPass::Create(const RHI::GPU_RenderPassInfo pInfo)
	{
		pCreateInfo = pInfo;
		CreateDrawInfo();

		if (pInfo.pEnableColor)
		{
			CreateColorImages();
		}
		if (pInfo.pEnableDepth)
		{
			CreateDepthImages();
		}

		const bool pIsDynamic = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;

		if (!pIsDynamic)
		{
			CreateRenderPass();
			CreateFramebuffers();
		}
	}

	void VK_RenderPass::CreateColorImages()
	{
		const uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkFormat pFormat = pDrawInfo.pColorFormats[0];

		// # Step 1: create the images
		pColorImages.pImages.resize(NumImages);
		pColorImages.pMemories.resize(NumImages);
		for (uint32_t i = 0; i < pColorImages.pImages.size(); i++)
		{
			VkImageUsageFlags Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			VkImageCreateInfo ImageInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = NULL,
				.flags = (VkImageCreateFlags)0,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = pFormat,
				.extent = VkExtent3D{ .width = pDrawInfo.pWidth, .height = pDrawInfo.pHeight, .depth = 1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = Usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = NULL,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			// # 1 Create the image object
			VkResult res = vkCreateImage(pDevice, &ImageInfo, NULL, &pColorImages.pImages[i]);
			VK_CHECK("vkCreateImage: VK_RenderPass", res);

			// # 2 Get the buffer memory requirements
			VkMemoryRequirements MemReqs = { 0 };
			vkGetImageMemoryRequirements(pDevice, pColorImages.pImages[i], &MemReqs);

			// # Step 3: get the memory type index
			uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);

			// # Step 4: allocate memory
			VkMemoryAllocateInfo MemAllocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = MemReqs.size,
				.memoryTypeIndex = MemoryTypeIndex
			};

			res = vkAllocateMemory(pDevice, &MemAllocInfo, NULL, &pColorImages.pMemories[i]);
			VK_CHECK("vkAllocateMemory", res);

			// # Step 5: bind memory
			res = vkBindImageMemory(pDevice, pColorImages.pImages[i], pColorImages.pMemories[i], 0);
			VK_CHECK("vkBindImageMemory", res);
		}

		// # Step 2: create image views
		pColorImages.pImageViews.resize(NumImages);
		for (uint32_t i = 0; i < pColorImages.pImageViews.size(); i++)
		{
			VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
			pColorImages.pImageViews[i] = CreateImageView(pColorImages.pImages[i], pDevice, pFormat, AspectFlags);
		}

		// # Step 3: create samplers
		pColorImages.pSamplers.resize(NumImages);
		for (uint32_t i = 0; i < pColorImages.pSamplers.size(); i++)
		{
			VkFilter MinFilter = VK_FILTER_LINEAR;
			VkFilter MaxFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			pColorImages.pSamplers[i] = CreateTextureSampler(MinFilter, MaxFilter, AddressMode);
		}

		pFrameImage.pImages = pColorImages.pImages;
		pFrameImage.pViews = pColorImages.pImageViews;
		pFrameImage.pSamplers = pColorImages.pSamplers;
	}

	void VK_RenderPass::CreateDepthImages()
	{
		const uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkFormat pFormat = pDrawInfo.pDepthFormat;

		// # Step 1: create the images
		pDepthImages.pImages.resize(NumImages);
		pDepthImages.pMemories.resize(NumImages);
		for (uint32_t i = 0; i < pDepthImages.pImages.size(); i++)
		{
			VkImageUsageFlags Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			VkImageCreateInfo ImageInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = NULL,
				.flags = (VkImageCreateFlags)0,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = pFormat,
				.extent = VkExtent3D{.width = pDrawInfo.pWidth, .height = pDrawInfo.pHeight, .depth = 1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = Usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = NULL,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			// # 1 Create the image object
			VkResult res = vkCreateImage(pDevice, &ImageInfo, NULL, &pDepthImages.pImages[i]);
			VK_CHECK("vkCreateImage: VK_RenderPass", res);

			// # 2 Get the buffer memory requirements
			VkMemoryRequirements MemReqs = { 0 };
			vkGetImageMemoryRequirements(pDevice, pDepthImages.pImages[i], &MemReqs);

			// # Step 3: get the memory type index
			uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);

			// # Step 4: allocate memory
			VkMemoryAllocateInfo MemAllocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = MemReqs.size,
				.memoryTypeIndex = MemoryTypeIndex
			};

			res = vkAllocateMemory(pDevice, &MemAllocInfo, NULL, &pDepthImages.pMemories[i]);
			VK_CHECK("vkAllocateMemory", res);

			// # Step 5: bind memory
			res = vkBindImageMemory(pDevice, pDepthImages.pImages[i], pDepthImages.pMemories[i], 0);
			VK_CHECK("vkBindImageMemory", res);
		}

		// # Step 2: create image views
		pDepthImages.pImageViews.resize(NumImages);
		for (uint32_t i = 0; i < pDepthImages.pImageViews.size(); i++)
		{
			VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			pDepthImages.pImageViews[i] = CreateImageView(pDepthImages.pImages[i], pDevice, pFormat, AspectFlags);
		}

		// # Step 3: create samplers
		pDepthImages.pSamplers.resize(NumImages);
		for (uint32_t i = 0; i < pDepthImages.pSamplers.size(); i++)
		{
			VkFilter MinFilter = VK_FILTER_LINEAR;
			VkFilter MaxFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			pDepthImages.pSamplers[i] = CreateTextureSampler(MinFilter, MaxFilter, AddressMode);
		}
	}

	void VK_RenderPass::CreateDrawInfo()
	{
		pDrawInfo.pEnableColor = pCreateInfo.pEnableColor;
		pDrawInfo.pEnableDepth = pCreateInfo.pEnableDepth;

		pDrawInfo.pWidth = pCreateInfo.pWidth;
		pDrawInfo.pHeight = pCreateInfo.pHeight;
		
		// # Choose color format
		if (pDrawInfo.pEnableColor)
		{
			for (uint32_t i = 0; i < pCreateInfo.pColorFormats.size(); i++)
			{	
				switch (pCreateInfo.pColorFormats[i])
				{

				case RHI::GPU_FORMAT_COLOR_RGBA8:
					pDrawInfo.pColorFormats.push_back(VK_FORMAT_R8G8B8A8_UNORM);
					break;

				case RHI::GPU_FORMAT_COLOR_BGRA8:
					pDrawInfo.pColorFormats.push_back(VK_FORMAT_B8G8R8A8_UNORM);
					break;

				default:
					pDrawInfo.pColorFormats.push_back(VK_FORMAT_UNDEFINED);
					break;
				}
			}
		}
		// # Choose depth format
		if (pDrawInfo.pEnableDepth)
		{
			switch (pCreateInfo.pDepthFormat)
			{
			case RHI::GPU_FORMAT_D32_FLOAT:
				pDrawInfo.pDepthFormat = VK_FORMAT_D32_SFLOAT;
				break;

			default:
				pDrawInfo.pDepthFormat = VK_FORMAT_UNDEFINED;
				break;
			}
		}
	}

	void VK_RenderPass::CreateRenderPass()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		// # Color attachments
		std::vector<VkAttachmentDescription> pColorAttachDescs;
		std::vector<VkAttachmentReference> pColorAttachRefs;
		for (uint32_t i = 0; i < pDrawInfo.pColorFormats.size(); i++)
		{
			VkAttachmentDescription ColorAttachDesc = {
				.flags = 0,
				.format = pDrawInfo.pColorFormats[i],
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
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
			pColorAttachRefs.push_back(ColorAttachRef);
		}

		// # Depth attachment
		VkAttachmentDescription DepthAttachDesc = {
			.flags = 0,
			.format = pDrawInfo.pDepthFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentReference DepthAttachRef = {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		// # Subpass description
		VkSubpassDescription SubpassDesc = {
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = pDrawInfo.pEnableColor ? (uint32_t)pDrawInfo.pColorFormats.size() : 0,
			.pColorAttachments = pDrawInfo.pEnableColor ? pColorAttachRefs.data() : NULL,
			.pResolveAttachments = NULL,
			.pDepthStencilAttachment = pDrawInfo.pEnableDepth ? &DepthAttachRef : NULL,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL
		};

		std::vector<VkAttachmentDescription> Attachments;
		
		// # Color attachments
		if (pDrawInfo.pEnableColor)
		{
			for (uint32_t i = 0; i < pDrawInfo.pColorFormats.size(); i++)
			{
				Attachments.push_back(pColorAttachDescs[i]);
			}
		}
		// # Depth attachment
		if (pDrawInfo.pEnableDepth)
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
			if (pDrawInfo.pEnableColor)
			{
				/*for (uint32_t j = 0; j < pDrawInfo.pColorFormats.size(); j++)
				{
					uint32_t ImageIndex = NumImages * j + i;
					Attachments.push_back(pColorImages.pImageViews[ImageIndex]);
				}*/
				Attachments.push_back(pColorImages.pImageViews[i]);
			}
			// # Depth image view
			if (pDrawInfo.pEnableDepth)
			{
				Attachments.push_back(pDepthImages.pImageViews[i]);
			}

			VkFramebufferCreateInfo CreateInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = pRenderPass,
				.attachmentCount = (uint32_t)Attachments.size(),
				.pAttachments = Attachments.data(),
				.width = pDrawInfo.pWidth,
				.height = pDrawInfo.pHeight,
				.layers = 1
			};

			VkResult res = vkCreateFramebuffer(
				pDevice, &CreateInfo, NULL, &pFramebuffers[i]
			);
			VK_CHECK("vkCreateFramebuffer: VK_RenderPass", res);
		}

	}

}