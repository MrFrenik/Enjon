#ifndef ENJON_MESH_H
#define ENJON_MESH_H

#include <System/Types.h>
#include <Defines.h>
#include <Math/Maths.h>
#include <Graphics/Vertex.h>
#include <vector>

namespace Enjon { namespace Graphics { 

	struct Vert
	{
		float Position[3];
		float Normals[3];
		float Tangent[3];
		float Bitangent[3];
		float UV[2];	
		GLubyte Color[4];
	};

	struct MeshInstance
	{
		std::vector<Vert> Verticies;
		std::vector<u32> Indicies;	
		GLenum DrawType;
		GLint DrawStart;
		GLint DrawCount;
		GLuint VAO;
		GLuint VBO;
		GLuint IBO;
	};

}}


#endif