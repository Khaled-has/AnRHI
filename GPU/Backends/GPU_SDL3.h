#ifndef GPU_SDL3_H
#define GPU_SDL3_H

#include "GPU_LibBackend.h"

#include <SDL3/SDL.h>

namespace lib_backend
{
	class GPU_SDL3 : public GPU_LibBackend
	{
	public:
		GPU_SDL3() {}

		virtual void Init(void* pWindow) override;

		virtual GPU_WinSize GetWindowSize() override;
		virtual void* GetHandle() override;

		virtual void CreateSurfaceForVulkan(void* pVkSurface) override;
		virtual void CreateSurfaceForDirectX12() override;

	private:
		void* pWin;
	};

	inline GPU_LibBackend* CreateSDL3Lib() 
	{ 
		return new GPU_SDL3; 
	}
}

#endif