#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D inSampler;

layout(location = 0) in vec2 fUV;
layout(location = 1) in vec4 fNormal;
layout(location = 2) in vec4 fWorldPosition;
layout(location = 3) in vec4 fEyePosition;
layout(location = 4) in vec4 fLightPosition;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 objectColor = texture(inSampler, fUV);

    // TODO: hard-code some values for now
    float shininess = 0.15;
    float ambientStrength = 0.35;
    vec4 lightColor = vec4(vec3(0.5), 1.0);
    vec4 ambient = ambientStrength * vec4(1.0);
    vec4 diffuseColor = vec4(1.0);
    vec4 specularColor = vec4(0.0);

    // Phong shading formula
    vec4 N = normalize(fNormal);
    vec4 L = normalize(fLightPosition - fWorldPosition);
    vec4 V = normalize(fEyePosition - fWorldPosition);
    vec4 H = normalize(L + V);
    vec4 R = reflect(-L, N);
    float NdotL = max(dot(N, L), 0);
    float RdotV = max(dot(R, V), 0);
    float NdotH = max(dot(N, H), 0);

    vec4 diffuse = NdotL * lightColor * diffuseColor;
    vec4 specular = pow(RdotV, shininess) * lightColor * specularColor;

    outColor = (ambient + diffuse + specular) * objectColor;
}