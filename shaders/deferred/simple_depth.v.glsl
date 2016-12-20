#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 uv;

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
    fs_out.TexCoords = vec2(uv.x, -uv.y);
    fs_out.Position = position;
}