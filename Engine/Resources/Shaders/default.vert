#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 uv;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 View;
    mat4 Projection;
} camera;

layout (set = 1, binding = 0) uniform Transform
{
    mat4 Transform;
} transform;

void main() {
    gl_Position = camera.Projection * camera.View * transform.Transform * vec4(position, 1.0);
    fragColor = color;
    uv = inUV;
}