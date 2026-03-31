#include "VK_Buffer.h"

#include "VK_wrappar.h"
#include "VK_Backend.h"

namespace GPU
{

	void VK_Buffer::Create(const void* pData, size_t pSize, RHI::GPU_BufferType pBufferType)
	{
		pType = pBufferType;
		// # Static buffer
		if (pBufferType == RHI::GPU_BUFFER_TYPE_STATIC)
		{
			CreateStaticBuf(pData, pSize);
		}
		// # Dynamic buffer
		else if (pBufferType == RHI::GPU_BUFFER_TYPE_DYNAMIC)
		{
			CreateDynamicBuf(pData, pSize);
		}
	}

	void VK_Buffer::CreateStaticBuf(const void* pData, size_t pSize)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		// # Step 1: create the staging buffer
		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags MemProps =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VK_BufferAndMemory StaginBuffer = CreateBuffer(pSize, Usage, MemProps);

		// # Step 2: map the memory if the stage buffer
		void* pMem = NULL;
		VkDeviceSize Offset = 0;
		VkMemoryMapFlags Flags = 0;
		VkResult res = vkMapMemory(
			pDevice, StaginBuffer.pMemory, Offset,
			StaginBuffer.pAllocationSize, Flags, &pMem
		);
		VK_CHECK("vkMapMemory", res);

		// # Step 3: copy the data to the stating buffer
		memcpy(pMem, pData, pSize);

		// # Step 4: unMap/release the mapped memory
		vkUnmapMemory(pDevice, StaginBuffer.pMemory);

		// # Step 5: create the final buffer
		Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		pBufferAndMemory = CreateBuffer(pSize, Usage, MemProps);

		// # Step 6: copy the staging buffer to the final buffer
		CopyBuffer(pBufferAndMemory.pBuffer, StaginBuffer.pBuffer, pSize);

		// # Step 7: release the resources of the staging buffer
		vkFreeMemory(pDevice, StaginBuffer.pMemory, NULL);
		vkDestroyBuffer(pDevice, StaginBuffer.pBuffer, NULL);
	}

	void VK_Buffer::CreateDynamicBuf(const void* pData, size_t pSize)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();

		pBuffers.resize(NumImages);
		for (uint32_t i = 0; i < NumImages; i++)
		{
			// # Step 1: create the staging buffer
			VkBufferUsageFlags Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			VkMemoryPropertyFlags MemProps =
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			pBuffers[i] = CreateBuffer(pSize, Usage, MemProps);

			// # Step 2: map the memory
			void* pMem = NULL;
			VkResult res = vkMapMemory(pDevice, pBuffers[i].pMemory, 0, pSize, 0, &pMem);
			VK_CHECK("vkMapMemory", res);

			memcpy(pMem, pData, pSize);
			vkUnmapMemory(pDevice, pBuffers[i].pMemory);
		}
	}

	void VK_Buffer::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		if (pType == RHI::GPU_BUFFER_TYPE_STATIC)
		{
			vkFreeMemory(pDevice, pBufferAndMemory.pMemory, NULL);
			vkDestroyBuffer(pDevice, pBufferAndMemory.pBuffer, NULL);
		}
		else if (pType == RHI::GPU_BUFFER_TYPE_DYNAMIC)
		{
			for (uint32_t i = 0; i < pBuffers.size(); i++)
			{
				vkFreeMemory(pDevice, pBuffers[i].pMemory, NULL);
				vkDestroyBuffer(pDevice, pBuffers[i].pBuffer, NULL);
			}
		}
	}

	void VK_Buffer::Update(const void* pData, size_t pSize)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		const uint32_t pImageIndex = VK_Backend::Get()->GetQueue().GetAcquiredImageIndex();

		if (pType == RHI::GPU_BUFFER_TYPE_DYNAMIC)
		{
			void* pMem = NULL;
			VkResult res = vkMapMemory(pDevice, pBuffers[pImageIndex].pMemory, 0, pSize, 0, &pMem);
			VK_CHECK("vkMapMemory", res);

			memcpy(pMem, pData, pSize);
			vkUnmapMemory(pDevice, pBuffers[pImageIndex].pMemory);
		}
		else if (pType == RHI::GPU_BUFFER_TYPE_STATIC)
		{
			GPU_LOG_ERROR(
				"ANRHI Error: You cannot update buffer with type: GPU_BUFFER_TYPE_STATIC\n 	  Change the type to GPU_BUFFER_TYPE_DYNAMIC or GPU_BUFFER_TYPE_DYNAMIC_RESIZEABLE"
			);
		}

	}

}
