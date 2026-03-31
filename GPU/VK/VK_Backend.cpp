#include "VK_Backend.h"

#include "VK_wrappar.h"
#include "VK_Shader.h"

#include "GPU/Backends/GPU_LibBackend.h"

#include <array>

namespace GPU {

	VK_Backend* VK_Backend::pVkInstance = nullptr;

	VK_Backend::VK_Backend()
	{
		pVkInstance = this;
	}

	bool VK_Backend::IsDeviceSupportBackend()
	{
		return false;
	}

	void VK_Backend::Backend_Init()
	{
		// 1 # Create device
		pDevice.Create();
		// 2 # Create swapchain
		pSwapChain.Create(false);
		// 3 # Create command buffer pool
		pCmdBufPool.Create();
		// 4 # Create command buffers
		pCmdBufs.resize(pSwapChain.GetImageCount());
		pCmdBufPool.CreateCommandBuffers(pSwapChain.GetImageCount(), pCmdBufs.data());
		pCmdBufPool.CreateCommandBuffers(1, &pCopyCmdBuf);
		// 5 # Create load thread
		pLoadThread.Create();
		// 6 # Create queue
		pQueue.Create();
	}

	void VK_Backend::Backend_Exit()
	{
		// # Destroy screen image graphics pipeline
		pScreenImageBuffer.Destroy();
		pScreenImageShader.Destroy();
		pScreenImagePipeline.Destroy();
		// # Destroy queue
		pQueue.Destroy();
		// # Destroy swapchain
		pSwapChain.Destroy();
		// # Free command buffers
		pCmdBufPool.FreeCommandBuffers((uint32_t)pCmdBufs.size(), pCmdBufs.data());
		// # Destroy command buffer pool
		pCmdBufPool.Destroy();
		// # Destroy load thread
		pLoadThread.Destroy();
		// # Destroy device
		pDevice.Destroy();
	}

	void VK_Backend::BeginRendering()
	{
		pQueue.AcquireNextImage();
	}


	void VK_Backend::EndRendering()
	{
		uint32_t pImageIndex = pQueue.GetAcquiredImageIndex();

		pQueue.SubmitAsync(&pCmdBufs[pImageIndex]);

		pQueue.Present(pImageIndex);
	}

	std::vector<std::function<void(VkCommandBuffer, uint32_t)>>* VK_Backend::GetDrawCmdsArray()
	{
		return &pDrawCmdFuncs;
	}

	void VK_Backend::CreateScreenImageResources(VK_RenderPass* pFinalFrame)
	{
		const std::vector<float> pVertices = {
			//  Pos        UV
			-1.0, -1.0, 0.0, 0.0,
			 1.0, -1.0, 1.0, 0.0,
			 1.0,  1.0, 1.0, 1.0,

			-1.0, -1.0, 0.0, 0.0,
			 1.0,  1.0, 1.0, 1.0,
			-1.0,  1.0, 0.0, 1.0
		};

		pScreenImageBuffer.Create(
			pVertices.data(), 
			size_t(pVertices.size() * sizeof(float)), 
			RHI::GPU_BUFFER_TYPE_STATIC
		);

		std::vector<VK_PipelineBinding> pBindings;
		pBindings.push_back(
			VK_PipelineBinding{
				.pBinding = 0,
				.pDescType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pStageFlag = VK_SHADER_STAGE_VERTEX_BIT,
				.pBindingType = VK_BINDING_BUFFER_INFO,
				.pBuffer = &pScreenImageBuffer
			}
		);

		pBindings.push_back(
			VK_PipelineBinding{
				.pBinding = 1,
				.pDescType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pBindingType = VK_BINDING_FRAME_IMAGE_INFO,
				.pFrameImage = pFinalFrame->GetFrameImage()
			}
		);

		const char* pVertexShader =
			"#version 460\n"
			"layout (location = 0) out vec2 UVs;\n"
			"struct VertexData{\n"
			"float x, y, u, v;\n"
			"};\n"
			"layout (binding = 0) readonly buffer vertices { VertexData vtx[]; } in_vertices;\n"
			"void main() {\n"
			"vec2 pos = vec2(in_vertices.vtx[gl_VertexIndex].x, in_vertices.vtx[gl_VertexIndex].y);\n"
			"UVs = vec2(in_vertices.vtx[gl_VertexIndex].u, in_vertices.vtx[gl_VertexIndex].v);\n"
			"gl_Position = vec4(pos, 0.0, 1.0);\n"
			"}\n";

		const char* pFragmentShader =
			"#version 460\n"
			"layout (location = 0) in vec2 UVs;\n"
			"layout (location = 0) out vec4 out_Color;\n"
			"layout (binding = 1) uniform sampler2D texture1;\n"
			"void main() {\n"
			"out_Color = texture(texture1, UVs);\n"
			"}\n";
		pScreenImageShader.InitFromText(pVertexShader, pFragmentShader);

		pScreenImagePipeline.Create(&pBindings);
		const std::vector<VkFormat> pColorFormats = { pSwapChain.GetSurfaceFormat().format };
		pCurrentRenderPassDrawInfo = {
			.pEnableColor = true,
			.pEnableDepth = false,
			.pColorFormats = pColorFormats,
			.pDepthFormat = VK_FORMAT_UNDEFINED,
			.pWidth = 1440,
			.pHeight = 720
		};
		pCurrentRenderPass = pSwapChain.GetRenderPass();
		pCurrentShader = pScreenImageShader;
		pScreenImagePipeline.CreatePipeline();
	}

