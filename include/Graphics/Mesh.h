#pragma once
#ifndef ENJON_MESH_H
#define ENJON_MESH_H

#include "System/Types.h"
#include "Defines.h"
#include "Math/Maths.h"
#include "Graphics/Vertex.h"
#include "Asset/Asset.h"

#include <vector>

namespace Enjon { 

	struct Vert
	{
		float Position[3];
		float Normals[3];
		float Tangent[3];
		float UV[2];	
	};

	class Mesh : public Asset
	{
		public:
			Mesh();
			~Mesh();

			void Bind();
			void Unbind();
			void Submit();

			std::vector<Vert> Verticies;
			std::vector<u32> Indicies;	
			GLenum DrawType;
			GLint DrawStart;
			GLint DrawCount;
			GLuint VAO;
			GLuint VBO;
			GLuint IBO;
	};

}


#endif