#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"

namespace Enjon { namespace Graphics {

	Material::Material()
		: mShader(nullptr)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetTexture(EG::TextureSlotType type, GLTexture& texture)
	{
		mTextures[(Enjon::u32)type] = texture;
	}

	GLTexture Material::GetTexture(EG::TextureSlotType type)
	{
		return mTextures[(Enjon::u32)type];
	}

	GLSLProgram* Material::GetShader()
	{
		return mShader;
	}

	void Material::SetShader(EG::GLSLProgram* shader)
	{
		mShader = shader;
	}
}}