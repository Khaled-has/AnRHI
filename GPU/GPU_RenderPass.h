#ifndef GPU_RENDERPASS_H
#define GPU_RENDERPASS_H

#include <iostream>
#include <vector>

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

    typedef enum GPU_Format
	{
		GPU_FORMAT_UNDEFINE = 0,
		GPU_FORMAT_COLOR_RGBA8 = 1,
		GPU_FORMAT_COLOR_BGRA8 = 2,
		GPU_FORMAT_D32_FLOAT = 3,
	} GPU_Format;

	typedef struct GPU_RenderPassInfo
	{
		bool pEnableColor;
		bool pEnableDepth;
		std::vector<GPU_Format> pColorFormats;
		GPU_Format pDepthFormat;
		uint32_t pWidth;
		uint32_t pHeight;
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
		virtual void Create(const GPU_RenderPassInfo pInfo) = 0;
	};

	GPU_RenderPass* CreateRenderPass(GPU_RenderPassInfo pInfo);

}

#endif