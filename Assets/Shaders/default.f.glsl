#version 330 core

in DATA
{
	vec3 position;
	vec4 color;
	vec2 uv;
} fs_in;

out vec4 color;

uniform sampler2D tex;

void main()
{
	vec4 texel = texture(tex, fs_in.uv);
	if (texel.a < 0.5) discard;
	color = fs_in.color * texel;
}