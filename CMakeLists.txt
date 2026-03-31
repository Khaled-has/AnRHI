cmake_minimum_required(VERSION 3.26)

project("AnRHI")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL3
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern/SDL3")
find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3)

# Vulkan
set(VULKAN_PATH "${CMAKE_SOURCE_DIR}/extern/Vulkan")

if(MSVC) 
	add_compile_options("/utf-8")
endif()

# AnRHI sources
file(GLOB ANRHI_CPP "${CMAKE_SOURCE_DIR}/GPU/*.cpp")
file(GLOB VK_BACKEND_CPP "${CMAKE_SOURCE_DIR}/GPU/VK/*.cpp")
file(GLOB BACKENDS_CPP "${CMAKE_SOURCE_DIR}/GPU/Backends/*.cpp")

# Executable
set(DOC_1 "doc_1")
set(DOC_2 "doc_2")

add_executable(${DOC_1})
add_executable(${DOC_2})

target_sources(${DOC_1} PRIVATE
 "${CMAKE_SOURCE_DIR}/doc/doc_1.cpp"

 "${ANRHI_CPP}" "${VK_BACKEND_CPP}" "${BACKENDS_CPP}"

 "${CMAKE_SOURCE_DIR}/Log/Log.cpp"

 "extern/stb/stb_image.cpp"
 "GPU/GPU_Shader.cpp"
)

target_sources(${DOC_2} PRIVATE
 "${CMAKE_SOURCE_DIR}/doc/doc_2.cpp"

 "${ANRHI_CPP}" "${VK_BACKEND_CPP}" "${BACKENDS_CPP}"

 "${CMAKE_SOURCE_DIR}/Log/Log.cpp"

 "extern/stb/stb_image.cpp"
 "GPU/GPU_Shader.cpp"
)

# Target include directories
target_include_directories(${DOC_1} PRIVATE
						"${CMAKE_SOURCE_DIR}"							# source
						"${VULKAN_PATH}/include"					    # VK
						"${CMAKE_SOURCE_DIR}/extern/shaderc"			# shaderc
						"${CMAKE_SOURCE_DIR}/extern/spdlog/include"     # Spdlog
						"${CMAKE_SOURCE_DIR}/extern/glm"				# glm
						"${CMAKE_SOURCE_DIR}/extern/stb"				# stb
)

target_include_directories(${DOC_2} PRIVATE
						"${CMAKE_SOURCE_DIR}"							# source
						"${VULKAN_PATH}/include"					    # VK
						"${CMAKE_SOURCE_DIR}/extern/shaderc"			# shaderc
						"${CMAKE_SOURCE_DIR}/extern/spdlog/include"     # Spdlog
						"${CMAKE_SOURCE_DIR}/extern/glm"				# glm
						"${CMAKE_SOURCE_DIR}/extern/stb"				# stb
)

# Target link libararise
target_link_libraries(${DOC_1} PRIVATE
						"${VULKAN_PATH}/Lib/vulkan-1.lib"
						SDL3::SDL3
						"${CMAKE_SOURCE_DIR}/extern/shaderc/shaderc_shared.lib"
)

target_link_libraries(${DOC_2} PRIVATE
						"${VULKAN_PATH}/Lib/vulkan-1.lib"
						SDL3::SDL3
						"${CMAKE_SOURCE_DIR}/extern/shaderc/shaderc_shared.lib"
)

# If Windows we will use SDL3
if(WIN32)

	add_custom_command(TARGET ${DOC_1} POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${CMAKE_SOURCE_DIR}/extern/SDL3/lib/x64/SDL3.dll"
							$<TARGET_FILE_DIR:${DOC_1}>
	)

		add_custom_command(TARGET ${DOC_2} POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${CMAKE_SOURCE_DIR}/extern/SDL3/lib/x64/SDL3.dll"
							$<TARGET_FILE_DIR:${DOC_2}>
	)

endif()

# Definons
add_compile_definitions(GPU_DEBUG=TRUE)
add_compile_definitions(RES_PATH="${CMAKE_SOURCE_DIR}/res/")