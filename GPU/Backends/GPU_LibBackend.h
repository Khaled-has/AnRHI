#ifndef GPU_LIBBACKEND_H
#define GPU_LIBBACKEND_H

#include <iostream>

namespace lib_backend
{

	struct GPU_WinSize
	{
		uint32_t pWidth, pHeight;

		bool operator==(const GPU_WinSize& pSt)
		{
			return ((pSt.pWidth == pWidth) && (pSt.pHeight == pHeight));
		}
		bool operator!=(const GPU_WinSize& pSt)
		{
			return ((pSt.pWidth != pWidth) || (pSt.pHeight != pHeight));
		}
	};

	class GPU_LibBackend
	{
	public:
		GPU_LibBackend() 
		{
			pInstance = this;
		}

		virtual void Init(
#ifdef ADNROID
			void* pApp
#else
			void* pWindow
#endif
		) = 0;

		virtual GPU_WinSize GetWindowSize() = 0;
		virtual void* GetHandle() = 0;

		virtual void CreateSurfaceForVulkan(void* pVkSurface) = 0;
		virtual void CreateSurfaceForDirectX12() = 0;

		inline static GPU_LibBackend* GetInstance() { return pInstance; }

	private:
		inline static GPU_LibBackend* pInstance;
	};

}

#endif