#include "GPU_Texture.h"

#include "GPU_Backend.h"

#include "GPU/VK/VK_Texture.h"

namespace RHI
{

	GPU_Texture* CreateTexture(const char* pFilename)
	{
		GPU_BACKEND_TYPES BType = GPU_Backend::GetBackendType();

		if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_VULKAN)
		{
			return new GPU::VK_Texture(pFilename);
		}
		else if (BType == GPU_BACKEND_TYPES::GPU_BACKEND_DX12)
		{
			return NULL;
		}

		return NULL;
	}

}