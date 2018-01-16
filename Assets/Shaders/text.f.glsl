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

	// Sample the texture
	vec4 sampled = texture(tex, fs_in.uv);
	sampled.a = sampled.r; 

	// Final color
	color = vec4( fs_in.color.rgb * sampled.r, sampled.a );
}

