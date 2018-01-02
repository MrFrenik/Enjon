#include "TestProject.h"

#include <Entity/EntityManager.h> 
#include <Entity/Components/GraphicsComponent.h>
#include <Serialize/EntityArchiver.h>
#include <Graphics/GraphicsSubsystem.h>
#include <Asset/AssetManager.h>
#include <IO/InputManager.h>
#include <SubsystemCatalog.h>
#include <Engine.h>

#include <iostream>
 
Enjon::f32 distance = 5.0f;
Enjon::f32 worldTime = 0.0f; 

extern "C"
{ 
	ENJON_EXPORT void SetEngineInstance( Enjon::Engine* engine )
	{
		Enjon::Engine::SetInstance( engine );
	}

	void BindMetaClass( )
	{
		Enjon::MetaClassRegistry* registry = const_cast< Enjon::MetaClassRegistry* >( Enjon::Engine::GetInstance( )->GetMetaClassRegistry( ) );
		registry->RegisterMetaClass< Enjon::TestProject >( );
	}

	void UnbindMetaClass( )
	{
		Enjon::MetaClassRegistry* registry = const_cast< Enjon::MetaClassRegistry* >( Enjon::Engine::GetInstance( )->GetMetaClassRegistry( ) ); 
		registry->UnregisterMetaClass< Enjon::TestProject >( ); 
	}

	ENJON_EXPORT Enjon::Application* CreateApplication( Enjon::Engine* engine )
	{
		Enjon::TestProject* app = new Enjon::TestProject( );
		if ( app )
		{
			SetEngineInstance( engine );

			BindMetaClass( );
		}

		return app; 
	}

	ENJON_EXPORT void DeleteApplication( Enjon::Application* app )
	{
		if ( app )
		{
			delete app;
			app = nullptr;
			 
			UnbindMetaClass( );
		}
	}

	ENJON_EXPORT Enjon::usize GetApplicationSizeInBytes( )
	{
		return sizeof( Enjon::TestProject );
	} 
}

namespace Enjon 
{
	Result TestProject::Initialize( )
	{ 
		std::cout << "Startup game...\n";

		Enjon::EntityManager* entities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->ConstCast< Enjon::EntityManager >( );
		const Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

		// Allocate entities
		mEntity = entities->Allocate( );
		mFloor = entities->Allocate( );

		if ( mEntity.Get( ) )
		{
			// Attach graphics component
			auto gfx = mEntity.Get( )->AddComponent< Enjon::GraphicsComponent >( );
			gfx->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_sphere" ) );
			gfx->SetMaterial( am->GetAsset< Enjon::Material >( "NewMaterial1" ).Get( ) );

			Enjon::GraphicsSubsystem* gfxSub = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( );
			gfxSub->GetScene( )->AddRenderable( gfx->GetRenderable( ) );

			mEntity.Get( )->SetPosition( Enjon::Vec3( -73.0f, 5.0f, 0.0f ) );
			mEntity.Get( )->SetScale( 2.0f );
		}

		if ( mFloor.Get( ) )
		{
			// Attach graphics component
			auto gfx = mFloor.Get( )->AddComponent< Enjon::GraphicsComponent >( );
			gfx->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_cube" ) );
			gfx->SetMaterial( am->GetAsset< Enjon::Material >( "CerebusMaterial" ).Get( ) );

			Enjon::GraphicsSubsystem* gfxSub = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( );
			gfxSub->GetScene( )->AddRenderable( gfx->GetRenderable( ) );

			mFloor.Get( )->SetPosition( Enjon::Vec3( 0.0f, -1.0f, 0.0f ) );
			mFloor.Get( )->SetScale( Enjon::Vec3( 93.0f, 1.0f, 64.0f ) );
		}

		// Reset rotation time
		mRotationTime = 0.0f;
		mRotationAxis = Enjon::Vec3::YAxis( );
		mRotationSpeed = 10.0f;

		// Move camera to be right in front of entity and look at it
		Enjon::Camera* sceneCam = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetSceneCamera( )->ConstCast< Enjon::Camera >( );
		sceneCam->SetPosition( mEntity.Get( )->GetWorldPosition( ) + Enjon::Vec3( -8.0f, 2.0f, -15.0f ) );
		sceneCam->LookAt( mEntity.Get( )->GetWorldPosition( ) );

		return Enjon::Result::SUCCESS;
	}

