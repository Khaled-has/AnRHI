#include <iostream>
#include <vector>

#include <SDL3/SDL.h>

#include "GPU.h"
#include "Backends/GPU_SDL3.h"

#include "Log.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


/* -- You should to render a simple triangle rotating on the screen with uniform buffer with this documentation -- */

int main(int argc, char argv[])
{
	Log pLog;
	pLog.Init();

	SDL_Window* pWin = SDL_CreateWindow(
		"AnRHI-doc_1",
		1440, 720,
		SDL_WINDOW_VULKAN
	);

	// # Set the API graphic backend
	RHI::GPU_Backend* pBackend = RHI::CreateVulkanBackend();
	// # Set the window library
	pBackend->GetLibBackend() = lib_backend::CreateBackend_SDL3();
	// # Set the window handle 
	pBackend->GetLibBackend()->Init(pWin);
	// # Last initialize the backend
	pBackend->Backend_Init();

	// # Step 1: create the vertex buffer
	const std::vector<float> pVertices = { -0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -0.5f };
	RHI::GPU_Buffer* pVertexBuffer = RHI::CreateBuffer(
		pVertices.data(), sizeof(float) * pVertices.size(), RHI::GPU_BUFFER_TYPE_STATIC
	);

	// Uniform buffer
	struct {
		glm::mat4 proj = glm::mat4(1);
		glm::mat4 model = glm::mat4(1);
	} pUniform;

	pUniform.proj = glm::ortho(-1.0, 1.0, 1.0, -1.0);

	RHI::GPU_Buffer* pUniformBuffer = RHI::CreateBuffer(
		&pUniform, 
		sizeof(pUniform), 
		RHI::GPU_BUFFER_TYPE_DYNAMIC // This time we use dynamic for reChange the buffer in the run time
	);

	// # Step 2: create the draw command
	RHI::GPU_Draw* pDrawCmd = RHI::CreateDraw();

	// # Bind the vertex buffer with 0 binding
	pDrawCmd->SetBuffer(pVertexBuffer, RHI::GPU_BUFFER_TYPE_STATIC, 0);
	// # Bind the uniform buffer with 1 binding
	pDrawCmd->SetBuffer(pUniformBuffer, RHI::GPU_BUFFER_TYPE_DYNAMIC, 1);

	pDrawCmd->Create();

	// # Step 3: create the shader 
	// # ( Take Shor you reChange the shader with current API because AnRHI lit you all the designee in the shaders )
	RHI::GPU_Shader* pShader = RHI::CreateShader();
	pShader->InitFromFile(
		(std::string(RES_PATH) + "GLSL/" + "doc_2.vert").c_str(),
		(std::string(RES_PATH) + "GLSL/" + "doc_2.frag").c_str()
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
		// # Transform the model matrix to rotate the object
		pUniform.model = glm::rotate(pUniform.model, glm::radians(0.5f), glm::vec3(0, 0, 1));
		// # Update the new uniform data
		pUniformBuffer->Update(&pUniform, sizeof(pUniform));

		// # End the rendering
		pBackend->EndRendering();
	}

	// # Destroy the RHI components
	pVertexBuffer->Destroy();
	pUniformBuffer->Destroy();
	pDrawCmd->Destroy();
	pRenPassDrawObjs->Destroy();
	pShader->Destroy();

	pBackend->Backend_Exit();

	SDL_Quit();

	return 0;
}