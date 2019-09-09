// Fragment Begin

#version 330 core

const float kPi = 3.13159265;

layout (location = 0) out vec4 AlbedoOut;
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 MatPropsOut;

in VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
    mat3 TBN;
} fs_in;

// uniforms
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uAoMap;

float Metallic = 0.0;
float Roughness = 1.0;
float AO = 0.0;

const float u_emissiveIntensity = 10.0;

void main()
{
    // Translate normal to world space
    vec3 normal = texture(uNormalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal.xyz * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    // Get albedo color
    vec4 albedo = texture(uAlbedoMap, fs_in.TexCoords);
    if (albedo.a < 0.5) discard;

    Metallic  = texture(uMetallicMap, fs_in.TexCoords).r;
    Roughness = texture(uRoughnessMap, fs_in.TexCoords).r;
    AO        = texture(uAoMap, fs_in.TexCoords).r;

    AlbedoOut  = albedo;
    NormalsOut  = vec4(normal, 1.0);
    PositionOut = vec4(fs_in.FragPos, 1.0);
    EmissiveOut = vec4(texture(uEmissiveMap, fs_in.TexCoords).xyz * u_emissiveIntensity, 1.0);
    MatPropsOut = vec4(Metallic, Roughness, AO, 1.0);
}