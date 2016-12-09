#ifndef ENJON_GBUFFER_H
#define ENJON_GBUFFER_H

#include "System/Types.h"
#include "GLEW/glew.h"
#include "Defines.h"
#include "Math/Vec4.h"

namespace Enjon { namespace Graphics {

	enum class GBufferTextureType
	{
		DIFFUSE,
		NORMAL,
		POSITION,
		EMISSIVE,
		DEPTH,
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
		GBuffer(uint32 _Width, uint32 _Height);
		~GBuffer();

		void Bind(BindType Type = BindType::WRITE);
		void Unbind();

		GLuint inline GetTexture(GBufferTextureType Type) { return Textures[(GLuint)Type]; }
		GLuint inline GetDepth() { return DepthBuffer; }
		EM::Vec2 inline GetResolution() { return EM::Vec2(Width, Height); }
		void SetViewport(const EM::Vec4& Viewport);

	private:
		uint32 Width;
		uint32 Height;
		GLuint FBO;
		GLuint DepthBuffer;

		GLuint TargetIDs[GBufferTextureType::GBUFFER_TEXTURE_COUNT];
		GLuint Textures[GBufferTextureType::GBUFFER_TEXTURE_COUNT];

		EM::Vec4 Viewport;
	};

}}

#endif