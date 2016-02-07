#version 330 core

in DATA
{
	vec2 position;
	vec4 color;
	vec2 uv;
}fs_in;

out vec4 color;

uniform sampler2D tex;
uniform bool isLevel;
uniform bool isPaused;
uniform bool useOverlay;

// Multiplier for being hit
const float MULT = 100.0f; 
const float FIREMULT = 100.0f;

const float outerRadius = 0.4, innerRadius = 0.1, intensity = 0.5;

// Radius of vignette, where 0.5 results in circle fitting the screen
const float RADIUS = 0.75;

// softness of vignette, between 0.0 and 1.0
const float SOFTNESS = 0.45;

/* Colors */
const vec3 SEPIA = vec3(1.2, 1.0, 0.8);
const vec3 BLUE = vec3(0.7, 0.5, 1.2);
const vec3 DARKGREY = vec3(0.01, 0.01, 0.1);
const vec3 WHITE = vec3(1.00, 1.00, 1.0);

vec4 Overlay(vec4 texColor, vec3 overlay, float mixAmount);

void main() 
{
	vec4 texColor = texture2D(tex, fs_in.uv);
	
	if (isPaused)
	{
		if (useOverlay)
		{
			texColor = Overlay(texColor, SEPIA, 50.0f);
			//texColor = Overlay(texColor, SEPIA * DARKGREY, 0.25);

			// Mask color if hit
			if (fs_in.color.r == 0.0 && fs_in.color.a == 0.0) color = texColor * MULT;
			else color = fs_in.color * texColor;
		}
		else
		{
			color = fs_in.color * texColor;	
		}
	}
	else if (isLevel)
	{
		if (useOverlay)
		{
			texColor = Overlay(texColor, BLUE, 0.25f);

			// Mask color if hit
			if (fs_in.color.r == 0 && fs_in.color.a == 0) color = texColor * MULT;
			else color = fs_in.color * texColor;
		}
		else
		{
			color = fs_in.color * texColor;	
		}
	}

	else
	{
		if (useOverlay)
		{
			
			// Mask color if hit
			if (fs_in.color.r == 0 && fs_in.color.a == 0)
			{
			 	texColor = Overlay(texColor, SEPIA, 0.7f);
				color = texColor * MULT;
			}
			else if (fs_in.color.r >= .5 && fs_in.color.a == 0) 
			{
				texColor = Overlay(texColor, SEPIA, 100.0f);
				color = texColor * vec4(2.0f, 1.0f, 1.0f, 1.0f);
			}
			else 
			{
			 	texColor = Overlay(texColor, BLUE, 0.5f);
				color = fs_in.color * texColor;
			}
		}
		else
		{
			color = fs_in.color * texColor;
		}

	}
}

vec4 Overlay(vec4 textureColor, vec3 overlay, float mixAmount)
{
	vec4 texColor = textureColor;

	// Calculate relative position
	vec2 position = (gl_FragCoord.xy / vec2(1920, 1080)) - vec2(0.5, 0.5);

	// Determine vector length of the center position	
	float len = length(position);

	// Create smooth vignette
	float vignette = smoothstep(outerRadius, innerRadius, len);

	// Mix color with vignette
	texColor.rgb = mix(texColor.rgb, texColor.rgb * vignette, intensity);

	// Convert to grayscale using NTSC conversion weights
	float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));

	// Create tone from constant value
	vec3 overlayColor = vec3(gray) * overlay;

	// Use mix so that sepia effect is at 75%
	texColor.rgb = mix(texColor.rgb, overlayColor, mixAmount);

	return texColor;
}


