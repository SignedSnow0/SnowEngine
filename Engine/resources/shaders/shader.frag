#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//A structure has a base alignment equal to the largest base alignment of any of its members, rounded up to a multiple of 16.
layout(set = 0, binding = 1) uniform Camera {
    vec3 pos;
} camera;

struct PointLight{
    float constant;
    float linear;
    float quadratic;
    
	vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
layout(set = 0, binding = 2) uniform DirectionalLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
} dLight;
layout(set = 0, binding = 3) buffer PointLights {
    PointLight light[];
} pLights;
layout(set = 0, binding = 4) uniform SpotLight {
	float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;

	vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} sLight;

layout(set = 1, binding = 0) uniform ProcessingFlags {
	bool alphaBlending;
} flags;

layout(set = 1, binding = 1) uniform sampler2D albedo;
layout(set = 1, binding = 2) uniform sampler2D specularMap;
layout(set = 1, binding = 3) uniform sampler2D normal;

vec3 CalcDirLight(vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-dLight.direction);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    // combine
    vec3 ambient = dLight.ambient * vec3(texture(albedo, fragTexCoord));
    vec3 diffuse = dLight.diffuse * diff * vec3(texture(albedo, fragTexCoord));
    vec3 specular = dLight.specular * spec * vec3(texture(specularMap, fragTexCoord));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight pLight, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(pLight.position - fragPos);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    // attenuation
    float dist = length(pLight.position - fragPos);
    float attenuation = 1.0 / (pLight.constant + pLight.linear * dist + pLight.quadratic * (dist * dist));    

    vec3 ambient = pLight.ambient * vec3(texture(albedo, fragTexCoord));
    vec3 diffuse = pLight.diffuse * diff * vec3(texture(albedo, fragTexCoord));
    vec3 specular = pLight.specular * spec * vec3(texture(specularMap, fragTexCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(sLight.position - fragPos);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    // attenuation
    float dist = length(sLight.position - fragPos);
    float attenuation = 1.0 / (sLight.constant + sLight.linear * dist + sLight.quadratic * (dist * dist));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-sLight.direction));
    float epsilon = sLight.cutOff - sLight.outerCutOff;
    float intensity = clamp((theta - sLight.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = sLight.ambient * vec3(texture(albedo, fragTexCoord));
    vec3 diffuse = sLight.diffuse * diff * vec3(texture(albedo, fragTexCoord));
    vec3 specular = sLight.specular * spec * vec3(texture(specularMap, fragTexCoord));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

void main() {
	if(flags.alphaBlending && texture(albedo, fragTexCoord).a < 0.1)
		discard;

    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(camera.pos - fragPos);
    
	vec3 result = CalcDirLight(norm, viewDir);
    for(int i = 0; i < pLights.light.length(); i++)
        result += CalcPointLight(pLights.light[i], norm, fragPos, viewDir);    

    result += CalcSpotLight(norm, fragPos, viewDir);    

	outColor = vec4(result, 1.0);
}