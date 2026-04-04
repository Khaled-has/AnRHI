#include "GPU_Android.h"

#ifdef ANDROID

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include "VK/VK_Backend.h"

namespace lib_backend {

    void GPU_Android::Init(void* pApp) 
    {
        this->pApp = pApp;
    }

    GPU_WinSize GPU_Android::GetWindowSize() {
        ANativeWindow* pWin = static_cast<ANativeWindow*>(pApp);
        GPU_WinSize WinSize = {
            .pWidth = (uint32_t)ANativeWindow_getWidth(pWin),
            .pHeight = (uint32_t)ANativeWindow_getHeight(pWin)
        };

        return WinSize;
    }

    void* GPU_Android::GetHandle()
    {
        return pApp;
    }

    void GPU_Android::CreateSurfaceForVulkan(void* pVkSurface) 
    {
        VkAndroidSurfaceCreateInfoKHR CreateInfo = {
                .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                .pNext = NULL,
                .flags = 0,
                .window = static_cast<ANativeWindow*>(pApp)
        };

        VkSurfaceKHR* pSurface = static_cast<VkSurfaceKHR*>(pVkSurface);
        vkCreateAndroidSurfaceKHR(
            GPU::VK_Backend::Get()->GetDevice().GetInstance(),
            &CreateInfo,
            NULL,
            pSurface
        );
    }

    void GPU_Android::CreateSurfaceForDirectX12() {

    }
}

#endif