#ifndef ENJON_GBUFFER_H
#define ENJON_GBUFFER_H

#include "System/Types.h"
#include "GLEW/glew.h"
#include "Defines.h"
#include "Math/Vec4.h"

namespace Enjon {

	enum class GBufferTextureType
	{
		ALBEDO,
		NORMAL,
		POSITION,
		EMISSIVE,
		MAT_PROPS,
		GBUFFER_TEXTURE_COUNT
	};

	enum class BindType
	{
		READ, 
		WRITE
	};

	class GBuffer
	{
	public:
		GBuffer(){}
		GBuffer(u32 _Width, u32 _Height);
		~GBuffer();

		void Bind(BindType Type = BindType::WRITE);
		void Unbind();

		GLuint inline GetTexture(GBufferTextureType Type) { return Textures[(GLuint)Type]; }
		GLuint inline GetTexture(u32 index) { return Textures[index]; }
		GLuint inline GetDepth() { return DepthBuffer; }
		Vec2 inline GetResolution() { return Vec2(Width, Height); }
		void SetViewport(const EM::Vec4& Viewport);

		u32 GetWidth() { return Width; }
		u32 GetHeight() { return Height; }

		const char* FrameBufferToString(u32 i);

	private:
		uint32 Width;
		uint32 Height;
		GLuint FBO;
		GLuint DepthBuffer;

		GLuint TargetIDs[(u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT];
		GLuint Textures[(u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT];

		EM::Vec4 Viewport;
	}; 
}

#endif