#version 330 core

layout (location = 1) out vec4 depth;		// Normal

in DATA
{
	vec4 Position;
	vec2 TexCoords;
	vec4 Color;
} fs_in;

uniform sampler2D texture1;

void main()
{ 
	depth = vec4(0, 0, 0, 1);
	depth.r = gl_FragCoord.z;            
}