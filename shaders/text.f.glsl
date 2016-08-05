#version 330 core

in DATA
{
	vec2 position;
	vec4 color;
	vec2 uv;
}fs_in;

out vec4 color;

uniform sampler2D tex;

#define ALPHA_VALUE_BORDER 0.0

void main() 
{
	// Sample the texture
	vec4 sampled = vec4(1.0,1.0, 1.0, texture(tex, fs_in.uv).r);
	color = fs_in.color * sampled;

	//color = fs_in.color * texture2D(tex, fs_in.uv);
}

