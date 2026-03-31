#include "GPU_Backend.h"

#include "VK/VK_Backend.h"

namespace RHI
{
	GPU_Backend* CreateVulkanBackend()
	{
		return GPU::CreateVulkanBackend();
	}
}