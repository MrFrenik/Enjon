#version 330 core

const float kPi = 3.13159265;

layout (location = 0) out vec4 DiffuseOut;     // Diffuse
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;

in VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform vec3 diffuseColor = vec3(1, 1, 1);

// uniforms
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;

const float emissiveIntensity = 4.0;

void main()
{
    // Translate normal to world space
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    // Get diffuse color
    vec4 color = texture(diffuseMap, fs_in.TexCoords);
    if (color.a < 0.5) discard;
    
    DiffuseOut  = color * vec4(diffuseColor, 1.0);
    NormalsOut  = vec4(normal, 1.0);
    PositionOut = vec4(fs_in.FragPos, 1.0);
    EmissiveOut = texture2D(emissiveMap, fs_in.TexCoords) * 
                            vec4(emissiveIntensity, emissiveIntensity, emissiveIntensity, 1.0) * 
                            vec4(diffuseColor, 1.0);
}