#version 330 core
out vec4 FragColor;

#define KERNEL_SIZE 16

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D uDepthMap;

uniform vec3 samples[KERNEL_SIZE];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = KERNEL_SIZE;

uniform float radius = 0.5;
uniform float bias = 0.025;
uniform float uIntensity = 1.0;

// tile noise texture over screen based on screen dimensions divided by noise size

uniform mat4 view;
uniform mat4 projection;
uniform vec2 uScreenResolution;
uniform float near;
uniform float far;

float LinearizeDepth( float depth )
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return ( 2.0 * near * far ) / ( far + near - z * ( far - near ) );
}

void main( )
{
	mat4 inverseProjection = inverse( projection );

	vec3 sample_sphere[KERNEL_SIZE] = vec3[](
		  vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
		  vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
		  vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
		  vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
		  vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
		  vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
		  vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
		  vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
	);

	vec2 noiseScale = vec2( uScreenResolution.x / 128.0, uScreenResolution.y / 128.0 );

	// get input for SSAO algorithm
	vec3 fragPos = ( view * texture( gPosition, TexCoords ) ).xyz;
	vec3 normal = normalize( ( view * texture( gNormal, TexCoords ) ).rgb );
	vec3 randomVec = normalize( ( texture( texNoise, TexCoords * noiseScale ) ).xyz );

	float depth = LinearizeDepth( texture( uDepthMap, TexCoords ).r ) / far;

	// create TBN change-of-basis matrix: from tangent-space to view-space
	vec3 tangent = normalize( randomVec - normal * dot( randomVec, normal ) );
	vec3 bitangent = cross( normal, tangent );
	mat3 TBN = mat3( tangent, bitangent, normal );

	// iterate over the sample kernel and calculate occlusion factor
	float occlusion = 0.0;
	for ( int i = 0; i < KERNEL_SIZE; ++i )
	{
		// get sample position
		vec3 sample = TBN * sample_sphere[i]; // from tangent to view-space
		sample = fragPos + sample * radius;

		// project sample position (to sample texture) (to get position on screen/texture)
		vec4 offset = vec4( sample, 1.0 );
		offset = projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		 // get sample depth
		float sampleDepth = ( view * texture( gPosition, offset.xy ) ).z; 

		// range check & accumulate
		float rangeCheck = smoothstep( 0.0, 1.0, radius / abs( fragPos.z - sampleDepth ) );
		occlusion += ( sampleDepth >= sample.z + bias ? 1.0 : 0.0 ) * rangeCheck;
	}
	occlusion = 1.0 - ( occlusion / kernelSize );
	occlusion = max( 0.0, pow( occlusion, uIntensity ) );

	FragColor = depth >= 0.98 ? vec4( 1.0 ) : vec4( vec3( occlusion ), 1.0 );
}