#version 330 core

in DATA
{
	vec3 position;
	vec3 color;
}fs_in;

out vec4 FinalColor;

void main() 
{
	// Final color
	FinalColor = vec4( fs_in.color.rgb, 1.0 );
}

