cmake_minimum_required(VERSION 3.26)

project("AnRHI")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# extern
set(EXTERN_DIR "${CMAKE_SOURCE_DIR}/extern")

if(MSVC) 
	add_compile_options("/utf-8")
endif()

# Vulkan
set(VULKAN_PATH "${EXTERN_DIR}/Vulkan")

# SDL3
list(APPEND CMAKE_PREFIX_PATH "${EXTERN_DIR}/SDL3")
find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3)

# ImGui
#file(GLOB_RECURSE IMGUI_SOURCES
#	CONFIGURE_DEPENDS
#		"${EXTERN_DIR}/ImGui/*.cpp"
#)

#add_library(ImGui_Core OBJECT ${IMGUI_SOURCES})

# AnRHI
file(GLOB_RECURSE ANRHI_SOURCES
	CONFIGURE_DEPENDS
		"${CMAKE_SOURCE_DIR}/GPU/*.cpp"
		"${EXTERN_DIR}/stb/stb_image.cpp"
)

add_library(AnRHI_Core OBJECT ${ANRHI_SOURCES})

target_include_directories(AnRHI_Core PUBLIC
	"${CMAKE_SOURCE_DIR}/GPU"
	"${VULKAN_PATH}/Include"
	"${EXTERN_DIR}/shaderc"
	"${EXTERN_DIR}/spdlog/include"
	"${EXTERN_DIR}/stb"

	# Window libraries
	"${EXTERN_DIR}/SDL3/include"
)

#target_link_libraries(AnRHI_Core PRIVATE
#	ImGui_Core
#)

target_compile_definitions(AnRHI_Core PUBLIC
	GPU_DEBUG=TRUE
	RES_PATH="${CMAKE_SOURCE_DIR}/res/"
)

# Doc executable

set(DOC_APPS "doc_1" "doc_2")

foreach (APP IN LISTS DOC_APPS)

add_executable(${APP} "${CMAKE_SOURCE_DIR}/doc/${APP}.cpp")

target_include_directories(${APP} PUBLIC
	"${EXTERN_DIR}/glm"
)

target_link_libraries(${APP} PRIVATE
	AnRHI_Core
	"${VULKAN_PATH}/Lib/vulkan-1.lib"
	SDL3::SDL3
	"${EXTERN_DIR}/shaderc/shaderc_shared.lib"
)

if(WIN32)

	add_custom_command(TARGET ${APP} POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${EXTERN_DIR}/SDL3/lib/x64/SDL3.dll"
							$<TARGET_FILE_DIR:${APP}>
	)

endif()

endforeach()