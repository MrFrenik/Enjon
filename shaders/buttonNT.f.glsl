#version 330 core

in DATA
{
	vec4 position;
	vec4 color;
	vec2 uv;
}fs_in;

out vec4 color; 

void main() 
{ 
	color = fs_in.color; 
}