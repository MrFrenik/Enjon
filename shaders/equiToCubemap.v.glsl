#version 330 core
layout( location = 0 ) in vec3 aPos;

out vec3 localPos;

uniform mat4 camera = mat4(1.0);

void main()
{
	localPos = aPos;
	gl_Position = camera * vec4( localPos, 1.0 );
}
