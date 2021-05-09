#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D inSampler;

layout(location = 0) in vec2 fUV;
layout(location = 1) in vec3 fColor;

layout(location = 0) out vec4 outColor;

void main() {
    float textSample = texture(inSampler, fUV).g;
    if (textSample < 1)
        discard;

    outColor = vec4(fColor, textSample);
}