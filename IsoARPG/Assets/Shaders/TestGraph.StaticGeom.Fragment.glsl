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
vec2 polarCoords;

// Fragment Main
void main()
{
	// Base Color
{
	vec2 coords = fs_in.TexCoords;
	float len = length( coords );
	float angle = atan( coords.y, coords.x );
	polarCoords = vec2(len, angle);
	polarCoords = coords;
}
	AlbedoOut = vec4(polarCoords, 0.0, 1.0);

	// Normal
	NormalsOut = vec4( fs_in.TBN[2], 1.0 );

	// Material Properties
	MatPropsOut = vec4( clamp( 0.0, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), clamp( 1.0, 0.0, 1.0 ), 1.0);

	// Emissive
	EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );

	PositionOut = vec4( fs_in.FragPositionWorldSpace, 1.0 );
}
