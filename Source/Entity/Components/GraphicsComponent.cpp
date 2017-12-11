#include "Entity/Components/GraphicsComponent.h"
#include "Entity/EntityManager.h"
#include "Graphics/Scene.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"

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

	const Material* GraphicsComponent::GetMaterial() const
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

	void GraphicsComponent::SetMaterial(const Material* material)
	{
		mRenderable.SetMaterial(material);
	} 

	//====================================================================

	//void SetMaterial( const AssetHandle< Material >& material )
	//{
	//}

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

	Result GraphicsComponent::SerializeData( ByteBuffer* buffer ) const
	{
		// Write uuid of mesh
		mRenderable.GetMesh( ) ? buffer->Write< UUID >( mRenderable.GetMesh( )->GetUUID( ) ) : buffer->Write< UUID >( UUID::Invalid( ) );
		// Write uuid of material
		mRenderable.GetMaterial( ) ? buffer->Write< UUID >( mRenderable.GetMaterial( )->GetUUID( ) ) : buffer->Write< UUID >( UUID::Invalid( ) );

		return Result::SUCCESS;
	}

	//====================================================================

	Result GraphicsComponent::DeserializeData( ByteBuffer* buffer )
	{
		// Get asset manager
		const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );

		// Set mesh
		mRenderable.SetMesh( am->GetAsset< Mesh >( buffer->Read< UUID >( ) ) );
		// Set material 
		mRenderable.SetMaterial( am->GetAsset< Material >( buffer->Read< UUID >( ) ).Get( ) );

		return Result::SUCCESS;
	}

	//==================================================================== 
}
