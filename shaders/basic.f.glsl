#version 330 core

in DATA
{
	vec2 position;
	vec4 color;
	vec2 uv;
}fs_in;

out vec4 color;

uniform sampler2D tex;
uniform float time;
uniform int isLevel;

float MULT = 100.0f;

void main() 
{ 
	/* Super temporary, but makes a mask with the hit color over the texture */
	if (fs_in.color.r == 0 && fs_in.color.a == 0) color = texture(tex, fs_in.uv) * MULT;
	else color = texture(tex, fs_in.uv) * fs_in.color;
}