#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

#include <iostream>
#include <vector>

#include <Vulkan/vulkan.h>

#include "GPU_Texture.h"

namespace GPU
{

	class VK_Texture : public RHI::GPU_Texture
	{
	public:
		VK_Texture(const RHI::GPU_TextureInfo& pInfo);
		VK_Texture() {}
		~VK_Texture() {}

		virtual void Destroy() override;

		inline const VkImage& GetImage(uint32_t pIndex) const { return pImages[pIndex]; }
		inline const VkSampler& GetSampler(uint32_t pIndex) const { return pSamplers[pIndex]; }
		inline const VkImageView& GetView(uint32_t pIndex) const { return pViews[pIndex]; }

		inline VkFormat GetFormat() const { return pTexFormat; }
		inline const RHI::GPU_Size GetImageSize() const { return pSize; }

		inline const RHI::GPU_TextureState GetState() const { return pState; }

		void CreateVKTexture(
			const VkImage* pImage, const VkImageView* pImageView, uint32_t Count,
			VkFormat pFormat, RHI::GPU_TextureState pState, RHI::GPU_TextureType pType
		);
	private:
		std::vector<VkImage> pImages;
		std::vector<VkDeviceMemory> pMemories;
		std::vector<VkImageView> pViews;
		std::vector<VkSampler> pSamplers;

		RHI::GPU_TextureState pState;
		RHI::GPU_TextureType pType;
		RHI::GPU_Size pSize;

		VkFormat pTexFormat;

		void CreateTextureImageFromData(const void* pPixels, VkImage& pImage, VkDeviceMemory& pMemory, VkFormat pFormat, VkImageUsageFlagBits pAttachmentFlag, bool IsCubemap);
		void CreateImage(VkFormat pFormat, VkImage& pImage, VkDeviceMemory& pMemory, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags, bool IsCubemap);
		void UpdateTextureImage(VkFormat pFormat, VkImage pImage, int LayerCount, const void* pPixels, bool IsCubemap);
		void TransitionImageLayout(VkFormat pFormat, VkImage pImage, VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount);
	};

}

#endif