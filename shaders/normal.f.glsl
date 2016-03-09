#version 330 core
in vec2 TexCoords;

layout (location = 2) out vec4 normal;		// Normal

in DATA
{
	vec4 Position;
	vec2 TexCoords;
	vec4 Color;
} fs_in;

uniform sampler2D texture1;

void main()
{             
    normal = fs_in.Color * texture2D(texture1, fs_in.TexCoords);
}