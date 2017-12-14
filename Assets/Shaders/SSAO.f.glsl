#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D uDepthMap;

uniform vec2 uScreenResolution;

uniform vec3 samples[16];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 16;
uniform float radius = 0.5;
uniform float bias = 0.025; 
uniform float uIntensity = 1.0;

uniform mat4 view;
uniform mat4 projection;

uniform float near;
uniform float far;

float LinearizeDepth( float depth )
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	// tile noise texture over screen based on screen dimensions divided by noise size
	vec2 noiseScale = vec2(uScreenResolution.x/256.0, uScreenResolution.y/256.0); 
	
	float depth = LinearizeDepth( texture( uDepthMap, TexCoords ).r ) / far; 

    // get input for SSAO algorithm
    vec4 fp = view * vec4( texture(gPosition, TexCoords).rgb, 1.0 );
	vec3 fragPos = fp.xyz / fp.w;
	
    //vec4 n = view * vec4( texture(gNormal, TexCoords).rgb * 2.0 - 1.0, 1.0 );
	//vec3 normal = normalize( n.xyz / n.w ); 
	vec3 normal = normalize( texture( gNormal, TexCoords ).rgb * 2.0 - 1.0 );

	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	// create TBN change-of-basis matrix: from tangent-space to view-space
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// iterate over the sample kernel and calculate occlusion factor
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 sample = TBN * samples[i]; // from tangent to view-space 
		sample = fragPos + sample * radius; 
		
		// project sample position (to sample texture) (to get position on screen/texture)
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		
		// get sample depth
		vec4 sd = view * texture(gPosition, offset.xy);
		float sampleDepth = ( sd.xyz / sd.w ).z;
		
		// range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
	}
	occlusion = 1.0 - (occlusion / kernelSize); 
	occlusion = pow(occlusion, uIntensity);
	FragColor = depth >= 0.98 ? vec4( 1.0 ) : vec4( vec3( occlusion ), 1.0 );
}