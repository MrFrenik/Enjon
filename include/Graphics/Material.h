#pragma once
#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "Graphics/Color.h"
#include "System/Types.h"

#include <unordered_map>

namespace Enjon { 

	class GLSLProgram;

	enum class TextureSlotType
	{
		ALBEDO,
		NORMAL,
		EMISSIVE,
		METALLIC,
		ROUGHNESS,
		AO,
		TEXTURE_SLOT_COUNT
	};

	class Material
	{
		public:
			Material();
			~Material();

			void SetTexture(TextureSlotType type, const GLTexture& texture);
			GLTexture GetTexture(TextureSlotType type) const;

			void SetColor(TextureSlotType type, const ColorRGBA16& color);
			ColorRGBA16 GetColor(TextureSlotType type) const;

			GLSLProgram* GetShader();
			void SetShader(GLSLProgram* shader);

		private:
			GLTexture mTextures[(u32)TextureSlotType::TEXTURE_SLOT_COUNT];
			ColorRGBA16 mColors[(u32)TextureSlotType::TEXTURE_SLOT_COUNT];
			GLSLProgram* mShader;
	};
	
}


#endif