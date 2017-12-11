#version 330 core

layout (location = 0) out vec4 AlbedoOut;
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 MatPropsOut;


in VS_OUT
{
	vec3 FragPositionWorldSpace;
	vec2 TexCoords;
	mat3 TBN;
	mat3 TS_TBN;
	vec3 ViewPositionTangentSpace;
	vec3 FragPositionTangentSpace;
} fs_in;

// Global Uniforms
uniform float uWorldTime = 1.0f;
uniform vec3 uViewPositionWorldSpace;

// Variable Declarations
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughMap;

// Fragment Main
void main()
{
	// Base Color
vec4 albedoMap_sampler = texture2D( albedoMap, fs_in.TexCoords );
	AlbedoOut = vec4(albedoMap_sampler.rgb, 1.0);

	// Normal
	vec4 normalMap_sampler = texture2D( normalMap, fs_in.TexCoords );
	vec3 normal = normalize( normalMap_sampler.rgb * 2.0 - 1.0 );
	normal = normalize( fs_in.TBN * normal );
	NormalsOut = vec4( normal, 1.0 );

	// Material Properties
	vec4 metallicMap_sampler = texture2D( metallicMap, fs_in.TexCoords );
	vec4 roughMap_sampler = texture2D( roughMap, fs_in.TexCoords );
	MatPropsOut = vec4( clamp( metallicMap_sampler.rgb.x, 0.0, 1.0 ), clamp( roughMap_sampler.rgb.x, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), 1.0);

	// Emissive
	EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );

	PositionOut = vec4( fs_in.FragPositionWorldSpace, 1.0 );
}
