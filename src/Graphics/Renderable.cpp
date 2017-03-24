#include "Graphics/Renderable.h"
#include "Graphics/Scene.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"

#include <assert.h>

namespace Enjon  
{
	//--------------------------------------------------------------------
	Renderable::Renderable()
		: mMesh(nullptr), mMaterial(nullptr)
	{
	}

	//--------------------------------------------------------------------
	Renderable::~Renderable()
	{
	}

	//--------------------------------------------------------------------
	void Renderable::SetPosition(EM::Vec3& position)
	{
		mTransform.SetPosition(position);
	}

	//--------------------------------------------------------------------
	void Renderable::SetScale(EM::Vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void Renderable::SetScale(float scale)
	{
		mTransform.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void Renderable::SetRotation(EM::Quaternion& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	//--------------------------------------------------------------------
	void Renderable::OffsetRotation(const f32& Yaw, const f32& Pitch)
	{
		EM::Quaternion X = EM::Quaternion::AngleAxis(Yaw, 	EM::Vec3(0, 1, 0)); 						// Absolute Up
		EM::Quaternion Y = EM::Quaternion::AngleAxis(Pitch, mTransform.Rotation * EM::Vec3(1, 0, 0));	// Relative Right

		mTransform.Rotation = X * Y * mTransform.Rotation;
	}

	//--------------------------------------------------------------------
	void Renderable::SetMaterial(Material* material)
	{
		mMaterial = material;
	}

	//--------------------------------------------------------------------
	void Renderable::SetMesh(Mesh* mesh)
	{
		mMesh = mesh;
	}

	//--------------------------------------------------------------------
	void Renderable::SetScene(Scene* scene)
	{
		mScene = scene;
	}

	//--------------------------------------------------------------------
	void Renderable::SetColor(TextureSlotType type, ColorRGBA16& color)
	{
		assert(mMaterial != nullptr);
		mMaterial->SetColor(type, color);
	}
}










