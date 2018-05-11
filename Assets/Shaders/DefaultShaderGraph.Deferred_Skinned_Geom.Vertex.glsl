#version 330 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aVertexNormal;
layout (location = 2) in vec3 aVertexTangent;
layout (location = 3) in vec2 aVertexUV;
layout (location = 4) in vec4 aJointIndices;
layout (location = 5) in vec4 aJointWeights;

out VS_OUT
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
} vs_out;

// Gloabl Uniforms
uniform float uWorldTime = 1.0f;
uniform mat4 uViewProjection;
uniform mat4 uPreviousViewProjection;
uniform vec3 uViewPositionWorldSpace;
uniform mat4 uModel = mat4( 1.0f );
uniform mat4 uPreviousModel = mat4( 1.0f );
uniform vec4 uObjectID;

// Variable Declarations

// Vertex Main
void main()
{
}
