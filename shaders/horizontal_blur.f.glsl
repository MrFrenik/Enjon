#version 330 core

in DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_in;

in vec2 v_blurTexCoords[16];

out vec4 color;

uniform sampler2D tex;
uniform float weight;
uniform float blurWeights[16];

void main() 
{
	color = vec4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < 16; i++)
	{
		color += texture2D(tex, v_blurTexCoords[i]) * blurWeights[i] * weight;
	}
}

