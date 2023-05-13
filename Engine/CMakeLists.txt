add_library(Engine STATIC "Source/SnowEngine.h"
						  
						  "Source/Core/Types.h"
						  "Source/Core/Window.cpp"
						  "Source/Core/Window.h"

						  "Source/Graphics/Rhi/Buffers.cpp"
						  "Source/Graphics/Rhi/Buffers.h"
						  "Source/Graphics/Rhi/CommandBuffer.cpp"
						  "Source/Graphics/Rhi/CommandBuffer.h" 
						  "Source/Graphics/Rhi/Core.cpp"
						  "Source/Graphics/Rhi/Core.h"
						  "Source/Graphics/Rhi/DescriptorSet.cpp"
						  "Source/Graphics/Rhi/DescriptorSet.h" 
						  "Source/Graphics/Rhi/Gui.cpp"
						  "Source/Graphics/Rhi/Gui.h"
						  "Source/Graphics/Rhi/Image.cpp"
						  "Source/Graphics/Rhi/Image.h" 
						  "Source/Graphics/Rhi/Pipeline.cpp"
						  "Source/Graphics/Rhi/Pipeline.h"
						  "Source/Graphics/Rhi/RenderPass.cpp"
						  "Source/Graphics/Rhi/RenderPass.h"
						  "Source/Graphics/Rhi/Shader.cpp"
						  "Source/Graphics/Rhi/Shader.h" 
						  "Source/Graphics/Rhi/Surface.cpp" 
						  "Source/Graphics/Rhi/Surface.h"

						  "Source/Graphics/Vulkan/VkBuffers.cpp"
						  "Source/Graphics/Vulkan/VkBuffers.h" 
						  "Source/Graphics/Vulkan/VkCommandBuffer.cpp"
						  "Source/Graphics/Vulkan/VkCommandBuffer.h"
						  "Source/Graphics/Vulkan/VkCore.cpp" 
						  "Source/Graphics/Vulkan/VkCore.h"
						  "Source/Graphics/Vulkan/VkDescriptorSet.cpp"
						  "Source/Graphics/Vulkan/VkDescriptorSet.h"
						  "Source/Graphics/Vulkan/VkGui.cpp"
						  "Source/Graphics/Vulkan/VkGui.h" 
						  "Source/Graphics/Vulkan/VkImage.cpp"
						  "Source/Graphics/Vulkan/VkImage.h" 
						  "Source/Graphics/Vulkan/VkPipeline.cpp"
						  "Source/Graphics/Vulkan/VkPipeline.h"
						  "Source/Graphics/Vulkan/VkRenderPass.cpp" 
						  "Source/Graphics/Vulkan/VkRenderPass.h"
						  "Source/Graphics/Vulkan/VkShader.cpp" 
						  "Source/Graphics/Vulkan/VkShader.h" 
						  "Source/Graphics/Vulkan/VkSurface.cpp"
						  "Source/Graphics/Vulkan/VkSurface.h" 
						  "Source/Graphics/Vulkan/VkValidationLayer.cpp" 
						  "Source/Graphics/Vulkan/VkValidationLayer.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Engine PROPERTY CXX_STANDARD 20)
endif()

set(GLFW_BUILD_DOCS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("External/glfw/")
add_subdirectory("External/glm/")
add_subdirectory("External/imgui")

find_package(Vulkan REQUIRED FATAL_ERROR)

target_link_directories(Engine PUBLIC "$ENV{VULKAN_SDK}/Lib/")

target_link_libraries(Engine glfw
							 vulkan-1.lib
							 imgui)

target_include_directories(Engine PUBLIC "External/glfw/include/" 
								  PUBLIC "External/glm/"
								  PUBLIC "External/stb/"
								  PUBLIC "External/imgui/"
								  PUBLIC "$ENV{VULKAN_SDK}/Include/"
								  PRIVATE "Source/")