	Result TestProject::Update( f32 dt )
	{ 
		// Clamp rotation time between 0 and max float 
		mRotationTime += dt * mRotationSpeed;
		mRotationTime = Clamp( mRotationTime, 0.0f, std::numeric_limits<f32>::max( ) );

		if ( mEntity.Get( ) )
		{
			//mEntity.Get( )->Destroy( );
			mEntity.Get( )->SetRotation( Quaternion::AngleAxis( ToRadians( mRotationTime ), mRotationAxis ) ); 
			mEntity.Get( )->SetScale( std::sin( mRotationTime ) );
		} 
		else
		{
			Enjon::EntityManager* entities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->ConstCast< Enjon::EntityManager >( );
			const Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

			mEntity = entities->Allocate( );
			// Attach graphics component
			auto gfx = mEntity.Get( )->AddComponent< Enjon::GraphicsComponent >( );
			gfx->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_sphere" ) );
			gfx->SetMaterial( am->GetAsset< Enjon::Material >( "NewMaterial1" ).Get( ) );

			Enjon::GraphicsSubsystem* gfxSub = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( );
			gfxSub->GetScene( )->AddRenderable( gfx->GetRenderable( ) );

			mEntity.Get( )->SetPosition( Enjon::Vec3( -73.0f, 5.0f, 0.0f ) );
			mEntity.Get( )->SetScale( 2.0f );
		}

		if ( mFloor.Get( ) )
		{
			auto gfx = mFloor.Get( )->GetComponent< GraphicsComponent >( );
			if ( gfx )
			{
				auto mat = gfx->GetMaterial( );
				if ( mat )
				{
					const f32 maxIntensity = 3.0f;
					mat.Get( )->ConstCast<Material>( )->SetUniform( "emissiveIntensity", sin( mRotationTime / 250.0f ) * maxIntensity + maxIntensity );
				}
			}
		}

		return Result::PROCESS_RUNNING;
	}

	Result TestProject::ProcessInput( f32 dt )
	{
		const Input* input = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Input >( ); 

		if ( input->IsKeyPressed( KeyCode::U ) )
		{
			for ( auto i = 0; i < 100; ++i )
			{
				Enjon::EntityManager* entities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->ConstCast< Enjon::EntityManager >( );
				const Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

				EntityHandle handle = entities->Allocate( );
				// Attach graphics component
				auto gfx = handle.Get( )->AddComponent< Enjon::GraphicsComponent >( );
				gfx->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_sphere" ) );
				gfx->SetMaterial( am->GetAsset< Enjon::Material >( "NewMaterial1" ).Get( ) );

				Enjon::GraphicsSubsystem* gfxSub = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( );
				gfxSub->GetScene( )->AddRenderable( gfx->GetRenderable( ) );

				handle.Get( )->SetPosition( Enjon::Vec3( i, i, i ) );
				handle.Get( )->SetScale( ( (f32)i + 1.0 ) / 100.0f ); 

				mHandles.push_back( handle );
			}
		}

		if ( input->IsKeyPressed( KeyCode::Y ) )
		{
			for ( auto& h : mHandles )
			{
				if ( h.Get( ) )
				{
					h.Get( )->Destroy( );
				}
			}
			mHandles.clear( );
		}

		return Enjon::Result::SUCCESS; 
	}

	Result TestProject::Shutdown( )
	{ 
		std::cout << "Shutting down game...\n";

		// Deallocate entity
		if ( mEntity.Get( ) )
		{
			mEntity.Get( )->Destroy( );
		}
		if ( mFloor.Get( ) )
		{
			mFloor.Get( )->Destroy( );
		}
		for ( auto& h : mHandles )
		{
			if ( h.Get( ) )
			{
				h.Get( )->Destroy( );
			}
		}
		mHandles.clear( );

		return Result::SUCCESS;
	}

	Enjon::Result TestProject::SerializeData( Enjon::ByteBuffer* buffer ) const
	{
		const MetaClass* cls = this->Class( );

		// Write out property count to buffer
		buffer->Write< usize >( cls->GetPropertyCount( ) );

		// Iterate over properties and write out
		for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
		{
			// Get property
			const MetaProperty* prop = cls->GetProperty( i );

			if ( !prop )
			{
				continue;
			}

			// Write out property name
			buffer->Write< String >( prop->GetName( ) );

			// Write out property type
			buffer->Write< s32 >( (s32)prop->GetType( ) ); 

			switch ( prop->GetType( ) )
			{
				case MetaPropertyType::F32:
				{
					buffer->Write< usize >( sizeof( f32 ) );
					buffer->Write< f32 >( *cls->GetValueAs< f32 >( this, prop ) );
				} break;

				case MetaPropertyType::Vec3:
				{
					buffer->Write< usize >( sizeof( Vec3 ) ); 
					Vec3 v = *cls->GetValueAs< Vec3 >( this, prop );
					buffer->Write< f32 >( v.x );
					buffer->Write< f32 >( v.y );
					buffer->Write< f32 >( v.z );
				} break;

				case MetaPropertyType::EntityHandle:
				{ 
					// Need size, so create another ByteBuffer and serialize once, then get the size of the buffer after the write 
					// Better way would be to be able to write, calculate the delta, shift the buffer over by writing this BEFORE 
					// serializing out the entity, but I can worry about that later...
					ByteBuffer temp;
					EntityArchiver::Serialize( *cls->GetValueAs< EntityHandle >( this, prop ), &temp );
					usize bufferSize = temp.GetSize( ); 
					buffer->Write< usize >( bufferSize );

					// Serialize entity data
					EntityArchiver::Serialize( *cls->GetValueAs< EntityHandle >( this, prop ), buffer ); 
				} break;
			} 
		}

		return Enjon::Result::SUCCESS;
	}

