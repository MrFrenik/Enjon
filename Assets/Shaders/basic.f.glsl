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
	// vec4 tex_color = clamp(texture(tex, fs_in.uv) + vec4(1.0, 1.0, 1.0, 1.0), 0.0, 1.0);
	color = fs_in.color * texture(tex, fs_in.uv);
	// color = fs_in.color * tex_color;
}

