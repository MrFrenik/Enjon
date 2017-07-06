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
uniform float texCoordMultiplier;

vec2 uTexCoords;

vec2 texMult;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform float metallicFloat;
uniform sampler2D roughMap;

uniform float roughFloat;

vec3 roughMult;
uniform sampler2D aoMap;

// Fragment Main
void main()
{
	// Base Color

uTexCoords = fs_in.TexCoords;
texMult = texCoordMultiplier * fs_in.TexCoords;
vec4 albedoMap_sampler = texture2D( albedoMap, texMult );
	AlbedoOut = vec4(albedoMap_sampler.rgb, 1.0);

	// Normal
	vec4 normalMap_sampler = texture2D( normalMap, texMult );
	vec3 normal = normalize( normalMap_sampler.rgb * 2.0 - 1.0 );
	normal = normalize( fs_in.TBN * normal );
	NormalsOut = vec4( normal, 1.0 );

	// Material Properties
	
	vec4 roughMap_sampler = texture2D( roughMap, texMult );

roughMult = roughMap_sampler.rgb * roughFloat;
	vec4 aoMap_sampler = texture2D( aoMap, texMult );
	MatPropsOut = vec4( clamp( metallicFloat, 0.0, 1.0 ), clamp( roughMult.x, 0.0, 1.0 ), clamp( aoMap_sampler.rgb.x, 0.0, 1.0 ), 1.0);

	// Emissive
	EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );

	PositionOut = vec4( fs_in.FragPos, 1.0 );
}
