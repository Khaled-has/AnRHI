#ifndef VK_IMGUI_H
#define VK_IMGUI_H

#include "GPU_ImGui.h"

#include "VK/VK_CommandBufferPool.h"

namespace GPU_IMGUI
{

	class VK_ImGui : public RHI_IMGUI::GPU_ImGui
	{
	public:
		VK_ImGui() {}

		virtual void Init() override;
		virtual void ImGui_Rendering() override;

	private:
		GPU::VK_CommandBufferPool pCmdPool;
		std::vector<VkCommandBuffer> pCmdBufs;
	};

}

#endif