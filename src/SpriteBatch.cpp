#include "Graphics/SpriteBatch.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {

	SpriteBatch::SpriteBatch() : m_vbo(0), m_vao(0)
	{}


	SpriteBatch::~SpriteBatch()
	{}


	void SpriteBatch::Init() 
	{
		CreateVertexArray();
	}

	void SpriteBatch::Begin(GlyphSortType sortType /* = GlyphSortType::TEXTURE */) 
	{ 
		m_sortType = sortType;
		m_renderBatches.clear(); 

		// Clear glyphs
		m_glyphs.clear();
	}

	void SpriteBatch::End() 
	{
		// Resize glyph pointers to glyph vector size
		m_glyphpointers.resize(m_glyphs.size()); 

		// Set up glyph pointers to point to corresponding glyphs
		for (uint32_t i = 0; i < m_glyphs.size(); i++)
		{	
			m_glyphpointers[i] = &m_glyphs[i];
		}
		
		// Sort our glyphs according to sorting method provided (default is texture)
		SortGlyphs(); 

		// Create the renderbatches from the glyphs
		CreateRenderBatches();
	} 

	void SpriteBatch::Add(const Enjon::Math::Vec4& destRect, const Enjon::Math::Vec4& uvRect, GLuint texture, const ColorRGBA8& color /* = ColorRGBA8(255) */, float depth /* = 0.0f ) */ )
	{ 
		// Place back new glyph
		m_glyphs.emplace_back(NewGlyph(destRect, uvRect, texture, depth, color));
	}

	/* Adds glpyh to spritebatch to be rendered with specified rotation */
	void SpriteBatch::Add(const Enjon::Math::Vec4& destRect, const Enjon::Math::Vec4& uvRect, GLuint texture, const ColorRGBA8& color, float depth, float angle, Graphics::CoordinateFormat format)
	{
		// Place back new glyph
		m_glyphs.emplace_back(NewGlyph(destRect, uvRect, texture, depth, color, angle, format));
	}

	void SpriteBatch::RenderBatch() 
	{ 
		// Bind our VAO. This sets up the opengl state we need, including the vertex attribute pointers and binds the VBO 
		glBindVertexArray(m_vao);

		for (unsigned int i = 0; i < m_renderBatches.size(); i++) 
		{
			// Bind the texture of the renderbatch 
			glBindTexture(GL_TEXTURE_2D, m_renderBatches[i].texture);


			// Draw our object 
			glDrawArrays(GL_TRIANGLES, m_renderBatches[i].offset, m_renderBatches[i].numVertices);
		}

		// Unbind the VAO 
		glBindVertexArray(0);
	}
	

	void SpriteBatch::CreateRenderBatches() 
	{
		// This will store all the vertices that we need to upload 
		std::vector<Vertex> vertices;

		// Resize the buffer to the exact size we need so we can treat it like an array 
		vertices.resize(m_glyphpointers.size() * 6);

		// Return if glyphs are empty 
		if (m_glyphpointers.empty()) {
			return;
		}

		int offset = 0; // current offset
		int cv = 0; // current vertex

		// Add the first batch (would be even faster with an ebo) 
		// TODO(John)::Implement EBO 
		m_renderBatches.emplace_back(NewRenderBatch(offset, 6, m_glyphpointers[0]->texture));
		vertices[cv++] = m_glyphpointers[0]->topLeft;
		vertices[cv++] = m_glyphpointers[0]->bottomLeft;
		vertices[cv++] = m_glyphpointers[0]->bottomRight;
		vertices[cv++] = m_glyphpointers[0]->bottomRight;
		vertices[cv++] = m_glyphpointers[0]->topRight;
		vertices[cv++] = m_glyphpointers[0]->topLeft;
		offset += 6;

		// Add all the rest of the glyphs 
		for (unsigned int cg = 1; cg < m_glyphpointers.size(); cg++) 
		{ 
			// Check if this glyph can be part of the current batch 
			if (m_glyphpointers[cg]->texture != m_glyphpointers[cg - 1]->texture) 
			{
				// Make a new batch 
				m_renderBatches.emplace_back(NewRenderBatch(offset, 6, m_glyphpointers[cg]->texture));
			} 
			else 
			{
				// If it's part of the current batch, just increase numVertices 
				m_renderBatches.back().numVertices += 6;
			}

			vertices[cv++] = m_glyphpointers[cg]->topLeft;
			vertices[cv++] = m_glyphpointers[cg]->bottomLeft;
			vertices[cv++] = m_glyphpointers[cg]->bottomRight;
			vertices[cv++] = m_glyphpointers[cg]->bottomRight;
			vertices[cv++] = m_glyphpointers[cg]->topRight;
			vertices[cv++] = m_glyphpointers[cg]->topLeft;
			offset += 6;
		}

		// Bind our VBO (what makes this fast is that we only have to bind our vbo ONCE for all this vertex information) 
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		// Orphan the buffer (for speed) 
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		// Upload the data 
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		// Unbind the VBO 
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}

	void SpriteBatch::CreateVertexArray() {

		// Generate the VAO if it isn't already generated 
		if (m_vao == 0) {
			glGenVertexArrays(1, &m_vao);
		}
		
		// Bind the VAO. All subsequent opengl calls will modify its state.
		glBindVertexArray(m_vao);

		// Generate the VBO if it isn't already generated 
		if (m_vbo == 0) {
			glGenBuffers(1, &m_vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		// Tell opengl what attribute arrays we need 
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// This is the position attribute pointer 
		glVertexAttribPointer(GL_VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		// This is the color attribute pointer 
		glVertexAttribPointer(GL_VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		// This is the UV attribute pointer 
		glVertexAttribPointer(GL_VERTEX_ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		// Unbind the VAO 
		glBindVertexArray(0);

	}

	void SpriteBatch::SortGlyphs() 
	{ 
		switch (m_sortType) {
			case GlyphSortType::BACK_TO_FRONT:
				std::stable_sort(m_glyphpointers.begin(), m_glyphpointers.end(), CompareBackToFront);
				break;
			case GlyphSortType::FRONT_TO_BACK:
				std::stable_sort(m_glyphpointers.begin(), m_glyphpointers.end(), CompareFrontToBack);
				break;
			case GlyphSortType::TEXTURE:
				std::stable_sort(m_glyphpointers.begin(), m_glyphpointers.end(), CompareTexture);
				break;
		}
	}

	bool SpriteBatch::CompareFrontToBack(Glyph* a, Glyph* b) 
	{
		return (a->depth < b->depth);
	}

	bool SpriteBatch::CompareBackToFront(Glyph* a, Glyph* b)
	{
		return (a->depth > b->depth);
	}

	bool SpriteBatch::CompareTexture(Glyph* a, Glyph* b) 
	{
		return (a->texture < b->texture);
	}

}}
