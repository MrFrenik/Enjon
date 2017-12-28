#version 330 core

in DATA
{
	vec2 Position;	
	vec2 TexCoords;
}fs_in;

layout (location = 0) out vec4 ColorOut;

uniform sampler2D tex;

void main() 
{
	ColorOut = texture2D(tex, fs_in.TexCoords);
}

