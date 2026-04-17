#include "VK_Queue.h"

#include "VK_Backend.h"
#include "VK_wrappar.h"

namespace GPU
{

	VkSemaphore CreateVkSemaphore()
	{
		VkSemaphoreCreateInfo CreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0
		};

		VkSemaphore Semaphore;
		VkResult res = vkCreateSemaphore(
			VK_Backend::Get()->GetDevice().GetDevice(), &CreateInfo,
			NULL, &Semaphore
		);
		VK_CHECK("vkCreateSemaphore", res);

		return Semaphore;
	}

	void VK_Queue::Create()
	{
		const VK_Device& pDevice = VK_Backend::Get()->GetDevice();
		uint32_t QueueIndex   = pDevice.GetSelectedQueue();
		uint32_t QFamilyIndex = pDevice.GetSelectedQFamily();

		// # Get device queue
		vkGetDeviceQueue(pDevice.GetDevice(), QFamilyIndex, QueueIndex, &pQueue);

		// # Get swapchain images
		VkResult res = vkGetSwapchainImagesKHR(
			pDevice.GetDevice(), VK_Backend::Get()->GetSwapChain().GetSwapchain(),
			&pNumImages, NULL
		);
		VK_CHECK("vkGetSwapchainImagesKHR", res);

		CreateSyncObjects();
	}

	void VK_Queue::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		// # Destroy fences
		for (uint32_t i = 0; i < pInFlightFences.size(); i++)
		{
			vkDestroyFence(pDevice, pInFlightFences[i], NULL);
		}
		// # Destroy semaphores
		for (uint32_t i = 0; i < pImageAvailableSems.size(); i++)
		{
			vkDestroySemaphore(pDevice, pImageAvailableSems[i], NULL);
			vkDestroySemaphore(pDevice, pRenderFinishedSems[i], NULL);
		}
	}

	VkResult VK_Queue::AcquireNextImage()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		const VkSwapchainKHR& pSwapChain = VK_Backend::Get()->GetSwapChain().GetSwapchain();
		vkWaitForFences(pDevice, 1, &pInFlightFences[pFrameIndex], VK_TRUE, UINT64_MAX);
		vkResetFences(pDevice, 1, &pInFlightFences[pFrameIndex]);

		VkResult res = vkAcquireNextImageKHR(
			pDevice, pSwapChain,
			UINT64_MAX, pImageAvailableSems[pFrameIndex],
			VK_NULL_HANDLE, &pAcquiredImageIndex
		);

		if ((pImagesInFlight[pAcquiredImageIndex] != VK_NULL_HANDLE) &&
			(pImagesInFlight[pAcquiredImageIndex] != pInFlightFences[pFrameIndex]))
		{
			vkWaitForFences(pDevice, 1, &pImagesInFlight[pAcquiredImageIndex], VK_TRUE, UINT64_MAX);
		}

		pImagesInFlight[pAcquiredImageIndex] = pInFlightFences[pFrameIndex];

		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
		{
			return res;
		}
		VK_CHECK("vkAcquireNextImageKHR", res);
		return res;
	}

	void VK_Queue::SubmitSync(const VkCommandBuffer& CmdBuf) const
	{
		static VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo SubmitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = VK_NULL_HANDLE,
			.pWaitDstStageMask = VK_NULL_HANDLE,
			.commandBufferCount = 1,
			.pCommandBuffers = &CmdBuf,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = VK_NULL_HANDLE
		};

		VkResult res = vkQueueSubmit(pQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
		VK_CHECK("vkQueueSubmit", res);
	}

	void VK_Queue::SubmitAsync(const VkCommandBuffer* CmdBuf, uint32_t Count) 
	{
		static VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo SubmitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &pImageAvailableSems[pFrameIndex],
			.pWaitDstStageMask = waitFlags,
			.commandBufferCount = Count,
			.pCommandBuffers = CmdBuf,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &pRenderFinishedSems[pFrameIndex]
		};

		VkResult res = vkQueueSubmit(pQueue, 1, &SubmitInfo, pInFlightFences[pFrameIndex]);
		VK_CHECK("vkQueueSubmit", res);
	}

	VkResult VK_Queue::Present(uint32_t ImageIndex)
	{
		const VkSwapchainKHR& pSwapchain = VK_Backend::Get()->GetSwapChain().GetSwapchain();

		VkPresentInfoKHR PresentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &pRenderFinishedSems[pFrameIndex],
			.swapchainCount = 1,
			.pSwapchains = &pSwapchain,
			.pImageIndices = &ImageIndex,
			.pResults = NULL
		};

		VkResult res = vkQueuePresentKHR(pQueue, &PresentInfo);
		pFrameIndex = (pFrameIndex + 1) % pNumImages;

		WaitIdle();

		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
		{
			return res;
		}

		VK_CHECK("vkQueuePresentKHR", res);

		return res;
	}

	void VK_Queue::WaitIdle() const 
	{
		VkResult res = vkQueueWaitIdle(pQueue);
		VK_CHECK("vkQueueWaitIdle", res);
	}

	void VK_Queue::CreateSyncObjects()
	{
		pImageAvailableSems.resize(pNumImages);

		for (VkSemaphore& Sem : pImageAvailableSems)
		{
			Sem = CreateVkSemaphore();
		}

		pRenderFinishedSems.resize(pNumImages);

		for (VkSemaphore& Sem : pRenderFinishedSems)
		{
			Sem = CreateVkSemaphore();
		}

		pInFlightFences.resize(pNumImages);
		pImagesInFlight.resize(pNumImages, VK_NULL_HANDLE);

		VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		for (VkFence& Fence : pInFlightFences)
		{
			VkResult res = vkCreateFence(
				VK_Backend::Get()->GetDevice().GetDevice(),
				&fenceInfo, NULL, &Fence
			);
			VK_CHECK("vkCreateFence", res);
		}
	}

}
