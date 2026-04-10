#include "VK_ImGui.h"

#include "VK/VK_Backend.h"

namespace GPU_IMGUI
{

	void VK_ImGui::Init()
	{
		uint32_t NumImages = GPU::VK_Backend::Get()->GetSwapChain().GetImageCount();

		GPU::VK_Backend::Get()->GetCmdBufPool().CreateCommandBuffers(NumImages, pCmdBufs.data());

	}

	void VK_ImGui::ImGui_Rendering()
	{

	}

}