#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform Light{
	vec3 color;
	float intensity;
	vec3 pos;
	vec3 cameraPos;
	float specularStrength;
} light;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalMap;

void main() {
	//ambient
	vec3 ambient = light.color * light.intensity;
	//diffuse	
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(light.pos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light.color;
	//specular
	vec3 viewDir = normalize(light.cameraPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
	vec3 specular = light.specularStrength * spec * light.color;  

	outColor = vec4((ambient + diffuse + specular) * texture(normalMap, fragTexCoord).rgb, 1.0);
}