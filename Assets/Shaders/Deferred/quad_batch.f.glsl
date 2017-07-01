#version 330 core

layout (location = 0) out vec4 DiffuseOut;     // Diffuse
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 MatPropsOut;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec4 FragColor;
} fs_in;

// uniforms
uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_emissiveMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_aoMap;

void main()
{
    // Translate normal to world space
    vec3 normal = texture(u_normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    // Get diffuse color
    vec4 color = texture(u_albedoMap, fs_in.TexCoords);
    if (color.a < 0.4) discard;

    float Metallic  = texture2D(u_metallicMap, fs_in.TexCoords).r;
    float Roughness = texture2D(u_roughnessMap, fs_in.TexCoords).r;
    float AO        = texture2D(u_aoMap, fs_in.TexCoords).r;
    
    DiffuseOut  = color;
    NormalsOut  = vec4(normal, 1.0);
    PositionOut = vec4(fs_in.FragPos, 1.0);
    EmissiveOut = vec4( texture2D(u_emissiveMap, fs_in.TexCoords).rgb, 1.0 );
    MatPropsOut = vec4(Metallic, Roughness, AO, 1.0);
}