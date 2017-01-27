#ifndef ENJON_MODEL_ASSET_H
#define ENJON_MODEL_ASSET_H

#include <Defines.h>
#include <Graphics/GLSLProgram.h>
#include <Math/Transform.h>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#include <IO/ResourceManager.h>
#include <Math/Mat4.h>

namespace Enjon { namespace Graphics {

	struct ModelAsset
	{
		EG::GLSLProgram* mShader;
		EG::Material mMaterial;
		EG::MeshInstance* mMesh;
	};

	struct ModelInstance
	{
		ModelAsset* mAsset;
		EM::Transform mTransform;
	};

}}

#endif