#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec3 vertexBitangent;
layout (location = 4) in vec2 vertexUV;
layout (location = 5) in vec4 vertexColor;

out VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
    mat3 TBN;
    vec4 FragColor;
} vs_out;

uniform mat4 camera = mat4(1.0f);

void main()
{
	// Calculate vertex position in camera space
	gl_Position = camera * vec4(vertexPosition, 1.0);

	// TBN matrix
	mat3 TBN = mat3(vertexTangent, vertexBitangent, vertexNormal);

	// Output data
	vs_out.FragPos = vertexPosition;
	vs_out.TBN = TBN;
	vs_out.TexCoords = vec2(vertexUV.x, -vertexUV.y);
}