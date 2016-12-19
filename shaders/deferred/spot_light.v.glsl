#version 330 core

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexUV; 

out DATA
{
    vec2 TexCoords;
} fs_out;

void main()
{
    // Calculate vertex position in camera space
    gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);

    fs_out.TexCoords    = vec2(vertexUV.x, -vertexUV.y);
}