#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (binding = 0) uniform UBO 
{
	mat4 depthVP;
} ubo;

layout (push_constant) uniform Model{
	mat4 transform;
} model;

out gl_PerVertex {
    vec4 gl_Position;   
};

void main() {
	gl_Position = ubo.depthVP * model.transform * vec4(inPos, 1.0);
}