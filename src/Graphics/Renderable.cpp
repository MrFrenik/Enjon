#include "Graphics/Renderable.h"
#include "Graphics/Scene.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"

#include <assert.h>

namespace Enjon  
{
	//--------------------------------------------------------------------
	Renderable::Renderable()
		: mMaterial(nullptr)
	{
	}

	//--------------------------------------------------------------------
	Renderable::~Renderable()
	{
	} 

	//==============================================================

	Vec3 Renderable::GetPosition() const
	{
		return mTransform.GetPosition();
	}

	//==============================================================

	Vec3 Renderable::GetScale() const
	{
		return mTransform.GetScale();
	}

	//==============================================================

	Quaternion Renderable::GetRotation() const
	{
		return mTransform.GetRotation();
	}

	//==============================================================

	Material* Renderable::GetMaterial() 
	{ 
		return mMaterial; 
	}

	//==============================================================

	AssetHandle<Mesh> Renderable::GetMesh() const 
	{ 
		return mMesh; 
	}

	//==============================================================

	Scene* Renderable::GetScene() const 
	{ 
		return mScene; 
	}

	//==============================================================

	Transform Renderable::GetTransform() const 
	{ 
		return mTransform; 
	}

	//==============================================================

	void Renderable::SetTransform(const Transform& transform) 
	{ 
		mTransform = transform; 
	}

	//==============================================================

	void Renderable::SetPosition(const Vec3& position)
	{
		mTransform.SetPosition(position);
	}

	//--------------------------------------------------------------------
	void Renderable::SetScale(const Vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void Renderable::SetScale(const f32& scale)
	{
		mTransform.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void Renderable::SetRotation(const Quaternion& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	//==============================================================

	void Renderable::OffsetRotation(const f32& Yaw, const f32& Pitch)
	{
		Quaternion X = Quaternion::AngleAxis(Yaw, 	Vec3(0, 1, 0)); 						// Absolute Up
		Quaternion Y = Quaternion::AngleAxis(Pitch, mTransform.Rotation * Vec3(1, 0, 0));	// Relative Right

		mTransform.Rotation = X * Y * mTransform.Rotation;
	}

	//--------------------------------------------------------------------
	void Renderable::SetMaterial(Material* material)
	{
		mMaterial = material;
	}

	//--------------------------------------------------------------------
	void Renderable::SetMesh(const AssetHandle<Mesh>& mesh)
	{
		mMesh = mesh;
	}

	//--------------------------------------------------------------------
	void Renderable::SetScene(Scene* scene)
	{
		mScene = scene;
	}

	//--------------------------------------------------------------------
	void Renderable::SetColor(TextureSlotType type, const ColorRGBA16& color)
	{
		assert(mMaterial != nullptr);
		mMaterial->SetColor(type, color);
	}
}










