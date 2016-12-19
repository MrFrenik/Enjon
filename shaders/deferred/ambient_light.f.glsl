#version 330 core

layout (location = 0) out vec4 ColorOut;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform sampler2D DiffuseMap;
uniform sampler2D EmissiveMap;
uniform vec2 Resolution;
uniform vec3 AmbientColor;
uniform float AmbientIntensity;

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / Resolution;
}

void main()
{
    vec2 TexCoords = CalculateTexCoord();

    // Get diffuse color
    vec4 Diffuse = texture(DiffuseMap, TexCoords);

    vec4 Emissive = texture2D(EmissiveMap, TexCoords);

    // Diffuse
    vec4 DiffuseColor = Diffuse * vec4(AmbientColor * AmbientIntensity, 1.0);

   ColorOut = DiffuseColor + Emissive;
}