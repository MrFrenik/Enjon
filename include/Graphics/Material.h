#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "System/Types.h"

namespace Enjon { namespace Graphics {

	const static int TextureSlotCount = 3;

	enum TextureSlotType
	{
		DIFFUSE,
		NORMAL,
		EMISSIVE
	};

	struct MaterialInstance
	{
		GLTexture Textures[TextureSlotCount];
		float Shininess;
	};
	
}}


#endif