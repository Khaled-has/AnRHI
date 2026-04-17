#ifndef GPU_TEXTURE_H
#define GPU_TEXTURE_H

#include <iostream>

namespace RHI
{

	typedef enum GPU_Format
	{
		GPU_FORMAT_UNDEFINE    = 0,
		GPU_FORMAT_COLOR_BGRA8 = 1,
		GPU_FORMAT_COLOR_RGBA8 = 2,
		GPU_FORMAT_COLOR_RGB8  = 3,
		GPU_FORMAT_D32_FLOAT   = 4,
	} GPU_Format;

	typedef enum GPU_Aspect
	{
		GPU_ASPECT_COLOR_BIT = 0,
		GPU_ASPECT_DEPTH_BIT = 1
	} GPU_Aspect;

	typedef enum GPU_TextureState
	{
		GPU_TEXTURE_STATE_STATIC = 0,
		GPU_TEXTURE_STATE_DYNAMIC = 1
	} GPU_TextureState;

	typedef enum GPU_TextureType
	{
		GPU_TEXTURE_TYPE_2D = 0,
		GPU_TEXTURE_TYPE_CUBE_MAP = 1,
		GPU_TEXTURE_TYPE_ARRAY = 2
	} GPU_TextureType;

	typedef struct GPU_Size
	{
		uint32_t pWidth;
		uint32_t pHeight;
	} GPU_Size;

	typedef struct GPU_TextureInfo
	{
		GPU_TextureType pType;
		const void* pPixels;
		GPU_Size pSize;
		GPU_Format pFormat;
		GPU_Aspect pAspect;
		GPU_TextureState pState;
	} GPU_TextureInfo;

	class GPU_Texture
	{
	public:
		GPU_Texture() {}
		~GPU_Texture() {}

		virtual void Destroy() = 0;
	};

	GPU_Texture* CreateTexture(const GPU_TextureInfo& pInfo);

}

#endif