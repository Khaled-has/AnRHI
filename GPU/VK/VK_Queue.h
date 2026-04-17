#ifndef VK_QUEUE_H
#define VK_QUEUE_H

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "VK_SwapChain.h"

namespace GPU 
{

	class VK_Queue
	{
	public:
		VK_Queue() {}
		~VK_Queue() {}

		void Create();
		void Destroy();

		VkResult AcquireNextImage();
		inline uint32_t CurrentFrame() { return pFrameIndex; }

		void SubmitSync(const VkCommandBuffer& CmdBuf) const;
		void SubmitAsync(const VkCommandBuffer* CmdBuf, uint32_t Count = 1);

		VkResult Present(uint32_t ImageIndex);
		void WaitIdle() const;

		inline uint32_t GetAcquiredImageIndex() const { return pAcquiredImageIndex; }

	private:
		VkQueue pQueue = VK_NULL_HANDLE;

		std::vector<VkSemaphore> pRenderFinishedSems;
		std::vector<VkSemaphore> pImageAvailableSems;
		std::vector<VkFence> pInFlightFences;
		std::vector<VkFence> pImagesInFlight;

		uint32_t pNumImages = 0;
		uint32_t pFrameIndex = 0;
		uint32_t pAcquiredImageIndex = 0;

		void CreateSyncObjects();
	};

}

#endif