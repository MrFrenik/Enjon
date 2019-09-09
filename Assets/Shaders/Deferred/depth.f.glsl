#version 330 core

in DATA
{
	vec2 TexCoords;
}fs_in;

out vec4 Color;

uniform sampler2D uDepthMap;
uniform float near;
uniform float far;

float LinearizeDepth( float depth )
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() 
{
	float D = texture(uDepthMap, fs_in.TexCoords).r;
	float depth = LinearizeDepth( D ) / far;
	Color = vec4(vec3( depth ), 1.0);
}

