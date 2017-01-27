#version 330 core

layout (location = 0) out vec4 ColorOut;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform sampler2D DiffuseMap;
uniform sampler2D EmissiveMap;
// uniform sampler2D MaterialProperties;
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

    // float AO = texture2D(MaterialProperties, TexCoords).b;

    // Diffuse
    // vec4 DiffuseColor = Diffuse * vec4(AmbientColor * AmbientIntensity, 1.0) * AO;
    vec4 DiffuseColor = Diffuse * vec4(AmbientColor * AmbientIntensity, 1.0);

    // Brightness
	float lum = DiffuseColor.r * 0.2 + DiffuseColor.g * 0.7 + DiffuseColor.b * 0.1;

   ColorOut = DiffuseColor + Emissive;
}