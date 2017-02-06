#include "Entity/Components/GraphicsComponent.h"

namespace Enjon
{
	GraphicsComponent::GraphicsComponent()
	{
	}

	GraphicsComponent::GraphicsComponent(EG::Renderable& renderable)
		: mRenderable(renderable)
	{
	}

	GraphicsComponent::~GraphicsComponent()
	{
	}

	void GraphicsComponent::Update(float dt)
	{
	}

	void GraphicsComponent::SetPosition(EM::Vec3& position)
	{
		mRenderable.SetPosition(position);
	}

	void GraphicsComponent::SetScale(EM::Vec3& scale)
	{
		mRenderable.SetScale(scale);
	}

	void GraphicsComponent::SetOrientation(EM::Quaternion& orientation)
	{
		mRenderable.SetOrientation(orientation);
	}

	void GraphicsComponent::SetMaterial(EG::Material* material)
	{
		mRenderable.SetMaterial(material);
	}

	void GraphicsComponent::SetMesh(EG::Mesh* mesh)
	{
		mRenderable.SetMesh(mesh);
	}

	void GraphicsComponent::SetScene(EG::Scene* scene)
	{
		mRenderable.SetScene(scene);
	}
}
