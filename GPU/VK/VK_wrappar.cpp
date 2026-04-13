#include "VK_wrappar.h"

#include "VK_Backend.h"

#include "Backends/GPU_LibBackend.h"

namespace GPU
{
	uint32_t GetMemoryTypeIndex(uint32_t MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags)
	{
		const VkPhysicalDeviceMemoryProperties& MemProps = VK_Backend::Get()->GetDevice().GetSelectedDevice().m_memProps;

		for (uint32_t i = 0; i < MemProps.memoryTypeCount; i++)
		{
			const VkMemoryType& MemType = MemProps.memoryTypes[i];
			uint32_t CurBitmask = (1 << i);
			bool IsCurMemTypeSupported = (MemTypeBitsMask & CurBitmask);
			bool HasRequiredMemProps = ((MemType.propertyFlags & ReqMemPropFlags) == ReqMemPropFlags);

			if (IsCurMemTypeSupported && HasRequiredMemProps)
			{
				return i;
			}
		}

		VK_LOG_ERROR("Cannot find memory type for type {0} requested mem props {1}", (uint32_t)MemTypeBitsMask, ReqMemPropFlags);
		exit(1);
		return -1;
	}

	bool HasStencilComponent(VkFormat Format)
	{
		return ((Format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
			(Format == VK_FORMAT_D24_UNORM_S8_UINT));
	}

	void ImageMemBarrier(const VkCommandBuffer& CmdBuf, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount)
	{
		VkImageMemoryBarrier Barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = NULL,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = OldLayout,
			.newLayout = NewLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = Image,
			.subresourceRange = VkImageSubresourceRange {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = (uint32_t)LayerCount
			}
		};

		VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
		VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

		if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
			(Format == VK_FORMAT_D16_UNORM) ||
			(Format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
			(Format == VK_FORMAT_D32_SFLOAT) ||
			(Format == VK_FORMAT_S8_UINT) ||
			(Format == VK_FORMAT_D16_UNORM_S8_UINT) ||
			(Format == VK_FORMAT_D24_UNORM_S8_UINT))
		{
			Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(Format)) {
				Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_GENERAL) {
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} /* Convert back from read-only to updateable */
		else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			Barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} /* Convert from updateable texture to shader read-only */
		else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} /* Convert depth texture from undefined state to depth-stencil buffer */
		else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		} /* Wait for render pass to complete */
		else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			Barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
			Barrier.dstAccessMask = 0;
			/*
					sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
					destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			*/
			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} /* Convert back from read-only to color attachment */
		else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			Barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			Barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		} /* Convert from updateable texture to shader read-only */
		else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			Barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} /* Convert back from read-only to depth attachment */
		else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			Barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		} /* Convert from updateable depth texture to shader read-only */
		else if (OldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			Barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
			Barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			Barrier.dstAccessMask = 0;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else {
			printf("Unknown Barrier case\n");
			exit(1);
		}

		vkCmdPipelineBarrier(CmdBuf, sourceStage, destinationStage,
			0, 0, NULL, 0, NULL, 1, &Barrier);
	}

	void BeginDynamicRendering(const VkCommandBuffer& CmdBuf, uint32_t ImageIndex, VkClearValue* pClearColor, VkClearValue* pDepthValue, bool IsDepthTest)
	{
		VkRenderingAttachmentInfoKHR ColorAttachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
			.pNext = NULL,
			.imageView = VK_Backend::Get()->GetSwapChain().GetImageView(ImageIndex),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = pClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE
		};

		if (pClearColor)
		{
			ColorAttachment.clearValue = *pClearColor;
		}

		VkRenderingAttachmentInfo DepthAttachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = NULL,
			.imageView = IsDepthTest ? VK_Backend::Get()->GetSwapChain().GetDepthImageView(ImageIndex) : VK_NULL_HANDLE,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = pDepthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE
		};

		if (pDepthValue)
		{
			DepthAttachment.clearValue = *pDepthValue;
		}

		auto pWinSize = lib_backend::GPU_LibBackend::GetInstance()->GetWindowSize();

		VkRenderingInfoKHR RenderingInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
			.renderArea = {{0, 0}, { pWinSize.pWidth, pWinSize.pHeight }},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = &ColorAttachment,
			.pDepthAttachment = IsDepthTest ? &DepthAttachment : VK_NULL_HANDLE
		};

		vkCmdBeginRendering(CmdBuf, &RenderingInfo);
	}

	void BeginDynamicRendering(const VkCommandBuffer& CmdBuf, const VkImageView* pColorView, uint32_t pColorCount, const VkImageView* pDepthView, VkClearValue* pClearColor, VkClearValue* pDepthValue, bool IsColorAttch, bool IsDepthTest, RHI::GPU_RenderArea pArea)
	{

		std::vector <VkRenderingAttachmentInfoKHR> ColorAttachments;
		for (uint32_t i = 0; i < pColorCount; i++)
		{
			VkRenderingAttachmentInfoKHR ColorAttach = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
				.pNext = NULL,
				.imageView = *pColorView,
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.resolveImageView = VK_NULL_HANDLE,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp = pClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE
			};

			if (IsColorAttch)
			{
				ColorAttach.clearValue = *pClearColor;
			}

			ColorAttachments.push_back(ColorAttach);
		}

		VkRenderingAttachmentInfo DepthAttachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = NULL,
			.imageView = IsDepthTest ? *pDepthView : VK_NULL_HANDLE,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = pDepthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE
		};

		if (IsDepthTest)
		{
			DepthAttachment.clearValue = *pDepthValue;
		}

		VkRenderingInfoKHR RenderingInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
			.renderArea = {
				.offset{.x = (int32_t)pArea.pOffset.x, .y = (int32_t)pArea.pOffset.y },
				.extent{ .width = pArea.pExtent.width, .height = pArea.pExtent.height }
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = pColorCount,
			.pColorAttachments = ColorAttachments.data(),
			.pDepthAttachment = IsDepthTest ? &DepthAttachment : VK_NULL_HANDLE
		};

		vkCmdBeginRendering(CmdBuf, &RenderingInfo);
	}

	VK_BufferAndMemory CreateBuffer(size_t pSize, VkBufferUsageFlags pUsage, VkMemoryPropertyFlags pMemProp)
	{
		VkBufferCreateInfo bCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = pSize,
			.usage = pUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VK_BufferAndMemory bufferAndMem;

		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		// # Step 1: create a buffer
		VkResult res = vkCreateBuffer(pDevice, &bCreateInfo, NULL, &bufferAndMem.pBuffer);
		VK_CHECK("vkCreateBuffer", res);

		// # Step 2: get the buffer memory requitements
		VkMemoryRequirements MemReqs = {};
		vkGetBufferMemoryRequirements(pDevice, bufferAndMem.pBuffer, &MemReqs);

		bufferAndMem.pAllocationSize = MemReqs.size;

		// # Step 3: get the memory type index
		uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, pMemProp);

		// # Step 4: allocate memory
		VkMemoryAllocateInfo MemAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = MemReqs.size,
			.memoryTypeIndex = MemoryTypeIndex
		};

		res = vkAllocateMemory(pDevice, &MemAllocInfo, NULL, &bufferAndMem.pMemory);
		VK_CHECK("vkAllocateMemory", res);

		// # Step 5: bind memory
		res = vkBindBufferMemory(pDevice, bufferAndMem.pBuffer, bufferAndMem.pMemory, 0);
		VK_CHECK("vkBindBufferMemory", res);

		return bufferAndMem;
	}

	void CopyBuffer(VkBuffer pDst, VkBuffer pSrc, VkDeviceSize pSize)
	{
		const VkCommandBuffer& pCopyCmdBuf = VK_Backend::Get()->GetCopyCmdBuf();

		BeginCommandBuffer(pCopyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy BufferCopy = {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = pSize
		};

		vkCmdCopyBuffer(pCopyCmdBuf, pSrc, pDst, 1, &BufferCopy);

		vkEndCommandBuffer(pCopyCmdBuf);

		const VK_Queue& pQueue = VK_Backend::Get()->GetQueue();

		pQueue.SubmitSync(pCopyCmdBuf);

		pQueue.WaitIdle();
	}

	VkImageView CreateImageView(const VkImage& Image, VkDevice Device, VkFormat Format, VkImageAspectFlags AspectFlags)
	{
		VkImageViewCreateInfo ViewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.image = Image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = Format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = AspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkImageView ImageView;
		VkResult res = vkCreateImageView(Device, &ViewInfo, NULL, &ImageView);
		VK_CHECK("vkCreateImageView", res);

		return ImageView;
	}

	VkSampler CreateTextureSampler(VkFilter MinFilter, VkFilter MaxFilter,
		VkSamplerAddressMode AddressMode)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkSamplerCreateInfo SamplerInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = MinFilter,
			.minFilter = MaxFilter,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = AddressMode,
			.addressModeV = AddressMode,
			.addressModeW = AddressMode,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 1,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE
		};

		VkSampler Sampler;
		VkResult res = vkCreateSampler(pDevice, &SamplerInfo, VK_NULL_HANDLE, &Sampler);
		VK_CHECK("vkCreateSampler", res);
		return Sampler;
	}

	VkImageView CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, bool IsCubemap)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkImageViewCreateInfo ViewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.image = Image,
			.viewType = IsCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
			.format = Format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = AspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = IsCubemap ? 6u : 1u
			}
		};

		VkImageView ImageView;
		VkResult res = vkCreateImageView(pDevice, &ViewInfo, NULL, &ImageView);
		if (res != VK_SUCCESS)
		{
			printf("Cannot create image view\n");
		}

		return ImageView;
	}

	void CopyBufferToImage(VkImage Dst, VkBuffer Src, uint32_t ImageWidth, uint32_t ImageHeight, VkDeviceSize LayerSize, int LayerCount)
	{
		const VkCommandBuffer& CmdBuf = VK_Backend::Get()->GetCopyCmdBuf();

		BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		std::vector<VkBufferImageCopy> BufferImageCopy(LayerCount);

		for (int i = 0; i < LayerCount; i++)
		{
			VkBufferImageCopy bic = {
				.bufferOffset = i * LayerSize,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = VkImageSubresourceLayers{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = (uint32_t)i,
					.layerCount = (uint32_t)LayerCount
				},
				.imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
				.imageExtent = VkExtent3D{.width = ImageWidth, .height = ImageHeight, .depth = 1}
			};

			BufferImageCopy[i] = bic;
		}

		vkCmdCopyBufferToImage(
			CmdBuf, Src, Dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			(uint32_t)BufferImageCopy.size(), BufferImageCopy.data());

		vkEndCommandBuffer(CmdBuf);

		VK_Backend::Get()->GetQueue().SubmitSync(CmdBuf);

		VK_Backend::Get()->GetQueue().WaitIdle();
	}

	int GetBytesPerTexFormat(VkFormat Format)
	{
		switch (Format)
		{
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_UNORM:
			return 1;
		case VK_FORMAT_R16_SFLOAT:
			return 2;
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_SRGB:
			return 4;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 4 * sizeof(uint16_t);
		case VK_FORMAT_R32G32B32_SFLOAT:
			return 3 * sizeof(float);
		case VK_FORMAT_R8G8B8_SRGB:
			return 3;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 4 * sizeof(float);
		default:
			VK_LOG_ERROR("Unknown format: {0}", (uint32_t)Format);
			exit(1);
		}
		return 0;
	}

	VkFormat TranslateGPUFormatToVulkanFormat(RHI::GPU_Format pFormat)
	{
		switch (pFormat)
		{
		case RHI::GPU_FORMAT_COLOR_BGRA8:
			return VK_FORMAT_B8G8R8A8_UNORM;

		case RHI::GPU_FORMAT_COLOR_RGBA8:
			return VK_FORMAT_R8G8B8A8_UNORM;

		case RHI::GPU_FORMAT_D32_FLOAT:
			return VK_FORMAT_D32_SFLOAT;

		default:
			return VK_FORMAT_UNDEFINED;
		}

		return VK_FORMAT_UNDEFINED;
	}
}