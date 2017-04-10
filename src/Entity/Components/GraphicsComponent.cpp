#include "Entity/Components/GraphicsComponent.h"
#include "Entity/EntityManager.h"
#include "Graphics/Scene.h"

namespace Enjon
{
	//====================================================================

	GraphicsComponent::GraphicsComponent()
	{
		
	}

	//====================================================================

	GraphicsComponent::GraphicsComponent(const Renderable& renderable)
		: mRenderable(renderable)
	{
	}

	//====================================================================

	GraphicsComponent::~GraphicsComponent()
	{
		if (mRenderable.GetScene() != nullptr)
		{
			mRenderable.GetScene()->RemoveRenderable(&mRenderable);
		}
	}

	//====================================================================

	void GraphicsComponent::Destroy()
	{
		DestroyBase<GraphicsComponent>();
	}

	//====================================================================

	void GraphicsComponent::Update(f32 dt)
	{
		mRenderable.SetTransform(mEntity->GetWorldTransform());
	}
	
	//====================================================================
		
	Vec3 GraphicsComponent::GetPosition() const
	{ 
		return mRenderable.GetPosition(); 
	}
	
	//====================================================================

	Vec3 GraphicsComponent::GetScale() const
	{ 
		return mRenderable.GetScale(); 
	}

	//====================================================================

	Quaternion GraphicsComponent::GetRotation() const 
	{ 
		return mRenderable.GetRotation(); 
	}

	//====================================================================

	Material* GraphicsComponent::GetMaterial()
	{ 
		return mRenderable.GetMaterial(); 
	}

	//====================================================================

	AssetHandle<Mesh> GraphicsComponent::GetMesh() const
	{ 
		return mRenderable.GetMesh(); 
	}

	//====================================================================

	Scene* GraphicsComponent::GetScene() const
	{ 
		return mRenderable.GetScene(); 
	}

	//====================================================================

	Transform GraphicsComponent::GetTransform() const 
	{ 
		return mRenderable.GetTransform(); 
	} 

	//====================================================================

	Renderable* GraphicsComponent::GetRenderable()
	{ 
		return &mRenderable; 
	}
	
	/* Sets world transform */
	void GraphicsComponent::SetTransform( const Transform& transform )
	{
		mRenderable.SetTransform( transform );
	}

	//====================================================================

	void GraphicsComponent::SetPosition(const Vec3& position)
	{
		mRenderable.SetPosition(position);
	}

	//====================================================================

	void GraphicsComponent::SetScale(const Vec3& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void GraphicsComponent::SetScale(const f32& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void GraphicsComponent::SetRotation(const Quaternion& rotation)
	{
		mRenderable.SetRotation(rotation);
	}

	//====================================================================

	void GraphicsComponent::SetMaterial(Material* material)
	{
		mRenderable.SetMaterial(material);
	}

	//====================================================================

	void GraphicsComponent::SetMesh(const AssetHandle<Mesh>& mesh)
	{
		mRenderable.SetMesh(mesh);
	}

	//====================================================================

	void GraphicsComponent::SetScene(Scene* scene)
	{
		mRenderable.SetScene(scene);
	}

	//==================================================================== 

	void GraphicsComponent::SetRenderable(const Renderable& renderable)
	{
		mRenderable = renderable;
	}

	//====================================================================

	void GraphicsComponent::SetColor(TextureSlotType type, const ColorRGBA16& color)
	{
		mRenderable.SetColor(type, color);	
	}
	
}
