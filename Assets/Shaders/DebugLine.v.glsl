#version 330 core 

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;

out DATA
{
	vec3 position;
	vec3 color;
}fs_out;

uniform mat4 uProjection;
uniform mat4 uView;

void main() 
{
    gl_Position = uProjection * uView * vec4(vertexPosition, 1.0);

	fs_out.position = vertexPosition; 
	fs_out.color = vertexColor;
}

