#ifndef GPU_IMGUI_H
#define GPU_IMGUI_H

#include <iostream>
#include <string>
#include <vector>

namespace RHI_IMGUI
{

	class GPU_ImGui
	{
	public:
		GPU_ImGui() { pInstance = this; pImGuiIsUsed = true; }

		virtual void Init() = 0;
		virtual void ImGui_Rendering() = 0;
		
		inline static GPU_ImGui* GetInstance() { return pInstance; }

		bool IsImGuiUsed() { return pImGuiIsUsed; }

	private:
		static GPU_ImGui* pInstance;
		bool pImGuiIsUsed = false;
	};

}

#endif