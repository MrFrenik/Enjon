#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec4 vertexColor;
layout (location = 2) in vec2 vertexUV;

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

out DATA
{
	vec3 position;
	vec4 color;
	vec2 uv;	
} fs_out;

uniform mat4 camera = mat4(1.0f);
uniform Transform transform;

void main()
{
	// Transform vertex position
	vec3 pos = vertexPosition;
	pos = transform.scale * pos;
	pos = quaternionRotate(transform.orientation, pos);
	pos = transform.position + pos;

	// Calculate vertex position
	gl_Position = camera * vec4(pos, 1.0);

	// Output data
	fs_out.position = pos;
	fs_out.color = vec4(vertexColor.x, vertexColor.y, vertexColor.z, vertexColor.w);
	fs_out.uv = vec2(vertexUV.x, -vertexUV.y);
}