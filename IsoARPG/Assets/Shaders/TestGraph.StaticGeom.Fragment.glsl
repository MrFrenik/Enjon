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
	vec3 ViewPositionTangentSpace;
	vec3 FragPositionTangentSpace;
} fs_in;

// Global Uniforms
uniform float uWorldTime = 1.0f;
uniform vec3 uViewPositionWorldSpace;

// Variable Declarations
uniform sampler2D albedoMap2;
uniform sampler2D normalMap2;
uniform float metallic;
uniform float roughness;
uniform sampler2D emissiveMap;

// Fragment Main
void main()
{
	// Base Color
vec4 albedoMap2_sampler = texture2D( albedoMap2, fs_in.TexCoords );
	AlbedoOut = vec4(albedoMap2_sampler.rgb, 1.0);

	// Normal
	vec4 normalMap2_sampler = texture2D( normalMap2, fs_in.TexCoords );
	vec3 normal = normalize( normalMap2_sampler.rgb * 2.0 - 1.0 );
	normal = normalize( fs_in.TBN * normal );
	NormalsOut = vec4( normal, 1.0 );

	// Material Properties
	
	
	MatPropsOut = vec4( clamp( metallic, 0.0, 1.0 ), clamp( roughness, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), 1.0);

	// Emissive
	vec4 emissiveMap_sampler = texture2D( emissiveMap, fs_in.TexCoords );
	EmissiveOut = vec4(emissiveMap_sampler.rgb, 1.0);

	PositionOut = vec4( fs_in.FragPositionWorldSpace, 1.0 );
}
