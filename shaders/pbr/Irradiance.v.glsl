#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection = mat4(1.0);
uniform mat4 view;

out vec3 worldPos;

void main()
{
    worldPos = aPos;
    gl_Position = projection * view * vec4( worldPos, 1.0 );
}