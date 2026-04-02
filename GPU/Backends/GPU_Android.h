#ifndef GPU_ANDROID_H
#define GPU_ANDROID_H

#include "GPU_LibBackend.h"

namespace lib_backend
{
	class GPU_Android : public GPU_LibBackend
	{
	public:
		GPU_Android() {}

		virtual void Init(void* pWindow) override;

		virtual GPU_WinSize GetWindowSize() override;

		virtual void CreateSurfaceForVulkan(void* pVkSurface) override;
		virtual void CreateSurfaceForDirectX12() override;

	private:
		void* pWin;
	};
}

#endif