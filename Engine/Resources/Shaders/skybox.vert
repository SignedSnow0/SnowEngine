#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 outTexCoord;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 View;
    mat4 Projection;
} camera;

void main()
{
    gl_Position = camera.Projection * mat4(mat3(camera.View)) * vec4(inPosition, 1.0);
    outTexCoord = inPosition;
}