// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: FrameBuffer.h

#ifndef ENJON_FRAME_BUFFER_H
#define ENJON_FRAME_BUFFER_H
#pragma once

#include "System/Types.h"
#include "Math/Vec4.h"
#include "GLEW/glew.h"
#include "Defines.h"
#include "Graphics/Color.h"
#include "Base/Object.h"

namespace Enjon 
{ 
	enum class BindType
	{
		READ, 
		WRITE	
	};
 
	ENJON_CLASS( )
	class FrameBuffer : public Object
	{
		ENJON_CLASS_BODY( FrameBuffer )

		 public:

			/**
			* @brief
			*/
			virtual void ExplicitConstructor( ) override;

			/**
			* @brief
			*/
			FrameBuffer(u32 width, u32 height, GLuint texParam = GL_LINEAR);

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( ) override;

			/**
			* @brief
			*/
			virtual void Bind(BindType type = BindType::WRITE, bool clear = true);

			/**
			* @brief
			*/
			virtual void Unbind();

			/**
			* @brief
			*/
			GLuint inline GetTexture() const 	
			{ 
				return mTexture; 
			}

			/**
			* @brief
			*/
			GLuint inline GetDepthBuffer() const 		
			{ 
				return mDepthBuffer; 
			}

			/**
			* @brief
			*/
			Vec2 inline GetResolution() 		
			{ 
				return Vec2(mWidth, mHeight); 
			}

			void SetClearColor( const ColorRGBA32& color );

		protected:
			GLuint		mFrameBufferID; // The FBO ID
			GLuint		mTargetID; 		// The texture id
			GLuint		mTexture; 		// The OpenGL texture for the diffuse render target
			GLuint		mDepthBuffer; 	// Depth buffer handle
			GLuint		mDepthTexture; 	// Depth texture id
			u32			mWidth; 		// FBO width
			u32			mHeight; 		// FBO height
			Vec4		mViewport;
			ColorRGBA32 mClearColor = RGBA32_Black();
	};

}



#endif