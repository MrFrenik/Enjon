#include <cstddef>

#include "Graphics/Sprite.h"
#include "Graphics/Vertex.h"
#include "IO/ResourceManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {


	Sprite::Sprite()
		: m_vbo(0)
	{
	}


	Sprite::~Sprite()
	{
		//Delete buffers
		if (m_vbo != 0) {
			glDeleteBuffers(1, &m_vbo);
		}
	}

	//Initializes the sprite VBO. x, y, width, and height are
	//in the normalized device coordinate space. so, [-1, 1]
	void Sprite::Init(float x, float y, float width, float height, std::string texturePath) 
	{
		//Set up our private vars
		m_x = x;
		m_y = y;
		m_width = width;
		m_height = height;

		//Get texture from resource manager
		m_texture = Enjon::Input::ResourceManager::GetTexture(texturePath); 


		//Generate the buffer if it hasn't already been generated
		if (m_vbo == 0) glGenBuffers(1, &m_vbo); 

		//This array will hold our vertex data. We need 6 vertices, and each vertex has 2 floats for X and Y
		Vertex vertexData[6];

		//First Triangle
		SetPosition(vertexData[0], x + width, y + height);
		SetUV(vertexData[0], 1.0f, 1.0f);

		SetPosition(vertexData[1], x , y + height);
		SetUV(vertexData[1], 0.0f, 1.0f);

		SetPosition(vertexData[2], x , y);
		SetUV(vertexData[2], 0.0f, 0.0f);

		//Second Triangle
		SetPosition(vertexData[3], x , y);
		SetUV(vertexData[3], 0.0f, 0.0f);

		SetPosition(vertexData[4], x + width , y);
		SetUV(vertexData[4],1.0f, 0.0f);

		SetPosition(vertexData[5],x + width , y + height);
		SetUV(vertexData[5],1.0f, 1.0f);

		//Set all vertex colors to magenta
		for (int i = 0; i < 6; i++) {
		   SetColor(vertexData[i], 1.0f,0,1.0f,1.0f);
		} 

		//Tell opengl to bind our vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		//Upload the data to the GPU
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		//Unbind the buffer (optional)
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//Draws the sprite to the screen
	void Sprite::Draw() {

		glBindTexture( GL_TEXTURE_2D, m_texture.id );

		//bind the buffer object
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		
		//Tell opengl that we want to use our vertex attribute arrays
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		//This is the position attribute pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		//This is the color attribute pointer
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); 
		//This is the UV attribute pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv) );

		//Draw the 6 vertices to the screen
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Disable the vertex attrib array. This is not optional.
		glDisableVertexAttribArray(0);

		//Unbind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

}}