	Enjon::Result TestProject::DeserializeData( Enjon::ByteBuffer* buffer )
	{
		// Get property count
		usize propCount = buffer->Read< usize >( );

		const MetaClass* cls = this->Class( );

		for ( usize i = 0; i < propCount; ++i )
		{
			const MetaProperty* prop = cls->GetPropertyByName( buffer->Read< String >( ) );
			MetaPropertyType propType = ( MetaPropertyType )buffer->Read< s32 >( ); 
			usize propSize = buffer->Read< usize >( );

			// Can proceed to deserialize IFF the property exists and its type is correct 
			if ( prop && propType == prop->GetType() )
			{ 
				switch ( propType )
				{
					case MetaPropertyType::F32:
					{
						cls->SetValue( this, prop, buffer->Read< f32 >() );
					} break;

					case MetaPropertyType::Vec3:
					{
						f32 x = buffer->Read< f32 >( );
						f32 y = buffer->Read< f32 >( );
						f32 z = buffer->Read< f32 >( );
						cls->SetValue( this, prop, Vec3( x, y, z ) ); 
					} break;

					case MetaPropertyType::EntityHandle:
					{
						EntityHandle handle = EntityArchiver::Deserialize( buffer );
						cls->SetValue( this, prop, handle ); 
					} break;
				} 
			}
			// Otherwise property doesn't exist, so yeah...
			else
			{
				// Error... must skip ahead in buffer by size provided by type
				buffer->AdvanceReadPosition( propSize ); 
			}
		} 

		// Have to read the graphics component to the scene, unfortunately...
		if ( mEntity.Get() )
		{
			auto gfx = mEntity.Get( )->GetComponent< Enjon::GraphicsComponent >( );
			if ( gfx )
			{
				Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetScene( )->AddRenderable( gfx->GetRenderable( ) );
			}
		}
		if ( mFloor.Get() )
		{
			auto gfx = mFloor.Get( )->GetComponent< Enjon::GraphicsComponent >( );
			if ( gfx )
			{
				Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetScene( )->AddRenderable( gfx->GetRenderable( ) );
			}
		}

		return Enjon::Result::SUCCESS; 
	}

	////////////////////////////////////////////////////
	// Reflection //////////////////////////////////////

	template <>
	Enjon::MetaClass* Enjon::Object::ConstructMetaClass< Enjon::TestProject >( )
	{
		MetaClass* cls = new MetaClass( );

		// Construct properties
		cls->mPropertyCount = 5;
		cls->mProperties.resize( cls->mPropertyCount );
		cls->mProperties[ 0 ] = new Enjon::MetaProperty( MetaPropertyType::EntityHandle, "mFloor", ( u32 )&( ( TestProject* )0 )->mFloor, 0, MetaPropertyTraits( false, 0.000000f, 0.000000f ) );
		cls->mProperties[ 1 ] = new Enjon::MetaProperty( MetaPropertyType::F32, "mRotationTime", ( u32 )&( ( TestProject* )0 )->mRotationTime, 1, MetaPropertyTraits( false, 0.000000f, 0.000000f ) );
		cls->mProperties[ 2 ] = new Enjon::MetaProperty( MetaPropertyType::Vec3, "mRotationAxis", ( u32 )&( ( TestProject* )0 )->mRotationAxis, 2, MetaPropertyTraits( false, 0.000000f, 0.000000f ) );
		cls->mProperties[ 3 ] = new Enjon::MetaProperty( MetaPropertyType::F32, "mRotationSpeed", ( u32 )&( ( TestProject* )0 )->mRotationSpeed, 3, MetaPropertyTraits( false, 0.000000f, 0.000000f ) ); 
		cls->mProperties[ 4 ] = new Enjon::MetaProperty( MetaPropertyType::EntityHandle, "mEntity", ( u32 )&( ( TestProject* )0 )->mEntity, 4, MetaPropertyTraits( false, 0.000000f, 0.000000f ) );
 
		// Construct functions
		cls->mFunctionCount = 0;

		cls->mTypeId = 134;

		cls->mName = "TestProject";

		return cls;
	}

	// GetClassInternal
	const Enjon::MetaClass* Enjon::TestProject::GetClassInternal( ) const
	{
		Enjon::MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance( )->GetMetaClassRegistry( ) );
		const MetaClass* cls = mr->Get< TestProject >( );
		if ( !cls )
		{
			cls = mr->RegisterMetaClass< TestProject >( );
		}
		return cls;
	}

	// MetaClassRegistry::GetTypeId
	template <>
	Enjon::u32 MetaClassRegistry::GetTypeId< TestProject >( ) const
	{
		return 134;
	}

}

