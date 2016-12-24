#include "Graphics/QuadBatch.h"
#include "IO/ResourceManager.h"
#include <stdio.h>

#include <algorithm>

namespace Enjon { namespace Graphics {

	static EM::Vec3 Static_TL(-1.0f, 1.0f, 0.0f);
	static EM::Vec3 Static_BL(-1.0f, -1.0f, 0.0f);
	static EM::Vec3 Static_TR(1.0f, 1.0f, 0.0f);
	static EM::Vec3 Static_BR(1.0f, -1.0f, 0.0f);
	static EM::Vec3 Static_Normal(0.0f, 0.0f, 1.0f);
	static EM::Vec3 Static_Tangent(1.0f, 0.0f, 0.0f);
	static EM::Vec3 Static_BiTangent(0.0f, 1.0f, 0.0f);

	u32 QuadBatch::DrawCallCount = 0;
	u32 QuadBatch::DrawCallCountID = 1;

	QuadBatch::QuadBatch() : VAO(0), VBO(0)
	{
	}

	QuadBatch::~QuadBatch()
	{
	}

	QuadGlyph::QuadGlyph(EM::Vec2& Dimensions, EM::Transform& Transform, EM::Vec4& UVRect, GLuint _Texture, EG::ColorRGBA16& Color, float _Depth)
	:	Texture(_Texture), Depth(_Depth) 	
	{
		// Transform all verticies by model matrix
		EM::Mat4 Model;
		// L = T*R*S
		Model *= EM::Mat4::Translate(Transform.Position);
		Model *= EM::QuaternionToMat4(Transform.Orientation);
		Model *= EM::Mat4::Scale(Transform.Scale);

		EM::Vec4 Position, Normal, Tangent, Bitangent;
		EM::Vec3 T, B, N, P;
		EM::Vec2 UV;

		// Normal for all verticies
		Normal = Model * EM::Vec4(Static_Normal, 0.0);
		N = EM::Vec3::Normalize(Normal.XYZ());

		// Tangent for all verticies
		Tangent = Model * EM::Vec4(Static_Tangent, 0.0);
		T = EM::Vec3::Normalize(Tangent.XYZ());

		// Reorthogonalize with respect to N
		T = EM::Vec3::Normalize(T - T.Dot(N) * N);

		// Calculate Bitangent
		B = EM::Vec3::Normalize( ( Model * EM::Vec4( ( N.Cross(T) * 1.0), 0.0 )).XYZ());

		// Create basis verticies
		EM::Vec3 Basis_TL(-1.0f, 1.0f + Dimensions.y, 0.0f);
		EM::Vec3 Basis_BL(-1.0f, -1.0f, 0.0f);
		EM::Vec3 Basis_TR(1.0f + Dimensions.x, 1.0f + Dimensions.y, 0.0f);
		EM::Vec3 Basis_BR(1.0f + Dimensions.x, -1.0f, 0.0f);

		/* Set top left vertex */
 		Position 			= Model * EM::Vec4(Basis_TL, 1.0);
 		P 					= Position.XYZ() / Position.w;
		TL.Position[0] 		= P.x;
		TL.Position[1] 		= P.y;
		TL.Position[2] 		= P.z;
		TL.Normal[0] 		= N.x;
		TL.Normal[1] 		= N.y;
		TL.Normal[2] 		= N.z;
		TL.Tangent[0] 		= T.x;
		TL.Tangent[1] 		= T.y;
		TL.Tangent[2] 		= T.z;
		TL.Bitangent[0]		= B.x;
		TL.Bitangent[1]		= B.y;
		TL.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x, UVRect.y + UVRect.w);
		TL.UV[0] 			= UV.x; 
		TL.UV[1] 			= UV.y; 
		TL.Color[0] 		= Color.r;
		TL.Color[1] 		= Color.g;
		TL.Color[2] 		= Color.b;
		TL.Color[3] 		= Color.a;
		TL.ID 				= QuadBatch::DrawCallCountID;

