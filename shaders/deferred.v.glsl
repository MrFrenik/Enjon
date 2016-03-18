#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out DATA
{
	vec2 Position;	
	vec2 TexCoords;
}fs_out;

void main()
{             
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
    fs_out.TexCoords = texCoords;
    fs_out.Position = position;
}