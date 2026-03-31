#include "GPU_Draw.h"

#include "VK/VK_Draw.h"

namespace RHI
{

	GPU_Draw* CreateDraw()
	{
		GPU_BACKEND_TYPES BType = GPU_Backend::GetBackendType();

		if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_VULKAN)
		{
			return new GPU::VK_Draw();
		}
		else if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_DX12)
		{
			return NULL;
		}

		return NULL;
	}

}