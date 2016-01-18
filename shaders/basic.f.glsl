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
uniform vec2 resolution;

// Multiplier for being hit
const float MULT = 100, outerRadius = 0.5, innerRadius = 0.1, intensity = 0.7;

// Radius of vignette, where 0.5 results in circle fitting the screen
const float RADIUS = 0.75;

// softness of vignette, between 0.0 and 1.0
const float SOFTNESS = 0.45;

// sepia color
const vec3 SEPIA = vec3(1.2, 1.0, 0.8);

// blue color
const vec3 BLUE = vec3(0.7, 0.5, 1.2);

void main() 
{
	// Sample the texture
	vec4 texColor = texture2D(tex, fs_in.uv);

	// Calculate relative position
	vec2 position = (gl_FragCoord.xy / vec2(1920, 1080)) - vec2(0.5, 0.5);

	// Determine vector length of the center position	
	float len = length(position);

	// Create smooth vignette
	float vignette = smoothstep(outerRadius, innerRadius, len);

	// Mix color with vignette
	texColor.rgb = mix(texColor.rgb, texColor.rgb * vignette, intensity);

	// .2 GRAYSCALE

	// Convert to grayscale using NTSC conversion weights
	float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));

	// 3. SEPIA

	// Create sepia tone from constant value
	vec3 sepiaColor = vec3(gray) * BLUE;

	// Use mix so that sepia effect is at 75%
	texColor.rgb = mix(texColor.rgb, sepiaColor, 0.25);

	// Final color, multiplied by vertex color
	color = texColor * fs_in.color;

	// Mask color if hit
	// NOTE(John): Doesn't seem to be working, so might want to add some more information to the verticies
	//if (fs_in.color.r == 0 && fs_in.color.a == 0) color = texture(tex, fs_in.uv) * MULT;
	//else color = texture(tex, fs_in.uv) * fs_in.color;
	if (fs_in.color.a == 0) color = texColor * fs_in.color * MULT;
	else color = texColor * fs_in.color;
}



