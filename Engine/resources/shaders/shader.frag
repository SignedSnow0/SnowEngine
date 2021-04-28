#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GLSL_EXT_debug_printf : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//A structure has a base alignment equal to the largest base alignment of any of its members, rounded up to a multiple of 16.
layout(set = 0, binding = 1) uniform Light{
	float intensity;
	float specularStrength;
	vec3 color;
	vec3 pos;
	vec3 cameraPos;
} light;

layout(set = 1, binding = 0) uniform ProcessingFlags {
	bool alphaBlending;
} flags;

layout(set = 1, binding = 1) uniform sampler2D albedo;
layout(set = 1, binding = 2) uniform sampler2D diffuse;
layout(set = 1, binding = 3) uniform sampler2D normal;

vec3 CalcPhongLight() {
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

	return ambient + diffuse + specular;
}

void main() {
	if(flags.alphaBlending && texture(albedo, fragTexCoord).a < 0.1)
		discard;

	vec3 phong = CalcPhongLight();

	outColor = vec4(phong * texture(albedo, fragTexCoord).rgb, light.specularStrength);
}