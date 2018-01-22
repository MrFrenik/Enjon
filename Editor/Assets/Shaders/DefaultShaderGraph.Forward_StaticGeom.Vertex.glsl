#version 330 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aVertexNormal;
layout (location = 2) in vec3 aVertexTangent;
layout (location = 3) in vec3 aVertexUV;

out VS_OUT
{
	vec3 FragPositionWorldSpace;
	vec2 TexCoords;
	mat3 TBN;
	mat3 TS_TBN;
	vec3 ViewPositionTangentSpace;
	vec3 FragPositionTangentSpace;
	vec4 ObjectID;
} vs_out;

// Gloabl Uniforms
uniform float uWorldTime = 1.0f;
uniform mat4 uViewProjection;
uniform vec3 uViewPositionWorldSpace;
uniform mat4 uModel = mat4( 1.0f );
uniform vec4 uObjectID;

// Variable Declarations

// Vertex Main
void main()
{
	vec3 worldPosition = ( uModel * vec4( aVertexPosition, 1.0 ) ).xyz;
	gl_Position = uViewProjection * vec4( worldPosition, 1.0 );

	vec3 N = normalize( mat3(uModel) * aVertexNormal );
	vec3 T = normalize( mat3(uModel) * aVertexTangent );
	// Reorthogonalize with respect to N
	T = normalize( T - dot(T, N) * N );


	// Calculate Bitangent
	vec3 B = cross( N, T );

	// TBN
	mat3 TBN = mat3( T, B, N );


	// TS_TBN
	mat3 TS_TBN = transpose( TBN );

	// Output Vertex Data
	vs_out.FragPositionWorldSpace = worldPosition;
	vs_out.TexCoords = vec2( aVertexUV.x, -aVertexUV.y );
	vs_out.ViewPositionTangentSpace = TS_TBN * uViewPositionWorldSpace;
	vs_out.FragPositionTangentSpace = TS_TBN * vs_out.FragPositionWorldSpace;
	vs_out.TBN = TBN;
	vs_out.TS_TBN = TS_TBN;
	vs_out.ObjectID = uObjectID;
}
