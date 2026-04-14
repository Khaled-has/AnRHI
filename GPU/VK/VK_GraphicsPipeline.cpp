#include "VK_GraphicsPipeline.h"

#include "VK_wrappar.h"
#include "VK_Backend.h"

namespace GPU
{

	VkDescriptorType GetVKDescriptorType(RHI::GPU_BindingType pType)
	{
		switch (pType)
		{
		case RHI::GPU_BINDING_TYPE_STATIC_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		case RHI::GPU_BINDING_TYPE_DYNAMIC_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		case RHI::GPU_BINDING_TYPE_TEXTURE:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}

		exit(1);
	}

	VkShaderStageFlags GetVKShaderStageFlag(RHI::GPU_ShaderStage pStage)
	{
		switch (pStage)
		{
		case RHI::GPU_SHADER_STAGE_VERTEX_BIT:
			return VK_SHADER_STAGE_VERTEX_BIT;

		case RHI::GPU_SHADER_STAGE_FRAGMENT_BIT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
	}

	void VK_GraphicsPipeline::Create(const RHI::GPU_DrawInfo& pInfo)
	{
		pRenderArea = pInfo.pRenderArea;

		CreateDescriptorPool(pInfo.pBindings, pInfo.pBindCount);
		CreateDescriptorLayout(pInfo.pBindings, pInfo.pBindCount);
		AllocateDescriptorSets();
		UpdateDescriptorSets(pInfo.pBindings, pInfo.pBindCount);
	}

	void VK_GraphicsPipeline::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		vkDestroyDescriptorSetLayout(pDevice, pDescriptorSetLayout, NULL);
		vkDestroyDescriptorPool(pDevice, pDescriptorPool, NULL);

		vkDestroyPipeline(pDevice, pPipeline, NULL);
		vkDestroyPipelineLayout(pDevice, pPipelineLayout, NULL);
	}

	void VK_GraphicsPipeline::Bind(uint32_t ImageIndex)
	{
		const VkCommandBuffer& CmdBuf = VK_Backend::Get()->GetCmdBuf(ImageIndex);

		// 1 # Bind pipeline
		vkCmdBindPipeline(
			CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline
		);

		// 2 # Bind descriptor sets
		vkCmdBindDescriptorSets(
			CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipelineLayout,
			0, 1,
			&pDescriptorSets[ImageIndex], 0, NULL
		);
	}

	void VK_GraphicsPipeline::CreateDescriptorPool(const RHI::GPU_Binding* pBindings, uint32_t pCount)
	{
		const uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();
		std::vector<VkDescriptorPoolSize> PoolSizes;

		for (uint32_t i = 0; i < pCount; i++)
		{
			PoolSizes.push_back(
				VkDescriptorPoolSize{
					.type = GetVKDescriptorType(pBindings[i].pBindType),
					.descriptorCount = NumImages
				}
			);
		}

		VkDescriptorPoolCreateInfo PoolInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = 0,
			.maxSets = NumImages,
			.poolSizeCount = (uint32_t)PoolSizes.size(),
			.pPoolSizes = PoolSizes.data()
		};

