/*
* @info: This file has been generated. All changes will be lost.
* @file: TestGraph.StaticGeom.FragmentShader.glsl
*/

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

// Variable Declarations
float mult;

// Fragment Main
void main()
{
	// Base Color
mult = 0.0 * 0.0;
	AlbedoOut = vec4(vec3(mult, mult, mult), 1.0 );

	// Normal
	NormalsOut = vec4( fs_in.TBN[2], 1.0 );

	// Material Properties
	MatPropsOut = vec4( 0.0, 1.0, 0.0, 1.0);

	// Emissive
	EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );

	PositionOut = vec4( fs_in.FragPos, 1.0 );
}
