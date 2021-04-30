#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants {
    int isSelected;    
} pushConstants;

void main() {
    if (pushConstants.isSelected == 0) {
        outColor = vec4(fragColor, 1.0);
    } else {
        outColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}