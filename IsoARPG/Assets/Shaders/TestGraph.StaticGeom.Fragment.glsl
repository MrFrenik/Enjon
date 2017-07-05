#version 330 core

layout (location = 0) out vec4 AlbedoOut;
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 MatPropsOut;

in VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	mat3 TBN;
} fs_in;

// Global Uniforms
uniform float uWorldTime = 1.0f;

// Variable Declarations
uniform vec3 uColor;
uniform float texCoordMultiplier;

vec2 uTexCoords;

vec2 texMult;

uniform sampler2D normalMap;
uniform float metallicFloat;
uniform float roughFloat;
uniform vec3 emissiveColor;

uniform float emissiveIntensity;

vec3 emissiveMult;

uniform float timeMultiplier;

float timeNode;

float timeMultiplication;

float sinNode;

float clampMin;

float clampMax;

float clamp_sin;

vec3 sinMultNode;

// Fragment Main
void main()
{
	// Base Color

	AlbedoOut = vec4(uColor, 1.0);

	// Normal
	
uTexCoords = fs_in.TexCoords;
texMult = texCoordMultiplier * fs_in.TexCoords;
vec4 normalMap_sampler = texture2D( normalMap, texMult );
	vec3 normal = normalize( normalMap_sampler.rgb * 2.0 - 1.0 );
	normal = normalize( fs_in.TBN * normal );
	NormalsOut = vec4( normal, 1.0 );

	// Material Properties
	
	
	MatPropsOut = vec4( clamp( metallicFloat, 0.0, 1.0 ), clamp( roughFloat, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), 1.0);

	// Emissive
	

emissiveMult = emissiveColor * emissiveIntensity;

timeNode = uWorldTime;
timeMultiplication = timeMultiplier * timeNode;
sinNode = sin(timeMultiplication);
clampMin = 0.0;
clampMax = 1.0;
clamp_sin = clamp(sinNode, clampMin, clampMax);
sinMultNode = emissiveMult * clamp_sin;
	EmissiveOut = vec4(sinMultNode, 1.0);

	PositionOut = vec4( fs_in.FragPos, 1.0 );
}
