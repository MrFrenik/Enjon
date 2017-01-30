#include "Graphics/Renderable.h"
#include "Graphics/Scene.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"

namespace Enjon { namespace Graphics {

	Renderable::Renderable(EG::Mesh* mesh, EG::Material* material)
		: mMesh(mesh), mMaterial(material)
	{
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::SetPosition(EM::Vec3& position)
	{
		mTransform.SetPosition(position);
	}

	void Renderable::SetScale(EM::Vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void Renderable::SetOrientation(EM::Quaternion& orientation)
	{
		mTransform.SetOrientation(orientation);
	}

	void Renderable::OffsetOrientation(const f32& Yaw, const f32& Pitch)
	{
		EM::Quaternion X = EM::Quaternion::AngleAxis(Yaw, 	EM::Vec3(0, 1, 0)); 	// Absolute Up
		EM::Quaternion Y = EM::Quaternion::AngleAxis(Pitch, mTransform.Orientation * EM::Vec3(1, 0, 0));	// Relative Right

		mTransform.Orientation = X * Y * mTransform.Orientation;
	}

	void Renderable::SetMaterial(EG::Material* material)
	{
		mMaterial = material;
	}

	void Renderable::SetMesh(EG::Mesh* mesh)
	{
		mMesh = mesh;
	}

	void Renderable::SetScene(EG::Scene* scene)
	{
		mScene = scene;
	}

}}
