#version 330 core

in DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_in;

out vec4 color;

uniform sampler2D tex;

void main() 
{
	color = texture2D(tex, fs_in.TexCoords);
}

