#include "GPU_Buffer.h"

#include "VK/VK_Buffer.h"

#include "GPU_Backend.h"

namespace RHI
{
	GPU_Buffer* CreateBuffer(const void* pData, size_t pSize, GPU_BufferType pBufType)
	{
		if (GPU_Backend::GetBackendType() == GPU_BACKEND_TYPES::GPU_BACKEND_VULKAN)
		{
			return new GPU::VK_Buffer(pData, pSize, pBufType);
		}
		else if (GPU_Backend::GetBackendType() == GPU_BACKEND_TYPES::GPU_BACKEND_DX12)
		{
			return NULL;
		}

		return NULL;
	}
}