#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"

#include <assert.h>

namespace Enjon { namespace Graphics {

	//------------------------------------------------------------------------
	Material::Material()
		: mShader(nullptr)
	{
	}

	//------------------------------------------------------------------------
	Material::~Material()
	{
	}

	//------------------------------------------------------------------------
	void Material::SetTexture(EG::TextureSlotType type, GLTexture& texture)
	{
		assert((u32)type < (u32)TextureSlotType::TEXTURE_SLOT_COUNT);
		mTextures[(u32)type] = texture;
	}

	//------------------------------------------------------------------------
	GLTexture Material::GetTexture(EG::TextureSlotType type)
	{
		assert((u32)type < (u32)TextureSlotType::TEXTURE_SLOT_COUNT);
		return mTextures[(u32)type];
	}

	//------------------------------------------------------------------------
	void Material::SetColor(EG::TextureSlotType type, EG::ColorRGBA16& color)
	{
		assert((u32)type < (u32)TextureSlotType::TEXTURE_SLOT_COUNT);
		mColors[(u32)type] = color;
	}

	//------------------------------------------------------------------------
	EG::ColorRGBA16& Material::GetColor(EG::TextureSlotType type)
	{
		assert((u32)type < (u32)TextureSlotType::TEXTURE_SLOT_COUNT);
		return mColors[(u32)type];
	}

	//------------------------------------------------------------------------
	GLSLProgram* Material::GetShader()
	{
		return mShader;
	}

	//------------------------------------------------------------------------
	void Material::SetShader(EG::GLSLProgram* shader)
	{
		mShader = shader;
	}

}}