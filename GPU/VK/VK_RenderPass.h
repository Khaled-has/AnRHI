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

	struct VK_FrameImage
	{
		std::vector<VkImage> pImages;
		std::vector<VkImageView> pViews;
		std::vector<VkSampler> pSamplers;
	};

	struct VK_RenderPassDrawInfo
	{
		bool pEnableColor;
		bool pEnableDepth;
		std::vector<VkFormat> pColorFormats;
		VkFormat pDepthFormat;
		uint32_t pWidth;
		uint32_t pHeight;
	};

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

		inline const VK_FrameImage& GetFrameImage() const { return pFrameImage; }

	private:
		RHI::GPU_RenderPassInfo pCreateInfo;
		RHI::GPU_Clear pClearValue;

		struct Image_Data
		{
			std::vector<VkImage> pImages;
			std::vector<VkSampler> pSamplers;
			std::vector<VkImageView> pImageViews;
			std::vector<VkDeviceMemory> pMemories;
		};

		Image_Data pColorImages;
		Image_Data pDepthImages;

		VkRenderPass pRenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> pFramebuffers;

		VK_FrameImage pFrameImage;
		VK_RenderPassDrawInfo pDrawInfo;

		virtual void Create(const RHI::GPU_RenderPassInfo pInfo) override;

		void CreateColorImages();
		void CreateDepthImages();

		void CreateDrawInfo();

		void CreateFramebuffers();
		void CreateRenderPass();
	};

}

#endif