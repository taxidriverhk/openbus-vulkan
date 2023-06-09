#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D inSampler;
layout(set = 1, binding = 1) uniform samplerCube inSamplerEnvMap;

layout(location = 0) in vec2 fUV;
layout(location = 1) in vec4 fNormal;
layout(location = 2) in vec4 fWorldPosition;
layout(location = 3) in vec4 fEyePosition;
layout(location = 4) in vec4 fLightPosition;
layout(location = 5) in float fVisibility;
layout(location = 6) in vec3 fFogColor;

layout(location = 0) out vec4 outColor;

// TODO: hard-code some values for now
const vec4 lightColor = vec4(vec3(0.5), 1.0);
const float reflectivity = 0.1;
const float shininess = 32;
const float specularStrength = 0.5;
const float ambientStrength = 0.35;

void main() {
    vec4 objectColor = texture(inSampler, fUV);
    if (objectColor.a < 1)
        discard;

    vec4 ambient = ambientStrength * vec4(1.0);
    vec4 diffuseColor = vec4(1.0);
    vec4 specularColor = vec4(0.0);

    // Phong shading formula
    vec4 N = normalize(fNormal);
    vec4 L = normalize(fLightPosition - fWorldPosition);
    vec4 V = normalize(fEyePosition - fWorldPosition);
    vec4 R = reflect(-L, N);
    vec4 ER = reflect(-V, N);
    float NdotL = max(dot(N, L), 0);
    float RdotV = max(dot(R, V), 0);

    vec4 diffuse = NdotL * lightColor * diffuseColor;
    vec4 specular = specularStrength * pow(max(RdotV, 0.0), shininess) * lightColor;

    vec4 reflectionColor = vec4(texture(inSamplerEnvMap, ER.xyz).rgb, 1.0);

    outColor = (ambient + diffuse + specular) * objectColor;
    outColor = mix(outColor, reflectionColor, reflectivity);
    outColor = mix(vec4(fFogColor, 1.0), outColor, fVisibility);
    outColor.a = objectColor.a;
}