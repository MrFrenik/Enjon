#version 330 core

const float kPi = 3.13159265;

layout (location = 0) out vec4 DiffuseOut;     // Diffuse
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 MatPropsOut;
layout (location = 5) out vec4 UVOut;

in VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform vec3 u_albedoColor = vec3(1, 1, 1);

// uniforms
uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_emissiveMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_aoMap;

const float u_emissiveIntensity = 5.0;

out vec4 ColorOut;

void main()
{
    // Translate normal to world space
    vec3 normal = texture(u_normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    // Get albedo color
    vec4 color = texture(u_albedoMap, fs_in.TexCoords);
    if (color.a < 0.5) discard;

    float Metallic  = texture2D(u_metallicMap, fs_in.TexCoords).r;
    float Roughness = texture2D(u_roughnessMap, fs_in.TexCoords).r;
    float AO        = texture2D(u_aoMap, fs_in.TexCoords).r;
    
    DiffuseOut  = color * vec4(u_albedoColor, 1.0);
    NormalsOut  = vec4(normal, 1.0);
    PositionOut = vec4(fs_in.FragPos, 1.0);
    EmissiveOut = texture2D(u_emissiveMap, fs_in.TexCoords) * vec4(u_emissiveIntensity, u_emissiveIntensity, u_emissiveIntensity, 1.0);
    MatPropsOut = vec4(Metallic, Roughness, AO, 1.0);
    UVOut = vec4(fs_in.TexCoords.x, fs_in.TexCoords.y, 0.0, 1.0);

    ColorOut = color * vec4(u_albedoColor, 1.0);
}