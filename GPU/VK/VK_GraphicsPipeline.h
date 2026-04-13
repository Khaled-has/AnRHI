#ifndef VK_GRAPHICSPIPELINE_H
#define VK_GRAPHICSPIPELINE_H

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "VK_Buffer.h"
#include "VK_Texture.h"
#include "VK_RenderPass.h"

namespace GPU
{
	// # Type of binding attribute
	enum VK_BindingInfoType
	{
		VK_BINDING_UNDEFINE     = 0,
		VK_BINDING_BUFFER_INFO  = 1,
		VK_BINDING_IMAGE_INFO   = 2,
		VK_BINDING_FRAME_IMAGE_INFO = 3,
		VK_BINDING_UNIFORM_INFO = 4
	};

	// # The of bindings for descriptor sets
	struct VK_PipelineBinding
	{
		uint32_t pBinding;
		VkDescriptorType pDescType;
		VkShaderStageFlags pStageFlag;
		VK_BindingInfoType pBindingType;
		const VK_Buffer* pBuffer;
		const VK_Texture* pTexture;
		std::vector<VK_BufferAndMemory> pUniformBuffers;
	};

	// # The pipeline it self
	class VK_GraphicsPipeline
	{
	public:
		VK_GraphicsPipeline() {}
		~VK_GraphicsPipeline() {}

		void Create(const std::vector<VK_PipelineBinding>* pBindingsInfo);
		void Destroy();

		void Bind(uint32_t ImageIndex);
		void CreatePipeline();

	private:
		VkPipeline pPipeline					   = VK_NULL_HANDLE;
		VkPipelineLayout pPipelineLayout		   = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> pDescriptorSets;
		VkDescriptorSetLayout pDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool pDescriptorPool		   = VK_NULL_HANDLE;

		void CreateDescriptorPool(const std::vector<VK_PipelineBinding>* pBindingsInfo);
		void CreateDescriptorLayout(const std::vector<VK_PipelineBinding>* pBindingsInfo);
		void AllocateDescriptorSets();
		void UpdateDescriptorSets(const std::vector<VK_PipelineBinding>* pBindingsInfo);

	};

}

#endif