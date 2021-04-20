#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 fragtexCoord;

//set order must be the same in pipeline layout declaration and cmdbindescriptorset
layout(set = 0, binding = 0) uniform Camera {
	mat4 view;
	mat4 proj;
} camera;

layout(push_constant) uniform Model{
	mat4 transform;
} model;

void main() {
	fragPos = vec3(model.transform * vec4(inVertex,1.0));
	fragtexCoord = inTexCoord;
	fragNormal = inNormal;

	gl_Position = camera.proj * camera.view * vec4(fragPos, 1.0);	
}