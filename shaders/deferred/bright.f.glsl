#version 330 core

in DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_in;

out vec4 color;

uniform sampler2D tex;
uniform sampler2D u_emissiveMap;
uniform float u_threshold;

void main() 
{
	color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 FragColor = texture2D(tex, fs_in.TexCoords).rgb;
	vec3 EmissiveColor = texture2D(u_emissiveMap, fs_in.TexCoords).rgb;

	// Clamp to avoid exceeding max float
	FragColor = min(vec3(256 * 10, 256 * 10, 256 * 10), FragColor.rgb);

	// Calculate luminance of scene
	float brightness = dot(FragColor, vec3(0.2126, 0.7152, 0.0722));

	if (brightness > u_threshold || length(EmissiveColor) != 0)
	{
		vec3 output = clamp(FragColor, vec3(0), vec3(256));
		color = vec4(output * 0.1, 1.0) + vec4(EmissiveColor, 1.0);
	}
}

