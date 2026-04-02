#ifndef VK_WRAPPAR_H
#define VK_WRAPPAR_H

#include <assert.h>
#include <array>
#include "VK_Backend.h"

#if __has_include("GPU_Log.h")
#include "GPU_Log.h"
#endif

#ifdef ANRHI_LOG_ENABLED
#define VK_CHECK(_des, _res) if (_res != VK_SUCCESS) GPU_LOG_ERROR("GPU ( VK Backend ) Error: {0}  \n\t   at line ( {1} )  \n\t   file ( {2} )", _des, __LINE__, __FILE__);

#define VK_LOG_INFO(...) GPU_LOG_INFO(""); GPU_LOG_INFO("GPU : VK_Backend # "); GPU_LOG_INFO(__VA_ARGS__);
#define VK_LOG_ERROR(...) GPU_LOG_ERROR(""); GPU_LOG_ERROR("GPU : VK_Backend # Error: "); GPU_LOG_ERROR(__VA_ARGS__);
#define VK_LOG_WARN(...) GPU_LOG_WARN(""); GPU_LOG_WARN("GPU : VK_Backend # Warning: "); GPU_LOG_WARN(__VA_ARGS__);
#define VK_LOG_TRACE(...) GPU_LOG_TRACE(""); GPU_LOG_TRACE("GPU : VK_Backend # Info: "); GPU_LOG_TRACE(__VA_ARGS__);
#else
#define VK_CHECK(_des, _res)

#define VK_LOG_INFO(...) 
#define VK_LOG_ERROR(...) 
#define VK_LOG_WARN(...) 
#define VK_LOG_TRACE(...)
#endif

namespace GPU
{
	uint32_t GetMemoryTypeIndex(uint32_t MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags);

	bool HasStencilComponent(VkFormat Format);

	void ImageMemBarrier(const VkCommandBuffer& CmdBuf, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount);

	void BeginDynamicRendering(const VkCommandBuffer& CmdBuf, uint32_t ImageIndex, VkClearValue* pClearColor, VkClearValue* pDepthValue, bool IsDepthTest);

	void BeginDynamicRendering(const VkCommandBuffer& CmdBuf, const VkImageView* pColorView, const VkImageView* pDepthView, uint32_t ImageIndex, VkClearValue* pClearColor, VkClearValue* pDepthValue, bool IsColorAttch, bool IsDepthTest);

	VK_BufferAndMemory CreateBuffer(size_t pSize, VkBufferUsageFlags pUsage, VkMemoryPropertyFlags pMemProp);

	void CopyBuffer(VkBuffer pDst, VkBuffer pSrc, VkDeviceSize pSize);

	VkImageView CreateImageView(const VkImage& Image, VkDevice Device, VkFormat Format, VkImageAspectFlags AspectFlags);

	VkSampler CreateTextureSampler(VkFilter MinFilter, VkFilter MaxFilter,
		VkSamplerAddressMode AddressMode);

	VkImageView CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, bool IsCubemap);

	void CopyBufferToImage(VkImage Dst, VkBuffer Src, uint32_t ImageWidth, uint32_t ImageHeight, VkDeviceSize LayerSize, int LayerCount);

	int GetBytesPerTexFormat(VkFormat Format);
}
#endif