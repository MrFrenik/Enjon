#ifndef ENJON_MODEL_ASSET_H
#define ENJON_MODEL_ASSET_H

#include <Defines.h>
#include <Graphics/GLSLProgram.h>
#include <Math/Transform.h>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#include <IO/ResourceManager.h>
#include <Math/Mat4.h>

namespace Enjon {

	struct ModelAsset
	{
		GLSLProgram* mShader;
		Material mMaterial;
		Mesh* mMesh;
	};

	struct ModelInstance
	{
		ModelAsset* mAsset;
		EM::Transform mTransform;
	};

}

#endif