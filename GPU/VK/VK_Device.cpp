#include "VK_Device.h"

#include "VK_wrappar.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "GPU/Backends/GPU_LibBackend.h"

namespace GPU
{

	static void PrintImageUsageFlags(const VkImageUsageFlags& flags)
	{

		if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		{
			VK_LOG_INFO("Image usage transfer src is supported");
		}

		if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			VK_LOG_INFO("Image usage transfer dest is supported");
		}

		if (flags & VK_IMAGE_USAGE_SAMPLED_BIT)
		{
			VK_LOG_INFO("Image usage sampled is supported");
		}

		if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			VK_LOG_INFO("Image usage color attachment is supported");
		}

		if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			VK_LOG_INFO("Image usage depth stencil attachment is supported");
		}

		if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
		{
			VK_LOG_INFO("Image usage transient attachment is supported");
		}

		if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
			VK_LOG_INFO("Image usage input attachment is supported");
		}
	}

	static const char* PrintMemoryProperty(VkMemoryPropertyFlags PropertyFlags)
	{
		if (PropertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			return "DEVICE LOCAL ";
		}

		if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			return "HOST VISIBLE ";
		}

		if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
			return "HOST COHERENT ";
		}

		if (PropertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
			return "HOST CACHED ";
		}

		if (PropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
			return "LAZILY ALLOCATED ";
		}

		if (PropertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
			return "PROTECTED ";
		}

		return "unknown";
	}

	static VkFormat FindSupportedFormat(VkPhysicalDevice Device, const std::vector<VkFormat>& Candidates,
		VkImageTiling Tiling, VkFormatFeatureFlags Features)
	{
		for (int i = 0; i < Candidates.size(); i++)
		{
			VkFormat Format = Candidates[i];
			VkFormatProperties Props;
			vkGetPhysicalDeviceFormatProperties(Device, Format, &Props);

			if ((Tiling == VK_IMAGE_TILING_LINEAR) &&
				(Props.linearTilingFeatures & Features) == Features)
			{
				return Format;
			}
			else if ((Tiling == VK_IMAGE_TILING_OPTIMAL) &&
				(Props.optimalTilingFeatures & Features) == Features)
			{
				return Format;
			}
		}

		VK_LOG_ERROR("Failed to find supported format!");
		exit(1);
	}

	static VkFormat FindDepthFormat(VkPhysicalDevice Device)
	{
		std::vector<VkFormat> Candidates = {
			VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		VkFormat DepthFormat = FindSupportedFormat(
			Device, Candidates, VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		VK_LOG_INFO("Device depth format: {0} {1} {2}",
			DepthFormat == VK_FORMAT_D32_SFLOAT ? "VK_FORMAT_D32_SFLOAT" : "",
			DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ? "VK_FORMAT_D32_SFLOAT_S8_UINT" : "",
			DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT ? "VK_FORMAT_D24_UNORM_S8_UINT" : ""
		);
		
		return DepthFormat;
	}

	const char* GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT Severity)
	{

		switch (Severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			return "Verbose";

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			return "Info";

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			return "Warning";

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			return "Error";

		default:
			VK_LOG_ERROR("Invalid severity code");
			exit(1);
		}

		return "NO SUCH SEVERITY!";
	}

	const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type)
	{

		switch (Type) {
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			return "General";

		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			return "Validation";

		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			return "Performance";

#ifdef _WIN64 // doesn't work on my Linux for some reason
		case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
			return "Device address binding";
#endif

		default:
			printf("Invalid type code %d\n", Type);
			exit(1);
		}

		return "NO SUCH TYPE!";
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
		VkDebugUtilsMessageTypeFlagsEXT Type,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::string pT = GetDebugSeverityStr(Severity);
		if (pT == "Error")
		{
			VK_LOG_ERROR("Debug callback: {0} \n		Severity {1}\n			Type {2}\n		   Object ",
				pCallbackData->pMessage, GetDebugSeverityStr(Severity), GetDebugType(Type)
			);
		}
		else
		{
			VK_LOG_TRACE("Debug callback: {0} \n		Severity {1}\n			Type {2}\n		   Object ",
				pCallbackData->pMessage, GetDebugSeverityStr(Severity), GetDebugType(Type)
			);
		}


		for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
#ifdef _WIN32
			if (pT == "Error")
			{
				VK_LOG_ERROR("{0}", pCallbackData->pObjects[i].objectHandle);
			}
			else
			{
				VK_LOG_TRACE("{0}", pCallbackData->pObjects[i].objectHandle);
			}
#else
			if (GetDebugSeverityStr(Severity) == "Error")
			{
				VK_LOG_ERROR("{0}", pCallbackData->pObjects[i].objectHandle);
			}
			else
			{
				VK_LOG_TRACE("{0}", pCallbackData->pObjects[i].objectHandle);
			}
#endif
		}

		return VK_FALSE;  // The calling function should not be aborted
	}

	bool VK_Device::IsExtSupported(const char* _Ext, uint32_t pPhyDeviceIndex) const
	{
		return m_pPhyDevices[pPhyDeviceIndex].IsExtensionSupported(_Ext);
	}

	bool VK_Device::IsExtSupported(const char* _Ext) const
	{
		return GetSelectedDevice().IsExtensionSupported(_Ext);
	}

	void VK_Device::Create()
	{
		CreateInstance();
#ifdef ANDROID
		// Dont't compile it for android
#else
		// Debug utils messenger
		CreateDebugCallMessenger();
#endif
		CreateSurface();
		CreatePhysicalDevice();
		CreateDevice();
	}

	void VK_Device::Destroy()
	{
		// # Destroy device
		vkDestroyDevice(pDevice, NULL);
		// # Destroy surface
		vkDestroySurfaceKHR(pInstance, pSurface, NULL);

		// # Destroy debug utils messenger
#ifdef WIN32
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (!vkDestroyDebugUtilsMessengerEXT)
		{
			VK_LOG_ERROR("Cannot find address of vkDestroyDebugUtilsMessengerEXT");
			exit(1);
		}
		vkDestroyDebugUtilsMessengerEXT(pInstance, pDebugMessenger, NULL);
#endif
		// # Destroy instance
		vkDestroyInstance(pInstance, NULL);
	}

	const VK_PhysicalDevice& VK_Device::GetPhysicalDevice(uint32_t Index)
	{
		return m_pPhyDevices[Index];
	}

	const VK_PhysicalDevice& VK_Device::GetSelectedDevice()
	{
		return m_pPhyDevices[pSelectedPhyDevIndex];
	}

	void VK_Device::CreateInstance()
	{
		// # Get the instance api version
		uint32_t InstanceVersion;
		VkResult res = vkEnumerateInstanceVersion(&InstanceVersion);
		VK_CHECK("vkEnumerateInstanceVersion", res);

		pInstanceVersion.Major = VK_API_VERSION_MAJOR(InstanceVersion);
		pInstanceVersion.Minor = VK_API_VERSION_MINOR(InstanceVersion);
		pInstanceVersion.Patch = VK_API_VERSION_PATCH(InstanceVersion);

		// # Create instance
		VkApplicationInfo AppInfo;
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.pNext = NULL;
		AppInfo.pApplicationName = "RHI Editor ( 3D )";
		AppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		AppInfo.pEngineName = "RHI Editor ( 3D )";
		AppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		AppInfo.apiVersion = VK_MAKE_API_VERSION(
			0, pInstanceVersion.Major, pInstanceVersion.Minor, pInstanceVersion.Patch
		);


		std::vector<const char*> Layers = VK_Get_Layers(VK_Layers);
		std::vector<const char*> Extensions = VK_Get_Extensions(VK_Extensions);
		for (int i = 0; i < Extensions.size(); i++) { VK_LOG_INFO("{0}", Extensions[i]); }
		VkInstanceCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		CreateInfo.pNext = NULL;
		CreateInfo.flags = 0;
		CreateInfo.pApplicationInfo = &AppInfo;
		CreateInfo.enabledLayerCount = (uint32_t)Layers.size();
		CreateInfo.ppEnabledLayerNames = Layers.data();
		CreateInfo.enabledExtensionCount = (uint32_t)Extensions.size();
		CreateInfo.ppEnabledExtensionNames = Extensions.data();

		res = vkCreateInstance(&CreateInfo, NULL, &pInstance);
		VK_CHECK("vkCreateInstance", res);
	}

	void VK_Device::CreateDebugCallMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.pNext = NULL,

			.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

			.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

			.pfnUserCallback = &DebugCallback,
			.pUserData = NULL
		};

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
		vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkCreateDebugUtilsMessengerEXT");
		if (!vkCreateDebugUtilsMessenger)
		{
			VK_LOG_ERROR("Cannot find address of vkCreateDebugUtilsMessenger");
			exit(1);
		}

		VkResult res = vkCreateDebugUtilsMessenger(pInstance, &MessengerCreateInfo, NULL, &pDebugMessenger);
		VK_CHECK("vkCreateDebugUtilsMessenger", res);
	}

	void VK_Device::CreateSurface()
	{
		lib_backend::GPU_LibBackend::GetInstance()->CreateSurfaceForVulkan(&pSurface);
	
#ifdef WIN32
		VK_LOG_INFO("Windows surface created");
#elif  __linux__
		VK_LOG_INFO("linux surface created");
#endif
	}

	void VK_Device::CreatePhysicalDevice()
	{
		/*	 # Get the physical devices on this device	 */
		VkResult res = vkEnumeratePhysicalDevices(pInstance, &pPhyDevCount, NULL);
		VK_CHECK("vkEnumeratePhysicalDevices", res);

		VK_LOG_INFO("Physical devices count: {0}", pPhyDevCount);

		m_pPhyDevices.resize(pPhyDevCount);

		std::vector<VkPhysicalDevice> Devices(pPhyDevCount);

		res = vkEnumeratePhysicalDevices(pInstance, &pPhyDevCount, Devices.data());
		VK_CHECK("vkEnumeratePhysicalDevices", res);

		for (uint32_t i = 0; i < pPhyDevCount; i++)
		{
			VkPhysicalDevice PhyDev = Devices[i];

			m_pPhyDevices[i].pPhysDevice = PhyDev;

			// # Device properties
			vkGetPhysicalDeviceProperties(PhyDev, &m_pPhyDevices[i].m_devProps);

			VK_LOG_INFO("Device name: {0} | Device type: {1}", m_pPhyDevices[i].m_devProps.deviceName, GetDeviceType(m_pPhyDevices[i].m_devProps.deviceType));

			// # Device API version
			uint32_t apiV = m_pPhyDevices[i].m_devProps.apiVersion;
			m_pPhyDevices[i].m_apiVersion.Major = VK_API_VERSION_MAJOR(apiV);
			m_pPhyDevices[i].m_apiVersion.Minor = VK_API_VERSION_MINOR(apiV);
			m_pPhyDevices[i].m_apiVersion.Patch = VK_API_VERSION_PATCH(apiV);

			VK_LOG_INFO("Api Version Supported: {0}.{1}.{2}", m_pPhyDevices[i].m_apiVersion.Major, m_pPhyDevices[i].m_apiVersion.Minor, m_pPhyDevices[i].m_apiVersion.Patch);

			// # Device extensions
			uint32_t ExtensionsCount;
			vkEnumerateDeviceExtensionProperties(m_pPhyDevices[i].pPhysDevice, NULL, &ExtensionsCount, NULL);

			m_pPhyDevices[i].m_extensions.resize(ExtensionsCount);

			vkEnumerateDeviceExtensionProperties(m_pPhyDevices[i].pPhysDevice, NULL, &ExtensionsCount, m_pPhyDevices[i].m_extensions.data());

			VK_LOG_INFO("Physical device extensions");
			/*for (const VkExtensionProperties& e : m_pPhyDevices[i].m_extensions)
			{
				VK_LOG_INFO("	{0}", e.extensionName);
			}*/

			// # Device Queue families properties
			uint32_t NumQFamilies = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(PhyDev, &NumQFamilies, NULL);
			VK_LOG_INFO("	Num of queue family: {0}", NumQFamilies);

			m_pPhyDevices[i].m_qFamilyProps.resize(NumQFamilies);
			m_pPhyDevices[i].m_qSupportsPresent.resize(NumQFamilies);

			vkGetPhysicalDeviceQueueFamilyProperties(PhyDev, &NumQFamilies, m_pPhyDevices[i].m_qFamilyProps.data());

			for (uint32_t q = 0; q < NumQFamilies; q++)
			{
				const VkQueueFamilyProperties& QFamilyProp = m_pPhyDevices[i].m_qFamilyProps[q];

				VK_LOG_INFO("	Family {0} Num queues: {1} ", q, QFamilyProp.queueCount);
				VkQueueFlags Flags = QFamilyProp.queueFlags;
				VK_LOG_INFO("	GFX {0}, Compute {1}, Transfer {2}, Sparse binding {3}",
					(Flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
					(Flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
					(Flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
					(Flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No"
				);

				res = vkGetPhysicalDeviceSurfaceSupportKHR(PhyDev, q, pSurface, &(m_pPhyDevices[i].m_qSupportsPresent[q]));
				VK_CHECK("vkGetPhysicalDeviceSurfaceSupportKHR", res);

				VK_LOG_INFO("	 This Queue Support Present: {0}", m_pPhyDevices[i].m_qSupportsPresent[q] ? "Yes" : "No");
			}

			// # Device surface formats
			uint32_t NumFormats = 0;
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhyDev, pSurface, &NumFormats, NULL);
			VK_CHECK("vkGetPhysicalDeviceSurfaceFormatsKHR ( 1 )", res);
			assert(NumFormats > 0);

			m_pPhyDevices[i].m_surfaceFormats.resize(NumFormats);

			res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhyDev, pSurface, &NumFormats, m_pPhyDevices[i].m_surfaceFormats.data());
			VK_CHECK("vkGetPhysicalDeviceSurfaceFormatsKHR ( 2 )", res);

			/*for (int j = 0; j < NumFormats; j++)
			{
				const VkSurfaceFormatKHR& SurfaceFormat = m_pPhyDevices[i].m_surfaceFormats[j];
				VK_LOG_INFO("	 Format {0} color space {1}", SurfaceFormat.format, SurfaceFormat.colorSpace);
			}*/

			// # Device surface capabilities
			res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhyDev, pSurface, &(m_pPhyDevices[i].m_surfaceCaps));
			VK_CHECK("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", res);

			PrintImageUsageFlags(m_pPhyDevices[i].m_surfaceCaps.supportedUsageFlags);

			// # Device present modes
			uint32_t NumPresentModes = 0;

			res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhyDev, pSurface, &NumPresentModes, NULL);
			VK_CHECK("vkGetPhysicalDeviceSurfacePresentModesKHR ( 1 )", res);
			assert(NumPresentModes != 0);

			m_pPhyDevices[i].m_presentModes.resize(NumPresentModes);

			res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhyDev, pSurface, &NumPresentModes, m_pPhyDevices[i].m_presentModes.data());
			VK_CHECK("vkGetPhysicalDeviceSurfacePresentModesKHR ( 2 )", res);

			VK_LOG_INFO("Number of presentation modes {0}", NumPresentModes);

			// # Device memory properties
			vkGetPhysicalDeviceMemoryProperties(PhyDev, &(m_pPhyDevices[i].m_memProps));

			VK_LOG_INFO("Num memory types {0}", m_pPhyDevices[i].m_memProps.memoryTypeCount);
			for (uint32_t j = 0; j < m_pPhyDevices[i].m_memProps.memoryTypeCount; j++)
			{
				VK_LOG_INFO("{0}: flags {1} heap {2} ", j,
					PrintMemoryProperty(m_pPhyDevices[i].m_memProps.memoryTypes[j].propertyFlags),
					m_pPhyDevices[i].m_memProps.memoryTypes[j].heapIndex
					);
			}

			// # Device features
			vkGetPhysicalDeviceFeatures(PhyDev, &m_pPhyDevices[i].m_features);

			// # Device depth format
			m_pPhyDevices[i].m_depthFormat = FindDepthFormat(PhyDev);
		}

		// # Select physical device
		for (uint32_t i = 0; i < m_pPhyDevices.size(); i++)
		{
			for (uint32_t j = 0; j < m_pPhyDevices[i].m_qFamilyProps.size(); j++)
			{
				const VkQueueFamilyProperties& QFamilyProp = m_pPhyDevices[i].m_qFamilyProps[j];

				if ((QFamilyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT) && ((bool)m_pPhyDevices[i].m_qSupportsPresent[j] == VK_TRUE))
				{
					pSelectedPhyDevIndex = i;
					pSelectedPhyDevQueueIndex = j;
					VK_LOG_INFO(
						"Using GFX device {0} and queue family {1}", 
						pSelectedPhyDevIndex, pSelectedPhyDevQueueIndex
					);
				}
			}
		}
	}

	void VK_Device::CreateDevice()
	{
		float pPriorites[] = { 1.0f };

		VkDeviceQueueCreateInfo qInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0, // must be zero
			.queueFamilyIndex = pSelectedPhyDevQueueIndex,
			.queueCount = 1,
			.pQueuePriorities = &pPriorites[0]
		};

		std::vector<const char*> DevExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
		};

		// # Device extensions support
		bool DeviceSupportsDynamicRendering = GetSelectedDevice().IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

		bool Instance_is_1_3_or_more = (pInstanceVersion.Major > 1) || (pInstanceVersion.Minor >= 3);

		if (Instance_is_1_3_or_more && DeviceSupportsDynamicRendering)
		{
			VK_LOG_INFO("The Vulkan instance and device support dynamic rendering as a core feature");
			m_pPhyDevices[pSelectedPhyDevIndex].pIsDynamicSupported = true;
		}
		else if (pInstanceVersion.Minor == 2)
		{
			if (DeviceSupportsDynamicRendering)
			{
				DevExts.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
				m_pPhyDevices[pSelectedPhyDevIndex].pIsDynamicSupported = true;
			}
			else
			{
				VK_LOG_WARN("The system doesn't support dynamic rendering");
				m_pPhyDevices[pSelectedPhyDevIndex].pIsDynamicSupported = false;
			}
		}
		else {
			VK_LOG_WARN("The system doesn't support dynamic rendering");
			m_pPhyDevices[pSelectedPhyDevIndex].pIsDynamicSupported = false;
		}

		//m_pPhyDevices[pSelectedPhyDevIndex].pIsDynamicSupported = false; // # I change between dynamic and render pass for test

		if (GetSelectedDevice().m_features.geometryShader == VK_FALSE)
		{
			VK_LOG_WARN("The Geometry shader is not supported!");
		}
		if (GetSelectedDevice().m_features.tessellationShader == VK_FALSE)
		{
			VK_LOG_WARN("The Tessellation shader is not supported!");
		}

		VkPhysicalDeviceFeatures DeviceFeature = { 0 };
		DeviceFeature.geometryShader = VK_TRUE;
		DeviceFeature.tessellationShader = VK_TRUE;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR DynamicRenderingFeature = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			.pNext = NULL,
			.dynamicRendering = VK_TRUE
		};

		VkDeviceCreateInfo DeviceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &DynamicRenderingFeature,
			.flags = 0,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &qInfo,
			.enabledLayerCount = 0,			// DEPRECATED
			.ppEnabledLayerNames = NULL,	// DEPRECATED
			.enabledExtensionCount = (uint32_t)DevExts.size(),
			.ppEnabledExtensionNames = DevExts.data(),
			.pEnabledFeatures = &DeviceFeature
		};

		VkResult res = vkCreateDevice(GetSelectedDevice().pPhysDevice, &DeviceCreateInfo, NULL, &pDevice);
		VK_CHECK("vkCreateDevice", res);
	}
}