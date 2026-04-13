#include "VK_Texture.h"

#include "VK_wrappar.h"
#include "VK_Backend.h"
#include "VK_Buffer.h"

namespace GPU
{

	void VK_Texture::BindData(RHI::GPU_TextureType pTexType, const void* pPixels, unsigned int pWidth, unsigned int pHeight, RHI::GPU_Format pFormat, RHI::GPU_TextureState pState)
	{
		ImageWidth = pWidth; ImageHeight = pHeight;
		pTexFormat = TranslateGPUFormatToVulkanFormat(pFormat);
		this->pState = pState; this->pType = pTexType;

		const uint32_t NumImages = VK_Backend::Get()->GetSwapChain().GetImageCount();

		if (pState == RHI::GPU_TEXTURE_STATE_STATIC)
		{
			pImages.resize(1);
			pViews.resize(1);
			pSamplers.resize(1);
			pMemories.resize(1);
		}
		else if (pState == RHI::GPU_TEXTURE_STATE_DYNAMIC)
		{
			pImages.resize(NumImages);
			pViews.resize(NumImages);
			pSamplers.resize(NumImages);
			pMemories.resize(NumImages);
		}

		for (uint32_t i = 0; i < pImages.size(); i++)
		{
			// # Step 1: create the image object and populate it with pixels
			CreateTextureImageFromData(pPixels, pImages[i], pMemories[i], pTexFormat, false); // Hard coded for now.

			// # Step 2: create image view
			VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT; // Hard coded for now
			pViews[i] = CreateImageView(pImages[i], pTexFormat, AspectFlags, false);

			VkFilter MinFilter = VK_FILTER_LINEAR;
			VkFilter MaxFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			// # Step 4: create the texture sampler
			pSamplers[i] = CreateTextureSampler(MinFilter, MaxFilter, AddressMode);
		}
	}

	void VK_Texture::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		
		for (uint32_t i = 0; i < pImages.size(); i++)
		{
			vkFreeMemory(pDevice, pMemories[i], NULL);
			vkDestroyImageView(pDevice, pViews[i], NULL);
			vkDestroySampler(pDevice, pSamplers[i], NULL);
			vkDestroyImage(pDevice, pImages[i], NULL);
		}
	}

	void VK_Texture::CreateVKTexture(const VkImage* pImage, const VkImageView* pImageView, uint32_t Count, VkFormat pFormat, RHI::GPU_TextureState pState, RHI::GPU_TextureType pType)
	{
		for (uint32_t i = 0; i < Count; i++)
		{
			pImages.push_back(pImage[i]);
			pViews.push_back(pImageView[i]);
		}

		pTexFormat = pFormat;
		this->pState = pState;
		this->pType = pType;
	}

	void VK_Texture::CreateTextureImageFromData(const void* pPixels, VkImage& pImage, VkDeviceMemory& pMemory, VkFormat pFormat, bool IsCubemap)
	{
		VkImageUsageFlagBits Usage;

		if (pState == RHI::GPU_TEXTURE_STATE_STATIC)
		{
			Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		}
		else if (pState == RHI::GPU_TEXTURE_STATE_DYNAMIC)
		{
			Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		}

		VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		CreateImage(pFormat, pImage, pMemory, Usage, PropertyFlags, IsCubemap);

		if (pState == RHI::GPU_TEXTURE_STATE_STATIC)
		{
			UpdateTextureImage(pFormat, pImage, 1, pPixels, IsCubemap);
		}
	}

	void VK_Texture::CreateImage(VkFormat pFormat, VkImage& pImage, VkDeviceMemory& pMemory, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags, bool IsCubemap)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkImageCreateInfo ImageInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = IsCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : (VkImageCreateFlags)0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = pFormat,
			.extent = VkExtent3D{.width = ImageWidth, .height = ImageHeight, .depth = 1},
			.mipLevels = 1,
			.arrayLayers = IsCubemap ? 6u : 1u,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = UsageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = NULL,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		// # Step 1: create the image object
		VkResult res = vkCreateImage(pDevice, &ImageInfo, NULL, &pImage);
		VK_CHECK("vkCreateImage", res);

		// # Step 2: get the buffer memory requirements
		VkMemoryRequirements MemReqs = { 0 };
		vkGetImageMemoryRequirements(pDevice, pImage, &MemReqs);

		// # Step 3: get the memory type index
		uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);

		// # Step 4: allocate memory
		VkMemoryAllocateInfo MemAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.allocationSize = MemReqs.size,
			.memoryTypeIndex = MemoryTypeIndex
		};

		res = vkAllocateMemory(pDevice, &MemAllocInfo, NULL, &pMemory);
		VK_CHECK("vkAllocateMemory", res);

		// # Step 5: bind memory
		res = vkBindImageMemory(pDevice, pImage, pMemory, 0);
		VK_CHECK("vkBindImageMemory", res);
	}

	void VK_Texture::UpdateTextureImage(VkFormat pFormat, VkImage pImage, int LayerCount, const void* pPixels, bool IsCubemap)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		int BytesPerPixel = GetBytesPerTexFormat(pFormat);

		VkDeviceSize LayerSize = ImageWidth * ImageHeight * BytesPerPixel;
		VkDeviceSize ImageSize = LayerCount * LayerSize;

		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags Properties =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VK_BufferAndMemory StagingTex = CreateBuffer(ImageSize, Usage, Properties);

		void* pMem = NULL;
		VkResult res = vkMapMemory(pDevice, StagingTex.pMemory, 0, ImageSize, 0, &pMem);
		VK_CHECK("vkMapMemory", res);
		memcpy(pMem, pPixels, ImageSize);
		vkUnmapMemory(pDevice, StagingTex.pMemory);

		TransitionImageLayout(pFormat, pImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, LayerCount);

		CopyBufferToImage(pImage, StagingTex.pBuffer, ImageWidth, ImageHeight, LayerSize, LayerCount);

		TransitionImageLayout(pFormat, pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, LayerCount);

		vkFreeMemory(pDevice, StagingTex.pMemory, NULL);
		vkDestroyBuffer(pDevice, StagingTex.pBuffer, NULL);
	}

	void VK_Texture::TransitionImageLayout(VkFormat pFormat, VkImage pImage, VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount)
	{
		const VkCommandBuffer& CmdBuf = VK_Backend::Get()->GetCopyCmdBuf();

		BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		ImageMemBarrier(CmdBuf, pImage, pFormat, OldLayout, NewLayout, LayerCount);

		vkEndCommandBuffer(CmdBuf);

		VK_Backend::Get()->GetQueue().SubmitSync(CmdBuf);

		VK_Backend::Get()->GetQueue().WaitIdle();
	}

}