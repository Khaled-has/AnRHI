#ifndef GPU_TEXTURE_H
#define GPU_TEXTURE_H

namespace RHI
{

	class GPU_Texture
	{
	public:
		GPU_Texture() {}
		~GPU_Texture() {}

		virtual void Create(const char* pFilename) = 0;
		virtual void Read(const void* pPixels, int w, int h) = 0;
		virtual void Destroy() = 0;
	};

	GPU_Texture* CreateTexture(const char* pFilename);
	GPU_Texture* CreateTexture(const void* pPixels, int w, int h);

}

#endif