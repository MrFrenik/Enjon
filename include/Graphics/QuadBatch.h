#ifndef ENJON_QUAD_BATCH_H
#define ENJON_QUAD_BATCH_H

#include <GLEW/glew.h>
#include <vector>

#include "System/Types.h"
#include "Math/Maths.h"
#include "Defines.h"
#include "Asset/Asset.h"
#include "Graphics/Color.h"

#define GL_QUAD_VERTEX_ATTRIB_POSITION 	0
#define GL_QUAD_VERTEX_ATTRIB_NORMAL 	1
#define GL_QUAD_VERTEX_ATTRIB_TANGENT 	2
#define GL_QUAD_VERTEX_ATTRIB_BITANGENT 3
#define GL_QUAD_VERTEX_ATTRIB_UV 		4
#define GL_QUAD_VERTEX_ATTRIB_COLOR 	5

namespace Enjon { 

	class Material;
	class GraphicsScene;

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
		QuadGlyph( const Transform& Transform, const Vec4& UVRect, GLuint _Texture, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f );
		QuadGlyph( const Vec2& Dimensions, const Transform& Transform, const Vec4& UVRect, GLuint _Texture, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f );
		QuadGlyph( const Vector<Vec3>& Points, const Vec4& UVRect, GLuint _Texture, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f );
		QuadGlyph( const Vec3& TL, const Vec3& BL, const Vec3& BR, const Vec3& TR, const Vec4& UVRect, GLuint _Texture, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f);

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

			void SetGraphicsScene(GraphicsScene* scene);
			GraphicsScene* GetGraphicsScene() { return mGraphicsScene; }

			// Sets material of quadbatch
			void SetMaterial(const AssetHandle<Material>& mat);

			// Gets material of quadbatch
			AssetHandle<Material> GetMaterial() { return mMaterial; }

			// Adds quadglyph to quadbatch to be rendered
			void Add( const Transform& Transform, const Vec4& UVRect = Vec4(0, 0, 1, 1), GLuint Texture = 0, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f );

			// Adds quadglyph to quadbatch to be rendered with base quad defined
			void Add( const Vec2& Dimensions, const Transform& Transform, const Vec4& UVRect, GLuint Texture = 0, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f );

			// Adds quadglyph to quadbatch to be rendered with base quad defined
			void Add( const Vec3& TLP, const Vec3& BLP, const Vec3& BRP, const Vec3& TRP, const Vec4& UVRect, GLuint Texture = 0, const ColorRGBA32& Color = RGBA32(1.0f), const f32& Depth = 1.0f );

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
			AssetHandle<Material> mMaterial;
			GraphicsScene* mGraphicsScene 		= nullptr;
	};

}




#endif














