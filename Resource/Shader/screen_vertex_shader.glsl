#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferInput{
    mat4 projection;
} inUniform;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inPosition;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 fUV;

void main() {
    fUV = inUV;
    gl_Position = inUniform.projection * vec4(inPosition.xy, 0.0, 1.0);
}