		VkResult res = vkCreateDescriptorPool(
			VK_Backend::Get()->GetDevice().GetDevice(),
			&PoolInfo, NULL, &pDescriptorPool
		);
		VK_CHECK("vkCreateDescriptorPool", res);
		
	}

	void VK_GraphicsPipeline::CreateDescriptorLayout(const RHI::GPU_Binding* pBindings, uint32_t pCount)
	{
		std::vector<VkDescriptorSetLayoutBinding> Bindings;

		for (uint32_t i = 0; i < pCount; i++)
		{
			Bindings.push_back(
				VkDescriptorSetLayoutBinding{
					.binding = pBindings[i].pBinding,
					.descriptorType = GetVKDescriptorType(pBindings[i].pBindType),
					.descriptorCount = 1,
					.stageFlags = GetVKShaderStageFlag(pBindings[i].pStage)
				}
			);
		}

		VkDescriptorSetLayoutCreateInfo CreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.bindingCount = (uint32_t)Bindings.size(),
			.pBindings = Bindings.data()
		};

		VkResult res = vkCreateDescriptorSetLayout(
			VK_Backend::Get()->GetDevice().GetDevice(),
			&CreateInfo, NULL, &pDescriptorSetLayout
		);
		VK_CHECK("vkCreateDescriptorSetLayout", res);
	}

	void VK_GraphicsPipeline::AllocateDescriptorSets()
	{
		const uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();
		std::vector<VkDescriptorSetLayout> Layouts(NumImages, pDescriptorSetLayout);

		VkDescriptorSetAllocateInfo AllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = NULL,
			.descriptorPool = pDescriptorPool,
			.descriptorSetCount = NumImages,
			.pSetLayouts = Layouts.data()
		};

		pDescriptorSets.resize(NumImages);

		VkResult res = vkAllocateDescriptorSets(
			VK_Backend::Get()->GetDevice().GetDevice(),
			&AllocateInfo, pDescriptorSets.data()
		);
	}

	void VK_GraphicsPipeline::UpdateDescriptorSets(const RHI::GPU_Binding* pBindings, uint32_t pCount)
	{

		for (uint32_t i = 0; i < VK_Backend::Get()->GetSwapChain().GetImageCount(); i++)
		{
			std::vector<VkWriteDescriptorSet> WriteDescriptorSets{};

			for (uint32_t j = 0; j < pCount; j++)
			{
				// # Buffer
				if (pBindings[j].pBindType == RHI::GPU_BINDING_TYPE_STATIC_BUFFER)
				{
					VkDescriptorBufferInfo BufferInfo = {
						.buffer = reinterpret_cast<const VK_Buffer*>(pBindings[j].pBuffer)->GetBuffer().pBuffer,
						.offset = 0,
						.range = VK_WHOLE_SIZE
					};

					WriteDescriptorSets.push_back(
						VkWriteDescriptorSet{
							.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							.dstSet = pDescriptorSets[i],
							.dstBinding = pBindings[j].pBinding,
							.dstArrayElement = 0,
							.descriptorCount = 1,
							.descriptorType = GetVKDescriptorType(pBindings[j].pBindType),
							.pBufferInfo = &BufferInfo
					});
				}
				// # Image
				else if (pBindings[j].pBindType == RHI::GPU_BINDING_TYPE_TEXTURE)
				{
					const VK_Texture* pTex = reinterpret_cast<const VK_Texture*>(pBindings[j].pTexture);

					uint32_t pImageIndex = pTex->GetState() == RHI::GPU_TEXTURE_STATE_DYNAMIC ? i : 0;
					VkDescriptorImageInfo ImageInfo = {
						.sampler = pTex->GetSampler(pImageIndex),
						.imageView = pTex->GetView(pImageIndex),
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					};

					WriteDescriptorSets.push_back(
						VkWriteDescriptorSet{
							.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							.dstSet = pDescriptorSets[i],
							.dstBinding = pBindings[j].pBinding,
							.dstArrayElement = 0,
							.descriptorCount = 1,
							.descriptorType = GetVKDescriptorType(pBindings[j].pBindType),
							.pImageInfo = &ImageInfo
					});	
				}
				// # Uniform buffer
				else if (pBindings[j].pBindType == RHI::GPU_BINDING_TYPE_DYNAMIC_BUFFER)
				{
					VkDescriptorBufferInfo UniformInfo = {
						.buffer = reinterpret_cast<const VK_Buffer*>(pBindings[j].pBuffer)->GetBuffers()[i].pBuffer,
						.offset = 0,
						.range = VK_WHOLE_SIZE
					};

					WriteDescriptorSets.push_back(
						VkWriteDescriptorSet{
							.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							.dstSet = pDescriptorSets[i],
							.dstBinding = pBindings[j].pBinding,
							.dstArrayElement = 0,
							.descriptorCount = 1,
							.descriptorType = GetVKDescriptorType(pBindings[j].pBindType),
							.pBufferInfo = &UniformInfo
						}
					);
				}
			}

			vkUpdateDescriptorSets(
			VK_Backend::Get()->GetDevice().GetDevice(), (uint32_t)WriteDescriptorSets.size(),
			WriteDescriptorSets.data(), 0, NULL
			);
		}
	}

	void VK_GraphicsPipeline::CreatePipeline()
	{
		bool IsColorAttachmentTheSame = true;
		const auto pCurrenDrawInfo = VK_Backend::Get()->GetCurrentRenderPassInfo();

		VK_Shader pShader = VK_Backend::Get()->GetCurrentShader();
		VkShaderModule pVs = pShader.GetVertexShader();
		VkShaderModule pFs = pShader.GetFragmentShader();

		// # Create the pipeline
		VkPipelineShaderStageCreateInfo ShaderStagesCreateInfo[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = pVs,
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = pFs,
			.pName = "main"
		} };

		VkPipelineVertexInputStateCreateInfo VertexInputInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		};

		VkPipelineInputAssemblyStateCreateInfo IACreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkViewport VP = {
			.x = (float)pRenderArea.pOffset.x,
			.y = (float)pRenderArea.pOffset.y,
			.width = (float)pRenderArea.pExtent.width,
			.height = (float)pRenderArea.pExtent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		VkRect2D Scissor = {
			.offset = {
				.x = pRenderArea.pOffset.x, .y = pRenderArea.pOffset.y
			},
			.extent = {
				.width = pRenderArea.pExtent.width, .height = pRenderArea.pExtent.height
			}
		};

		VkPipelineViewportStateCreateInfo ViewportCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &VP,
			.scissorCount = 1,
			.pScissors = &Scissor
		};

		VkPipelineRasterizationStateCreateInfo RastCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.lineWidth = 1.0f
		};

		VkPipelineMultisampleStateCreateInfo MSCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f
		};

		VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {},
			.back = {},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f
		};

		VkPipelineColorBlendAttachmentState BlendAttachState = {
			.blendEnable = VK_FALSE,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
		};

		VkPipelineColorBlendStateCreateInfo BlendCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &BlendAttachState
		};

		std::vector<VkFormat> pFormats;
		for (uint32_t i = 0; i < pCurrenDrawInfo.pColorTexCount; i++)
		{
			pFormats.push_back(reinterpret_cast<const VK_Texture*>(&pCurrenDrawInfo.pColorTextures[i])->GetFormat());
		}
		VkPipelineRenderingCreateInfo RenderingInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			.pNext = NULL,
			.colorAttachmentCount = (uint32_t)pCurrenDrawInfo.pColorTexCount,
			.pColorAttachmentFormats = pFormats.data(),
			.depthAttachmentFormat = pCurrenDrawInfo.pEnableDepth ? reinterpret_cast<const VK_Texture*>(pCurrenDrawInfo.pDepthTexture)->GetFormat() : VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		VkPipelineLayoutCreateInfo LayoutInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &pDescriptorSetLayout
		};

		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkResult res = vkCreatePipelineLayout(pDevice, &LayoutInfo, NULL, &pPipelineLayout);
		VK_CHECK("vkCreatePipelineLayout: Model\n", res);

		VkGraphicsPipelineCreateInfo GPipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported ? &RenderingInfo : NULL,
			.stageCount = (sizeof(ShaderStagesCreateInfo) / sizeof(VkPipelineShaderStageCreateInfo)),
			.pStages = &ShaderStagesCreateInfo[0],
			.pVertexInputState = &VertexInputInfo,
			.pInputAssemblyState = &IACreateInfo,
			.pViewportState = &ViewportCreateInfo,
			.pRasterizationState = &RastCreateInfo,
			.pMultisampleState = &MSCreateInfo,
			.pDepthStencilState = &DepthStencilCreateInfo,
			.pColorBlendState = &BlendCreateInfo,
			.layout = pPipelineLayout,
			.renderPass = VK_Backend::Get()->GetCurrentRenderPass(),
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		res = vkCreateGraphicsPipelines(pDevice, VK_NULL_HANDLE, 1, &GPipelineInfo, NULL, &pPipeline);
		VK_CHECK("vkCreateGraphicsPipeline\n", res);
	}

}