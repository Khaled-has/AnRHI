#ifndef GPU_DRAW_H
#define GPU_DRAW_H

#include <iostream>

#include "GPU_Backend.h"
#include "GPU_Buffer.h"
#include "GPU_Texture.h"

namespace RHI
{
	typedef enum GPU_DrawType
	{
		GPU_DRAW_TYPE_ARRAY    = 0,
		GPU_DRAW_TYPE_ELEMENTS = 1
	} GPU_DrawType;

	typedef enum GPU_BindingType
	{
		GPU_BINDING_TYPE_STATIC_BUFFER  = 0,
		GPU_BINDING_TYPE_DYNAMIC_BUFFER = 1,
		GPU_BINDING_TYPE_TEXTURE		= 2,
	} GPU_BindingType;

	typedef enum GPU_ShaderStage
	{
		GPU_SHADER_STAGE_VERTEX_BIT   = 0,
		GPU_SHADER_STAGE_FRAGMENT_BIT = 1
	} GPU_ShaderStage;

	typedef struct GPU_Binding
	{
		uint32_t pBinding;
		GPU_BindingType pBindType;
		GPU_ShaderStage pStage;
		const GPU_Buffer* pBuffer;
		const GPU_Texture* pTexture;
	} GPU_Binding;

	typedef struct GPU_DrawInfo
	{
		GPU_DrawType pDrawType;
		uint32_t pBindCount;
		const GPU_Binding* pBindings;
		GPU_RenderArea pRenderArea;
	} GPU_DrawInfo;

	class GPU_DrawCmd
	{
	public:
		GPU_DrawCmd() {}
		~GPU_DrawCmd() {}

		virtual void Destroy() = 0;

		virtual void Draw(uint32_t pFirstVertex, uint32_t pVertexCount) = 0;
	};

	GPU_DrawCmd* CreateDraw(const GPU_DrawInfo& pDrawInfo);
}

#endif