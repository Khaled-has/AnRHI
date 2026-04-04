#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "GPU_Buffer.h"

namespace GPU
{

	typedef enum GPU_BufferType {
		GPU_BUFFER_TYPE_STATIC,
		GPU_BUFFER_TYPE_DYNAMIC,
		GPU_BUFFER_TYPE_DYNAMIC_RESIZEABLE
	} GPU_BufferType;

	struct VK_BufferAndMemory {
		VkBuffer pBuffer = VK_NULL_HANDLE;
		VkDeviceMemory pMemory = VK_NULL_HANDLE;
		VkDeviceSize pAllocationSize = 0;
	};

	class VK_Buffer : public RHI::GPU_Buffer
	{
	public:
		VK_Buffer() {}
		VK_Buffer(const void* pData, size_t pSize, RHI::GPU_BufferType pBufferType)
		{
			Create(pData, pSize, pBufferType);
		}
		~VK_Buffer() {}

		virtual void Create(const void* pData, size_t pSize, RHI::GPU_BufferType pBufferType) override;
		virtual void Destroy() override;

		virtual void Update(const void* pData, size_t pSize) override;

		inline const VK_BufferAndMemory& GetBuffer() const { return pBufferAndMemory; }
		inline const std::vector<VK_BufferAndMemory>& GetBuffers() const { return pBuffers; }

	private:
		VK_BufferAndMemory pBufferAndMemory;
		std::vector<VK_BufferAndMemory> pBuffers;

		RHI::GPU_BufferType pType;

		void CreateStaticBuf(const void* pData, size_t pSize);
		void CreateDynamicBuf(const void* pData, size_t pSize);
	};

}

#endif