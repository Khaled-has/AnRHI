#ifndef GPU_BUFFER_H
#define GPU_BUFFER_H

#include <iostream>

namespace RHI
{
	typedef enum GPU_BufferType
	{
		GPU_BUFFER_TYPE_STATIC,
		GPU_BUFFER_TYPE_DYNAMIC,
		GPU_BUFFER_TYPE_DYNAMIC_RESIZEABLE
	} GPU_BufferType;

	class GPU_Buffer
	{
	public:
		GPU_Buffer() {}
		~GPU_Buffer() {}

		virtual void Create(const void* pData, size_t pSize, GPU_BufferType pBufferType) = 0;
		virtual void Destroy() = 0;

		virtual void Update(const void* pData, size_t pSize) = 0;
	};

	GPU_Buffer* CreateBuffer(const void* pData, size_t pSize, GPU_BufferType pBufType);

}

#endif