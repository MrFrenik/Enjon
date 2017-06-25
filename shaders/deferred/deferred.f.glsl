// Fragment Begin

#version 330 core

const float kPi = 3.13159265;

layout (location = 0) out vec4 AlbedoOut;     // Diffuse
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

uniform vec4 u_albedoColor = vec4(1, 1, 1, 1);

// uniforms
uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_emissiveMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_aoMap;

float Metallic = 0.0;
float Roughness = 1.0;
float AO = 0.0;

const float u_emissiveIntensity = 10.0;

void main()
{
    // Translate normal to world space
    vec3 normal = texture(u_normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal.xyz * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    // Get albedo color
    vec4 albedo = texture(u_albedoMap, fs_in.TexCoords);
    if (albedo.a < 0.5) discard;
    //albedo = vec4(vec3(pow(albedo.r, 2.2), pow(albedo.g, 2.2), pow(albedo.b, 2.2)), 1.0);

    Metallic  = texture2D(u_metallicMap, fs_in.TexCoords).r;
    Roughness = texture2D(u_roughnessMap, fs_in.TexCoords).r;
    AO        = texture2D(u_aoMap, fs_in.TexCoords).r;

    AlbedoOut  = albedo * vec4(u_albedoColor.xyz, 1.0);
    NormalsOut  = vec4(normal, 1.0);
    PositionOut = vec4(fs_in.FragPos, 1.0);
    EmissiveOut = vec4(texture2D(u_emissiveMap, fs_in.TexCoords).xyz * u_emissiveIntensity, 1.0);
    MatPropsOut = vec4(Metallic, Roughness, AO, 1.0);
}