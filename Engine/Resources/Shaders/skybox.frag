#version 450

layout (location = 0) in vec3 inTexCoord;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 1) uniform samplerCube skybox;

void main()
{    
    outFragColor = texture(skybox, inTexCoord);
}