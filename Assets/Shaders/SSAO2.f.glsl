#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
 
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D uDepthMap;
uniform float near;
uniform float far;

float LinearizeDepth( float depth )
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return ( 2.0 * near * far ) / ( far + near - z * ( far - near ) );
}

vec2 saturate(vec2 input)
{
	return clamp(input, 0.0, 1.0);
}

void main()
{ 
  FragColor = vec4(1.0, 1.0, 1.0, 1.0); 
}