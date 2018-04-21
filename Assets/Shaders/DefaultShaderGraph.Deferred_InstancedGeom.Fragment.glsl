#version 330 core

layout (location = 0) out vec4 AlbedoOut;
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 EmissiveOut;
layout (location = 3) out vec4 MatPropsOut;

layout (location = 4) out vec4 ObjectIDOut;

layout (location = 5) out vec4 VelocityOut;


in VS_OUT
{
	vec3 FragPositionWorldSpace;
	vec2 TexCoords;
	mat3 TBN;
	mat3 TS_TBN;
	vec3 ViewPositionTangentSpace;
	vec3 FragPositionTangentSpace;
	vec4 ObjectID;
	vec4 PreviousFragPositionClipSpace;
	vec4 CurrentFragPositionClipSpace;
} fs_in;

// Global Uniforms
uniform float uWorldTime = 1.0f;
uniform vec3 uViewPositionWorldSpace;
uniform mat4 uPreviousViewProjection;
uniform mat4 uViewProjection;

// Variable Declarations

// Fragment Main
void main()
{
	// Base Color
	AlbedoOut = vec4( 1.0, 1.0, 1.0, 1.0 );

	// Normal
	NormalsOut = vec4( fs_in.TBN[2], 1.0 );

	// Material Properties
	MatPropsOut = vec4( clamp( 0.0, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), 1.0);

	// Emissive
	EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );

	ObjectIDOut = fs_in.ObjectID;
	vec2 _a = ( fs_in.CurrentFragPositionClipSpace.xy / fs_in.CurrentFragPositionClipSpace.w ) * 0.5 + 0.5;
	vec2 _b = ( fs_in.PreviousFragPositionClipSpace.xy / fs_in.PreviousFragPositionClipSpace.w ) * 0.5 + 0.5;
	vec2 _vel = vec2( _a - _b );
	VelocityOut = vec4( _vel, 0.0, 1.0 );
}
