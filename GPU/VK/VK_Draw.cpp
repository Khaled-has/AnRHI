#include "VK_Draw.h"

#include "VK_wrappar.h"
#include "VK_Backend.h"

namespace GPU
{
	void VK_Draw::SetBuffer(const RHI::GPU_Buffer* pBuffer, RHI::GPU_BufferType pBufType, uint32_t pBindIndex)
	{
		if (pBufType == RHI::GPU_BUFFER_TYPE_STATIC)
		{
			pBindings.push_back(
				VK_PipelineBinding{
					.pBinding = pBindIndex,
					.pDescType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pStageFlag = VK_SHADER_STAGE_VERTEX_BIT,
					.pBindingType = VK_BINDING_BUFFER_INFO,
					.pBuffer = (const VK_Buffer*)pBuffer
				}
			);
		}
		else if (pBufType == RHI::GPU_BUFFER_TYPE_DYNAMIC)
		{
			const VK_Buffer* pBuf = (VK_Buffer*)pBuffer;
			pBindings.push_back(
				VK_PipelineBinding{
					.pBinding = pBindIndex,
					.pDescType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pStageFlag = VK_SHADER_STAGE_VERTEX_BIT,
					.pBindingType = VK_BINDING_UNIFORM_INFO,
					.pUniformBuffers = pBuf->GetBuffers()
				}
			);
		}
	}

	void VK_Draw::SetTexture(const RHI::GPU_Texture* pTexture, uint32_t pBindIndex)
	{
		pBindings.push_back(
			VK_PipelineBinding{
				.pBinding = pBindIndex,
				.pDescType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pBindingType = VK_BINDING_IMAGE_INFO,
				.pTexture = (const VK_Texture*)pTexture
			}
		);
	}

	void VK_Draw::InitBindings()
	{
		pGraphPipeline.Create(&pBindings);
	}

	void VK_Draw::Destroy()
	{
		pShader.Destroy();
		pGraphPipeline.Destroy();
	}

	void VK_Draw::Draw(uint32_t pFirstVertex, uint32_t pVertexCount)
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

	void VK_Draw::DrawCommand(VkCommandBuffer CmdBuf, uint32_t ImageIndex)
	{
		pGraphPipeline.Bind(ImageIndex);

		vkCmdDraw(CmdBuf, pVertexCount, 1, pFirstVertex, 0);
	}
}