		/* Set bottom left vertex */
 		Position 			= Model * EM::Vec4(Basis_BL, 1.0);
 		P 					= Position.XYZ() / Position.w;
		BL.Position[0] 		= P.x;
		BL.Position[1] 		= P.y;
		BL.Position[2] 		= P.z;
		BL.Normal[0] 		= N.x;
		BL.Normal[1] 		= N.y;
		BL.Normal[2] 		= N.z;
		BL.Tangent[0] 		= T.x;
		BL.Tangent[1] 		= T.y;
		BL.Tangent[2] 		= T.z;
		BL.Bitangent[0]		= B.x;
		BL.Bitangent[1]		= B.y;
		BL.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x, UVRect.y);
		BL.UV[0] 			= UV.x; 
		BL.UV[1] 			= UV.y; 
		BL.Color[0] 		= Color.r;
		BL.Color[1] 		= Color.g;
		BL.Color[2] 		= Color.b;
		BL.Color[3] 		= Color.a;
		BL.ID 				= QuadBatch::DrawCallCountID;

		/* Set top right vertex */
 		Position 			= Model * EM::Vec4(Basis_TR, 1.0);
 		P 					= Position.XYZ() / Position.w;
		TR.Position[0] 		= P.x;
		TR.Position[1] 		= P.y;
		TR.Position[2] 		= P.z;
		TR.Normal[0] 		= N.x;
		TR.Normal[1] 		= N.y;
		TR.Normal[2] 		= N.z;
		TR.Tangent[0] 		= T.x;
		TR.Tangent[1] 		= T.y;
		TR.Tangent[2] 		= T.z;
		TR.Bitangent[0]		= B.x;
		TR.Bitangent[1]		= B.y;
		TR.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x + UVRect.z, UVRect.y + UVRect.w);
		TR.UV[0] 			= UV.x; 
		TR.UV[1] 			= UV.y; 
		TR.Color[0] 		= Color.r;
		TR.Color[1] 		= Color.g;
		TR.Color[2] 		= Color.b;
		TR.Color[3] 		= Color.a;
		TR.ID 				= QuadBatch::DrawCallCountID;

		/* Set bottom right vertex */
 		Position 			= Model * EM::Vec4(Basis_BR, 1.0);
 		P 					= Position.XYZ() / Position.w;
		BR.Position[0] 		= P.x;
		BR.Position[1] 		= P.y;
		BR.Position[2] 		= P.z;
		BR.Normal[0] 		= N.x;
		BR.Normal[1] 		= N.y;
		BR.Normal[2] 		= N.z;
		BR.Tangent[0] 		= T.x;
		BR.Tangent[1] 		= T.y;
		BR.Tangent[2] 		= T.z;
		BR.Bitangent[0]		= B.x;
		BR.Bitangent[1]		= B.y;
		BR.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x + UVRect.z, UVRect.y);
		BR.UV[0] 			= UV.x; 
		BR.UV[1] 			= UV.y; 
		BR.Color[0] 		= Color.r;
		BR.Color[1] 		= Color.g;
		BR.Color[2] 		= Color.b;
		BR.Color[3] 		= Color.a;
		BR.ID 				= QuadBatch::DrawCallCountID;
	}

	QuadGlyph::QuadGlyph(EM::Transform& Transform, EM::Vec4& UVRect, GLuint _Texture, EG::ColorRGBA16& Color, float _Depth)
	:	Texture(_Texture), Depth(_Depth)
	{
		// Transform all verticies by model matrix
		EM::Mat4 Model;
		// L = T*R*S
		Model *= EM::Mat4::Translate(Transform.Position);
		Model *= EM::QuaternionToMat4(Transform.Orientation);
		Model *= EM::Mat4::Scale(Transform.Scale);

		EM::Vec4 Position, Normal, Tangent, Bitangent;
		EM::Vec3 T, B, N, P;
		EM::Vec2 UV;

		// Normal for all verticies
		Normal = Model * EM::Vec4(Static_Normal, 0.0);
		N = EM::Vec3::Normalize(Normal.XYZ());

		// Tangent for all verticies
		Tangent = Model * EM::Vec4(Static_Tangent, 0.0);
		T = EM::Vec3::Normalize(Tangent.XYZ());

		// Reorthogonalize with respect to N
		T = EM::Vec3::Normalize(T - T.Dot(N) * N);

		// Calculate Bitangent
		B = EM::Vec3::Normalize( ( Model * EM::Vec4( ( N.Cross(T) * 1.0), 0.0 )).XYZ());

		/* Set top left vertex */
 		Position 			= Model * EM::Vec4(Static_TL, 1.0);
 		P 					= Position.XYZ() / Position.w;
		TL.Position[0] 		= P.x;
		TL.Position[1] 		= P.y;
		TL.Position[2] 		= P.z;
		TL.Normal[0] 		= N.x;
		TL.Normal[1] 		= N.y;
		TL.Normal[2] 		= N.z;
		TL.Tangent[0] 		= T.x;
		TL.Tangent[1] 		= T.y;
		TL.Tangent[2] 		= T.z;
		TL.Bitangent[0]		= B.x;
		TL.Bitangent[1]		= B.y;
		TL.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x, UVRect.y + UVRect.w);
		TL.UV[0] 			= UV.x; 
		TL.UV[1] 			= UV.y; 
		TL.Color[0] 		= Color.r;
		TL.Color[1] 		= Color.g;
		TL.Color[2] 		= Color.b;
		TL.Color[3] 		= Color.a;
		TL.ID 				= QuadBatch::DrawCallCountID;

		/* Set bottom left vertex */
 		Position 			= Model * EM::Vec4(Static_BL, 1.0);
 		P 					= Position.XYZ() / Position.w;
		BL.Position[0] 		= P.x;
		BL.Position[1] 		= P.y;
		BL.Position[2] 		= P.z;
		BL.Normal[0] 		= N.x;
		BL.Normal[1] 		= N.y;
		BL.Normal[2] 		= N.z;
		BL.Tangent[0] 		= T.x;
		BL.Tangent[1] 		= T.y;
		BL.Tangent[2] 		= T.z;
		BL.Bitangent[0]		= B.x;
		BL.Bitangent[1]		= B.y;
		BL.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x, UVRect.y);
		BL.UV[0] 			= UV.x; 
		BL.UV[1] 			= UV.y; 
		BL.Color[0] 		= Color.r;
		BL.Color[1] 		= Color.g;
		BL.Color[2] 		= Color.b;
		BL.Color[3] 		= Color.a;
		BL.ID 				= QuadBatch::DrawCallCountID;


		/* Set top right vertex */
 		Position 			= Model * EM::Vec4(Static_TR, 1.0);
 		P 					= Position.XYZ() / Position.w;
		TR.Position[0] 		= P.x;
		TR.Position[1] 		= P.y;
		TR.Position[2] 		= P.z;
		TR.Normal[0] 		= N.x;
		TR.Normal[1] 		= N.y;
		TR.Normal[2] 		= N.z;
		TR.Tangent[0] 		= T.x;
		TR.Tangent[1] 		= T.y;
		TR.Tangent[2] 		= T.z;
		TR.Bitangent[0]		= B.x;
		TR.Bitangent[1]		= B.y;
		TR.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x + UVRect.z, UVRect.y + UVRect.w);
		TR.UV[0] 			= UV.x; 
		TR.UV[1] 			= UV.y; 
		TR.Color[0] 		= Color.r;
		TR.Color[1] 		= Color.g;
		TR.Color[2] 		= Color.b;
		TR.Color[3] 		= Color.a;
		TR.ID 				= QuadBatch::DrawCallCountID;

		/* Set bottom right vertex */
 		Position 			= Model * EM::Vec4(Static_BR, 1.0);
 		P 					= Position.XYZ() / Position.w;
		BR.Position[0] 		= P.x;
		BR.Position[1] 		= P.y;
		BR.Position[2] 		= P.z;
		BR.Normal[0] 		= N.x;
		BR.Normal[1] 		= N.y;
		BR.Normal[2] 		= N.z;
		BR.Tangent[0] 		= T.x;
		BR.Tangent[1] 		= T.y;
		BR.Tangent[2] 		= T.z;
		BR.Bitangent[0]		= B.x;
		BR.Bitangent[1]		= B.y;
		BR.Bitangent[2]		= B.z;
		UV 					= EM::Vec2(UVRect.x + UVRect.z, UVRect.y);
		BR.UV[0] 			= UV.x; 
		BR.UV[1] 			= UV.y; 
		BR.Color[0] 		= Color.r;
		BR.Color[1] 		= Color.g;
		BR.Color[2] 		= Color.b;
		BR.Color[3] 		= Color.a;
		BR.ID 				= QuadBatch::DrawCallCountID;
	}

	void QuadBatch::Init()
	{
		CreateVertexArray();
	}

	void QuadBatch::Begin(QuadGlyphSortType _SortType)
	{
		// Set sort type
		SortType = _SortType;

		// Clear previous batches
		RenderBatches.clear();

		// Clear glyphs
		QuadGlyphs.clear();
	}

	void QuadBatch::End()
	{
		// Resize glyph pointers to glyph vector size
		QuadGlyphPointers.resize(QuadGlyphs.size());

		// Set up glyph pointers to point to corresponding glyphs
		for (uint32_t i = 0; i < QuadGlyphs.size(); i++)
		{
			QuadGlyphPointers[i] = &QuadGlyphs[i];
		}

		// Sort glyphs according to type
		SortGlyphs();

		// Create render batches
		CreateRenderBatches();
	}

	void QuadBatch::Add(
						EM::Transform& Transform, 
						EM::Vec4& UVRect, 
						GLuint Texture, 
						ColorRGBA16& Color, 
						float Depth
						)
	{
		QuadGlyphs.emplace_back(Transform, UVRect, Texture, Color, Depth);
		DrawCallCountID++;
	}

	// Adds quadglyph to quadbatch to be rendered with base quad defined
	void QuadBatch::Add(
					EM::Vec2& Dimensions, 
					EM::Transform& Transform, 
					EM::Vec4& UVRect, 
					GLuint Texture, 
					ColorRGBA16& Color, 
					float Depth
				)
	{
		QuadGlyphs.emplace_back(Dimensions, Transform, UVRect, Texture, Color, Depth);
		DrawCallCountID++;
	}

	void QuadBatch::RenderBatch()
	{
		// Bind vao
		glBindVertexArray(VAO);

		// Activate texture
		glActiveTexture(GL_TEXTURE0);

		for (uint32_t i = 0; i < RenderBatches.size(); i++)
		{
			DrawCallCount++;

			// Bind texture for rendering
			glBindTexture(GL_TEXTURE_2D, RenderBatches[i].Texture);

			// Draw quad
			glDrawArrays(GL_TRIANGLES, RenderBatches[i].Offset, RenderBatches[i].NumVerticies);
		}
	}

	void QuadBatch::CreateRenderBatches()
	{
		// Return if glyphs are empty
		if (QuadGlyphPointers.empty()) return;

		// Store all verticies to be uploaded
		std::vector<QuadVert> Verticies;

		// Resize to exact size
		Verticies.resize(QuadGlyphPointers.size() * 6);

		uint32_t Offset = 0;
		uint32_t CV = 0;

		RenderBatches.emplace_back(Offset, 6, QuadGlyphPointers[0]->Texture);
		Verticies[CV++] = QuadGlyphPointers[0]->TL;
		Verticies[CV++] = QuadGlyphPointers[0]->BL;
		Verticies[CV++] = QuadGlyphPointers[0]->BR;
		Verticies[CV++] = QuadGlyphPointers[0]->BR;
		Verticies[CV++] = QuadGlyphPointers[0]->TR;
		Verticies[CV++] = QuadGlyphPointers[0]->TL;
		Offset += 6;

		for (uint32_t i = 1; i < QuadGlyphPointers.size(); i++)
		{
			if (QuadGlyphPointers[i]->Texture != QuadGlyphPointers[i -1]->Texture)
			{
				//Make new batch
				RenderBatches.emplace_back(Offset, 6, QuadGlyphPointers[i]->Texture);
			}
			else
			{
				RenderBatches.back().NumVerticies += 6;
			}

			Verticies[CV++] = QuadGlyphPointers[i]->TL;
			Verticies[CV++] = QuadGlyphPointers[i]->BL;
			Verticies[CV++] = QuadGlyphPointers[i]->BR;
			Verticies[CV++] = QuadGlyphPointers[i]->BR;
			Verticies[CV++] = QuadGlyphPointers[i]->TR;
			Verticies[CV++] = QuadGlyphPointers[i]->TL;
			Offset += 6;
		}

		// Bind vbo
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// Orphan data
		glBufferData(GL_ARRAY_BUFFER, Verticies.size() * sizeof(QuadVert), nullptr, GL_DYNAMIC_DRAW);
		// Upload data
		glBufferSubData(GL_ARRAY_BUFFER, 0, Verticies.size() * sizeof(QuadVert), Verticies.data());
		// Unbind vbo
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void QuadBatch::CreateVertexArray()
	{
		// Generate the vao if not already generated
		if (VAO == 0) glGenVertexArrays(1, &VAO);

		// Bind the vao
		glBindVertexArray(VAO);

		// Generate VBO
		if (VBO == 0) glGenBuffers(1, &VBO);

		// Bind vbo
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// Tell opengl which attributes we'll need
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);

		// Position
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)offsetof(QuadVert, Position));
		// Normal
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)offsetof(QuadVert, Normal));
		// Tangent
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)offsetof(QuadVert, Tangent));
		// Bitangent
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)offsetof(QuadVert, Bitangent));
		// UV
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)offsetof(QuadVert, UV));
		// Color
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)offsetof(QuadVert, Color));
		// ID
		glVertexAttribPointer(GL_QUAD_VERTEX_ATTRIB_ID, 1, GL_UNSIGNED_INT, GL_TRUE, sizeof(QuadVert), (void*)offsetof(QuadVert, ID));

		// Unbind VAO
		glBindVertexArray(0);		
	}

	void QuadBatch::SortGlyphs()
	{
		switch(SortType)
		{
			case QuadGlyphSortType::TEXTURE:
			{
				std::stable_sort(QuadGlyphPointers.begin(), QuadGlyphPointers.end(), CompareTexture);
			} break;
			case QuadGlyphSortType::FRONT_TO_BACK:
			{
				std::stable_sort(QuadGlyphPointers.begin(), QuadGlyphPointers.end(), CompareFrontToBack);
			} break;
			case QuadGlyphSortType::BACK_TO_FRONT:
			{
				std::stable_sort(QuadGlyphPointers.begin(), QuadGlyphPointers.end(), CompareBackToFront);
			} break;
		}
	}

	bool QuadBatch::CompareFrontToBack(QuadGlyph* A, QuadGlyph* B)
	{
		return (A->Depth < B->Depth);
	}

	bool QuadBatch::CompareBackToFront(QuadGlyph* A, QuadGlyph* B)
	{
		return (A->Depth > B->Depth);
	}

	bool QuadBatch::CompareTexture(QuadGlyph* A, QuadGlyph* B)
	{
		return (A->Texture < B->Texture);
	}

}}