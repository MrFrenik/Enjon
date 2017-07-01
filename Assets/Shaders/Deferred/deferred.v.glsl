#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUV;

out VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 u_camera = mat4(1.0f);
uniform mat4 u_model;

void main()
{
	// ModelViewProjection matrix
	mat4 MVP = u_camera * u_model;

	vec3 pos = vec3(u_model * vec4(vertexPosition, 1.0)); 

	// Calculate vertex position in camera space
	gl_Position = u_camera * u_model * vec4(vertexPosition, 1.0);

	vec3 N = normalize( ( u_model * vec4( vertexNormal, 0.0 ) ).xyz );
	vec3 T = normalize( ( u_model * vec4( vertexTangent, 0.0 ) ).xyz );

	// Reorthogonalize with respect to N
	T = normalize(T - dot(T, N) * N);

	// Calculate bitangent
	// vec3 B = normalize( ( u_model * vec4( ( cross( vertexNormal, vertexTangent.xyz ) * 1.0), 0.0 )).xyz);
	vec3 B = cross( N, T );
	mat3 TBN = mat3(T, B, N);

	// Output data
	vs_out.FragPos = vec3(u_model * vec4(vertexPosition, 1.0));
	vs_out.TexCoords = vec2(vertexUV.x, -vertexUV.y);	
	vs_out.TBN = TBN;
}