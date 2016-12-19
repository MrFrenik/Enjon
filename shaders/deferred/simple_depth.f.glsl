#version 330 core

layout (location = 0) out vec4 depth;  	// Diffuse

in DATA
{
	vec3 Position;	
	vec2 TexCoords;
}fs_in;

uniform float Near;
uniform float Far;

float LinearizeDepth(float Depth)
{
    float z = Depth * 2.0 - 1.0;
    return (2.0 * Near * Far) / (Far + Near - z * (Far - Near));
}

void main()
{
    float Depth = LinearizeDepth(gl_FragCoord.z);
    depth = vec4(Depth, Depth, Depth, 1.0);
}