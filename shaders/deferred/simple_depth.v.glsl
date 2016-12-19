#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 LightSpaceMatrix;
uniform mat4 Model;

out DATA
{
	vec3 Position;	
	vec2 TexCoords;
}fs_out;

void main()
{             
    gl_Position = LightSpaceMatrix * Model * vec4(position, 1.0f);
    fs_out.TexCoords = texCoords;
    fs_out.Position = position;
}