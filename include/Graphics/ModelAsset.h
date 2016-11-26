#ifndef ENJON_MODEL_ASSET_H
#define ENJON_MODEL_ASSET_H

#include <Defines.h>
#include <Graphics/GLSLProgram.h>
#include <Graphics/Transform.h>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#include <IO/ResourceManager.h>
#include <Math/Mat4.h>

namespace Enjon { namespace Graphics {

	struct ModelAsset
	{
		EG::GLSLProgram* Shader;
		EG::MaterialInstance Material;
		EG::MeshInstance* Mesh;
	};

	struct ModelInstance
	{
		ModelAsset* Asset;
		EG::Transform Transform;
	};

}}

#endif