#ifndef SPRITEBATCH_H
#define SPRITEBATCH_H

#include "Graphics/Vertex.h"

#include "GLEW/glew.h"
#include "Math/Maths.h"
#include "System/Types.h"

#include <vector>
#include <algorithm> 

#define GL_VERTEX_ATTRIB_POSITION    0
#define GL_VERTEX_ATTRIB_COLOR       1
#define GL_VERTEX_ATTRIB_UV          2

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {

	enum class GlyphSortType { NONE, FRONT_TO_BACK, BACK_TO_FRONT, TEXTURE };

	struct Glyph 
	{ 
		GLuint texture;
		float depth; 
		Vertex topLeft;
		Vertex bottomLeft;
		Vertex topRight;
		Vertex bottomRight;
	};

	inline Glyph NewGlyph(const Enjon::Math::Vec4& destRect, const Enjon::Math::Vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color)
	{
		Glyph glyph;

		/* Set topLeft vertex */
		glyph.topLeft = NewVertex(destRect.x, destRect.y + destRect.w, uvRect.x, uvRect.y + uvRect.w, color.r, color.g, color.b, color.a);

		/* Set bottomLeft vertex */
		glyph.bottomLeft = NewVertex(destRect.x, destRect.y, uvRect.x, uvRect.y, color.r, color.g, color.b, color.a);

		/* Set bottomRight vertex */
		glyph.bottomRight = NewVertex(destRect.x + destRect.z, destRect.y, uvRect.x + uvRect.z, uvRect.y, color.r, color.g, color.b, color.a); 
		
		/* Set topRight vertex */
		glyph.topRight = NewVertex(destRect.x + destRect.z, destRect.y + destRect.w, uvRect.x + uvRect.z, uvRect.y + uvRect.w, color.r, color.g, color.b, color.a);

		/* Set texture */
		glyph.texture = texture;

		/* Set depth */
		glyph.depth = depth;

		return glyph;
	}

	// TODO(John)::Consider making this strictly POD struct
	struct RenderBatch 
	{
		//RenderBatch(GLuint Offset, GLuint NumVertices, GLuint Texture) : offset(Offset),
		//	numVertices(NumVertices), texture(Texture) {
		//}

		GLuint offset;
		GLuint numVertices;
		GLuint texture;
	};

	inline RenderBatch NewRenderBatch(GLuint Offset, GLuint NumVerticies, GLuint Texture)
	{
		RenderBatch renderbatch;

		renderbatch.offset = Offset;
		renderbatch.numVertices = NumVerticies;
		renderbatch.texture = Texture;

		return renderbatch;
	}

	// TODO(John)::Might even consider making this strictly POD
	class SpriteBatch
	{
	public:
		SpriteBatch();
		~SpriteBatch();

		void Init();

		void Begin(GlyphSortType sortType = GlyphSortType::TEXTURE);
		void End();

		void Add(const Enjon::Math::Vec4& destRect, const Enjon::Math::Vec4& uvRect, GLuint texture = 0, const ColorRGBA8& color = RGBA8(255), float depth = 0.0f);

		void RenderBatch();

		unsigned int inline GetRenderBatchesSize() const { return m_renderBatches.size(); }

	private:
		void CreateRenderBatches();
		void CreateVertexArray();
		void SortGlyphs();

		static bool CompareFrontToBack(Glyph* a, Glyph* b);
		static bool CompareBackToFront(Glyph* a, Glyph* b);
		static bool CompareTexture(Glyph* a, Glyph* b);

		GLuint m_vbo;
		GLuint m_vao;

		GlyphSortType m_sortType;

		std::vector<Glyph> m_glyphs;
		std::vector<Glyph*> m_glyphpointers;
		std::vector<Enjon::Graphics::RenderBatch> m_renderBatches;
	};
}} 

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
#endif