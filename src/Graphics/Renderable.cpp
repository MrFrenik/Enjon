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
