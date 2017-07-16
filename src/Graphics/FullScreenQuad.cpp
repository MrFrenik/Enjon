#include "Graphics/FullScreenQuad.h"

namespace Enjon { 

	FullScreenQuad::FullScreenQuad()
	{
		const float quadVertices[ ] = {
			// positions  // texture Coords
			-1.0f,  1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays( 1, &mVAO );
		glGenBuffers( 1, &mVBO );
		glBindVertexArray( mVAO );
		glBindBuffer( GL_ARRAY_BUFFER, mVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), &quadVertices, GL_STATIC_DRAW );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), ( void* )0 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), ( void* )( 2 * sizeof( float ) ) );

		// Unbind vao 
		glBindVertexArray( 0 ); 
	}

	FullScreenQuad::~FullScreenQuad()
	{
		// Clean up mVAO
	}

	void FullScreenQuad::Bind()
	{
		glBindVertexArray(mVAO);
	}

	void FullScreenQuad::Unbind()
	{
		glBindVertexArray(0);
	}

	void FullScreenQuad::Submit()
	{
		Bind( );
		{
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ); 
		}
		Unbind( );
	}
			
}
