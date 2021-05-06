#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 fragtexCoord;
layout(location = 3) out vec4 fragShadowCoord;

//set order must be the same in pipeline layout declaration and cmdbindescriptorset
layout(set = 0, binding = 0) uniform Camera {
	mat4 view;
	mat4 proj;
} camera;

layout(set = 0, binding = 5) uniform Light {
	mat4 viewProj;
} light;

layout(push_constant) uniform Model{
	mat4 transform;
} model;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

void main() {
	fragPos = vec3(model.transform * vec4(inPos,1.0));
	fragtexCoord = inTexCoord;
	fragNormal = mat3(transpose(inverse(model.transform))) * inNormal;
	fragShadowCoord = (biasMat * light.viewProj * vec4(fragPos, 1.0));

	gl_Position = camera.proj * camera.view * model.transform * vec4(inPos, 1.0);	
}