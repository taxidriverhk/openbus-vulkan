#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferInput {
    mat4 model;
    mat4 view;
    mat4 projection;
} inUniform;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fColor;
layout(location = 1) out vec2 fUV;

void main() {
    gl_Position = inUniform.projection * inUniform.view * inUniform.model * vec4(inPosition, 0.0, 1.0);
    fColor = inColor;
	fUV = inUV;
}