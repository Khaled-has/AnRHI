#include "VK_Backend.h"

#include "VK_wrappar.h"
#include "VK_Shader.h"

#include "Backends/GPU_LibBackend.h"

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

#ifdef RHI_IMGUI_ENABLE

	void VK_Backend::ImGui_Init()
	{
	}

	void VK_Backend::ImGui_Exit()
	{
	}

	void VK_Backend::ImGui_Rendering()
	{
	}

#endif

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

	void VK_Backend::CreateScreenImageResources(const VK_Texture* pFinalTexture)
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
				.pBindingType = VK_BINDING_IMAGE_INFO,
				.pTexture = pFinalTexture
			}
		);

		const std::vector<uint32_t> VertexShaderBin = {
			119734787, 65536, 851979, 54, 0, 131089, 1, 393227, 1, 1280527431, 1685353262, 808793134,
			0, 196622, 0, 1, 524303, 0, 4, 1852399981, 0, 18, 29, 45, 196611, 2, 460, 655364, 1197427783,
			1279741775, 1885560645, 1953718128, 1600482425, 1701734764, 1919509599, 1769235301, 25974,
			524292, 1197427783, 1279741775, 1852399429, 1685417059, 1768185701, 1952671090, 6649449,
			262149, 4, 1852399981, 0, 196613, 9, 7565168, 327685, 10, 1953654102, 1631877221, 24948,
			262150, 10, 0, 120, 262150, 10, 1, 121, 262150, 10, 2, 117, 262150, 10, 3, 118, 327685, 12,
			1953654134, 1936024425, 0, 262150, 12, 0, 7894134, 327685, 14, 1985965673, 1769239141, 7562595,
			393221, 18, 1449094247, 1702130277, 1684949368, 30821, 196613, 29, 7558741, 393221, 43, 1348430951,
			1700164197, 2019914866, 0, 393222, 43, 0, 1348430951, 1953067887, 7237481, 458758, 43, 1, 1348430951,
			1953393007, 1702521171, 0, 458758, 43, 2, 1130327143, 1148217708, 1635021673, 6644590, 458758, 43, 3,
			1130327143, 1147956341, 1635021673, 6644590, 196613, 45, 0, 327752, 10, 0, 35, 0, 327752, 10, 1, 35, 4,
			327752, 10, 2, 35, 8, 327752, 10, 3, 35, 12, 262215, 11, 6, 16, 196679, 12, 3, 262216, 12, 0, 24, 327752,
			12, 0, 35, 0, 196679, 14, 24, 262215, 14, 33, 0, 262215, 14, 34, 0, 262215, 18, 11, 42, 262215, 29, 30, 0,
			196679, 43, 2, 327752, 43, 0, 11, 0, 327752, 43, 1, 11, 1, 327752, 43, 2, 11, 3, 327752, 43, 3, 11, 4,
			131091, 2, 196641, 3, 2, 196630, 6, 32, 262167, 7, 6, 2, 262176, 8, 7, 7, 393246, 10, 6, 6, 6, 6, 196637,
			11, 10, 196638, 12, 11, 262176, 13, 2, 12, 262203, 13, 14, 2, 262165, 15, 32, 1, 262187, 15, 16, 0, 262176,
			17, 1, 15, 262203, 17, 18, 1, 262176, 20, 2, 6, 262187, 15, 24, 1, 262176, 28, 3, 7, 262203, 28, 29, 3, 262187,
			15, 31, 2, 262187, 15, 35, 3, 262167, 39, 6, 4, 262165, 40, 32, 0, 262187, 40, 41, 1, 262172, 42, 6, 41, 393246,
			43, 39, 6, 42, 42, 262176, 44, 3, 43, 262203, 44, 45, 3, 262187, 6, 47, 0, 262187, 6, 48, 1065353216, 262176,
			52, 3, 39, 327734, 2, 4, 0, 3, 131320, 5, 262203, 8, 9, 7, 262205, 15, 19, 18, 458817, 20, 21, 14, 16, 19, 16,
			262205, 6, 22, 21, 262205, 15, 23, 18, 458817, 20, 25, 14, 16, 23, 24, 262205, 6, 26, 25, 327760, 7, 27, 22, 26,
			196670, 9, 27, 262205, 15, 30, 18, 458817, 20, 32, 14, 16, 30, 31, 262205, 6, 33, 32, 262205, 15, 34, 18, 458817,
			20, 36, 14, 16, 34, 35, 262205, 6, 37, 36, 327760, 7, 38, 33, 37, 196670, 29, 38, 262205, 7, 46, 9, 327761, 6, 49,
			46, 0, 327761, 6, 50, 46, 1, 458832, 39, 51, 49, 50, 47, 48, 327745, 52, 53, 45, 16, 196670, 53, 51, 65789, 65592
		};

		const std::vector<uint32_t> FragmentShaderBin = {
			119734787, 65536, 851979, 20, 0, 131089, 1, 393227, 1, 1280527431, 1685353262, 808793134, 0, 196622, 0, 1, 458767,
			4, 4, 1852399981, 0, 9, 17, 196624, 4, 7, 196611, 2, 460, 655364, 1197427783, 1279741775, 1885560645, 1953718128,
			1600482425, 1701734764, 1919509599, 1769235301, 25974, 524292, 1197427783, 1279741775, 1852399429, 1685417059, 1768185701,
			1952671090, 6649449, 262149, 4, 1852399981, 0, 327685, 9, 1601467759, 1869377347, 114, 327685, 13, 1954047348, 828732021,
			0, 196613, 17, 7558741, 262215, 9, 30, 0, 262215, 13, 33, 1, 262215, 13, 34, 0, 262215, 17, 30, 0, 131091, 2, 196641, 3,
			2, 196630, 6, 32, 262167, 7, 6, 4, 262176, 8, 3, 7, 262203, 8, 9, 3, 589849, 10, 6, 1, 0, 0, 0, 1, 0, 196635, 11, 10, 262176,
			12, 0, 11, 262203, 12, 13, 0, 262167, 15, 6, 2, 262176, 16, 1, 15, 262203, 16, 17, 1, 327734, 2, 4, 0, 3, 131320, 5, 262205,
			11, 14, 13, 262205, 15, 18, 17, 327767, 7, 19, 14, 18, 196670, 9, 19, 65789, 65592
		};

		pScreenImageShader.InitSPIR_V(VertexShaderBin, FragmentShaderBin);

		pScreenImagePipeline.Create(&pBindings);
		const std::vector<VkFormat> pColorFormats = { pSwapChain.GetSurfaceFormat().format };
		auto pWinSize = lib_backend::GPU_LibBackend::GetInstance()->GetWindowSize();

		RHI::GPU_Texture* pTex = new VK_Texture();
		reinterpret_cast<VK_Texture*>(pTex)->CreateVKTexture(
			&pSwapChain.GetImage(0), &pSwapChain.GetImageView(0), pSwapChain.GetImageCount(),
			pSwapChain.GetSurfaceFormat().format, RHI::GPU_TEXTURE_STATE_DYNAMIC, RHI::GPU_TEXTURE_TYPE_2D
		);

		pCurrentRenderPassInfo = {
			.pEnableColor = true,
			.pEnableDepth = false,
			.pColorTexCount = 1,
			.pColorTextures = pTex,
			.pDepthTexture = NULL,
			.pRenderArea = {
				.pOffset { .x = 0, .y = 0 },
				.pExtent { .width = pWinSize.pWidth, .height = pWinSize.pHeight }
			}
		};

		pCurrentRenderPass = pSwapChain.GetRenderPass();
		pCurrentShader = pScreenImageShader;
		pScreenImagePipeline.CreatePipeline();
	}

	void VK_Backend::BeginRecord()
	{
		pDrawCmdFuncs.clear();
	}

	void VK_Backend::EndRecord(const RHI::GPU_Texture* pFinalTexture)
	{
		CreateScreenImageResources((const VK_Texture*)pFinalTexture);
		
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

				BeginDynamicRendering(
					CmdBuf, i, &pClear[0], &pClear[1], false
				);
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