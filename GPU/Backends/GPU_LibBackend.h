#ifndef GPU_LIBBACKEND_H
#define GPU_LIBBACKEND_H

#include <iostream>

namespace lib_backend
{

	struct GPU_WinSize
	{
		uint32_t pWidth, pHeight;
	};

	typedef enum GPU_WinLib
	{
		SDL3 = 0,
		GLFW = 1,
		ANDROID_GAME_ACTIVITY = 2
	} GPU_WinLib;

	class GPU_LibBackend
	{
	public:
		GPU_LibBackend() 
		{
			pInstance = this;
		}

		virtual void Init(void* pWindow) = 0;

		virtual GPU_WinLib GetWindowLib() = 0;
		virtual GPU_WinSize GetWindowSize() = 0;
		virtual void* GetWindowHandle() = 0;

		virtual void CreateSurfaceForVulkan(void* pVkSurface) = 0;
		virtual void CreateSurfaceForDirectX12() = 0;

		inline static GPU_LibBackend* GetInstance() { return pInstance; }

	private:
		inline static GPU_LibBackend* pInstance;
	};

}

#endif