#include "GPU_Backend.h"

#include "VK/VK_Backend.h"

#if __has_include("GPU_Log.h")
#include "GPU_Log.h"
#endif

namespace RHI
{
	GPU_Backend::GPU_Backend()
	{
#ifdef ENABLE_GPU_LOG
		GPU_Log Log;
		Log.Init();
#endif
	}

	GPU_Backend* CreateVulkanBackend()
	{
		return GPU::CreateVulkanBackend();
	}
}