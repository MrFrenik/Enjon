#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec3 vertexUV;

out VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	mat3 TBN;
} vs_out;

// Gloabl Uniforms
uniform float uWorldTime = 1.0f;
uniform mat4 uViewProjection;
uniform mat4 uModel = mat4( 1.0f );

// Variable Declarations

// Vertex Main
void main()
{
	vec3 worldPosition = ( uModel * vec4( vertexPosition, 1.0 ) ).xyz;
	gl_Position = uViewProjection * vec4( worldPosition, 1.0 );

	// Reorthogonalize with respect to N
	vec3 N = normalize( ( uModel * vec4( vertexNormal, 0.0 ) ).xyz );
	vec3 T = normalize( ( uModel * vec4( vertexTangent, 0.0 ) ).xyz );

	// Calculate Bitangent
	vec3 B = cross( N, T );

	// TBN
	mat3 TBN = mat3( T, B, N );

	// Output Vertex Data
	vs_out.FragPos = worldPosition;
	vs_out.TexCoords = vec2( vertexUV.x, -vertexUV.y );
	vs_out.TBN = TBN;
}
