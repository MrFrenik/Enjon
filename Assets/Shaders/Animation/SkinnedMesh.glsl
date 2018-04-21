#version 330 core

layout (location = 0) out vec4 AlbedoOut;
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 EmissiveOut;
layout (location = 3) out vec4 MatPropsOut;
layout (location = 4) out vec4 ObjectIDOut;
 layout (location = 5) out vec4 VelocityOut;

in VS_OUT
{
	mat3 TBN;
	vec4 ObjectID;
	vec4 PreviousFragPositionClipSpace;
	vec4 CurrentFragPositionClipSpace;
} fs_in;

// Global Uniforms
uniform vec3 uViewPositionWorldSpace;
uniform mat4 uPreviousViewProjection;
uniform mat4 uViewProjection;

// Variable Declarations

// Fragment Main
void main()
{
	// Base Color
	AlbedoOut = vec4(1.0, 1.0, 1.0, 1.0);

	// Normal
	vec3 normal = fs_in.TBN[2];
	NormalsOut = vec4( normal, 1.0 );

	// Material Properties
	MatPropsOut = vec4( 0.0, 1.0, 1.0, 1.0);

	// Emissive
	EmissiveOut = vec4(0.0, 0.0, 0.0, 1.0);

	// Object id
	ObjectIDOut = fs_in.ObjectID;

	// Object Velocity
	vec2 _a = ( fs_in.CurrentFragPositionClipSpace.xy / fs_in.CurrentFragPositionClipSpace.w ) * 0.5 + 0.5;
	vec2 _b = ( fs_in.PreviousFragPositionClipSpace.xy / fs_in.PreviousFragPositionClipSpace.w ) * 0.5 + 0.5;
	vec2 _vel = vec2( _a - _b );
	VelocityOut = vec4( _vel, 0.0, 1.0 );
}
