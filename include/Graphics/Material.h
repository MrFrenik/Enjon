#pragma once
#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "Graphics/Color.h"
#include "System/Types.h"

#include <unordered_map>

namespace Enjon { namespace Graphics {

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

			void SetTexture(EG::TextureSlotType, GLTexture& texture);
			GLTexture GetTexture(EG::TextureSlotType type);

			GLSLProgram* GetShader();
			void SetShader(GLSLProgram* shader);

		private:
			EG::GLTexture mTextures[(Enjon::u32)TextureSlotType::TEXTURE_SLOT_COUNT];
			EG::ColorRGBA16 mColors[(Enjon::u32)TextureSlotType::TEXTURE_SLOT_COUNT];
			EG::GLSLProgram* mShader;
	};
	
}}


#endif