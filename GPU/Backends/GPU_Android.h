#ifndef GPU_ANDROID_H
#define GPU_ANDROID_H

#include "GPU_LibBackend.h"

namespace lib_backend
{
	class GPU_Android : public GPU_LibBackend
	{
	public:
		GPU_Android() {}

		virtual void Init(void* pApp) override;

		virtual GPU_WinSize GetWindowSize() override;
		virtual void* GetHandle() override;

		virtual void CreateSurfaceForVulkan(void* pVkSurface) override;
		virtual void CreateSurfaceForDirectX12() override;

	private:
		void* pApp;
	};

	inline GPU_LibBackend* CreateAndroidLib()
	{
		return new GPU_Android;
	}
}

#endif