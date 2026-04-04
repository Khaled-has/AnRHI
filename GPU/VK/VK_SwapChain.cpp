#include "VK_SwapChain.h"

#include "VK_Backend.h"
#include "VK_wrappar.h"

#include "Backends/GPU_LibBackend.h"

namespace GPU
{

    void CreateImage(VK_Image& pImage, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat pFormat, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags)
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		VkImageCreateInfo ImageInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = (VkImageCreateFlags)0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = pFormat,
			.extent = VkExtent3D{.width = ImageWidth, .height = ImageHeight, .depth = 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = UsageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = NULL,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		// # Step 1: create the image object
		VkResult res = vkCreateImage(pDevice, &ImageInfo, NULL, &pImage.pImage);
		VK_CHECK("vkCreateImage", res);

		// # Step 2: get the buffer memory requirements
		VkMemoryRequirements MemReqs = { 0 };
		vkGetImageMemoryRequirements(pDevice, pImage.pImage, &MemReqs);

		// # Step 3: get the memory type index
		uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);

		// # Step 4: allocate memory
		VkMemoryAllocateInfo MemAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.allocationSize = MemReqs.size,
			.memoryTypeIndex = MemoryTypeIndex
		};

		res = vkAllocateMemory(pDevice, &MemAllocInfo, NULL, &pImage.pMem);
		VK_CHECK("vkAllocateMemory", res);

		// # Step 5: bind memory
		res = vkBindImageMemory(pDevice, pImage.pImage, pImage.pMem, 0);
		VK_CHECK("vkBindImageMemory", res);
	}

	uint32_t ChooseNumImages(const VkSurfaceCapabilitiesKHR& Capabilities)
	{
		uint32_t RequestedNumImage = Capabilities.minImageCount + 1;

		uint32_t FinalNumImages = 0;

		if ((Capabilities.maxImageCount > 0) && (RequestedNumImage > Capabilities.maxImageCount))
		{
			FinalNumImages = Capabilities.maxImageCount;
		}
		else {
			FinalNumImages = RequestedNumImage;
		}

		if (FinalNumImages > 3)
			FinalNumImages = 3;

		return FinalNumImages;
	}

	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> PresentModes)
	{
		for (uint32_t i = 0; i < PresentModes.size(); i++)
		{
			if (PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				VK_LOG_INFO("Selected Present Mode Is: VK_PRESENT_MODE_MALIBOX_KHR");
				return PresentModes[i];
			}
		}
		VK_LOG_INFO("Selected Present Mode Is: VK_PRESENT_MODE_FIFO_KHR");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& SurfaceFormats)
	{
		for (uint32_t i = 0; i < SurfaceFormats.size(); i++)
		{
			if ((SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
				(SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
			{
				return SurfaceFormats[i];
			}
		}
		VK_LOG_WARN("No Format & Color Space");
		return SurfaceFormats[0];
	}

	void VK_SwapChain::Create(bool pEnableDepthTest)
	{
		pDepthTest = pEnableDepthTest;

		CreateSwapchain();

		// # Create depth resources
		if (pDepthTest)
		{
			CreateDepthResources();
		}

		/* 
			# If the selected device does't support dynamic rendering
			  then we want to create a RenderPass & Framebuffers
		*/
		if (!VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported)
		{
			CreateRenderPass();
			CreateFramebuffers();

			VK_LOG_INFO("Using RenderPass");
		}
		else
		{
			VK_LOG_INFO("Using Dynamic rendering");
		}
	}

	void VK_SwapChain::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		// # Destroy RenderPass & Framebuffers
		if (!VK_Backend::Get()->GetDevice().GetSelectedDevice().pIsDynamicSupported)
		{
			for (uint32_t i = 0; i < pFramebuffers.size(); i++)
			{
				vkDestroyFramebuffer(pDevice, pFramebuffers[i], NULL);
			}

			vkDestroyRenderPass(pDevice, pRenderPass, NULL);
		}

		// # Destroy swapchain's image views
		for (VkImageView& Im : pImageViews)
		{
			vkDestroyImageView(pDevice, Im, NULL);
		}

		// # Destroy depth resources
		for (uint32_t i = 0; i < pDepthImages.size(); i++)
		{
			vkDestroyImageView(pDevice, pDepthImages[i].pView, NULL);
			vkFreeMemory(pDevice, pDepthImages[i].pMem, NULL);
			vkDestroyImage(pDevice, pDepthImages[i].pImage, NULL);
		}

		// # Destroy swapchain
		vkDestroySwapchainKHR(pDevice, pSwapChain, NULL);
	}

	void VK_SwapChain::CreateSwapchain()
	{
		const VK_Device& pDevice = VK_Backend::Get()->GetDevice();

		// # Choose num swapchain images
		const VkSurfaceCapabilitiesKHR& SurfaceCaps = pDevice.GetSelectedDevice().m_surfaceCaps;
		uint32_t NumImages = ChooseNumImages(SurfaceCaps);

		// # Choose present mode
		const std::vector<VkPresentModeKHR>& PresentModes = pDevice.GetSelectedDevice().m_presentModes;
		VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);
		//PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

		// # Choose swapchain surface format & color space
		pSwChainSurfaceFormat = ChooseSurfaceFormatAndColorSpace(pDevice.GetSelectedDevice().m_surfaceFormats);

		// # Create the swapchain
		VkSwapchainCreateInfoKHR SwapChainCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = NULL,
			.flags = 0,
			.surface = VK_Backend::Get()->GetDevice().GetSurface(),
			.minImageCount = NumImages,
			.imageFormat = pSwChainSurfaceFormat.format,
			.imageColorSpace = pSwChainSurfaceFormat.colorSpace,
			.imageExtent = SurfaceCaps.currentExtent,
			.imageArrayLayers = 1,
			.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = SurfaceCaps.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = PresentMode,
			.clipped = VK_TRUE
		};

		VkResult res = vkCreateSwapchainKHR(pDevice.GetDevice(), &SwapChainCreateInfo, NULL, &pSwapChain);
		VK_CHECK("vkCreateSwapchainKHR", res);

		// # Get the swapchain's images
		uint32_t NumSwapChainImages = 0;
		res = vkGetSwapchainImagesKHR(pDevice.GetDevice(), pSwapChain, &NumSwapChainImages, NULL);
		VK_CHECK("Cannot calculate swapchain images", res);
		NumImages = NumSwapChainImages;
		//assert(NumImages == NumSwapChainImages);

		pImages.resize(NumSwapChainImages);
		pImageViews.resize(NumSwapChainImages);

		res = vkGetSwapchainImagesKHR(pDevice.GetDevice(), pSwapChain, &NumSwapChainImages, pImages.data());
		VK_CHECK("Cannot get swapchain's images", res);

		// # Create swapchain image views
		for (uint32_t i = 0; i < pImageViews.size(); i++)
		{
			pImageViews[i] = CreateImageView(
				pImages[i], pDevice.GetDevice(), pSwChainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT
			);
		}
	}

	void VK_SwapChain::CreateRenderPass()
	{
		const VK_Device& pDevice = VK_Backend::Get()->GetDevice();

		VkAttachmentDescription ColorAttachDesc = {
			.flags = 0,
			.format = pSwChainSurfaceFormat.format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference ColorAttachRef = {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkFormat DepthFormat = pDevice.GetSelectedDevice().m_depthFormat;

		VkAttachmentDescription DepthAttachDesc = {
			.flags = 0,
			.format = DepthFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference DepthAttachmentRef = {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription SubpassDesc = {
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachments = &ColorAttachRef,
			.pResolveAttachments = NULL,
			.pDepthStencilAttachment = pDepthTest ? &DepthAttachmentRef : NULL,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL
		};

		std::vector<VkAttachmentDescription> Attachments;
		Attachments.push_back(ColorAttachDesc);

		if (pDepthTest)
		{
			Attachments.push_back(DepthAttachDesc);
		}

		VkRenderPassCreateInfo RenderCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.attachmentCount = (uint32_t)Attachments.size(),
			.pAttachments = Attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &SubpassDesc,
			.dependencyCount = 0,
			.pDependencies = NULL
		};

		VkResult res = vkCreateRenderPass(pDevice.GetDevice(), &RenderCreateInfo, NULL, &pRenderPass);
		VK_CHECK("vkCreateRenderPass", res);
	}

	void VK_SwapChain::CreateFramebuffers()
	{
		pFramebuffers.resize(GetImageCount());

		auto pWinSize = lib_backend::GPU_LibBackend::GetInstance()->GetWindowSize();

		for (uint32_t i = 0; i < GetImageCount(); i++)
		{
			std::vector<VkImageView> Attachments;
			Attachments.push_back(GetImageView(i));

			// # Depth images
			if (pDepthTest)
			{
				Attachments.push_back(pDepthImages[i].pView);
			}

			VkFramebufferCreateInfo CreateInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = pRenderPass,
				.attachmentCount = (uint32_t)Attachments.size(),
				.pAttachments = Attachments.data(),
				.width = pWinSize.pWidth,
				.height = pWinSize.pHeight,
				.layers = 1
			};

			VkResult res = vkCreateFramebuffer(
				VK_Backend::Get()->GetDevice().GetDevice(), &CreateInfo,
				NULL, &pFramebuffers[i]
			);
			VK_CHECK("vkCreateFramebuffer", res);
		}
	}

	void VK_SwapChain::CreateDepthResources()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();
		uint32_t NumImages = GetImageCount();

		pDepthImages.resize(NumImages);

		auto pWinSize = lib_backend::GPU_LibBackend::GetInstance()->GetWindowSize();

		VkFormat DepthFormat = VK_Backend::Get()->GetDevice().GetSelectedDevice().m_depthFormat;

		for (uint32_t i = 0; i < NumImages; i++)
		{
			VkImageUsageFlagBits Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			CreateImage(
				pDepthImages[i], pWinSize.pWidth, pWinSize.pHeight, DepthFormat,
				Usage, PropertyFlags
			);

			VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout NewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			pDepthImages[i].pView = CreateImageView(
				pDepthImages[i].pImage, pDevice, DepthFormat,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
		}
	}

}