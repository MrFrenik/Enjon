#version 330 core

in VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
    mat3 TBN;
} fs_in;

out vec4 ColorOut;

uniform vec4 diffuseColor = vec4(1, 1, 1, 1);

// uniforms
uniform sampler2D diffuseMap;
uniform sampler2D emissiveMap;

void main()
{
    vec4 Diffuse = texture(diffuseMap, fs_in.TexCoords);
    if (Diffuse.a < 0.5) discard;
    ColorOut = diffuseColor * Diffuse;
}