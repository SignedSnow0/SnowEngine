%VULKAN_SDK%/bin/glslc.exe base_shader.vert -o spirv/base_shader.vert.spv
%VULKAN_SDK%/bin/glslc.exe base_shader.frag -o spirv/base_shader.frag.spv
%VULKAN_SDK%/bin/glslc.exe mapping_shader.frag -o spirv/mapping_shader.frag.spv
pause