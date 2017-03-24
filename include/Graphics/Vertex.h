#ifndef ENJON_VERTEX_H
#define ENJON_VERTEX_H

#include <GLEW/glew.h>

#include "Graphics/Color.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon {

	struct Position2f 
	{ 
		Position2f() {}
		Position2f(float X, float Y)
			: x(X), y(Y)	
		{}

		float x;
		float y;
	}; 
	
	struct Position3f 
	{ 
		Position3f() {}
		Position3f(float X, float Y, float Z)
			: x(X), y(Y), z(Z)	
		{}

		float x;
		float y;
		float z;
	}; 

	struct UV
	{ 
		UV() {}
		UV(float U, float V)
			: u(U), v(V)
		{}

		float u;
		float v;
	};

	inline UV CreateUV(float U, float V)
	{
		UV uv;
		uv.u = U;
		uv.v = V;
		return uv;
	} 

	/* Strict POD struct for Vertex */
	struct Vertex 
	{ 
		Position2f position;
		ColorRGBA16 color;
		UV uv; 
	};

	inline Vertex NewVertex(GLfloat x, GLfloat y, GLfloat u, GLfloat v, GLfloat r,  GLfloat g, GLfloat b, GLfloat a)	
	{
		Vertex vertex;

		vertex.position.x = x;
		vertex.position.y = y;
		vertex.uv.u = u;
		vertex.uv.v = v;
		vertex.color.r = r;
		vertex.color.g = g;
		vertex.color.b = b;
		vertex.color.a = a;

		return vertex;
	}

	inline void SetPosition(Vertex& vertex, float x, float y)
	{
		vertex.position.x = x;
		vertex.position.y = y;
	}

	inline void SetUV(Vertex& vertex, float u, float v)
	{
		vertex.uv.u = u;
		vertex.uv.v = v;
	}

	inline void SetColor(Vertex& vertex, GLfloat r, GLfloat g, GLfloat b, GLfloat a )
	{
		vertex.color.r = r;
		vertex.color.g = g;
		vertex.color.b = b;
		vertex.color.a = a;
	}
	
	struct Vertex3 
	{
		Position3f position;
		ColorRGBA8 color;
		struct UV uv;

		void SetPosition(float x, float y, float z)
		{
			position.x = x;
			position.y = y;
			position.z = z;
		}

		void SetUV(float u, float v)
		{
			uv.u = u;
			uv.v = v;
		}

		void SetColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a )
		{
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
		}
	}; 

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
#endif