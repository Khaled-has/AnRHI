#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

#include <iostream>
#include <vector>

#include <Vulkan/vulkan.h>

#include "GPU/GPU_Texture.h"

namespace GPU
{

	class VK_Texture : public RHI::GPU_Texture
	{
	public:
		VK_Texture() {}
		VK_Texture(const char* pFilename)
		{
			Create(pFilename);
		}
		~VK_Texture() {}

		virtual void Create(const char* pFilename) override;
		virtual void Destroy() override;

		inline const VkImage& GetImage() const { return pImage; }
		inline const VkSampler& GetSampler() const { return pSampler; }
		inline const VkImageView& GetView() const { return pView; }

	private:
		VkImage pImage		   = VK_NULL_HANDLE;
		VkDeviceMemory pMemory = VK_NULL_HANDLE;
		VkImageView pView	   = VK_NULL_HANDLE;
		VkSampler pSampler     = VK_NULL_HANDLE;

		uint32_t ImageWidth  = 0;
		uint32_t ImageHeight = 0;
		uint32_t ImageChannels = 0;

		void CreateTextureImageFromData(const void* pPixels, VkFormat pFormat, bool IsCubemap);
		void CreateImage(VkFormat pFormat, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags, bool IsCubemap);
		void UpdateTextureImage(VkFormat pFormat, int LayerCount, const void* pPixels, bool IsCubemap);
		void TransitionImageLayout(VkFormat pFormat, VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount);
	};

}

#endif