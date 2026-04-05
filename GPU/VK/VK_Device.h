#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <stdio.h>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#define BIN(_N) (1 << _N)

namespace GPU {

	/*	 # Physical devises types	*/
	enum PhyDeviceType
	{
		NONE		= 0,
		INTEL_HD	= BIN(1),
		INTEL_UHD	= BIN(2),
		NIVDIA		= BIN(3),
		AMD			= BIN(4)
	};

	inline std::string GetDeviceType(VkPhysicalDeviceType pType)
	{
		std::string result = "";

		if ((pType & VK_PHYSICAL_DEVICE_TYPE_OTHER) == VK_PHYSICAL_DEVICE_TYPE_OTHER)
			result += "Other";

		if ((pType & VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			result += "Integrated GPU";

		if ((pType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			result += "Discrete GPU";

		if ((pType & VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
			result += "Virtual GPU";

		if ((pType & VK_PHYSICAL_DEVICE_TYPE_CPU) == VK_PHYSICAL_DEVICE_TYPE_CPU)
			result += "CPU";

		if ((pType & VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM) == VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM)
			result += "Max Enum";

		return result;
	}

	/*	 # Vulkan extensions	*/
	enum VKExtensions
	{
		VK_SURFACE_EXT_NAME		= BIN(5),
		VK_WIN32_SURFACE_EXT	= BIN(6),
		VK_LINUX_SURFACE_EXT	= BIN(7),
		VK_ANDROID_SURFACE_EXT  = BIN(8),
		VK_DEBUG_UTILS_EXT		= BIN(9)
	};

	//	 # Hard coded extensions used
	inline unsigned int VK_Extensions =
		VK_SURFACE_EXT_NAME |
#ifdef WIN32
		VK_WIN32_SURFACE_EXT | VK_DEBUG_UTILS_EXT;
#elif ANDROID
		VK_ANDROID_SURFACE_EXT;
#elif __linux__
		VK_LINUX_SURFACE_EXT | VK_DEBUG_UTILS_EXT;
#endif


	inline const std::vector<const char*> VK_Get_Extensions(unsigned int _ext)
	{
		std::vector<const char*> EXT;
		
		if ((_ext & VK_SURFACE_EXT_NAME) == VK_SURFACE_EXT_NAME)
			EXT.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

		if ((_ext & VK_WIN32_SURFACE_EXT) == VK_WIN32_SURFACE_EXT)
			EXT.push_back("VK_KHR_win32_surface");

		if((_ext & VK_LINUX_SURFACE_EXT) == VK_LINUX_SURFACE_EXT)
			EXT.push_back("VK_KHR_xcb_surface");

		if ((_ext & VK_ANDROID_SURFACE_EXT) == VK_ANDROID_SURFACE_EXT)
			EXT.push_back("VK_KHR_android_surface");

		if ((_ext & VK_DEBUG_UTILS_EXT) == VK_DEBUG_UTILS_EXT)
			EXT.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return EXT;
	}
	
	/*	 # Vulkan layers	*/
	enum VKLayers
	{
		VK_VALIDATION_LAYER		= BIN(10)
	};

	//	 # Hard coded layers used
#if !defined(ANDROID)
	inline unsigned int VK_Layers = VK_VALIDATION_LAYER;
#else
	inline unsigned int VK_Layers;
#endif

	inline const std::vector<const char*> VK_Get_Layers(unsigned int _lyr)
	{
		std::vector<const char*> LYR;

#ifdef _ANDROID
#else
		if ((_lyr & VK_VALIDATION_LAYER) == VK_VALIDATION_LAYER)
			LYR.push_back("VK_LAYER_KHRONOS_validation");
#endif
		return LYR;
	}


	/*	 # Physical device	 */
	struct VK_PhysicalDevice
	{
		VkPhysicalDevice pPhysDevice;
		VkPhysicalDeviceProperties m_devProps;
		std::vector<VkQueueFamilyProperties> m_qFamilyProps;
		std::vector<VkBool32> m_qSupportsPresent;
		std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
		VkSurfaceCapabilitiesKHR m_surfaceCaps;
		VkPhysicalDeviceMemoryProperties m_memProps;
		std::vector<VkPresentModeKHR> m_presentModes;
		VkPhysicalDeviceFeatures m_features;
		VkFormat m_depthFormat;
		bool pIsDynamicSupported;
		struct {
			int Variant = 0;
			int Major = 0;
			int Minor = 0;
			int Patch = 0;
		} m_apiVersion;
		std::vector<VkExtensionProperties> m_extensions;

		inline bool IsExtensionSupported(const char* pExt) const
		{
			std::string Ext = pExt;

			for (uint32_t i = 0; i < m_extensions.size(); i++)
			{
				std::string e = m_extensions[i].extensionName;
				if (Ext == e)
				{
					return true;
				}
			}
			return false;
		}
	};

	/*	 # The Device it self	*/
	class VK_Device
	{
	public:
		VK_Device() {}
		~VK_Device() {}

		// # Get device
		inline const VkInstance& GetInstance() const { return pInstance; }
		inline const VkSurfaceKHR& GetSurface() const { return pSurface; }
		inline const VK_PhysicalDevice& GetSelectedDevice() const { return m_pPhyDevices[pSelectedPhyDevIndex]; }
		inline const VK_PhysicalDevice& GetPhysicalDevice(uint32_t Index) const { return m_pPhyDevices[Index]; }
		inline const VkDevice& GetDevice() const { return pDevice; }

		inline uint32_t GetSelectedQueue() const { return pSelectedPhyDevQueueIndex; }
		inline uint32_t GetSelectedQFamily() const { return pSelectedPhyDevQueueFamilyIndex; }

		// # Device checks
		bool IsExtSupported(const char* _Ext, uint32_t pPhyDeviceIndex) const;
		bool IsExtSupported(const char* _Ext) const;

		// # Get the number of physical devices
		inline uint32_t GetPhyDevCount() const { return pPhyDevCount; }

		// #  Init & Destroy Vulkan context ( Instance - Surface - PhysicalDev - Device )
		void Create();
		void Destroy();

	private:
		VkInstance pInstance					 = VK_NULL_HANDLE;
		VkSurfaceKHR pSurface					 = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT pDebugMessenger = VK_NULL_HANDLE;
		VkDevice pDevice						 = VK_NULL_HANDLE;

		struct {
			int Major = 0;
			int Minor = 0;
			int Patch = 0;
		} pInstanceVersion;

		/*	# Physical devices  */
		uint32_t pPhyDevCount					 = 0;
		uint32_t pSelectedPhyDevIndex			 = 0;
		uint32_t pSelectedPhyDevQueueIndex	     = 0;
		uint32_t pSelectedPhyDevQueueFamilyIndex = 0;

		std::vector<VK_PhysicalDevice> m_pPhyDevices;

		const VK_PhysicalDevice& GetPhysicalDevice(uint32_t Index);
		const VK_PhysicalDevice& GetSelectedDevice();

		/* # Init functions */
		void CreateInstance();
		void CreateDebugCallMessenger();
		void CreateSurface();
		void CreatePhysicalDevice();
		void CreateDevice();
	};

}

#endif