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
	vec3 ViewPositionTangentSpace;
	vec3 FragPositionTangentSpace;
} vs_out;

// Gloabl Uniforms
uniform float uWorldTime = 1.0f;
uniform mat4 uViewProjection;
uniform vec3 uViewPositionWorldSpace;
uniform mat4 uModel = mat4( 1.0f );

// Variable Declarations

// Vertex Main
void main()
{
	vec3 worldPosition = ( uModel * vec4( aVertexPosition, 1.0 ) ).xyz;
	gl_Position = uViewProjection * vec4( worldPosition, 1.0 );

	// Reorthogonalize with respect to N
	vec3 N = normalize( ( uModel * vec4( aVertexNormal, 0.0 ) ).xyz );
	vec3 T = normalize( ( uModel * vec4( aVertexTangent, 0.0 ) ).xyz );

	// Calculate Bitangent
	vec3 B = cross( N, T );

	// TBN
	mat3 TBN = mat3( T, B, N );


	// TS_TBN
	vec3 TS_T = normalize(mat3(uModel) * aVertexTangent);
	vec3 TS_N = normalize(mat3(uModel) * aVertexNormal);
	vec3 TS_B = normalize(cross(TS_N, TS_T));
	mat3 TS_TBN = transpose(mat3( TS_T, TS_B, TS_N ));

	// Output Vertex Data
	vs_out.FragPositionWorldSpace = worldPosition;
	vs_out.TexCoords = vec2( aVertexUV.x, -aVertexUV.y );
	vs_out.ViewPositionTangentSpace = TS_TBN * uViewPositionWorldSpace;
	vs_out.FragPositionTangentSpace = TS_TBN * vs_out.FragPositionWorldSpace;
	vs_out.TBN = TBN;
}
