#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout (set = 1, binding = 1) uniform sampler2D albedo;

void main() 
{
    outColor = vec4(texture(albedo, uv).rgb, 1.0);
}