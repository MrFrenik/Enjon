#version 330 core

layout (location = 0) out vec4 diffuse;  	// Diffuse

in DATA
{
	vec4 Position;
	vec2 TexCoords;
	vec4 Color;
} fs_in;

uniform sampler2D texture1;

void main()
{             
    diffuse = fs_in.Color * texture(texture1, fs_in.TexCoords);
}