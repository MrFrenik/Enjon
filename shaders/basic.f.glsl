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

void main() 
{ 
	/* Super temporary, but makes a mask with the hit color over the texture */
	if (isLevel == 1) color = fs_in.color * texture(tex, fs_in.uv); 
	else if (fs_in.color == vec4(1.0f, 0.0f, 0.0f, 1.0f)) color = texture(tex, fs_in.uv) * vec4((1 + tan(3.14f / 2.0f)) / 2.0f, 0.0f, 0.0f, 1.0f); 
	else color = fs_in.color * texture(tex, fs_in.uv) * vec4((1 + sin(time)) / 2.0f, (1 + cos(time)) / 2.0f, (1 + tan(time)) / 2.0f, 1.0f);
}