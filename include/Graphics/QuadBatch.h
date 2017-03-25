#ifndef ENJON_QUAD_BATCH_H
#define ENJON_QUAD_BATCH_H

#include <GLEW/glew.h>
#include <vector>

#include "System/Types.h"
#include "Math/Maths.h"
#include "Defines.h"
#include "Graphics/Color.h"

#define GL_QUAD_VERTEX_ATTRIB_POSITION 	0
#define GL_QUAD_VERTEX_ATTRIB_NORMAL 	1
#define GL_QUAD_VERTEX_ATTRIB_TANGENT 	2
#define GL_QUAD_VERTEX_ATTRIB_BITANGENT 3
#define GL_QUAD_VERTEX_ATTRIB_UV 		4
#define GL_QUAD_VERTEX_ATTRIB_COLOR 	5

namespace Enjon { 

	class Material;
	class Scene;

	struct QuadVert
	{
		float Position[3];
		float Normal[3];
		float Tangent[3];
		float Bitangent[3];
		float UV[2];
		float Color[4];
		u32 ID;
	};

	enum class QuadGlyphSortType
	{
		NONE,
		FRONT_TO_BACK,
		BACK_TO_FRONT,
		TEXTURE
	};

	struct QuadGlyph
	{
		QuadGlyph(Transform& Transform, Vec4& UVRect, GLuint _Texture, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);
		QuadGlyph(Vec2& Dimensions, Transform& Transform, Vec4& UVRect, GLuint _Texture, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);
		QuadGlyph(std::vector<Vec3>& Points, Vec4& UVRect, GLuint _Texture, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);
		QuadGlyph(Vec3& TL, Vec3& BL, Vec3& BR, Vec3& TR, Vec4& UVRect, GLuint _Texture, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);

		GLuint Texture;
		QuadVert TL;
		QuadVert BL;
		QuadVert TR;
		QuadVert BR;
		float Depth;
	};

	struct QuadRenderBatch
	{
		QuadRenderBatch(GLuint _Offset, GLuint _NumVerts, GLuint _Texture)
			: Offset(_Offset), NumVerticies(_NumVerts), Texture(_Texture)
		{}

		GLuint Offset;
		GLuint NumVerticies;
		GLuint Texture; // Set to a texture array
	};

	class QuadBatch
	{
		public:
	
			enum class QuadBatchStatus
			{
				READY, 
				NOT_READY
			};

			QuadBatch();
			~QuadBatch();

			void Init();
			void Begin(QuadGlyphSortType SortType = QuadGlyphSortType::TEXTURE);
			void End();

			void SetScene(Scene* scene);
			Scene* GetScene() { return mScene; }

			// Sets material of quadbatch
			void SetMaterial(Material* mat);

			// Gets material of quadbatch
			Material* GetMaterial() { return mMaterial; }

			// Adds quadglyph to quadbatch to be rendered
			void Add(Transform& Transform, Vec4& UVRect, GLuint Texture = 0, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);

			// Adds quadglyph to quadbatch to be rendered with base quad defined
			void Add(Vec2& Dimensions, Transform& Transform, Vec4& UVRect, GLuint Texture = 0, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);

			// Adds quadglyph to quadbatch to be rendered with base quad defined
			void Add(Vec3& TLP, Vec3& BLP, Vec3& BRP, Vec3& TRP, Vec4& UVRect, GLuint Texture = 0, ColorRGBA16& Color = RGBA16(1.0f), float Depth = 1.0f);

			// Renders entire batch to screen
			void RenderBatch();

		static u32 DrawCallCount; 
		static u32 DrawCallCountID; 

		private:
			void CreateRenderBatches();
			void CreateVertexArray();
			void SortGlyphs();

			static bool CompareFrontToBack(QuadGlyph* A, QuadGlyph* B);
			static bool CompareBackToFront(QuadGlyph* A, QuadGlyph* B);
			static bool CompareTexture(QuadGlyph* A, QuadGlyph* B);

			QuadBatchStatus mStatus = QuadBatchStatus::NOT_READY;
			GLuint VBO;
			GLuint VAO;
			QuadGlyphSortType SortType;
			std::vector<QuadGlyph> QuadGlyphs;
			std::vector<QuadGlyph*> QuadGlyphPointers;
			std::vector<QuadRenderBatch> RenderBatches;
			Material* mMaterial = nullptr;
			Scene* mScene 		= nullptr;
	};

}




#endif














