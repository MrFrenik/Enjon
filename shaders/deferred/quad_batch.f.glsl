#version 330 core

const float kPi = 3.13159265;

layout (location = 0) out vec4 DiffuseOut;     // Diffuse
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 DepthOut;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec4 FragColor;
} fs_in;

// uniforms
uniform sampler2D diffuseMap;

void main()
{
    // Translate normal to world space
    // vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // normal = normalize(normal * 2.0 - 1.0);
    // normal = normalize(fs_in.TBN * normal);

    // Get diffuse color
    vec4 color = texture(diffuseMap, fs_in.TexCoords);
    if (color.a < 0.5) discard;
    
    DiffuseOut  = color;
    NormalsOut  = color;
    PositionOut = vec4(fs_in.FragPos, 1.0);
    EmissiveOut = vec4(1.0, 1.0, 1.0, 1.0);
    DepthOut    = vec4(1.0);
}