#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"

#include <assert.h>

namespace Enjon {

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
	void Material::SetTexture(TextureSlotType type, const GLTexture& texture)
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		mTextures[(u32)type] = texture;
	} 

	//------------------------------------------------------------------------
	//GLTexture Material::GetTexture(TextureSlotType type) const
	//{
	//	assert((u32)type < (u32)TextureSlotType::Count);
	//	return mTextures[(u32)type];
	//}
	
	//------------------------------------------------------------------------
	
	void Material::SetTexture(const TextureSlotType& type, const AssetHandle<Texture>& textureHandle)
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		mTextureHandles[(u32)type] = textureHandle;
	}

	AssetHandle<Texture> Material::GetTexture(const TextureSlotType& type) const
	{ 
		assert((u32)type < (u32)TextureSlotType::Count);
		return mTextureHandles[(u32)type];
	}

	//------------------------------------------------------------------------
	void Material::SetColor(TextureSlotType type, const ColorRGBA16& color)
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		mColors[(u32)type] = color;
	}

	//------------------------------------------------------------------------
	ColorRGBA16 Material::GetColor(TextureSlotType type) const
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		return mColors[(u32)type];
	}

	//------------------------------------------------------------------------
	GLSLProgram* Material::GetShader()
	{
		return mShader;
	}

	//------------------------------------------------------------------------
	void Material::SetShader(GLSLProgram* shader)
	{
		mShader = shader;
	}

}