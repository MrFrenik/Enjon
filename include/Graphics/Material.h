#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "System/Types.h"

namespace Enjon { namespace Graphics {

	const u32 TextureSlotCount = 2;

	enum TextureSlotType
	{
		DIFFUSE,
		NORMAL
	};

	struct MaterialInstance
	{
		GLTexture Textures[TextureSlotCount];
	};
	
}}


#endif