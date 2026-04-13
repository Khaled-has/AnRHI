#include "GPU_RenderPass.h"

#include "GPU_Backend.h"

#include "VK/VK_RenderPass.h"

namespace RHI
{
	GPU_RenderPass* CreateRenderPass(const GPU_RenderPassInfo& pInfo)
	{
		GPU_BACKEND_TYPES BType = GPU_Backend::GetBackendType();

		if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_VULKAN)
		{
			return new GPU::VK_RenderPass(pInfo);
		}
		else if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_DX12)
		{
			return NULL;
		}

		return NULL;
	}
}