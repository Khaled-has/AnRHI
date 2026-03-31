#include "VK_CommandBufferPool.h"

#include "VK_Backend.h"
#include "VK_wrappar.h"

#include "GPU/Backends/GPU_LibBackend.h"

namespace GPU
{

	void VK_CommandBufferPool::Create()
	{
		const VK_Device& pDevice = VK_Backend::Get()->GetDevice();

		VkCommandPoolCreateInfo CreateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = pDevice.GetSelectedQFamily()
		};

		VkResult res = vkCreateCommandPool(pDevice.GetDevice(), &CreateInfo, NULL, &pCommandPool);
		VK_CHECK("vkCreateCommandPool", res);
	}

	void VK_CommandBufferPool::Destroy()
	{
		vkDestroyCommandPool(VK_Backend::Get()->GetDevice().GetDevice(), pCommandPool, NULL);
	}

	void VK_CommandBufferPool::CreateCommandBuffers(uint32_t Count, VkCommandBuffer* CmdBufs)
	{
		VkCommandBufferAllocateInfo AllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = pCommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = Count
		};

		VkResult res = vkAllocateCommandBuffers(
			VK_Backend::Get()->GetDevice().GetDevice(), &AllocateInfo, CmdBufs
		);
		VK_CHECK("vkAllocateCommandBuffers", res);
	}

	void VK_CommandBufferPool::FreeCommandBuffers(uint32_t Count, VkCommandBuffer* CmdBufs)
	{
		vkFreeCommandBuffers(
			VK_Backend::Get()->GetDevice().GetDevice(), pCommandPool,
			Count, CmdBufs
		);
	}

	void BeginCommandBuffer(const VkCommandBuffer& CmdBuf, VkCommandBufferUsageFlags UsageFlags)
	{
		VkCommandBufferBeginInfo Info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = UsageFlags,
			.pInheritanceInfo = NULL
		};

		VkResult res = vkBeginCommandBuffer(CmdBuf, &Info);
		VK_CHECK("vkBeginCommandBuffer", res);
	}

	void RecordCommandBuffer(const std::function<void(const VkCommandBuffer&, uint32_t ImageIndex)>& DrawCommand)
	{
		const VkSurfaceFormatKHR pFormat = VK_Backend::Get()->GetSwapChain().GetSurfaceFormat();
		const bool pIsDynamicSupported = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported;
		const bool pIsDepthTest = VK_Backend::Get()->GetSwapChain().IsDepthTest();

		const VkRenderPass& pRenderPass = VK_Backend::Get()->GetSwapChain().GetRenderPass();

		// # RenderPass begin info
		std::array<VkClearValue, 2> pClear;
		pClear[0].color = {0.0, 0.0, 0.0, 1.0};
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

			BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			// # Begin rendering with Dynamic
			if (pIsDynamicSupported)
			{
				ImageMemBarrier(
					CmdBuf, Image, pFormat.format,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1
				);

				BeginDynamicRendering(CmdBuf, i, &pClear[0], &pClear[1], pIsDepthTest);
			}
			// # Begin rendering with RenderPass
			else
			{
				RenderPassBeginInfo.framebuffer = VK_Backend::Get()->GetSwapChain().GetFramebuffer(i);
				vkCmdBeginRenderPass(CmdBuf, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			}

			// # Do the draw command
			DrawCommand(CmdBuf, i);

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