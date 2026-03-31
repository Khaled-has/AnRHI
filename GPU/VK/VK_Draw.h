#ifndef VK_DRAW_H
#define VK_DRAW_H

#include "GPU/GPU_Draw.h"

#include "VK_GraphicsPipeline.h"
#include "VK_Buffer.h"
#include "VK_Texture.h"
#include "VK_Shader.h"

namespace GPU
{

	class VK_Draw : public RHI::GPU_Draw
	{
	public:
		VK_Draw() {}
		~VK_Draw() {}

		virtual void SetBuffer(const RHI::GPU_Buffer* pBuffer, RHI::GPU_BufferType pBufType, uint32_t pBindIndex) override;
		virtual void SetTexture(const RHI::GPU_Texture* pTexture, uint32_t pBindIndex) override;
		virtual void Create() override;

		virtual void Destroy() override;

		virtual void Draw(uint32_t pFirstVertex, uint32_t pVertexCount) override;

	private:
		VK_GraphicsPipeline pGraphPipeline;
		std::vector<VK_PipelineBinding> pBindings;

		VK_Shader pShader;

		uint32_t pFirstVertex = 0;
		uint32_t pVertexCount = 3;

		void DrawCommand(VkCommandBuffer CmdBuf, uint32_t ImageIndex);
	};

}

#endif