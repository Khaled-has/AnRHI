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
		VK_Texture() {}
		~VK_Texture() {}

		virtual void BindData(RHI::GPU_TextureType pTexType, const void* pPixels, unsigned int pWidth, unsigned int pHeight, RHI::GPU_Format pFormat, RHI::GPU_TextureState pState) override;
		virtual void Destroy() override;

		inline const VkImage& GetImage(uint32_t pIndex) const { return pImages[pIndex]; }
		inline const VkSampler& GetSampler(uint32_t pIndex) const { return pSamplers[pIndex]; }
		inline const VkImageView& GetView(uint32_t pIndex) const { return pViews[pIndex]; }

		inline VkFormat GetFormat() const { return pTexFormat; }
		inline const RHI::GPU_Size GetImageSize() const { return RHI::GPU_Size{ .pWidth = ImageWidth, .pHeight = ImageHeight }; }

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

		VkFormat pTexFormat = VK_FORMAT_B8G8R8A8_UNORM;

		uint32_t ImageWidth  = 0;
		uint32_t ImageHeight = 0;
		uint32_t ImageChannels = 0;

		void CreateTextureImageFromData(const void* pPixels, VkImage& pImage, VkDeviceMemory& pMemory, VkFormat pFormat, bool IsCubemap);
		void CreateImage(VkFormat pFormat, VkImage& pImage, VkDeviceMemory& pMemory, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags, bool IsCubemap);
		void UpdateTextureImage(VkFormat pFormat, VkImage pImage, int LayerCount, const void* pPixels, bool IsCubemap);
		void TransitionImageLayout(VkFormat pFormat, VkImage pImage, VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount);
	};

}

#endif