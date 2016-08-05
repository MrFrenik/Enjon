#version 330 core

in DATA
{
	vec2 position;
	vec4 color;
	vec2 uv;
}fs_in;

out vec4 color;

uniform sampler2D tex;

void main() 
{
	color = fs_in.color * texture2D(tex, fs_in.uv);
}

