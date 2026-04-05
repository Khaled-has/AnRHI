#include <iostream>
#include <vector>

#include <SDL3/SDL.h>

#include "GPU.h"
#include "Backends/GPU_SDL3.h"

/* -- You should to render a simple triangle with this documentation -- */

int main(int argc, char argv[])
{
	SDL_Window* pWin = SDL_CreateWindow(
		"AnRHI-doc_1",
		1440, 720,
		SDL_WINDOW_VULKAN
	);

	// # Set the API graphic backend
	RHI::GPU_Backend* pBackend = RHI::CreateVulkanBackend();
	// # Set the window library
	pBackend->GetLibBackend() = lib_backend::CreateSDL3Lib();
	// # Set the window handle 
	pBackend->GetLibBackend()->Init(pWin);
	// # Last initialize the backend
	pBackend->Backend_Init();

	// # Step 1: create the vertex buffer
	const std::vector<float> pVertices = { -0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -0.5f };
	RHI::GPU_Buffer* pVertexBuffer = RHI::CreateBuffer(
		pVertices.data(), sizeof(float) * pVertices.size(), RHI::GPU_BUFFER_TYPE_STATIC
	);

	// # Step 2: create the draw command
	RHI::GPU_Draw* pDrawCmd = RHI::CreateDraw();
	pDrawCmd->SetBuffer(pVertexBuffer, RHI::GPU_BUFFER_TYPE_STATIC, 0);

	// # When you finsh init the bindings
	pDrawCmd->InitBindings();

	// # Step 3: create the shader 
	// # ( Take Shor you reChange the shader with current API because AnRHI lit you all the designee in the shaders )
	RHI::GPU_Shader* pShader = RHI::CreateShader();
	pShader->InitFromFile(
		(std::string(RES_PATH) + "GLSL/" + "doc_1.vert").c_str(),
		(std::string(RES_PATH) + "GLSL/" + "doc_1.frag").c_str()
		);

	// # Step 4: create the render pass
	const std::vector<RHI::GPU_Format> pColorFormats = { RHI::GPU_FORMAT_COLOR_RGBA8 };

	RHI::GPU_RenderPassInfo pRenPassInfo = {
		.pEnableColor = true,
		.pEnableDepth = false,
		.pColorFormats = pColorFormats,
		.pDepthFormat = RHI::GPU_FORMAT_UNDEFINE,
		.pWidth = 1440,
		.pHeight = 720
	};
	RHI::GPU_RenderPass* pRenPassDrawObjs = RHI::CreateRenderPass(pRenPassInfo);

	// # Step 5: record the draw commands on the render pass
	pBackend->BeginRecord();

	// # Start the render pass
	pRenPassDrawObjs->Begin(
		RHI::GPU_Clear{ .pColor{0, 0, 0, 1}, .pDepth{.depth = 1.0f, .stencil = 0} }
	);

	// # Active the shader before you submit the draw command
	pShader->Active();

	// # Draw command for draw the data you include it on it
	pDrawCmd->Draw(0, 3);

	// # End the render pass
	pRenPassDrawObjs->End();

	// # Set the final render pass you want to render it on the screen
	pBackend->EndRecord(pRenPassDrawObjs);

	SDL_Event pEv;
	bool pRunning = true;
	while (pRunning)
	{
		while (SDL_PollEvent(&pEv))
		{
			if (pEv.type == SDL_EVENT_QUIT)
				pRunning = false;
		}

		// # Begin the rendering
		pBackend->BeginRendering();

		// # Warning: every buffer update should to execute here between the: BeginRendering() |...| EndRendering()
		//pVertexBuffer->Update(newData, size); | For example..

		// # End the rendering
		pBackend->EndRendering();
	}

	// # Destroy the RHI components
	pVertexBuffer->Destroy();
	pDrawCmd->Destroy();
	pRenPassDrawObjs->Destroy();
	pShader->Destroy();

	pBackend->Backend_Exit();

	SDL_Quit();

	return 0;
}