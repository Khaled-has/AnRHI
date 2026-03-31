#ifndef VK_SHADER_H
#define VK_SHADER_H

#include <iostream>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "GPU/GPU_Shader.h"

namespace GPU
{

	class VK_Shader : public RHI::GPU_Shader
	{
	public:
		VK_Shader() {}
		~VK_Shader() {}

		virtual void InitFromFile(const char* pVertexFilename, const char* pFragmentFilename) override;
		virtual void InitFromText(const char* pVertexShader, const char* pFragmentShader) override;

		virtual void Active() override;

		virtual void Destroy() override;

		inline const VkShaderModule& GetVertexShader() const { return pVS; }
		inline const VkShaderModule& GetFragmentShader() const { return pFS; }

	private:
		VkShaderModule pVS = VK_NULL_HANDLE;
		VkShaderModule pFS = VK_NULL_HANDLE;

		void CreateVertexShader(const char* pVertex);
		void CreateFragmentShader(const char* pFragment);
	};

}

#endif