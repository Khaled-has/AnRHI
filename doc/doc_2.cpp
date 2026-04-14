#include <iostream>
#include <vector>

#include <SDL3/SDL.h>

#include "GPU.h"
#include "Backends/GPU_SDL3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


/* -- You should to render a simple triangle rotating on the screen with uniform buffer with this documentation -- */

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

	// Uniform buffer
	struct {
		glm::mat4 proj = glm::mat4(1);
		glm::mat4 model = glm::mat4(1);
	} pUniform;

	pUniform.proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

	RHI::GPU_Buffer* pUniformBuffer = RHI::CreateBuffer(
		&pUniform, 
		sizeof(pUniform), 
		RHI::GPU_BUFFER_TYPE_DYNAMIC // This time we use dynamic for reChange the buffer in the run time
	);

	// # Create the bindings
	RHI::GPU_Binding pBindings[] = {
		{
			.pBinding = 0,
			.pBindType = RHI::GPU_BINDING_TYPE_STATIC_BUFFER,
			.pStage = RHI::GPU_SHADER_STAGE_VERTEX_BIT,
			.pBuffer = pVertexBuffer
		},
		{
			.pBinding = 1,
			.pBindType = RHI::GPU_BINDING_TYPE_DYNAMIC_BUFFER,
			.pStage = RHI::GPU_SHADER_STAGE_VERTEX_BIT,
			.pBuffer = pUniformBuffer
		}
	};
	// # Create the draw info
	RHI::GPU_DrawInfo pDrawInfo = {
		.pDrawType = RHI::GPU_DRAW_TYPE_ARRAY,
		.pBindCount = 2,
		.pBindings = &pBindings[0],
		.pRenderArea = {
			.pOffset{.x = 0, .y = 0 },
			.pExtent{.width = 1440, .height = 720 }
		}
	};

	// # Step 2: create the draw command
	RHI::GPU_DrawCmd* pDrawCmd = RHI::CreateDraw(pDrawInfo);

	// # Step 3: create the shader 
	// # ( Take Shor you reChange the shader with current API because AnRHI lit you all the designee in the shaders )
	RHI::GPU_Shader* pShader = RHI::CreateShader();
	pShader->InitFromFile( // # This function shown if you implemnt shaderc if you not you will don't show it
		(std::string(RES_PATH) + "GLSL/" + "doc_2.vert").c_str(),
		(std::string(RES_PATH) + "GLSL/" + "doc_2.frag").c_str()
	);
	// # You can you this function if you want more controle
	// -> pShader->InitFromSPIRvFile("file.vert.spv", "file.frag.spv");
	// -> pShader->InitSPIR_V(SPITV_Code_vert, SPITV_Code_frag);

	// # Step 4: create the render pass
	RHI::GPU_Texture* pColorTextureAttach = RHI::CreateTexture();
	pColorTextureAttach->BindData(
		RHI::GPU_TEXTURE_TYPE_2D,
		nullptr,
		1440, 720,
		RHI::GPU_FORMAT_COLOR_BGRA8,
		RHI::GPU_TEXTURE_STATE_DYNAMIC
	);

	RHI::GPU_RenderPassInfo pRenPassInfo = {
		.pEnableColor = true,
		.pEnableDepth = false,
		.pColorTexCount = 1,
		.pColorTextures = pColorTextureAttach,
		.pDepthTexture = nullptr,
		.pRenderArea = {
			.pOffset {.x = 0, .y = 0 },
			.pExtent{.width = 1440, .height = 720 }
		}
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
	pBackend->EndRecord(pColorTextureAttach);

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
	pColorTextureAttach->Destroy();

	pShader->Destroy();
	pDrawCmd->Destroy();

	pRenPassDrawObjs->Destroy();

	pBackend->Backend_Exit();

	SDL_Quit();

	return 0;
}