#ifndef GPU_BACKEND_H
#define GPU_BACKEND_H

#include "GPU_Texture.h"
#include "GPU_RenderPass.h"

#include "Backends/GPU_LibBackend.h"

#if __has_include("imgui/GPU_ImGui.h")
#include "imgui/GPU_ImGui.h"
#define RHI_IMGUI_ENABLE
#endif

namespace RHI {

	enum class GPU_BACKEND_TYPES
	{
		GPU_BACKEND_VULKAN,
		GPU_BACKEND_DX12
	};

	class GPU_Backend
	{
	public:
		GPU_Backend();
		~GPU_Backend() {}

		virtual bool IsDeviceSupportBackend() = 0;

		virtual void Backend_Init() = 0;
		virtual void Backend_Exit() = 0;

#ifdef RHI_IMGUI_ENABLE
		virtual void ImGui_Init() = 0;
		virtual void ImGui_Exit() = 0;

		virtual void ImGui_Rendering() = 0;
#endif

		virtual void BeginRecord() = 0;
		virtual void EndRecord(const RHI::GPU_Texture* pFinalTexture) = 0;

		virtual void BeginRendering() = 0;
		virtual void EndRendering() = 0;

		inline static GPU_BACKEND_TYPES& GetBackendType() { return pBackendType; }
		inline lib_backend::GPU_LibBackend*& GetLibBackend() { return pLibBackend; }

	private:
		inline static GPU_BACKEND_TYPES pBackendType;
		lib_backend::GPU_LibBackend* pLibBackend;
	};

	GPU_Backend* CreateVulkanBackend();

}

#endif