	void VK_Backend::BeginRecord()
	{
		pDrawCmdFuncs.clear();
	}

	void VK_Backend::EndRecord(RHI::GPU_RenderPass* pFinalRenderPass)
	{
		CreateScreenImageResources((VK_RenderPass*)pFinalRenderPass);

		const VkSurfaceFormatKHR pFormat = VK_Backend::Get()->GetSwapChain().GetSurfaceFormat();
		const bool pIsDynamicSupported = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;
		const bool pIsDepthTest = VK_Backend::Get()->GetSwapChain().IsDepthTest();

		const VkRenderPass& pRenderPass = VK_Backend::Get()->GetSwapChain().GetRenderPass();

		// # RenderPass begin info
		std::array<VkClearValue, 2> pClear;
		pClear[0].color = { 0.0, 0.0, 0.0, 1.0 };
		pClear[1].depthStencil = { .depth = 1.0f, .stencil = 0 };

		auto pWinSize = lib_backend::GPU_LibBackend::GetInstance()->GetWindowSize();

		VkRenderPassBeginInfo RenderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = NULL,
			.renderPass = pRenderPass,
			.renderArea = {
				.offset = {
					.x = 0, .y = 0
				},
				.extent = {
					.width = pWinSize.pWidth,
					.height = pWinSize.pHeight
				}
			},
			.clearValueCount = pIsDepthTest ? 2u : 1u,
			.pClearValues = &pClear[0],
		};

		for (uint32_t i = 0; i < VK_Backend::Get()->GetSwapChain().GetImageCount(); i++)
		{
			const VkCommandBuffer& CmdBuf = VK_Backend::Get()->GetCmdBuf(i);
			const VkImage& Image = VK_Backend::Get()->GetSwapChain().GetImage(i);
			const VkImageView& View = pSwapChain.GetImageView(i);

			BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			// # Submit the draw commands
			for (auto& Cmd : pDrawCmdFuncs)
			{
				Cmd(CmdBuf, i);
			}

			// # Begin rendering with Dynamic
			if (pIsDynamicSupported)
			{
				ImageMemBarrier(
					CmdBuf, Image, pFormat.format,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1
				);

				BeginDynamicRendering(CmdBuf, &View, NULL, i, &pClear[0], &pClear[1], true, false);
			}
			// # Begin rendering with RenderPass
			else
			{
				RenderPassBeginInfo.framebuffer = VK_Backend::Get()->GetSwapChain().GetFramebuffer(i);
				vkCmdBeginRenderPass(CmdBuf, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			}

			// -- # Draw screen image ( up scaling to the window size )
			pScreenImagePipeline.Bind(i);
			vkCmdDraw(CmdBuf, 6, 1, 0, 0);

			// # End rendering with Dynamic
			if (pIsDynamicSupported)
			{
				vkCmdEndRendering(CmdBuf);

				ImageMemBarrier(
					CmdBuf, Image, pFormat.format,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1
				);
			}
			// # End rendering with RenderPass
			else
			{
				vkCmdEndRenderPass(CmdBuf);
			}

			VkResult res = vkEndCommandBuffer(CmdBuf);
			VK_CHECK("vkEndCommandBuffer", res);
		}
	}

}