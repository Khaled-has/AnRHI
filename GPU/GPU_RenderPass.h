#ifndef GPU_RENDERPASS_H
#define GPU_RENDERPASS_H

#include <iostream>
#include <vector>

#include "GPU_Texture.h"

namespace RHI
{

	typedef struct GPU_Clear
	{
		struct {
			float r, g, b, a;
		} pColor;

		struct {
			float depth;
			uint32_t stencil;
		} pDepth;

	} GPU_Clear;

	typedef struct GPU_RenderArea
	{
		struct {
			int32_t x, y;
		} pOffset;

		struct {
			uint32_t width, height;
		} pExtent;

	} GPU_RenderArea;

	typedef struct GPU_RenderPassInfo
	{
		bool pEnableColor;
		bool pEnableDepth;
		uint32_t pColorTexCount;
		const GPU_Texture* pColorTextures;
		const GPU_Texture* pDepthTexture;
		GPU_RenderArea pRenderArea;
	} GPU_RenderPassInfo;

	class GPU_RenderPass
	{
	public:
		GPU_RenderPass() {}
		~GPU_RenderPass() {}

		virtual void Destroy() = 0;

		virtual void Begin(GPU_Clear pClear) = 0;
		virtual void End() = 0;

	private:
		virtual void Create(const GPU_RenderPassInfo& pInfo) = 0;
	};

	GPU_RenderPass* CreateRenderPass(const GPU_RenderPassInfo& pInfo);

}

#endif