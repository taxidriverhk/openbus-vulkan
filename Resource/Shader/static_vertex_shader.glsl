#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferInput{
    mat4 view;
    mat4 projection;
    vec3 eyePosition;
    vec3 lightPosition;
} inUniform;
layout(set = 2, binding = 0) uniform InstanceBufferInput{
    mat4 transformation;
} inInstance;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 fUV;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out vec4 fWorldPosition;
layout(location = 3) out vec4 fEyePosition;
layout(location = 4) out vec4 fLightPosition;
layout(location = 5) out float fVisibility;
layout(location = 6) out vec3 fFogColor;

layout(push_constant) uniform MeshPushContant{
    float fogDensity;
    float fogGradient;
    vec3 fogColor;
} inMeshPushConstant;

void main() {
    fUV = inUV;
    fNormal = inInstance.transformation * vec4(inNormal, 0.0);

    fEyePosition = vec4(inUniform.eyePosition, 1.0);
    fLightPosition = vec4(inUniform.lightPosition, 1.0);

    fWorldPosition = inInstance.transformation * vec4(inPosition, 1.0);
    vec4 fCameraLocalPosition = inUniform.view * fWorldPosition;
    float distance = length(fCameraLocalPosition.xyz);
    fVisibility = exp(-pow(distance * inMeshPushConstant.fogDensity, inMeshPushConstant.fogGradient));
    fVisibility = clamp(fVisibility, 0.0, 1.0);
    fFogColor = inMeshPushConstant.fogColor;

    gl_Position = inUniform.projection * fCameraLocalPosition;
}