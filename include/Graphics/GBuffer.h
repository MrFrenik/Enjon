#ifndef ENJON_GBUFFER_H
#define ENJON_GBUFFER_H

#include "Graphics/FrameBuffer.h"
#include "System/Types.h"
#include "GLEW/glew.h"
#include "Defines.h"
#include "Math/Vec4.h"

namespace Enjon {

	enum class GBufferTextureType
	{
		ALBEDO,
		NORMAL,
		EMISSIVE,
		MAT_PROPS,
		OBJECT_ID,
		VELOCITY,
		GBUFFER_TEXTURE_COUNT
	}; 

	ENJON_CLASS( )
	class GBuffer : public FrameBuffer
	{
		ENJON_CLASS_BODY( )

		public:
			/**
			*@brief
			*/
			GBuffer( ) = default;

			/**
			*@brief
			*/
			GBuffer(u32 _Width, u32 _Height);

			/**
			*@brief
			*/
			~GBuffer();

			/**
			*@brief
			*/
			virtual void Bind(BindType Type = BindType::WRITE, bool clear = true) override;

			/**
			*@brief
			*/
			virtual void Unbind();

			/**
			*@brief
			*/
			GLuint inline GetTexture(GBufferTextureType Type) 
			{ 
				return mTextures[(GLuint)Type]; 
			}

			/**
			*@brief
			*/
			GLuint inline GetTexture(u32 index) 
			{ 
				return mTextures[index]; 
			}

			/**
			*@brief
			*/
			GLuint inline GetDepth()
			{ 
				return mDepthTexture; 
			} 

			/**
			*@brief
			*/
			void SetViewport(const Vec4& Viewport); 
 
			/**
			*@brief
			*/
			const char* FrameBufferToString(u32 i);

		private:
			GLuint mTargetIDs[(u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT];
			GLuint mTextures[(u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT]; 
	}; 
}

#endif