#include "VK_Shader.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "VK_wrappar.h"
#include "VK_Backend.h"

#ifdef ANDROID
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#endif

namespace GPU
{

	std::vector<uint32_t> ReadFile(const std::string& pPath)
	{
#ifdef ANDROID
		AAssetManager* mgr = reinterpret_cast<android_app*>(lib_backend::GPU_LibBackend::GetInstance()->GetHandle())->activity->assetManager;
		AAsset* pAsset = AAssetManager_open(mgr, pPath.c_str(), AASSET_MODE_BUFFER);
		size_t pSize = AAsset_getLength(pAsset);
		std::vector<uint32_t> buffer(pSize / 4);
		AAsset_read(pAsset, buffer.data(), pSize);
		AAsset_close(pAsset);

		return buffer;

#else
		std::ifstream file(pPath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			VK_LOG_ERROR("Shader: Failed to open file");
		}

		size_t pSize = file.tellg();
		std::vector<uint32_t> buffer(pSize / 4);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), pSize);
		file.close();

		return buffer;
#endif
	}

#ifdef GPU_ENABLE_RUNTIME_SHADING

	std::string ReadFileString(const std::string& pPath)
	{
		std::ifstream file(pPath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			VK_LOG_ERROR("Shader: Failed to open file");
		}

		size_t pSize = file.tellg();
		std::string buffer(pSize, '\0');

		file.seekg(0);
		file.read(buffer.data(), pSize);
		file.close();

		return buffer;
	}

	std::vector<uint32_t> CompileShader(
		const std::string& source,
		shaderc_shader_kind kind,
		const std::string& name
	)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		auto result = compiler.CompileGlslToSpv(
			source,
			kind,
			name.c_str(),
			options
		);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			VK_LOG_ERROR("Shader Error: {0}", result.GetErrorMessage());
		}
		
		return { result.cbegin(), result.cend() };
	}

	VkShaderModule CreateShaderModuleFromBinary(std::string pFileName, shaderc_shader_kind pKind)
	{
		// # Step 1: Read spv
		std::string pFinalPath = pFileName;
		std::string pSource = ReadFileString(pFinalPath);

		// # Step 2: Compile spv
		std::vector<uint32_t> spv = CompileShader(
			pSource,
			pKind,
			pFileName
		);
		
		// # Step 3: Create the shader
		VkShaderModuleCreateInfo CreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = size_t(spv.size() * sizeof(uint32_t)),
			.pCode = spv.data()
		};

		VkShaderModule pShaderModule = VK_NULL_HANDLE;
		VkResult res = vkCreateShaderModule(
			VK_Backend::Get()->GetDevice().GetDevice(), &CreateInfo, 
			NULL, &pShaderModule
		);
		VK_CHECK("vkCreateShaderModule", res);

		return pShaderModule;
	}

	VkShaderModule CreateShaderModuleDirctly(std::string pSource, shaderc_shader_kind pKind)
	{
		// # Step 1: Compile spv
		std::vector<uint32_t> spv = CompileShader(
			pSource,
			pKind,
			pSource
		);

		// # Step 2: Create the shader
		VkShaderModuleCreateInfo CreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = size_t(spv.size() * sizeof(uint32_t)),
			.pCode = spv.data()
		};

		VkShaderModule pShaderModule = VK_NULL_HANDLE;
		VkResult res = vkCreateShaderModule(
			VK_Backend::Get()->GetDevice().GetDevice(), &CreateInfo,
			NULL, &pShaderModule
		);
		VK_CHECK("vkCreateShaderModule", res);

		return pShaderModule;
	}

	void VK_Shader::InitFromFile(const char* pVertexFilename, const char* pFragmentFilename)
	{
		pVS = CreateShaderModuleFromBinary(pVertexFilename, shaderc_vertex_shader);
		pFS = CreateShaderModuleFromBinary(pFragmentFilename, shaderc_fragment_shader);
	}

	void VK_Shader::InitFromText(const char* pVertexShader, const char* pFragmentShader)
	{
		pVS = CreateShaderModuleDirctly(pVertexShader, shaderc_vertex_shader);
		pFS = CreateShaderModuleDirctly(pFragmentShader, shaderc_fragment_shader);
	}
#endif

	VkShaderModule CreateShaderFromSPIR_V(const std::vector<uint32_t>& Buffer)
	{
		VkShaderModuleCreateInfo CreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = size_t(Buffer.size() * sizeof(uint32_t)),
			.pCode = Buffer.data()
		};

		VkShaderModule Shader = VK_NULL_HANDLE;
		VkResult res = vkCreateShaderModule(
			VK_Backend::Get()->GetDevice().GetDevice(),
			&CreateInfo, NULL,
			&Shader
		);
		VK_CHECK("vkCreateShaderModule", res);

		return Shader;
	}

	void VK_Shader::InitFromSPIRvFile(const char* pVertexFilename, const char* pFragmentFilename)
	{
		pVS = CreateShaderFromSPIR_V(ReadFile(pVertexFilename));
		pFS = CreateShaderFromSPIR_V(ReadFile(pFragmentFilename));
	}

	void VK_Shader::InitSPIR_V(
		const std::vector<uint32_t>& pVertex,
		const std::vector<uint32_t>& pFragment
	)
	{
		pVS = CreateShaderFromSPIR_V(pVertex);
		pFS = CreateShaderFromSPIR_V(pFragment);
	}

	void VK_Shader::Destroy()
	{
		const VkDevice& pDevice = VK_Backend::Get()->GetDevice().GetDevice();

		vkDestroyShaderModule(pDevice, pVS, NULL);
		vkDestroyShaderModule(pDevice, pFS, NULL);
	}

}