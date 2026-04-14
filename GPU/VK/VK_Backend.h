#ifndef VK_BACKEND_H
#define VK_BACKEND_H

#include "GPU_Backend.h"

#include "VK_Device.h"
#include "VK_SwapChain.h"
#include "VK_CommandBufferPool.h"
#include "VK_Queue.h"
#include "VK_Thread.h"
#include "VK_RenderPass.h"
#include "VK_Buffer.h"
#include "VK_GraphicsPipeline.h"
#include "VK_DrawCmd.h"

namespace GPU {

	class VK_Backend : public RHI::GPU_Backend
	{
	public:
		VK_Backend();
		~VK_Backend() {}

		virtual bool IsDeviceSupportBackend() override;

		virtual void Backend_Init() override;
		virtual void Backend_Exit() override;

#ifdef RHI_IMGUI_ENABLE
		virtual void ImGui_Init() override;
		virtual void ImGui_Exit() override;

		virtual void ImGui_Rendering() override;
#endif

		virtual void BeginRecord() override;
		virtual void EndRecord(const RHI::GPU_Texture* pFinalTexture) override;

		virtual void BeginRendering() override;
		virtual void EndRendering() override;

		/*	# The instance of Vulkan backend  */
		static VK_Backend* Get() { return pVkInstance; }

		/*	# The backend cores	 */
		inline const VK_Device& GetDevice() { return pDevice; }
		inline const VK_SwapChain& GetSwapChain() { return pSwapChain; }
		inline const VK_CommandBufferPool& GetCmdBufPool() const { return pCmdBufPool; }
		inline VK_Queue& GetQueue() { return pQueue; }
		inline const VK_Thread& GetLoadThread() { return pLoadThread; }

		inline const VkCommandBuffer& GetCmdBuf(uint32_t Index) { return pCmdBufs[Index]; }
		inline const VkCommandBuffer& GetCopyCmdBuf() const { return pCopyCmdBuf; }

		std::vector<std::function<void(VkCommandBuffer, uint32_t)>>* GetDrawCmdsArray();

		RHI::GPU_RenderPassInfo& GetCurrentRenderPassInfo() { return pCurrentRenderPassInfo; }
		inline VkRenderPass& GetCurrentRenderPass() { return pCurrentRenderPass; }
		inline VK_Shader& GetCurrentShader() { return pCurrentShader; }
	private:
		static VK_Backend* pVkInstance;

		VK_Device pDevice;
		VK_SwapChain pSwapChain;
		VK_CommandBufferPool pCmdBufPool;
		VK_Queue pQueue;
		VK_Thread pLoadThread;

		VkRenderPass pCurrentRenderPass = VK_NULL_HANDLE;
		VK_Shader pCurrentShader;
		RHI::GPU_RenderPassInfo pCurrentRenderPassInfo{};

		std::vector<VkCommandBuffer> pCmdBufs;
		VkCommandBuffer pCopyCmdBuf = VK_NULL_HANDLE;	// It's hard coded, I will use another thread later on.

		std::vector<std::function<void(VkCommandBuffer, uint32_t)>> pDrawCmdFuncs;

		// Screen image
		VK_Buffer pScreenImageBuffer;
		VK_Shader pScreenImageShader;
		VK_GraphicsPipeline pScreenImagePipeline;

		void CreateScreenImageResources(const VK_Texture* pFinalTexture);
	};

	inline VK_Backend* CreateVulkanBackend()
	{
		RHI::GPU_Backend::GetBackendType() = RHI::GPU_BACKEND_TYPES::GPU_BACKEND_VULKAN;
		return new VK_Backend;
	}

}

#endif