#include "GPU_DrawCmd.h"

#include "VK/VK_DrawCmd.h"

namespace RHI
{

	GPU_DrawCmd* CreateDraw(const GPU_DrawInfo& pDrawInfo)
	{
		GPU_BACKEND_TYPES BType = GPU_Backend::GetBackendType();

		if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_VULKAN)
		{
			return new GPU::VK_DrawCmd(pDrawInfo);
		}
		else if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_DX12)
		{
			return NULL;
		}

		return NULL;
	}

}