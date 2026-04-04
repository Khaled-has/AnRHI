#ifndef GPU_SHADER_H
#define GPU_SHADER_H

#include <iostream>
#include <vector>

#if __has_include(<shaderc.hpp>)
#include <shaderc.hpp>
#define GPU_ENABLE_RUNTIME_SHADING

#elif (__has_include(<shaderc/shaderc.hpp>)
#include <shaderc/shaderc.hpp>
#define GPU_ENABLE_RUNTIME_SHADING

#endif

namespace RHI
{

	class GPU_Shader
	{
	public:
		GPU_Shader() {}
		~GPU_Shader() {}

		virtual void InitFromSPIRvFile(const char* pVertexFilename, const char* pFragmentFilename) = 0;
		virtual void InitSPIR_V(
			const std::vector<uint32_t>& pVertex,
			const std::vector<uint32_t>& pFragment
		) = 0;
#ifdef GPU_ENABLE_RUNTIME_SHADING
		virtual void InitFromFile(const char* pVertexFilename, const char* pFragmentFilename) = 0;
		virtual void InitFromText(const char* pVertexShader, const char* pFragmentShader) = 0;
#endif

		virtual void Active() = 0;

		virtual void Destroy() = 0;
	};

	GPU_Shader* CreateShader();

}

#endif