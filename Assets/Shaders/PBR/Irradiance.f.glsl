#version 330 core
out vec4 fragColor;
in vec3 worldPos;

// From learnopengl.com irradiance_convolution.fs

uniform samplerCube envMap;

const float PI = 3.14159265359;

void main()
{
	// The world vector acts as the normal of a tangent surface from the world origin.
	// Given this normal, calculate all incoming radiance of the environment.
	vec3 n = normalize( worldPos );

	vec3 irradiance = vec3( 0.0 );	

	// Tangent space calculation from origin
	vec3 up = vec3( 0.0, 1.0, 0.0 );
	vec3 right = cross( up, n );
	up = cross( n, right );

	float sampleDelta = 0.025;
	float nSamples = 0.0;
	for ( float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta )
	{
		for ( float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta )
		{
			// Spherical to cartesian space( in tangent space )
			vec3 tangentSample = vec3( sin( theta ) * cos( phi ), sin( theta ) * sin( phi ), cos( theta ) );
			// Tangent to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * n;

			irradiance += texture( envMap, sampleVec ).rgb * cos( theta ) * sin( theta );
			nSamples++;
		}
	}

	irradiance = PI * irradiance * ( 1.0 / float( nSamples ) );
	fragColor = vec4( irradiance, 1.0 ); 
}