#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D inSampler;

layout(location = 0) in vec3 fNormal;
layout(location = 1) in vec2 fUV;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 objectColor = texture(inSampler, fUV);
	if (objectColor.a < 1.0)
		discard;

	outColor = objectColor;
}