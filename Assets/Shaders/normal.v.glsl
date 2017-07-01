#version 330 core 
layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec4 vertexColor;
layout (location = 2) in vec2 vertexUV;

out DATA
{
	vec4 Position;
	vec2 TexCoords;
	vec4 Color;
} fs_out;

out vec4 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
 
    gl_Position = projection * view * model * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);
	fs_out.TexCoords = vec2(vertexUV.x, -vertexUV.y); 
	fs_out.Color = vertexColor;
	fs_out.Position = model * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);

	FragPos = view * model * vec4(vertexPosition.x, vertexPosition.y, 0.0f, 1.0f);
}


