#ifndef GPU_SHADER_H
#define GPU_SHADER_H

#include <iostream>

namespace RHI
{

	class GPU_Shader
	{
	public:
		GPU_Shader() {}
		~GPU_Shader() {}

		virtual void InitFromFile(const char* pVertexFilename, const char* pFragmentFilename) = 0;
		virtual void InitFromText(const char* pVertexShader, const char* pFragmentShader) = 0;

		virtual void Active() = 0;

		virtual void Destroy() = 0;
	};

	GPU_Shader* CreateShader();

}

#endif