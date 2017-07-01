#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec3 vertexBitangent;
layout (location = 4) in vec2 vertexUV;

struct Transform
{
	vec3 position;
	vec4 orientation;
	vec3 scale;
};

vec3 quaternionRotate(vec4 q, vec3 v)
{
	vec3 t = 2.0 * cross(q.xyz, v);
	return (v + q.w * t + cross(q.xyz, t)); 
}

out VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentLightPos;
	vec3 TangentLightDir; 
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	vec3 TangentViewDir;
} vs_out;

uniform mat4 camera = mat4(1.0f);
uniform mat4 model;
uniform vec3 lightPosition;
uniform vec3 viewPos;
uniform vec3 viewDir;

void main()
{
	// ModelViewProjection matrix
	mat4 MVP = camera * model;

	vec3 pos = vec3(model * vec4(vertexPosition, 1.0)); 

	// Calculate vertex position in camera space
	gl_Position = camera * model * vec4(vertexPosition, 1.0);

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * vertexTangent);
	vec3 N = normalize(normalMatrix * vertexNormal);

	// Reorthogonalize with respect to N
	T = normalize(T - dot(T, N) * N);

	// Calculate bitangent
	vec3 B = cross(T, N);
	mat3 TBN = transpose(mat3(T, B, N));

	// Output data
	vs_out.FragPos = vec3(model * vec4(vertexPosition, 1.0));
	vs_out.TexCoords = vec2(vertexUV.x, -vertexUV.y);	
	vs_out.TangentLightDir = TBN * (lightPosition - vs_out.FragPos);
	vs_out.TangentLightPos 	= TBN * lightPosition; 					// Bring light position into tangent space
	vs_out.TangentViewPos 	= TBN * viewPos;						// Bring camera view position into tangent space 
	vs_out.TangentFragPos 	= TBN * vs_out.FragPos;					// Bring fragment position into tangent space
	vs_out.TangentViewDir 	= TBN * viewDir;
}