#ifndef GPU_DRAW_H
#define GPU_DRAW_H

#include <iostream>

#include "GPU_Backend.h"
#include "GPU_Buffer.h"
#include "GPU_Texture.h"

namespace RHI
{

	class GPU_Draw
	{
	public:
		GPU_Draw() {}
		~GPU_Draw() {}

		virtual void SetBuffer(const GPU_Buffer* pBuffer, GPU_BufferType pBufType, uint32_t pBindIndex) = 0;
		virtual void SetTexture(const GPU_Texture* pTexture, uint32_t pBindIndex) = 0;

		virtual void InitBindings() = 0;
		virtual void Destroy() = 0;

		virtual void Draw(uint32_t pFirstVertex, uint32_t pVertexCount) = 0;
	};

	GPU_Draw* CreateDraw();
}

#endif