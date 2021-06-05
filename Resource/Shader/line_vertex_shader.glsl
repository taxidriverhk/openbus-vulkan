#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferInput{
    mat4 view;
    mat4 projection;
    vec3 eyePosition;
    vec3 lightPosition;
} inUniform;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPosition;

layout(location = 0) out vec3 fColor;

void main() {
    fColor = inColor;
    gl_Position = inUniform.projection * inUniform.view * vec4(inPosition, 1.0);
}
