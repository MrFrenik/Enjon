#version 330 core

in DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_in;

out vec4 Color;

uniform sampler2D DepthTexture;

void main() 
{
	float D = texture2D(DepthTexture, fs_in.TexCoords).r;
	Color = vec4(D, D, D, 1.0);
}

