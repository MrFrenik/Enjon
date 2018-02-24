#version 330 core 

#define MAX_SAMPLES 16

// Texture samplers
uniform sampler2D uInputTextureMap;
uniform sampler2D uVelocityMap; 

// Uniforms
uniform float uVelocityScale = 1.0;
uniform bool uEnabled = true;
 
in DATA
{
   vec2 Position; 
   vec2 TexCoords;
}fs_in;

// Final output fragment color
out vec4 Color; 
 
void main( void ) 
{ 
	if ( uEnabled )
	{
		vec2 texelSize = 1.0 / vec2( textureSize( uInputTextureMap, 0 ) );
		vec2 screenTexCoords = gl_FragCoord.xy * texelSize;

		vec2 velocity = texture( uVelocityMap, screenTexCoords ).rg;
		velocity *= uVelocityScale;

		// Calculate speed
		float speed = length( velocity / texelSize );
		int nSamples = clamp( int( speed ), 1, MAX_SAMPLES );

		Color = texture( uInputTextureMap, screenTexCoords );
		for ( int i = 1; i < nSamples; ++i ) 
		{
			vec2 offset = velocity * ( float( i ) / float( nSamples - 1 ) - 0.5 );
			Color += texture( uInputTextureMap, screenTexCoords + offset );
		}

		Color /= float( nSamples );
		Color.a = 1.0; 
	}
	else
	{
		vec2 texelSize = 1.0 / vec2( textureSize( uInputTextureMap, 0 ) );
		vec2 screenTexCoords = gl_FragCoord.xy * texelSize;
		Color = texture( uInputTextureMap, screenTexCoords );
	}
}
