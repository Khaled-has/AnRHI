#ifndef VK_DRAW_H
#define VK_DRAW_H

#include "GPU_DrawCmd.h"

#include "VK_GraphicsPipeline.h"
#include "VK_Buffer.h"
#include "VK_Texture.h"
#include "VK_Shader.h"

namespace GPU
{

	class VK_DrawCmd : public RHI::GPU_DrawCmd
	{
	public:
		VK_DrawCmd(const RHI::GPU_DrawInfo& pDrawInfo);
		~VK_DrawCmd() {}

		virtual void Destroy() override;

		virtual void Draw(uint32_t pFirstVertex, uint32_t pVertexCount) override;

	private:
		VK_GraphicsPipeline pGraphPipeline;

		VK_Shader pShader;

		uint32_t pFirstVertex = 0;
		uint32_t pVertexCount = 3;

		void DrawCommand(VkCommandBuffer CmdBuf, uint32_t ImageIndex);
	};

}

#endif