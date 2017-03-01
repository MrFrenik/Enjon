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
	GraphicsComponent::GraphicsComponent(EG::Renderable& renderable)
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
	void GraphicsComponent::SetOrientation(EM::Quaternion& orientation)
	{
		mRenderable.SetOrientation(orientation);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetMaterial(EG::Material* material)
	{
		mRenderable.SetMaterial(material);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetMesh(EG::Mesh* mesh)
	{
		mRenderable.SetMesh(mesh);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetScene(EG::Scene* scene)
	{
		mRenderable.SetScene(scene);
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetRenderable(EG::Renderable& renderable)
	{
		mRenderable = renderable;
	}

	//--------------------------------------------------------------------
	void GraphicsComponent::SetColor(EG::TextureSlotType type, EG::ColorRGBA16& color)
	{
		mRenderable.SetColor(type, color);	
	}
}
