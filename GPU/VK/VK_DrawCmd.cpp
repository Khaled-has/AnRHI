#include "VK_DrawCmd.h"

#include "VK_wrappar.h"
#include "VK_Backend.h"

namespace GPU
{

	VK_DrawCmd::VK_DrawCmd(const RHI::GPU_DrawInfo& pDrawInfo)
	{
		pGraphPipeline.Create(pDrawInfo);
	}

	void VK_DrawCmd::Destroy()
	{
		pShader.Destroy();
		pGraphPipeline.Destroy();
	}

	void VK_DrawCmd::Draw(uint32_t pFirstVertex, uint32_t pVertexCount)
	{
		// # Rerecord if any thing change
		this->pFirstVertex = pFirstVertex;
		this->pVertexCount = pVertexCount;

		// # Create the graphics pipeline
		pGraphPipeline.CreatePipeline();

		// # Bind the draw function in the draw commands queue
		VK_Backend::Get()->GetDrawCmdsArray()->push_back(
			[&](VkCommandBuffer Cmd, uint32_t ImageIndex)
			{
				DrawCommand(Cmd, ImageIndex);
			}
		);
	}

	void VK_DrawCmd::DrawCommand(VkCommandBuffer CmdBuf, uint32_t ImageIndex)
	{
		pGraphPipeline.Bind(ImageIndex);

		vkCmdDraw(CmdBuf, pVertexCount, 1, pFirstVertex, 0);
	}
}