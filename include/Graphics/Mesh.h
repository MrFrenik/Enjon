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

	class MeshAssetLoader;

	class Mesh : public Asset
	{
		friend MeshAssetLoader;

		ENJON_OBJECT( Mesh )

		public:
			Mesh();
			~Mesh();

			void Bind();
			void Unbind();
			void Submit(); 

			std::vector<Vert> Verticies;
			std::vector<u32> Indicies;	

			GLenum DrawType;
			GLint DrawStart = 0;
			GLint DrawCount = 0;
			GLuint VAO = 0;
			GLuint VBO = 0;
			GLuint IBO = 0;
	}; 
}


#endif