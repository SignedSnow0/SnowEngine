#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 fragTexCoords;

layout (binding = 0) uniform Camera{
	mat4 view;
	mat4 proj;
} camera;

void main() {
	fragTexCoords = inPos;
	gl_Position = camera.proj * camera.view * vec4(inPos, 1.0);
}