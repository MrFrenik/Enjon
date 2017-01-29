#version 330 core

layout (location = 0) out vec4 ColorOut;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform sampler2D u_albedoMap;
uniform sampler2D u_emissiveMap;
uniform vec2 u_resolution;
uniform vec3 u_ambientColor;
uniform float u_ambientIntensity;

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / u_resolution;
}

void main()
{
    vec2 TexCoords = CalculateTexCoord();

    // Get diffuse color
    vec4 Diffuse = texture(u_albedoMap, TexCoords);

    vec4 Emissive = texture2D(u_emissiveMap, TexCoords);

    // Diffuse
    vec4 DiffuseColor = Diffuse * vec4(u_ambientColor * u_ambientIntensity, 1.0);

   ColorOut = DiffuseColor + Emissive;
   // ColorOut = vec4(u_ambientColor, 1.0) + (ColorOut) * 0.0001;
}