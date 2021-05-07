%VULKAN_SDK%/bin/glslc.exe shader.vert -o spirv/shader.vert.spv
%VULKAN_SDK%/bin/glslc.exe shader.frag -o spirv/shader.frag.spv
%VULKAN_SDK%/bin/glslc.exe shadowMapping.vert -o spirv/shadowMapping.vert.spv
%VULKAN_SDK%/bin/glslc.exe skybox.vert -o spirv/skybox.vert.spv
%VULKAN_SDK%/bin/glslc.exe skybox.frag -o spirv/skybox.frag.spv
pause