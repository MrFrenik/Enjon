#version 330 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aVertexNormal;
layout (location = 2) in vec3 aVertexTangent;
layout (location = 3) in vec2 aVertexUV;
layout (location = 4) in ivec4 aJointIndices;
layout (location = 5) in ivec4 aJointIndices2;
layout (location = 6) in ivec4 aJointIndices3;
layout (location = 7) in vec4 aJointWeights;
layout (location = 8) in vec4 aJointWeights2;
layout (location = 9) in vec4 aJointWeights3;
const int MAX_JOINTS = 210;

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
uniform mat4 uJointTransforms[MAX_JOINTS];

// Variable Declarations

// Vertex Main
void main()
{
	int i0 = int( aJointIndices[0] );
	int i1 = int( aJointIndices[1] );
	int i2 = int( aJointIndices[2] );
	int i3 = int( aJointIndices[3] );
	int i4 = int( aJointIndices2[0] );
	int i5 = int( aJointIndices2[1] );
	int i6 = int( aJointIndices2[2] );
	int i7 = int( aJointIndices2[3] );
	int i8 = int( aJointIndices3[0] );
	int i9 = int( aJointIndices3[1] );
	int i10 = int( aJointIndices3[2] );
	int i11 = int( aJointIndices3[3] );
	
	// Calculate joint transform
	mat4 jointTransform = uJointTransforms[i0] * aJointWeights[0];
	jointTransform += uJointTransforms[i1] * aJointWeights[1];
	jointTransform += uJointTransforms[i2] * aJointWeights[2];
	jointTransform += uJointTransforms[i3] * aJointWeights[3];
	jointTransform += uJointTransforms[i4] * aJointWeights2[0];
	jointTransform += uJointTransforms[i5] * aJointWeights2[1];
	jointTransform += uJointTransforms[i6] * aJointWeights2[2];
	jointTransform += uJointTransforms[i7] * aJointWeights2[3];
	jointTransform += uJointTransforms[i8] * aJointWeights3[0];
	jointTransform += uJointTransforms[i9] * aJointWeights3[1];
	jointTransform += uJointTransforms[i10] * aJointWeights3[2];
	jointTransform += uJointTransforms[i11] * aJointWeights3[3];
	
	vec4 posL = jointTransform * vec4( aVertexPosition, 1.0 );
	vec3 worldPosition = ( uModel * posL ).xyz;
	gl_Position = uViewProjection * vec4( worldPosition, 1.0 );

	vec3 normalL = ( jointTransform * vec4(aVertexNormal, 0.0) ).xyz;
	vec3 N = normalize( mat3(uModel) * normalL );
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
	vs_out.CurrentFragPositionClipSpace = gl_Position;
	vs_out.PreviousFragPositionClipSpace = uPreviousViewProjection * uPreviousModel * jointTransform * vec4( aVertexPosition, 1.0 );
	vs_out.TBN = TBN;
	vs_out.TS_TBN = TS_TBN;
	vs_out.ObjectID = uObjectID;
}
