#include <iostream>

#include "GPU.h"
#include "Backends/GPU_SDL3.h"

#include <stb_image.h>

/* -- You should to render a simple rectangle with texture in this documentation -- */

void* LoadTextureFromFile(const char* pFilename, int* w, int* h, int* channels)
{
	stbi_uc* pPixels = stbi_load(pFilename, w, h, channels, NULL);

	return pPixels;
}

int main(int argc, char* argv[])
{
	SDL_Window* pWin = SDL_CreateWindow(
		"AnRHI-doc_3",
		1360, 720,
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
	const std::vector<float> pVertices = 
	{  // Position        UV
		-0.8f, -0.8f, 0.0f, 0.0f,
		 0.8f, -0.8f, 1.0f, 0.0f,
		 0.8f,  0.8f, 1.0f, 1.0f,

		 0.8f,  0.8f, 1.0f, 1.0f,
		-0.8f, -0.8f, 0.0f, 0.0f,
		-0.8f,  0.8f, 0.0f, 1.0f
	};
	RHI::GPU_Buffer* pVertexBuffer = RHI::CreateBuffer(
		pVertices.data(), sizeof(float) * pVertices.size(), RHI::GPU_BUFFER_TYPE_STATIC
	);

	// # Step 2: create the texture
	int w, h, channels;
	void* pPixels = LoadTextureFromFile(
		(std::string(RES_PATH) + "HollowKnight.png").c_str(),
		&w, &h, &channels
		);

	RHI::GPU_TextureInfo pTexInfo = {
		.pType = RHI::GPU_TEXTURE_TYPE_2D,
		.pPixels = pPixels,
		.pSize = {.pWidth = (uint32_t)w, .pHeight = (uint32_t)h },
		.pFormat = RHI::GPU_FORMAT_COLOR_RGBA8,
		.pAspect = RHI::GPU_ASPECT_COLOR_BIT,
		.pState = RHI::GPU_TEXTURE_STATE_STATIC
	};
	RHI::GPU_Texture* pTexture = RHI::CreateTexture(pTexInfo);

	stbi_image_free(pPixels);

	// # Create the bindings
	RHI::GPU_Binding pBindings[] = {
		// # 1 : Vertex buffer binding
		{
			.pBinding = 0,
			.pBindType = RHI::GPU_BINDING_TYPE_STATIC_BUFFER,
			.pStage = RHI::GPU_SHADER_STAGE_VERTEX_BIT,
			.pBuffer = pVertexBuffer
		},
		// # 2 : Texture binding
		{
			.pBinding = 1,
			.pBindType = RHI::GPU_BINDING_TYPE_TEXTURE,
			.pStage = RHI::GPU_SHADER_STAGE_FRAGMENT_BIT,
			.pTexture = pTexture
		}
	};
	// # Create the draw info
	RHI::GPU_DrawInfo pDrawInfo = {
		.pDrawType = RHI::GPU_DRAW_TYPE_ARRAY,
		.pBindCount = 2,
		.pBindings = &pBindings[0],
		.pRenderArea = {
			.pOffset{.x = 0, .y = 0 },
			.pExtent{.width = 1360, .height = 720 }
		}
	};

	// # Step 2: create the draw command
	RHI::GPU_DrawCmd* pDrawCmd = RHI::CreateDraw(pDrawInfo);

	// # Step 3: create the shader 
	// # ( Take Shor you reChange the shader with current API because AnRHI lit you all the designee in the shaders )
	RHI::GPU_Shader* pShader = RHI::CreateShader();
	pShader->InitFromFile( // # This function shown if you implement shaderc if you not you will don't show it
		(std::string(RES_PATH) + "GLSL/" + "doc_3.vert").c_str(),
		(std::string(RES_PATH) + "GLSL/" + "doc_3.frag").c_str()
	);

	// # You can use this functions if you want more controle
	// -> pShader->InitFromSPIRvFile("file.vert.spv", "file.frag.spv");
	// -> pShader->InitSPIR_V(SPITV_Code_vert, SPITV_Code_frag);

	// # Step 4: create the attachments texture
	RHI::GPU_TextureInfo pTexColorInfo = {
		.pType = RHI::GPU_TEXTURE_TYPE_2D,
		.pPixels = nullptr,
		.pSize = RHI::GPU_Size{.pWidth = 1360, .pHeight = 720 },
		.pFormat = RHI::GPU_FORMAT_COLOR_BGRA8,
		.pAspect = RHI::GPU_ASPECT_COLOR_BIT,
		.pState = RHI::GPU_TEXTURE_STATE_DYNAMIC
	};
	RHI::GPU_Texture* pColorTextureAttach = RHI::CreateTexture(pTexColorInfo);

	// # Step 5: create the render pass
	RHI::GPU_RenderPassInfo pRenPassInfo = {
		.pEnableColor = true,
		.pEnableDepth = false,
		.pColorTexCount = 1,
		.pColorTextures = pColorTextureAttach,
		.pDepthTexture = nullptr,
		.pRenderArea = {
			.pOffset {.x = 0, .y = 0 },
			.pExtent{.width = 1360, .height = 720 }
		}
	};

	RHI::GPU_RenderPass* pRenPassDrawObjs = RHI::CreateRenderPass(pRenPassInfo);
	// # Step 6: record the draw commands on the render pass
	pBackend->BeginRecord();

	// # Start the render pass
	pRenPassDrawObjs->Begin(
		RHI::GPU_Clear{ .pColor{0, 0, 0, 1}, .pDepth{.depth = 1.0f, .stencil = 0} }
	);

	// # Active the shader before you submit the draw command
	pShader->Active();

	// # Draw command for draw the data you include it on it
	pDrawCmd->Draw(0, 6);

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
		//pVertexBuffer->Update(newData, size); | For example..

		// # End the rendering
		pBackend->EndRendering();
	}

	// # Destroy the RHI components
	pVertexBuffer->Destroy();
	pTexture->Destroy();
	pColorTextureAttach->Destroy();

	pShader->Destroy();
	pDrawCmd->Destroy();

	pRenPassDrawObjs->Destroy();

	pBackend->Backend_Exit();

	SDL_Quit();

	return 0;
}