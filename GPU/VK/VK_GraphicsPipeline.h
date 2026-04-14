#ifndef VK_GRAPHICSPIPELINE_H
#define VK_GRAPHICSPIPELINE_H

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "VK_Buffer.h"
#include "VK_Texture.h"
#include "VK_RenderPass.h"

#include "GPU_DrawCmd.h"

namespace GPU
{

	// # The pipeline it self
	class VK_GraphicsPipeline
	{
	public:
		VK_GraphicsPipeline() {}
		~VK_GraphicsPipeline() {}

		void Create(const RHI::GPU_DrawInfo& pInfo);
		void Destroy();

		void Bind(uint32_t ImageIndex);
		void CreatePipeline();

	private:
		VkPipeline pPipeline					   = VK_NULL_HANDLE;
		VkPipelineLayout pPipelineLayout		   = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> pDescriptorSets;
		VkDescriptorSetLayout pDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool pDescriptorPool		   = VK_NULL_HANDLE;

		RHI::GPU_RenderArea pRenderArea;

		void CreateDescriptorPool(const RHI::GPU_Binding* pBindings, uint32_t pCount);
		void CreateDescriptorLayout(const RHI::GPU_Binding* pBindings, uint32_t pCount);
		void AllocateDescriptorSets();
		void UpdateDescriptorSets(const RHI::GPU_Binding* pBindings, uint32_t pCount);

	};

}

#endif