#include "Entity/Components/GraphicsComponent.h"
#include "Entity/EntityManager.h"
#include "Graphics/Scene.h"

namespace Enjon
{
	//--------------------------------------------------------------------
	GraphicsComponent::GraphicsComponent()
	{
		
	}

	//--------------------------------------------------------------------
	GraphicsComponent::GraphicsComponent(Renderable& renderable)
		: mRenderable(renderable)
	{
	}

	//--------------------------------------------------------------------
	GraphicsComponent::~GraphicsComponent()
	{
		if (mRenderable.GetScene() != nullptr)
		{
			mRenderable.GetScene()->RemoveRenderable(&mRenderable);
		}
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::Destroy()
	{
		DestroyBase<GraphicsComponent>();
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::Update(float dt)
	{
		mRenderable.SetTransform(mEntity->GetWorldTransform());
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetPosition(EM::Vec3& position)
	{
		mRenderable.SetPosition(position);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetScale(EM::Vec3& scale)
	{
		mRenderable.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetScale(f32 scale)
	{
		mRenderable.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetRotation(EM::Quaternion& rotation)
	{
		mRenderable.SetRotation(rotation);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetMaterial(Material* material)
	{
		mRenderable.SetMaterial(material);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetMesh(Mesh* mesh)
	{
		mRenderable.SetMesh(mesh);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetScene(Scene* scene)
	{
		mRenderable.SetScene(scene);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetRenderable(Renderable& renderable)
	{
		mRenderable = renderable;
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetColor(TextureSlotType type, ColorRGBA16& color)
	{
		mRenderable.SetColor(type, color);	
	}
}
