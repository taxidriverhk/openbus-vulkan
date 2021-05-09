#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferInput{
    float screenWidth;
    float screenHeight;
} inUniform;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inPosition;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 fUV;
layout(location = 1) out vec3 fColor;

void main() {
    fUV = inUV;
    fColor = inColor;
    gl_Position = vec4(
        inPosition.x / inUniform.screenWidth - 0.5,
        inPosition.y / inUniform.screenHeight - 0.5,
        0.0,
        1.0);
}