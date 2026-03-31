#include "GPU_SDL3.h"

#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>

#include "GPU/VK/VK_Backend.h"

namespace lib_backend
{

	void GPU_SDL3::Init(void* pWindow)
	{
		pWin = pWindow;
	}

	GPU_WinLib GPU_SDL3::GetWindowLib()
	{
		return SDL3;
	}

	GPU_WinSize GPU_SDL3::GetWindowSize()
	{
		int w, h;
		SDL_GetWindowSize(static_cast<SDL_Window*>(pWin), &w, &h);
		GPU_WinSize pWinSize = {
			.pWidth = (uint32_t)w,
			.pHeight = (uint32_t)h
		};
		return pWinSize;
	}

	void* GPU_SDL3::GetWindowHandle()
	{
		return pWin;
	}

	void GPU_SDL3::CreateSurfaceForVulkan(void* pVkSurface)
	{
		VkSurfaceKHR* pSurface = static_cast<VkSurfaceKHR*>(pVkSurface);

		SDL_Vulkan_CreateSurface(
			static_cast<SDL_Window*>(pWin),
			GPU::VK_Backend::Get()->GetDevice().GetInstance(),
			NULL, pSurface
		);
	}

	void GPU_SDL3::CreateSurfaceForDirectX12()
	{
	}
}