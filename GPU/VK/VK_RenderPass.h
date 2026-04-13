#ifndef VK_RENDERPASS_H
#define VK_RENDERPASS_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "GPU_RenderPass.h"

#include <vulkan/vulkan.h>

namespace GPU
{

	class VK_RenderPass : public RHI::GPU_RenderPass
	{
	public:
		VK_RenderPass() {}
		VK_RenderPass(const RHI::GPU_RenderPassInfo pInfo)
		{
			Create(pInfo);
		}
		~VK_RenderPass() {}

		virtual void Destroy() override;

		virtual void Begin(RHI::GPU_Clear pClear) override;
		virtual void End() override;

		void StartPass(const VkCommandBuffer& pCmdBuf, uint32_t ImageIndex);
		void EndPass(const VkCommandBuffer& pCmdBuf, uint32_t ImageIndex);

	private:
		RHI::GPU_RenderPassInfo pRenderPassInfo;
		RHI::GPU_Clear pClearValue;

		struct Frame_Data
		{
			std::vector<VkImage> pColorImages;
			std::vector<VkImageView> pColorImageViews;
			std::vector<VkFormat> pColorImageFormats;

			VkImage pDepthImage;
			VkImageView pDepthImageView;
			VkFormat pDepthFormat;
		};

		std::vector<Frame_Data> pImageFrames;

		VkRenderPass pRenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> pFramebuffers;

		virtual void Create(const RHI::GPU_RenderPassInfo& pInfo) override;

		void CreateFramebuffers();
		void CreateRenderPass();
	};

}

#endif