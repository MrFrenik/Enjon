#version 330

// Adapted from hlsl to glsl https://www.gamedev.net/articles/programming/graphics/a-simple-and-practical-approach-to-ssao-r2753

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D uDepthMap;

float random_size = 128.0;

uniform float radius = 0.5;
uniform float bias = 0.025;
uniform float uIntensity = 1.0;
uniform float uScale = 1.0;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 uProjMatrixInv;
uniform mat4 uViewMatrixInv;
uniform vec2 uScreenResolution;
uniform vec3 uCameraPosition;
uniform float uNear;
uniform float uFar;

// this is supposed to get the world position from the depth buffer
vec3 WorldPosFromDepth(float depth, vec2 TexCoord) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uProjMatrixInv * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = uViewMatrixInv * viewSpacePosition;

    return worldSpacePosition.xyz;
}

vec3 getNormal(vec2 uv)
{
	return normalize( (texture(gNormal, uv) ).xyz);
}

vec2 getRandom(vec2 uv)
{
	return normalize( ( texture(texNoise, uScreenResolution * uv / random_size ) ).xy);
}

float LinearizeDepth( float depth )
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return ( 2.0 * uNear * uFar ) / ( uFar + uNear - z * ( uFar - uNear ) );
}

float doAmbientOcclusion(vec2 tcoord, vec2 uv, vec3 p, vec3 cnorm, float depth)
{
	float b = bias * depth;
	float sampleDepth = texture( uDepthMap, tcoord + uv ).r;
	vec3 samplePos = WorldPosFromDepth( sampleDepth, tcoord + uv );
	vec3 diff = samplePos - p;
	vec3 v = normalize( diff );
	float d = length( diff ) * uScale;
	return max( 0.0, dot( cnorm, v ) - b ) * ( 1.0 / ( 1.0 + d ) );
}

void main()
{ 
	const vec2 vec[4] = vec2[](vec2(1,0),vec2(-1,0), vec2(0,1),vec2(0,-1));

	vec3 n = getNormal(TexCoords);
	vec2 rand = getRandom(TexCoords); 
	float depth = texture( uDepthMap, TexCoords ).r;
	vec3 p = WorldPosFromDepth( depth, TexCoords );

	float ao = 0.0f;
	float rad = radius / ( depth + 0.00001 );

	//**SSAO Calculation**//
	int kernelSize = 4 * 4;
	int iterations = 4;
	for (int j = 0; j < iterations; ++j)
	{
	  vec2 coord1 = reflect( vec[j], rand ) * rad;
	  vec2 coord2 = vec2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);
	  
	  ao += doAmbientOcclusion(TexCoords,coord1*0.25, p, n, depth);
	  ao += doAmbientOcclusion(TexCoords,coord2*0.5, p, n, depth);
	  ao += doAmbientOcclusion(TexCoords,coord1*0.75, p, n, depth);
	  ao += doAmbientOcclusion(TexCoords,coord2, p, n, depth);
	}
	ao = 1.0 - ( ao / kernelSize );
	ao = clamp( pow( ao, uIntensity ), 0.0, 1.0 );
	FragColor = vec4(vec3(ao, ao, ao),  1.0);
}