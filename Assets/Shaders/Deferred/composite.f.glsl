#version 330 core

in DATA
{
	vec2 Position;	
	vec2 TexCoords;
}fs_in;

out vec4 color;

uniform sampler2D tex;
uniform sampler2D u_blurTexSmall;
uniform sampler2D u_blurTexMedium;
uniform sampler2D u_blurTexLarge;
uniform float u_exposure;
uniform float u_gamma;
uniform float u_bloomScalar;
uniform float u_saturation;

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;


vec3 Uncharted2Tonemap(vec3 x)
{
     return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main() 
{
	// Mix bloom
	vec3 hdrColor = max( vec3( 0.0 ), texture2D(tex, fs_in.TexCoords).rgb );
	vec3 bloomColor = texture2D(u_blurTexSmall, fs_in.TexCoords).rgb + 
						texture2D(u_blurTexMedium, fs_in.TexCoords).rgb + 
						texture2D(u_blurTexLarge, fs_in.TexCoords).rgb;
	bloomColor = bloomColor / 3.0;
	hdrColor += bloomColor * u_bloomScalar;

	// Tone mapping
	vec3 result = vec3(1.0) - exp(-hdrColor * u_exposure);
	result = pow(result, vec3(1.0 / u_gamma));

	// Saturation
	float lum = result.r * 0.2 + result.g * 0.7 + result.b * 0.1;
	vec3 diff = result.rgb - vec3(lum);

	// Final
	color = vec4(vec3(diff) * u_saturation + lum, 1.0); 
}

