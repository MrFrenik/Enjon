#version 330 core

in DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_in;

out vec4 color;

uniform sampler2D tex;
uniform float u_threshold;

void main() 
{
	color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 FragColor = texture2D(tex, fs_in.TexCoords).rgb;
	float brightness = dot(FragColor, vec3(0.2126, 0.7152, 0.0722));

	if (brightness > u_threshold)
	{
		vec3 output = clamp(FragColor, vec3(0), vec3(1));
		color = vec4(output, 1.0);
	}
}

