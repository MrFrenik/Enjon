#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "System/Types.h"

namespace Enjon { namespace Graphics {

	enum TextureSlotType
	{
		ALBEDO,
		NORMAL,
		EMISSIVE,
		METALLIC,
		ROUGHNESS,
		TEXTURE_SLOT_COUNT
	};

	struct MaterialInstance
	{
		GLTexture Textures[TEXTURE_SLOT_COUNT];
		float Shininess;
	};
	
}}


#endif