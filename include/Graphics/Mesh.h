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

	ENJON_CLASS( Construct )
	class Mesh : public Asset
	{
		friend MeshAssetLoader;

		ENJON_CLASS_BODY( )

		public:
			/*
			* @brief Constructor
			*/
			Mesh();

			/*
			* @brief Destructor
			*/
			~Mesh();

			/*
			* @brief
			*/
			void Bind() const;

			/*
			* @brief
			*/
			void Unbind() const;

			/* 
			* @brief 
			*/
			void Submit() const; 

			/*
			* @brief
			*/
			u32 GetDrawCount( ) const; 

			/*
			* @brief
			*/
			u32 GetVAO( ) const;

			/*
			* @brief
			*/
			u32 GetVBO( ) const;

			/*
			* @brief
			*/
			u32 GetIBO( ) const;

		protected:

			/*
			* @brief Protected Constructor
			*/
			Mesh( const Enjon::String& filePath );

			/*
			* @brief
			*/
			Result Release( ); 

		public:
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

		protected: 
			Vector< Vert > mVerticies;

			Vector< u32 > mIndicies;	

			GLenum mDrawType;
			GLint mDrawStart = 0;
			GLint mDrawCount = 0;
			GLuint mVAO = 0;
			GLuint mVBO = 0;
			GLuint mIBO = 0;
	}; 
}


#endif