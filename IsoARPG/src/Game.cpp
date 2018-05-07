#include "Game.h"

#include <Entity/EntityManager.h>
#include <Entity/Components/GraphicsComponent.h>
#include <Entity/Components/PointLightComponent.h>
#include <Graphics/GraphicsSubsystem.h>
#include <Graphics/DirectionalLight.h>
#include <Graphics/QuadBatch.h>
#include <Math/Vec3.h>
#include <Math/Random.h>
#include <Math/Common.h>
#include <System/Types.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>
#include <Asset/Asset.h>
#include <Asset/AssetManager.h>
#include <Asset/TextureAssetLoader.h>
#include <Asset/MeshAssetLoader.h>
#include <Asset/ShaderGraphAssetLoader.h>
#include <Asset/FontAssetLoader.h>
#include <Utils/FileUtils.h>
#include <Utils/Signal.h>
#include <Utils/Property.h>
#include <Graphics/Font.h>
#include <Serialize/UUID.h>
#include <Serialize/ByteBuffer.h> 
#include <Engine.h>
#include <Graphics/ShaderGraph.h>
#include <Serialize/ObjectArchiver.h>
#include <Serialize/AssetArchiver.h>
#include <SubsystemCatalog.h>
#include <Serialize/EntityArchiver.h>

#include <fmt/printf.h>
#include <lz4/lz4.h>

#include <Defines.h>
#include <Engine.h>

#include <stdio.h>

#include <array>
#include <iostream>
#include <string.h>
#include <filesystem>

#include <STB/stb_image.h>

#include <STB/stb_image_write.h> 

#include <Bullet/btBulletDynamicsCommon.h> 

#include <Base/MetaClassRegistry.h>
#include <ImGui/imgui.h>

using namespace Enjon;

#define ADD_ASSET_TO_PROJECT 1

struct ActiveSceneWrapper
{
	Enjon::Vector< Enjon::EntityHandle > mEntities;
	Enjon::Camera mCamera;

	void Unload( )
	{ 
		// Deallocate entities after writing
		for ( auto& e : mEntities )
		{
			// Have to destroy children first as well...
			for ( auto& c : e.Get( )->GetChildren( ) )
			{
				c.Get( )->Destroy( ); 
			}

			// Destroy entity
			e.Get( )->Destroy( ); 
		} 

		// Clear entities
		mEntities.clear( );
	}
};

Enjon::Texture* mNewTexture = nullptr;
Enjon::Mesh* mMesh = nullptr;
Enjon::PointLight* mPointLight = nullptr;

std::vector<btRigidBody*> mBodies;
btDiscreteDynamicsWorld* mDynamicsWorld;

// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
btAlignedObjectArray<btCollisionShape*> collisionShapes;

// Physics entities to align with rigid bodies
std::vector<Enjon::Entity*> mPhysicsEntities;

Enjon::String mAssetsDirectoryPath; 
Enjon::AssetManager* mAssetManager;

f32 ballSpeed = 10.0f;

Enjon::Signal<f32> testSignal;
Enjon::Property<f32> testProperty;

struct ActiveSceneWrapper mActiveScene;

void Game::TestObjectSerialize( )
{
	using namespace Enjon;

	AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->ConstCast< AssetManager >();
	TestNamespace::PointLight writeTestObject; 
	ObjectArchiver archiver; 
	FontAssetLoader assetLoader;

	// Set value and texture
	writeTestObject.mFloatValue = 1.0f;
	writeTestObject.mUintValue = 3;
	writeTestObject.mTexture = am->GetAsset< Texture >( "materials.scuffedplastic.albedo" );
	writeTestObject.mID = UUID::GenerateUUID( );
	writeTestObject.mName = assetLoader.Class()->GetName();
	writeTestObject.mIntValue = -23; 
	writeTestObject.mHashMap["Bob"] = 5;
	writeTestObject.mHashMap["Billy"] = 1234;
	writeTestObject.mHashMap["John"] = 354;
	writeTestObject.mHashMap["Mark"] = 3;

	writeTestObject.mStaticArrayConstant[0] = "Zero";
	writeTestObject.mStaticArrayConstant[1] = "One";
	writeTestObject.mStaticArrayConstant[2] = "Two";
 
	for ( Enjon::u32 i = 0; i < Enjon::kConstantValue; ++i )
	{
		writeTestObject.mStaticArrayConstVariable[i] = i * 0.215f;
	}

	for ( Enjon::u32 i = 0; i < 5; ++i )
	{
		writeTestObject.mDynamicArray.push_back( i * 2 );
	}

	// Serialize test object
	archiver.Serialize( &writeTestObject ); 

	String outputPath = am->GetAssetsDirectoryPath( ) + "/Cache/testObject";

	// Write to file
	archiver.WriteToFile( outputPath ); 
 
	// Cool, now de-serialize all objects in file
	Vector<Object*> objects;
	archiver.Deserialize( outputPath, objects ); 

	// Test serializing/deserializing directional lights in a scene
	Enjon::u32 i = 0;
	for ( auto& dl : mGfx->GetGraphicsScene()->GetDirectionalLights() )
	{ 
		String outputAssetPath = am->GetCachedAssetsDirectoryPath( ) + "directionalLight" + std::to_string(i);
		ObjectArchiver oa;
		oa.Serialize( dl );
		oa.WriteToFile( outputAssetPath ); 
		oa.Deserialize( outputAssetPath, objects ); 
		i++;
	} 

	{ 
		// Will not serialize if not needed!
		Enjon::String assetPath( "Textures/beast.png" );
		am->AddToDatabase( assetPath );

		// Will not serialize if not needed!
		assetPath = Enjon::String( "Models/bb8.obj" );
		am->AddToDatabase( assetPath );

		Enjon::AssetHandle< Enjon::Texture > cachedTexture = am->GetAsset< Enjon::Texture >( "textures.beast" );
		Enjon::Material* gunMat = const_cast< Enjon::Material* >( mGun.Get( )->GetComponent< GraphicsComponent >( )->GetMaterial( ).Get() );
		//gunMat->SetTexture( Enjon::TextureSlotType::Albedo, cachedTexture );

		Enjon::AssetHandle< Enjon::Mesh > cachedMesh = am->GetAsset< Enjon::Mesh >( "models.unit_cube" );
		mGun.Get( )->GetComponent< GraphicsComponent >( )->SetMesh( cachedMesh );
	} 

	{
		AssetArchiver assetArchiver; 
		assetArchiver.Serialize( am->GetAsset< Enjon::Mesh >( "models.unit_cube" ).Get( ) );
	}

	{
		// A way to construct new types of objects - Will be given all default parameters when constructed - NEEDS TO GO THROUGH FACTORY FOR THIS EVENTUALLY
		// Will not construct if named asset already exists in loader - will return asset with that name
		Enjon::AssetHandle< Enjon::Material > newMat = am->ConstructAsset< Enjon::Material >( "NewMaterial" ); 

		// Construct cerebus material and save
		Enjon::AssetHandle< Enjon::Material > cerebusMat = am->ConstructAsset< Enjon::Material >( "CerebusMaterial" ); 
		if ( 1 )
		{
			cerebusMat.Get( )->SetShaderGraph( am->GetAsset< Enjon::ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" ) );
			cerebusMat.Get( )->ConstCast< Material >()->SetUniform( "albedoMap", am->GetAsset< Enjon::Texture >( "materials.cerebus.albedo" ) );
			cerebusMat.Get( )->ConstCast< Material >()->SetUniform( "normalMap", am->GetAsset< Enjon::Texture >( "materials.cerebus.normal" ) ); 
			cerebusMat.Get( )->ConstCast< Material >()->SetUniform( "metallicMap", am->GetAsset< Enjon::Texture >( "materials.cerebus.metallic" ) );
			cerebusMat.Get( )->ConstCast< Material >()->SetUniform( "roughMap", am->GetAsset< Enjon::Texture >( "materials.cerebus.roughness" ) );
			cerebusMat.Get( )->ConstCast< Material >()->SetUniform( "emissiveMap", am->GetAsset< Enjon::Texture >( "materials.cerebus.emissive" ) );
			cerebusMat.Get( )->ConstCast< Material >()->SetUniform( "emissiveIntensity", 10.0f );
			cerebusMat.Save( ); 
		}

		Enjon::AssetHandle< Enjon::Material > mahogFloorMat = am->ConstructAsset< Enjon::Material >( "MahogFloorMaterial" );
		if ( 1 )
		{ 
			mahogFloorMat.Get( )->SetShaderGraph( am->GetAsset< Enjon::ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" ) );
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "albedoMap", am->GetAsset< Enjon::Texture >( "materials.mahogfloor.albedo" ) );
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "normalMap", am->GetAsset< Enjon::Texture >( "materials.mahogfloor.normal" ) ); 
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "metallicMap", am->GetAsset< Enjon::Texture >( "textures.black" ) );
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "roughMap", am->GetAsset< Enjon::Texture >( "materials.mahogfloor.roughness" ) );
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "emissiveMap", am->GetAsset< Enjon::Texture >( "textures.black" ) );
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "aoMap", am->GetAsset< Enjon::Texture >( "materials.mahogfloor.ao" ) );
			mahogFloorMat.Get( )->ConstCast< Material >()->SetUniform( "emissiveIntensity", 0.0f );
			mahogFloorMat.Save( ); 
		}

		Enjon::AssetHandle< Enjon::Material > paintPeelingMat = am->ConstructAsset< Enjon::Material >( "PaintPeeingMaterial" ); 
		if ( 1 )
		{
			paintPeelingMat.Get( )->SetShaderGraph( am->GetAsset< Enjon::ShaderGraph >( "shaders.shadergraphs.testgraph" ) );
			paintPeelingMat.Get( )->ConstCast< Material >()->SetUniform( "albedoMap", am->GetAsset< Enjon::Texture >( "materials.paintpeeling.albedo" ) );
			paintPeelingMat.Get( )->ConstCast< Material >()->SetUniform( "normalMap", am->GetAsset< Enjon::Texture >( "materials.paintpeeling.normal" ) ); 
			paintPeelingMat.Get( )->ConstCast< Material >()->SetUniform( "metallicMap", am->GetAsset< Enjon::Texture >( "materials.paintpeeling.metallic" ) );
			paintPeelingMat.Get( )->ConstCast< Material >()->SetUniform( "roughMap", am->GetAsset< Enjon::Texture >( "materials.paintpeeling.roughness" ) );
			paintPeelingMat.Save( ); 
		}

		Enjon::AssetHandle< Enjon::Material > harshBricksMat = am->ConstructAsset< Enjon::Material >( "HarshBricksMaterial" );
		if ( 1 )
		{
			harshBricksMat.Get( )->SetShaderGraph( am->GetAsset< Enjon::ShaderGraph >( "shaders.shadergraphs.testgraph" ) );
			harshBricksMat.Get( )->ConstCast< Material >()->SetUniform( "albedoMap", am->GetAsset< Enjon::Texture >( "materials.harshbricks.albedo" ) );
			harshBricksMat.Get( )->ConstCast< Material >()->SetUniform( "normalMap", am->GetAsset< Enjon::Texture >( "materials.harshbricks.normal" ) ); 
			harshBricksMat.Get( )->ConstCast< Material >()->SetUniform( "metallicMap", am->GetAsset< Enjon::Texture >( "materials.harshbricks.metallic" ) );
			harshBricksMat.Get( )->ConstCast< Material >()->SetUniform( "roughMap", am->GetAsset< Enjon::Texture >( "materials.harshbricks.roughness" ) );
			harshBricksMat.Save( ); 
		}

		// Set gun material
		mGun.Get( )->GetComponent< GraphicsComponent >( )->SetMaterial( paintPeelingMat.Get( ) );
	}

	// Serialize / Deserialize entity information
	{
		if ( 1 )
		{
			// Try serializing the camera Enjon::ObjectArchiver archiver;
			archiver.Serialize( mGfx->GetGraphicsSceneCamera( ) );
			archiver.WriteToFile( am->GetCachedAssetsDirectoryPath() + "camera" ); 
		}
		if ( 0 )
		{
			// Parent
			mSerializedEntity = mEntities->Allocate( ); 
			mSerializedEntity.Get( )->SetLocalPosition( Vec3( -8, 3, 0 ) );
			mSerializedEntity.Get( )->SetLocalRotation( Quaternion::AngleAxis( Math::ToRadians( 20.0f ), Vec3::XAxis( ) ) );
			mSerializedEntity.Get( )->SetLocalScale( 0.5f );
			auto gfxCmp = mSerializedEntity.Get( )->AddComponent< GraphicsComponent >( );
			auto plCmp = mSerializedEntity.Get( )->AddComponent< PointLightComponent >( );
			plCmp->SetColor( ColorRGBA32( 1.0f, 0.0f, 0.0f, 1.0f ) );
			plCmp->SetIntensity( 30.0f );
			plCmp->SetRadius( 50.0f );
			gfxCmp->SetMaterial( am->GetAsset< Material >( "NewMaterial" ).Get( ) );
			gfxCmp->SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) ); 

			// Child
			EntityHandle child = mEntities->Allocate( );
			auto childGfx = child.Get( )->AddComponent< GraphicsComponent >( );
			childGfx->SetMaterial( am->GetAsset< Material >( "NewMaterial" ).Get( ) );
			childGfx->SetMesh( am->GetAsset< Mesh >( "models.monkey" ) );
			mSerializedEntity.Get( )->AddChild( child );
			child.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 0, 1.5f, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.5f ) ) );

			// Other children
			EntityHandle child0 = mEntities->Allocate( );
			auto child0Gfx = child0.Get( )->AddComponent< GraphicsComponent >( );
			child0Gfx->SetMaterial( am->GetAsset< Material >( "NewMaterial" ).Get( ) );
			child0Gfx->SetMesh( am->GetAsset< Mesh >( "models.monkey" ) );
			child.Get( )->AddChild( child0 );
			child0.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( -2, 2, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.75f ) ) ); 

			EntityHandle child1 = mEntities->Allocate( );
			auto child1Gfx = child1.Get( )->AddComponent< GraphicsComponent >( );
			child1Gfx->SetMaterial( am->GetAsset< Material >( "NewMaterial" ).Get( ) );
			child1Gfx->SetMesh( am->GetAsset< Mesh >( "models.monkey" ) );
			child.Get( )->AddChild( child1 );
			child1.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 2, 2, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.75f ) ) ); 

			// Test serializing entity data
			EntityArchiver archiver; 
			archiver.Serialize( mSerializedEntity ); 
			String path = am->GetAssetsDirectoryPath( ) + "/Cache/testEntity";
			archiver.WriteToFile( path ); 

			mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( gfxCmp->GetRenderable( ) );
			mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( childGfx->GetRenderable( ) );
			mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( child0Gfx->GetRenderable( ) );
			mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( child1Gfx->GetRenderable( ) );
			mGfx->GetGraphicsScene( )->AddPointLight( plCmp->GetLight( ) );
		}
		else
		{
			EntityArchiver archiver;
			String path = am->GetAssetsDirectoryPath( ) + "/Cache/testEntity";
			mSerializedEntity = archiver.Deserialize( path );
			if ( mSerializedEntity.Get( ) )
			{
				auto gfxCmp = mSerializedEntity.Get( )->GetComponent< GraphicsComponent >( );
				if ( gfxCmp )
				{
					mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( gfxCmp->GetRenderable() );
				}

				auto pointLightComp = mSerializedEntity.Get( )->GetComponent< PointLightComponent >( );
				if ( pointLightComp )
				{
					mGfx->GetGraphicsScene( )->AddPointLight( pointLightComp->GetLight( ) );
				}

				auto addChildrenGfx = [ & ] ( const EntityHandle& handle )
				{
					for ( auto& c : handle.Get( )->GetChildren( ) )
					{
						auto cGfxCmp = c.Get( )->GetComponent< GraphicsComponent >( );
						if ( cGfxCmp )
						{
							mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( cGfxCmp->GetRenderable( ) );
						} 
					}
				};

				// Need an automated way of doing this in the graphics component ( maybe in late update or something )
				for ( auto& c : mSerializedEntity.Get( )->GetChildren( ) )
				{
					auto cGfxCmp = c.Get( )->GetComponent< GraphicsComponent >( );
					if ( cGfxCmp )
					{ 
						mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( cGfxCmp->GetRenderable( ) );
					}

					addChildrenGfx( c );
				}
			}
		}
	}

	// Serialize active scenes
	{
		bool serializeScene1 = false;
		bool serializeScene2 = false;

		// Scene 1
		if ( serializeScene1 )
		{
			Enjon::EntityHandle handle = mEntities->Allocate( );
			auto gfxCmp = handle.Get( )->AddComponent( Enjon::Object::GetClass< Enjon::GraphicsComponent >( ) )->ConstCast< Enjon::GraphicsComponent >();
			if ( gfxCmp )
			{
				gfxCmp->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_cube" ) );
				gfxCmp->SetMaterial( am->GetAsset< Enjon::Material >( "NewMaterial2" ).Get() );
			}
			handle.Get( )->SetLocalPosition( Enjon::Vec3( 1.0, 3.0f, -10.0f ) );
			handle.Get( )->SetLocalScale( Enjon::Vec3( 2.0f ) );
			handle.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( Math::ToRadians( 45.0f ), Enjon::Vec3::ZAxis( ) ) );

			Enjon::EntityHandle child = mEntities->Allocate( );
			auto childGfxCmp = child.Get( )->AddComponent< Enjon::GraphicsComponent >( );
			if ( childGfxCmp )
			{
				childGfxCmp->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_sphere" ) );
				childGfxCmp->SetMaterial( am->GetAsset< Enjon::Material >( "NewMaterial1" ).Get( ) );
			}
			handle.Get( )->AddChild( child );
			child.Get( )->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 0.0f, 2.0f, 0.0f ), Enjon::Quaternion( ), Enjon::Vec3( 0.25f ) ) );

			// Push back top-level entities
			Enjon::Vector< Enjon::EntityHandle > entityHandles; 
			entityHandles.push_back( handle );
			
			// Construct camera
			Enjon::Camera cam = *mGfx->GetGraphicsSceneCamera( );
			cam.SetPosition( Enjon::Vec3( 5.425f, 7.267f, -6.224f ) );
			cam.LookAt( handle.Get( )->GetWorldPosition( ) ); 

			ByteBuffer buffer;

			// Serialize camera
			Enjon::ObjectArchiver::Serialize( &cam, &buffer );
			
			// Serialize entity data
			buffer.Write< Enjon::u32 >( entityHandles.size() );
			for ( auto& e : entityHandles )
			{
				Enjon::EntityArchiver::Serialize( e, &buffer );
			}
	
			// Write to file
			buffer.WriteToFile( am->GetCachedAssetsDirectoryPath( ) + "scene_01" );

			// Deallocate entities after writing
			// Have to destroy children first as well...
			for ( auto& e : entityHandles )
			{
				for ( auto& c : e.Get( )->GetChildren( ) )
				{
					c.Get( )->Destroy( ); 
				}

				// Destroy entity
				e.Get( )->Destroy( ); 
			}
		}

		// Scene 2
		if ( serializeScene2 )
		{
			Enjon::EntityHandle handle = mEntities->Allocate( );
			auto gfxCmp = handle.Get( )->AddComponent( Enjon::Object::GetClass< Enjon::GraphicsComponent >( ) )->ConstCast< Enjon::GraphicsComponent >( );
			if ( gfxCmp )
			{
				gfxCmp->SetMesh( am->GetAsset< Enjon::Mesh >( "models.unit_sphere" ) );
				gfxCmp->SetMaterial( am->GetAsset< Enjon::Material >( "NewMaterial3" ).Get( ) );
			}
			auto plCmp = handle.Get( )->AddComponent( Enjon::Object::GetClass< Enjon::PointLightComponent >( ) )->ConstCast< Enjon::PointLightComponent >( );
			if ( plCmp )
			{
				plCmp->SetColor( Enjon::ColorRGBA32( 0.0f, 1.0f, 0.0f, 1.0f ) );
				plCmp->SetIntensity( 1000.0f );
				plCmp->SetRadius( 100.0f );
			}
			handle.Get( )->SetLocalPosition( Enjon::Vec3( 1.0, 2.0f, -10.0f ) );
			handle.Get( )->SetLocalScale( Enjon::Vec3( 3.0f ) );

			Enjon::Vector< Enjon::EntityHandle > entityHandles; 
			entityHandles.push_back( handle );
			
			// Construct camera
			Enjon::Camera cam = *mGfx->GetGraphicsSceneCamera( );
			cam.SetPosition( Enjon::Vec3( -3.9f, 6.2f, -4.6f ) );
			cam.LookAt( handle.Get( )->GetWorldPosition( ) ); 

			ByteBuffer buffer;

			// Serialize camera
			Enjon::ObjectArchiver::Serialize( &cam, &buffer );
			
			// Serialize entity data
			buffer.Write< Enjon::u32 >( entityHandles.size() );
			for ( auto& e : entityHandles )
			{
				Enjon::EntityArchiver::Serialize( e, &buffer );
			}
	
			// Write to file
			buffer.WriteToFile( am->GetCachedAssetsDirectoryPath( ) + "scene_02" );

			// Deallocate entities after writing
			for ( auto& e : entityHandles )
			{
				e.Get( )->Destroy( );
			}
		}
	}
}

//-------------------------------------------------------------

Game::Game()
{
	mApplicationName = "IsoARPG";
}

//-------------------------------------------------------------

Game::~Game()
{
}

//-------------------------------------------------------------

/*
Enjon::Result Game::Initialize( )
{
	//====================================
	// Basic project initialization
	//==================================== 
	// Set up assets path
	// This needs to be done in a project settings config file or in the cmake, not in source 
	mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("/IsoARPG/Assets/"); 
	Enjon::String cacheDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("/IsoARPG/Assets/Cache/"); 
	Enjon::String projectDirectory = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + "/IsoARPG/"; 
	
	// Get asset manager and set its properties ( I don't like this )
	mAssetManager = Enjon::Engine::GetInstance()->GetSubsystemCatalog()->Get<Enjon::AssetManager>()->ConstCast< Enjon::AssetManager >(); 
	// This also needs to be done through a config file or cmake
	mAssetManager->SetAssetsDirectoryPath( mAssetsDirectoryPath );
	mAssetManager->SetCachedAssetsDirectoryPath( cacheDirectoryPath );
	mAssetManager->SetDatabaseName( GetApplicationName( ) ); 
	mAssetManager->Initialize( ); 

	// Get Subsystems from engine
	Enjon::Engine* engine = Enjon::Engine::GetInstance();
	Enjon::SubsystemCatalog* subSysCatalog = engine->GetSubsystemCatalog();
	mGfx = subSysCatalog->Get<Enjon::GraphicsSubsystem>()->ConstCast< Enjon::GraphicsSubsystem >();
	mInput = subSysCatalog->Get<Enjon::Input>()->ConstCast< Enjon::Input >();

	//====================================

	// Loading a basic scene
	// How do?

	// Grab a scene from the assets manager? Does the project hold a uuid of some default scene that's to be loaded? Most likely...
	

}
*/

Enjon::Result Game::Initialize()
{ 
	// Set up assets path
	// This needs to be done in a project settings config file or in the cmake, not in source 
	mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("IsoARPG/Assets/"); 
	Enjon::String cacheDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("IsoARPG/Assets/Cache/"); 
	Enjon::String projectDirectory = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + "IsoARPG/"; 
	
	// Get asset manager and set its properties ( I don't like this )
	mAssetManager = Enjon::Engine::GetInstance()->GetSubsystemCatalog()->Get<Enjon::AssetManager>()->ConstCast< Enjon::AssetManager >(); 

	// This also needs to be done through a config file or cmake
	mAssetManager->SetAssetsDirectoryPath( mAssetsDirectoryPath );
	mAssetManager->SetCachedAssetsDirectoryPath( cacheDirectoryPath );
	mAssetManager->SetDatabaseName( GetApplicationName( ) ); 
	mAssetManager->Initialize( ); 

	// Get Subsystems from engine
	Enjon::Engine* engine = Enjon::Engine::GetInstance();
	Enjon::SubsystemCatalog* subSysCatalog = engine->GetSubsystemCatalog();
	mGfx = subSysCatalog->Get<Enjon::GraphicsSubsystem>()->ConstCast< Enjon::GraphicsSubsystem >();
	mInput = subSysCatalog->Get<Enjon::Input>()->ConstCast< Enjon::Input >();

	// Paths to resources
	Enjon::String toyBoxDispPath		= Enjon::String("Textures/toy_box_disp.png");
	Enjon::String toyBoxNormalPath		= Enjon::String("Textures/toy_box_normal.png");
	Enjon::String eyePath				= Enjon::String("Models/eye.obj");
	Enjon::String noisePath				= Enjon::String("Textures/worleyNoise.png");
	Enjon::String grassTexturePath		= Enjon::String("Textures/grass.png");
	Enjon::String quadPath				= Enjon::String("Models/quad.obj");
	Enjon::String rockPath				= Enjon::String("Models/rock.obj");
	Enjon::String cerebusMeshPath		= Enjon::String("Models/cerebus.obj");
	Enjon::String buddhaMeshPath		= Enjon::String("Models/buddha.obj");
	Enjon::String bunnyMeshPath			= Enjon::String("Models/bunny.obj");
	Enjon::String dragonMeshPath		= Enjon::String("Models/dragon.obj");
	Enjon::String monkeyMeshPath		= Enjon::String("Models/monkey.obj");
	Enjon::String sphereMeshPath		= Enjon::String("Models/unit_sphere.obj");
	Enjon::String cubeMeshPath			= Enjon::String("Models/unit_cube.obj");
	Enjon::String shaderballPath		= Enjon::String("Models/shaderball.obj");
	Enjon::String unitSpherePath		= Enjon::String("Models/unit_sphere.obj");
	Enjon::String unrealShaderBallPath	= Enjon::String("Models/unreal_shaderball.obj");
	Enjon::String unitShaderBallPath	= Enjon::String("Models/unit_shaderball.obj");
	Enjon::String catMeshPath			= Enjon::String("Models/cat.obj");
	Enjon::String dudeMeshPath			= Enjon::String("Models/dude.obj");
	Enjon::String shaderBallMeshPath	= Enjon::String("Models/shaderball.obj");
	Enjon::String cerebusAlbedoPath		= Enjon::String("Materials/Cerebus/Albedo.png"); 
	Enjon::String cerebusNormalPath		= Enjon::String("Materials/Cerebus/Normal.png"); 
	Enjon::String cerebusMetallicPath	= Enjon::String("Materials/Cerebus/Metallic.png"); 
	Enjon::String cerebusRoughnessPath	= Enjon::String("Materials/Cerebus/Roughness.png"); 
	Enjon::String cerebusEmissivePath	= Enjon::String("Materials/Cerebus/Emissive.png"); 
	Enjon::String mahogAlbedoPath		= Enjon::String("Materials/MahogFloor/Albedo.png"); 
	Enjon::String mahogNormalPath		= Enjon::String("Materials/MahogFloor/Normal.png"); 
	Enjon::String mahogMetallicPath		= Enjon::String("Materials/MahogFloor/Roughness.png"); 
	Enjon::String mahogRoughnessPath	= Enjon::String("Materials/MahogFloor/Roughness.png"); 
	Enjon::String mahogEmissivePath		= Enjon::String("Materials/MahogFloor/Emissive.png"); 
	Enjon::String mahogAOPath			= Enjon::String("Materials/MahogFloor/AO.png"); 
	Enjon::String woodAlbedoPath		= Enjon::String("Materials/WoodFrame/Albedo.png"); 
	Enjon::String woodNormalPath		= Enjon::String("Materials/WoodFrame/Normal.png"); 
	Enjon::String woodRoughnessPath		= Enjon::String("Materials/WoodFrame/Roughness.png"); 
	Enjon::String woodMetallicPath		= Enjon::String("Materials/WoodFrame/Metallic.png"); 
	Enjon::String plasticAlbedoPath		= Enjon::String("Materials/ScuffedPlastic/Albedo.png"); 
	Enjon::String plasticNormalPath		= Enjon::String("Materials/ScuffedPlastic/Normal.png"); 
	Enjon::String plasticRoughnessPath	= Enjon::String("Materials/ScuffedPlastic/Roughness.png"); 
	Enjon::String plasticMetallicPath	= Enjon::String("Materials/ScuffedPlastic/Metallic.png"); 
	Enjon::String plasticAOPath			= Enjon::String("Materials/ScuffedPlastic/AO.png"); 
	Enjon::String wornRedAlbedoPath		= Enjon::String("Materials/RustedIron/Albedo.png"); 
	Enjon::String wornRedNormalPath		= Enjon::String("Materials/RustedIron/Normal.png"); 
	Enjon::String wornRedRoughnessPath	= Enjon::String("Materials/RustedIron/Roughness.png"); 
	Enjon::String wornRedMetallicPath	= Enjon::String("Materials/RustedIron/Metallic.png"); 
	Enjon::String scuffedGoldAlbedoPath		= Enjon::String("Materials/ScuffedGold/Albedo.png"); 
	Enjon::String scuffedGoldNormalPath		= Enjon::String("Materials/ScuffedGold/Normal.png"); 
	Enjon::String scuffedGoldMetallicPath	= Enjon::String("Materials/ScuffedGold/Metallic.png"); 
	Enjon::String scuffedGoldRoughnessPath	= Enjon::String("Materials/ScuffedGold/Roughness.png"); 
	Enjon::String paintPeelingAlbedoPath = Enjon::String( "Materials/PaintPeeling/Albedo.png" );
	Enjon::String paintPeelingNormalPath = Enjon::String( "Materials/PaintPeeling/Normal.png" );
	Enjon::String paintPeelingRoughnessPath = Enjon::String( "Materials/PaintPeeling/Roughness.png" );
	Enjon::String paintPeelingMetallicPath = Enjon::String( "Materials/PaintPeeling/Metallic.png" );
	Enjon::String mixedMossAlbedoPath	= Enjon::String( "Materials/MixedMoss/Albedo.png" );
	Enjon::String mixedMossNormalPath	= Enjon::String( "Materials/MixedMoss/Normal.png" );
	Enjon::String mixedMossMetallicPath	= Enjon::String( "Materials/MixedMoss/Metallic.png" );
	Enjon::String mixedMossRoughnessPath	= Enjon::String( "Materials/MixedMoss/Roughness.png" );
	Enjon::String mixedMossAOPath		= Enjon::String( "Materials/MixedMoss/AO.png" );
	Enjon::String rockAlbedoPath		= Enjon::String("Materials/CopperRock/Albedo.png"); 
	Enjon::String rockNormalPath		= Enjon::String("Materials/CopperRock/Normal.png"); 
	Enjon::String rockRoughnessPath		= Enjon::String("Materials/CopperRock/Roughness.png"); 
	Enjon::String rockMetallicPath		= Enjon::String("Materials/CopperRock/Metallic.png"); 
	Enjon::String rockEmissivePath		= Enjon::String("Materials/CopperRock/Emissive.png"); 
	Enjon::String rockAOPath			= Enjon::String("Materials/CopperRock/AO.png"); 
	Enjon::String frontNormalPath		= Enjon::String("Textures/front_normal.png"); 
	Enjon::String brdfPath				= Enjon::String("Textures/brdf.png"); 
	Enjon::String waterPath				= Enjon::String("Textures/water.png"); 
	Enjon::String greenPath				= Enjon::String("Textures/green.png"); 
	Enjon::String redPath				= Enjon::String("Textures/red.png"); 
	Enjon::String bluePath				= Enjon::String("Textures/blue.png"); 
	Enjon::String blackPath				= Enjon::String("Textures/black.png"); 
	Enjon::String midGreyPath			= Enjon::String("Textures/grey.png"); 
	Enjon::String lightGreyPath			= Enjon::String("Textures/light_grey.png"); 
	Enjon::String whitePath				= Enjon::String("Textures/white.png"); 
	Enjon::String teapotPath			= Enjon::String( "Models/teapot.obj" );
	Enjon::String swordPath				= Enjon::String( "Models/sword.obj" );
	Enjon::String eyeNormal				= Enjon::String( "Textures/eye_NORMAL.png" );
	Enjon::String eyeAlbedo				= Enjon::String( "Textures/eyeball_COLOR1.png" );
	Enjon::String shaderGraphPath		= Enjon::String( "Shaders/ShaderGraphs/testGraph.sg" );
	Enjon::String staticGeomGraphPath	= Enjon::String( "Shaders/ShaderGraphs/DefaultStaticGeom.sg" );

	// Try loading font
	Enjon::String rootPath = engine->GetConfig( ).GetRoot( );
	Enjon::String fontPath = rootPath + "Assets/Fonts/WeblySleek/weblysleekuisb.ttf"; 
	
	// Add to asset database( will serialize the asset if not loaded from disk, otherwise will load the asset )
	mAssetManager->AddToDatabase( toyBoxDispPath );
	mAssetManager->AddToDatabase( toyBoxNormalPath );
	mAssetManager->AddToDatabase( unitSpherePath );
	mAssetManager->AddToDatabase( shaderGraphPath );
	mAssetManager->AddToDatabase( staticGeomGraphPath );
	mAssetManager->AddToDatabase( eyePath );
	mAssetManager->AddToDatabase( eyeAlbedo );
	mAssetManager->AddToDatabase( eyeNormal );
	mAssetManager->AddToDatabase( dragonMeshPath );
	mAssetManager->AddToDatabase( swordPath );
	mAssetManager->AddToDatabase( noisePath );
	mAssetManager->AddToDatabase( grassTexturePath );
	mAssetManager->AddToDatabase( quadPath );
	mAssetManager->AddToDatabase( rockPath );
	mAssetManager->AddToDatabase( monkeyMeshPath );
	mAssetManager->AddToDatabase( cerebusAlbedoPath );
	mAssetManager->AddToDatabase( cerebusNormalPath );
	mAssetManager->AddToDatabase( cerebusMetallicPath );
	mAssetManager->AddToDatabase( cerebusRoughnessPath );
	mAssetManager->AddToDatabase( cerebusEmissivePath );
	mAssetManager->AddToDatabase( mahogAlbedoPath );
	mAssetManager->AddToDatabase( mahogNormalPath );
	mAssetManager->AddToDatabase( mahogMetallicPath );
	mAssetManager->AddToDatabase( mahogRoughnessPath );
	mAssetManager->AddToDatabase( mahogEmissivePath );
	mAssetManager->AddToDatabase( mahogAOPath );
	mAssetManager->AddToDatabase( woodAlbedoPath );
	mAssetManager->AddToDatabase( woodNormalPath );
	mAssetManager->AddToDatabase( woodMetallicPath );
	mAssetManager->AddToDatabase( woodRoughnessPath );
	mAssetManager->AddToDatabase( wornRedAlbedoPath );
	mAssetManager->AddToDatabase( wornRedRoughnessPath );
	mAssetManager->AddToDatabase( wornRedMetallicPath );
	mAssetManager->AddToDatabase( wornRedNormalPath );
	mAssetManager->AddToDatabase( plasticAlbedoPath );
	mAssetManager->AddToDatabase( plasticNormalPath );
	mAssetManager->AddToDatabase( plasticRoughnessPath );
	mAssetManager->AddToDatabase( plasticMetallicPath );
	mAssetManager->AddToDatabase( rockAlbedoPath );
	mAssetManager->AddToDatabase( rockNormalPath );
	mAssetManager->AddToDatabase( rockMetallicPath );
	mAssetManager->AddToDatabase( rockRoughnessPath );
	mAssetManager->AddToDatabase( rockEmissivePath );
	mAssetManager->AddToDatabase( rockAOPath );
	mAssetManager->AddToDatabase( scuffedGoldAlbedoPath );
	mAssetManager->AddToDatabase( scuffedGoldNormalPath );
	mAssetManager->AddToDatabase( scuffedGoldMetallicPath );
	mAssetManager->AddToDatabase( scuffedGoldRoughnessPath );
	mAssetManager->AddToDatabase( paintPeelingAlbedoPath );
	mAssetManager->AddToDatabase( paintPeelingNormalPath );
	mAssetManager->AddToDatabase( paintPeelingMetallicPath );
	mAssetManager->AddToDatabase( paintPeelingRoughnessPath );
	mAssetManager->AddToDatabase( mixedMossAlbedoPath );
	mAssetManager->AddToDatabase( mixedMossNormalPath );
	mAssetManager->AddToDatabase( mixedMossMetallicPath );
	mAssetManager->AddToDatabase( mixedMossRoughnessPath );
	mAssetManager->AddToDatabase( mixedMossAOPath );
	mAssetManager->AddToDatabase( plasticAOPath );
	mAssetManager->AddToDatabase( frontNormalPath );
	mAssetManager->AddToDatabase( brdfPath );
	mAssetManager->AddToDatabase( cerebusMeshPath );
	mAssetManager->AddToDatabase( sphereMeshPath );
	mAssetManager->AddToDatabase( cubeMeshPath );
	mAssetManager->AddToDatabase( catMeshPath );
	mAssetManager->AddToDatabase( bunnyMeshPath );
	mAssetManager->AddToDatabase( buddhaMeshPath );
	mAssetManager->AddToDatabase( shaderBallMeshPath );
	mAssetManager->AddToDatabase( unitShaderBallPath );
	mAssetManager->AddToDatabase( unrealShaderBallPath );
	mAssetManager->AddToDatabase( greenPath );
	mAssetManager->AddToDatabase( redPath );
	mAssetManager->AddToDatabase( midGreyPath );
	mAssetManager->AddToDatabase( lightGreyPath );
	mAssetManager->AddToDatabase( bluePath );
	mAssetManager->AddToDatabase( blackPath );
	mAssetManager->AddToDatabase( whitePath );
	mAssetManager->AddToDatabase( teapotPath );
	mAssetManager->AddToDatabase( waterPath );
	mAssetManager->AddToDatabase( fontPath, false, false );

	// Assign font
	Enjon::String fontQualifiedName = Enjon::AssetLoader::GetQualifiedName( fontPath ); 
	mFont = mAssetManager->GetAsset< Enjon::UIFont >( fontQualifiedName );

	testProperty = mCameraSpeed; 

	testProperty.OnChange( ).Connect( [&] ( f32 val ) 
	{
		fmt::print( "Property change: {}\n", testProperty.Get() );
	} );

	testSignal.Connect( [&] ( f32 val ) 
	{
		fmt::print( "Emitted value: {}\n", val );
	} );
	
	testSignal.Connect( [&] ( f32 val ) 
	{
		fmt::print( "Another connection!\n" );
	} );

	// Set sphere mesh
	mSphereMesh = mAssetManager->GetAsset<Enjon::Mesh>( "models.unit_sphere" );

	// Get entity manager
	mEntities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->ConstCast< Enjon::EntityManager >(); 

	// Allocate handle
	mGreen = mEntities->Allocate( );
	mRed = mEntities->Allocate( );
	mBlue = mEntities->Allocate( );
	mGun = mEntities->Allocate( );
	mRock = mEntities->Allocate( );
	mRock2 = mEntities->Allocate( );
	Enjon::GraphicsComponent* rgc = mRock.Get( )->AddComponent( Enjon::Object::GetClass< Enjon::GraphicsComponent >( ) )->ConstCast< Enjon::GraphicsComponent >( );
	auto rgc2 = mRock2.Get( )->AddComponent< Enjon::GraphicsComponent >( );
	auto gc = mGun.Get()->AddComponent<Enjon::GraphicsComponent>(); 
	auto pc = mGun.Get()->AddComponent<Enjon::PointLightComponent>(); 

	mRed.Get()->AddComponent< Enjon::GraphicsComponent >( );
	mGreen.Get()->AddComponent< Enjon::GraphicsComponent >( );
	mBlue.Get()->AddComponent< Enjon::GraphicsComponent >( );

	pc->GetLight( )->SetPosition( Enjon::Vec3( 10.0f, 2.0f, 4.0f ) );
	pc->GetLight( )->SetIntensity( 20.0f );
	pc->GetLight( )->SetAttenuationRate( 0.2f );
	pc->GetLight( )->SetRadius( 100.0f );
	pc->GetLight( )->SetColor( Enjon::RGBA32_Orange( ) );

	mGun.Get()->SetLocalPosition(Enjon::Vec3(0.0f, 0.0f, 0.0f));
	mGun.Get()->SetLocalRotation( Enjon::Quaternion::AngleAxis( 45.0f, Enjon::Vec3::ZAxis() ) );
	gc->SetMesh(mAssetManager->GetAsset<Enjon::Mesh>("models.cat"));
	gc->SetMaterial(mGoldMat);

	mSun = new Enjon::DirectionalLight();
	mSun->SetIntensity(1.5f);
	mSun->SetColor(Enjon::RGBA32_White());

	auto mSun2 = new Enjon::DirectionalLight(Enjon::Vec3(0.5f, 0.5f, -0.75f), Enjon::RGBA32_SkyBlue(), 10.0f); 

	mFontMat = new Enjon::Material( );
	auto f = mFont.Get( );
	f->GetAtlas( 14 );
	mFontMat->TwoSided( true );

	mGreen.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMaterial( mGreenMat );
	mGreen.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >("models.unit_cube" ) );
	mRed.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMaterial( mRedMat ); 
	mRed.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >("models.unit_cube" ) );
	mBlue.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMaterial( mBlueMat );
	mBlue.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >("models.unit_cube" ) );

	mGun.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 0, 5, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 1.5f ) ) );
	mGreen.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 0, 7, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.5f ) ) );
	mRed.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 5, 7, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.25f ) ) );
	mBlue.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( -5, 7, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.25f ) ) );

	mGun.Get()->AddChild( mGreen ); 
	mGreen.Get()->AddChild( mRed );
	mGreen.Get()->AddChild( mBlue ); 

	rgc2->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >( "models.buddha" ) );
	rgc->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >( "models.cerebus" ) );
	rgc->SetMaterial( mGunMat );
	rgc2->SetMaterial( mPlasticMat );
	mRock.Get( )->SetLocalPosition( Enjon::Vec3( 5.0f, 5.0f, 20.0f ) );
	mRock.Get( )->SetLocalScale( Enjon::Vec3( 5.0f ) );
	
	mRock2.Get( )->SetLocalPosition( Enjon::Vec3( 10.0f, 0.0f, 20.0f ) );

	mBatch = new Enjon::QuadBatch();
	mBatch->Init();
	mBatch->Begin();
	{
		const Enjon::s32 dimSize = 25;
		for (Enjon::s32 i = -dimSize; i < dimSize; ++i)
		{
			for (Enjon::s32 j = -dimSize; j < dimSize; ++j)
			{
				Enjon::Vec3 pos(j * 2.0f, 0.0f, i * 2.0f);
				Enjon::Quaternion rot = Enjon::Quaternion::AngleAxis( Math::ToRadians(90.0f), Enjon::Vec3::XAxis());
				Enjon::Vec3 scale(1.0f);
				Enjon::Transform t(pos, rot, scale);
				mBatch->Add(t);
				mBatch->SetMaterial(mFloorMat);
			}
		} 
	}
	mBatch->End(); 

	// Set up text batch
	mTextBatch = new Enjon::QuadBatch( );
	mTextBatch->Init( );
	mTextBatch->SetMaterial( mFontMat );

	enum class GreyScale
	{
		Black,
		Grey,
		LightGrey,
		White,
		Count
	};


	if (mGfx)
	{
		auto scene = mGfx->GetGraphicsScene();
		scene->AddDirectionalLight( mSun );
		//scene->AddDirectionalLight( mSun2 );
		scene->AddStaticMeshRenderable(gc->GetRenderable());
		scene->AddPointLight( pc->GetLight( ) );
		scene->AddStaticMeshRenderable( mGreen.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );
		scene->AddStaticMeshRenderable( mRed.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );
		scene->AddStaticMeshRenderable( mBlue.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) ); 
		scene->AddStaticMeshRenderable( mRock.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) ); 
		scene->AddStaticMeshRenderable( mRock2.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) ); 
		scene->AddQuadBatch(mBatch);
		scene->AddQuadBatch(mTextBatch);
		scene->SetAmbientColor(Enjon::SetOpacity(Enjon::RGBA32_White(), 0.1f));

		// Set graphics camera position
		Enjon::Camera* cam = mGfx->GetGraphicsSceneCamera()->ConstCast< Enjon::Camera >();
		cam->SetPosition(Enjon::Vec3(0.0f, 5.0f, -10.0f));
		cam->LookAt(Enjon::Vec3(0, 0, 0));
	} 

	// Set up test serializable object 
	mTestObject.mHashMap["Bob"] = 2;
	mTestObject.mHashMap["Billy"] = 234;
	mTestObject.mHashMap["John"] = 10;

	// Set up ImGui window
	mShowEntities = true;
	auto showEntities = [&]()
	{
		// Docking windows
		if (ImGui::BeginDock("Entities", &mShowEntities))
		{
			if ( ImGui::CollapsingHeader( "Scene##header" ) )
			{
				static Enjon::String sceneFilePath = "";
				char buffer[256];
				std::strncpy( buffer, sceneFilePath.c_str( ), 256 );
				if ( ImGui::InputText( "Scene Path", buffer, 256 ) )
				{
					sceneFilePath = Enjon::String( buffer );
				}

				if ( ImGui::Button( "Load Scene..." ) )
				{
					const Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
					Enjon::ByteBuffer buffer( am->GetCachedAssetsDirectoryPath( ) + sceneFilePath );
					if ( buffer.GetStatus( ) == Enjon::BufferStatus::ReadyToRead )
					{
						// Need to unload all of previous scene data
						mActiveScene.Unload( );

						// Deserialize camera data
						Enjon::ObjectArchiver::Deserialize( &buffer, &mActiveScene.mCamera );

						// Deserialize entities
						Enjon::u32 entityCount = buffer.Read< Enjon::u32 >( );
						for ( Enjon::u32 i = 0; i < entityCount; ++i )
						{
							Enjon::EntityHandle entityHandle = Enjon::EntityArchiver::Deserialize( &buffer );
							mActiveScene.mEntities.push_back( entityHandle );

							if ( entityHandle.Get( )->HasComponent< Enjon::GraphicsComponent >( ) )
							{
								mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( entityHandle.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );

								for ( auto& c : entityHandle.Get( )->GetChildren( ) )
								{
									if ( c.Get( )->HasComponent< Enjon::GraphicsComponent >( ) )
									{
										mGfx->GetGraphicsScene( )->AddStaticMeshRenderable( c.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );
									}
								}
							}
						}
					}

					// Set the scene camera to active scene's camera
					*mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( ) = mActiveScene.mCamera;
				} 
			}

			// Load camera
			if ( ImGui::Button( "Load Camera" ) )
			{ 
				const Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
				Enjon::ObjectArchiver deserializeBuffer;
				deserializeBuffer.Deserialize( am->GetCachedAssetsDirectoryPath( ) + "camera", mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( ) );
			}

			// Load file
			if ( ImGui::CollapsingHeader( "Load Resource" ) )
			{
				static Enjon::String resourceFilePath = "";
				char buffer[256];
				std::strncpy( buffer, resourceFilePath.c_str( ), 256 );
				if ( ImGui::InputText( "File Path", buffer, 256 ) )
				{
					resourceFilePath = Enjon::String( buffer );
				} 

				if ( ImGui::Button( "Load File" ) )
				{
					mAssetManager->AddToDatabase( resourceFilePath );
				}
			} 

			ImGui::Text( Enjon::String( "Entities: " + std::to_string( Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->GetActiveEntities( ).size( ) ) ).c_str() );
			ImGui::Text( "Gun:" );
			auto position 	= mGun.Get( )->GetLocalPosition();
			auto scale 		= mGun.Get( )->GetLocalScale();
			auto rotation 	= mGun.Get( )->GetLocalRotation();

			f32 pos[] = {position.x, position.y, position.z}; 
			f32 scl[] = {scale.x, scale.y, scale.z}; 
			f32 rot[] = {rotation.x, rotation.y, rotation.z}; 

			ImGui::InputFloat3("Position", pos);
			ImGui::InputFloat3("Scale", scl);
			ImGui::InputFloat3("Rotation", rot);

			mGun.Get( )->SetLocalPosition(v3(pos[0], pos[1], pos[2]));
			mGun.Get( )->SetLocalScale(v3(scl[0], scl[1], scl[2]));
			mGun.Get( )->SetLocalRotation(quat(rot[0], rot[1], rot[2], rotation.w));

			for ( auto c : mGun.Get( )->GetChildren( ) )
			{
				ImGui::Text( "Child Local:" );
				Enjon::Transform trans = c.Get( )->GetLocalTransform( );
				auto position 	= trans.GetPosition();
				auto scale 		= trans.GetScale();
				auto rotation 	= trans.GetRotation();

				f32 pos[] = {position.x, position.y, position.z}; 
				f32 scl[] = {scale.x, scale.y, scale.z}; 
				f32 rot[] = {rotation.x, rotation.y, rotation.z}; 

				ImGui::PushID( c.GetID( ) );
				ImGui::InputFloat3("Position", pos);
				ImGui::InputFloat3("Scale", scl);
				ImGui::InputFloat3("Rotation", rot);
				ImGui::PopID( ); 
			}
			
			for ( auto c : mGun.Get( )->GetChildren( ) )
			{
				ImGui::Text( "Child World:" );
				Enjon::Transform trans = c.Get( )->GetWorldTransform( );
				auto position 	= trans.GetPosition();
				auto scale 		= trans.GetScale();
				auto rotation 	= trans.GetRotation();

				f32 pos[] = {position.x, position.y, position.z}; 
				f32 scl[] = {scale.x, scale.y, scale.z}; 
				f32 rot[] = {rotation.x, rotation.y, rotation.z}; 

				ImGui::PushID( c.GetID( ) );
				ImGui::InputFloat3("Position", pos);
				ImGui::InputFloat3("Scale", scl);
				ImGui::InputFloat3("Rotation", rot);
				ImGui::PopID( ); 
			}

			if ( ImGui::SliderFloat( "Camera Speed", &mCameraSpeed, 0.1f, 20.0f ) )
			{
				testProperty = mCameraSpeed;
			} 

			ImGui::SliderFloat("Ball Speed", &ballSpeed, 0.1f, 100.0f);

			if ( ImGui::Button( "Emit signal" ) )
			{
				testSignal.Emit( mCameraSpeed );
			}

			auto cam = mGfx->GetGraphicsSceneCamera( );
			ImGui::InputFloat3( "Cam Position", ( float* )&cam->GetPosition( ) );
			ImGui::InputFloat4( "Cam Rotation", ( float* )&cam->GetRotation( ) );

			f32 ar = cam->GetAspectRatio( );
			ImGui::InputFloat( "AspectRatio", &ar );

			auto vp = mGfx->GetViewport( );
			ImGui::InputInt2( "View Dimensions", ( s32* )&vp );
			
			ImGui::Text( "32 bit prop size: %d", sizeof( Enjon::Property<f32> ) );
			ImGui::Text( "32 bit signal size: %d", sizeof( Enjon::Signal<f32> ) ); 

			ImGuiManager* igm = EngineSubsystem( ImGuiManager );

			if ( ImGui::CollapsingHeader( "Test Object" ) )
			{
				igm->DebugDumpObject( &mTestObject );
			}

			if ( ImGui::CollapsingHeader( "Scene" ) )
			{
				igm->DebugDumpObject( mGfx->GetGraphicsScene( ) );
			}

			if ( ImGui::CollapsingHeader( "Sun" ) )
			{
				igm->DebugDumpObject( mSun ); 
			}

			// Testing meta functions
			if ( ImGui::CollapsingHeader( "Entity" ) )
			{ 
				const Enjon::Material* gfxMat = mGun.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetMaterial( ).Get();
				igm->DebugDumpObject( gfxMat ); 
			} 

			if ( ImGui::CollapsingHeader( "PointLight" ) )
			{
				auto plc = mGun.Get( )->GetComponent< Enjon::PointLightComponent >( );
				auto light = plc->GetLight( );
				igm->DebugDumpObject( light );

				Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* > ( light->Class( ) ); 
				Enjon::MetaFunction* getWPFunc = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "GetPosition" ) );
				Enjon::MetaFunction* setWPFunc = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "SetLocalPosition" ) );
				if ( getWPFunc )
				{
					Enjon::Vec3 wp = getWPFunc->Invoke< Enjon::Vec3 >( light );
					Enjon::String label = "Func: " + getWPFunc->GetName( );
					if ( ImGui::SliderFloat3( label.c_str( ), ( float* )&wp, 0.0f, 100.0f ) )
					{
						if ( setWPFunc )
						{
							setWPFunc->Invoke< void >( light, wp );
						}
					}
				} 
			}
			if ( ImGui::CollapsingHeader( "Textures" ) )
			{
				auto am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( )->ConstCast< Enjon::AssetManager >();
				auto textures = am->GetAssets< Enjon::Texture >( );
				for ( auto& t : *textures ) 
				{
					if ( t.second.GetAsset() )
					{
						const Enjon::Texture* tex = t.second.GetAsset()->Cast< Enjon::Texture >( );
						Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* > ( tex->Class( ) ); 

						if ( ImGui::TreeNode( tex->GetName( ).c_str( ) ) )
						{
							igm->DebugDumpObject( tex ); 

							Enjon::MetaFunction* func = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "GetWidth" ) );
							if ( func )
							{
								// TODO(): THIS NEEDS TO DO TYPECHECKING
								Enjon::u32 funcRes = func->Invoke< Enjon::u32 >( tex );
								ImGui::InputInt( "Width: ", (s32*)&funcRes );
							}

							ImGui::TreePop( );
						}
					} 
				}
			}
			if ( ImGui::CollapsingHeader( "Material" ) )
			{
				auto am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
				Enjon::Material* mat = mGunMat;
				if ( mat )
				{
					igm->DebugDumpObject( mat );
				} 
			}
			if ( ImGui::CollapsingHeader( "GraphicsComponent" ) )
			{
				auto gc = mGun.Get( )->GetComponent< Enjon::GraphicsComponent >( );
				igm->DebugDumpObject( gc ); 
			} 

			// Show texture
			if ( mNewTexture )
			{
				ImGui::Image( ImTextureID( mNewTexture->GetTextureId( ) ), ImVec2( 100, 100 ) );
			}

			if ( mFont )
			{
				char buf[ 256 ];
				std::strncpy( buf, mWorldString.c_str( ), 256 );
				if ( ImGui::InputText( "World String", buf, 256 ) )
				{
					mWorldString = Enjon::String( buf );
				}
			}
		}

		ImGui::EndDock();
	};

	ImGuiManager* igm = EngineSubsystem( ImGuiManager );

	// Set up way to dock these initially at start up of all systems
	igm->RegisterWindow("Entities", showEntities);

	// Set up docking layout
	igm->RegisterDockingLayout(GUIDockingLayout("Entities", "Game View", GUIDockSlotType::Slot_Left, 0.1f));

	//------------------------------------------------------
	// Physics	
	//------------------------------------------------------

	// Set up collision configuration
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

	// Collsiion dispatcher
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	// Broad phase interface
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	// Default constraint solver
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	// Set up dynamics world
	mDynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	// Set gravity
	mDynamicsWorld->setGravity(btVector3(0, -10, 0));

	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(500.), btScalar(1.0), btScalar(500.)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -1.f, 0));

		btScalar mass(0.);

		// Rigid body is dynamic iff mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) groundShape->calculateLocalInertia(mass, localInertia);

		// Using motionstate is optional, it provides interpolation capabilities and only synches active objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRestitution(0.0);
		body->setFriction(10.0);
		body->setDamping(10.0, 10.0);

		// Add body to dynamics world
		mDynamicsWorld->addRigidBody(body); 
	}

	Enjon::Vec3 Vertices[] = {
		Enjon::Vec3( -0.5f, 0.5f, 0.5f ),
		Enjon::Vec3( 0.5f, 0.5f, 0.5f ),
		Enjon::Vec3( 0.5f, -0.5f, 0.5f ),
		Enjon::Vec3( -0.5f, -0.5f, 0.5f ),
		Enjon::Vec3( -0.5f, 0.5f, -0.5f ),
		Enjon::Vec3( 0.5f, 0.5f, -0.5f ),
		Enjon::Vec3( 0.5f, -0.5f, -0.5f ),
		Enjon::Vec3( -0.5f, -0.5f, -0.5f ) 
	};

	// Define object transform
	Enjon::Vec3 ObjectPosition = Enjon::Vec3( -25.0f, 1.0f, 5.0f );
	Enjon::Quaternion ObjectRotation;
									   
	Enjon::Vec3 ObjectScale( 3.0f, 1.0f, 1.0f ); 

	// Find View matrix
	Enjon::Vec3 CamPosition = Enjon::Vec3( -27.466789f, 2.70275f, 8.79574f );
	Enjon::Quaternion CamRotation = Enjon::Quaternion( -0.194232f, -0.243939f, -0.049514f, 0.948411f );
	Enjon::Vec3 CamForward = CamRotation * Enjon::Vec3( 0.0f, 0.0f, -1.0f );
	Enjon::Vec3 CamUp = CamRotation * Enjon::Vec3( 0.0f, 1.0f, 0.0f );
	Enjon::Mat4x4 ViewMatrix = Enjon::Mat4x4::LookAt( CamPosition, CamPosition + CamForward, CamUp );

	// Find perspective matrix
	f32 FOV = 60.0f;
	f32 Near = 0.01f;
	f32 Far = 1000.0f;
	f32 SW = 1400.0f;
	f32 SH = 900.0f;
	f32 AspectRatio = 1.837563f;
	Enjon::Mat4x4 ProjectionMatrix = Enjon::Mat4x4::Perspective( FOV, AspectRatio, Near, Far );


	// Calculate model matrix
	Enjon::Mat4x4 ModelMatrix = Enjon::Mat4x4::Identity( );
	ModelMatrix *= Enjon::Mat4x4::Translate( ObjectPosition );
	ModelMatrix *= Enjon::QuaternionToMat4x4( ObjectRotation );
	ModelMatrix *= Enjon::Mat4x4::Scale( ObjectScale );

	Enjon::String output = "";

	// Calculate biased projected position
	for ( usize i = 0; i < 8; ++i  )
	{
		Enjon::Vec4 ProjectedPositionBiased = ProjectionMatrix * ViewMatrix * ModelMatrix * Enjon::Vec4( Vertices[i], 1.0f ); 

		// Divide out the bias to get corrected projected position
		Enjon::Vec3 NDC = ( ProjectedPositionBiased / ProjectedPositionBiased.w ).XYZ( );

		Enjon::Vec2 WindowCoords = Enjon::Vec2( ( NDC.x + 1.0f ) * ( SW / 2.0f ), ( NDC.y + 1.0f ) * ( SH / 2.0f ) );

		std::stringstream ss;
		Enjon::Vec2 coords = WindowCoords / Enjon::Vec2( SW, SH );
		ss << "vert_ " << i << " = Math.Vec2(" << coords.x << ", " << 1.0 - coords.y << " )" << "\n";
		output += ss.str( ); 
	}

	std::ofstream outFile( "E:/Documents/School/Graphics/Assignment_1/Jackson_01/output.txt" );
	if ( outFile )
	{
		outFile.write( output.c_str( ), output.length( ) );
		outFile.close( );
	} 
	Enjon::Quaternion q = Enjon::Quaternion::AngleAxis( Math::ToRadians( 45.0f ), Enjon::Vec3::YAxis( ) ) *
						  Enjon::Quaternion::AngleAxis( Math::ToRadians( -36.0f ), Enjon::Vec3::ZAxis( ) );
	Enjon::Mat4x4 mat = Enjon::QuaternionToMat4x4( q );


	TestObjectSerialize( );
	
	return Enjon::Result::SUCCESS; 
} 

//-------------------------------------------------------------
Enjon::Result Game::Update(Enjon::f32 dt)
{ 
	//// Update entity manager
	//mEntities->Update(dt);

	static Enjon::f32 t = 0.0f;
	t += 0.1f * dt;

	Enjon::GraphicsComponent* gc 	= nullptr;
	Enjon::GraphicsComponent* gc2 	= nullptr;
	Enjon::GraphicsComponent* gc3 	= nullptr; 

	if ( mGun.Get( ) ) 
	{
		mGun.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( t * 5.0f, Enjon::Vec3::ZAxis( ) ) 
							* Enjon::Quaternion::AngleAxis( t * 5.0f, Enjon::Vec3::YAxis() ) );
	} 
	
	if ( mRock.Get( ) ) 
	{
		mRock.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( t * 2.0f, Enjon::Vec3::YAxis() ) );
	} 

	if ( mSerializedEntity.Get( ) )
	{
		f32 turnRate = std::sinf( t * 10.0f );
		Enjon::Vec3 pos = mSerializedEntity.Get( )->GetWorldPosition( );
		mSerializedEntity.Get( )->SetLocalPosition( Enjon::Vec3( pos.x, turnRate * 2.0f + 3.0f, pos.z ) );
		mSerializedEntity.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( turnRate * 1.5f, Enjon::Vec3::ZAxis( ) ) );

		auto children = mSerializedEntity.Get( )->GetChildren( );
		if ( children.size( ) > 0 ) 
		{
			Enjon::EntityHandle child = children.at( 0 );
			child.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( turnRate * 10.0f, Enjon::Vec3::YAxis( ) ) );

			auto cc = child.Get( )->GetChildren( );
			for ( auto& c : cc )
			{
				c.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( turnRate * 2.0f, Enjon::Vec3::XAxis( ) ) );
			}
		}
	}

	mGreen.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( t * 10.0f, Enjon::Vec3::YAxis( ) ) );
	mRed.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( t * 10.0f, Enjon::Vec3::XAxis( ) ) );
	mBlue.Get( )->SetLocalRotation( Enjon::Quaternion::AngleAxis( t * 10.0f, Enjon::Vec3::XAxis( ) ) );

	// Physics simulation
	mDynamicsWorld->stepSimulation( 1.f / 60.f, 10 );

	// Step through physics bodies and update entity position
	for ( u32 i = 0; i < ( u32 )mBodies.size( ); ++i )
	{
		btRigidBody* body = mBodies.at( i );
		Enjon::Entity* entity = mPhysicsEntities.at( i );
		btTransform trans;

		if ( body && body->getMotionState( ) )
		{
			body->getMotionState( )->getWorldTransform( trans );
			if ( entity && entity->HasComponent<Enjon::GraphicsComponent>( ) )
			{
				auto gComp = entity->GetComponent<Enjon::GraphicsComponent>( );
				Enjon::Vec3 pos = Enjon::Vec3( trans.getOrigin( ).getX( ), trans.getOrigin( ).getY( ), trans.getOrigin( ).getZ( ) );
				Enjon::Quaternion rot = Enjon::Quaternion( trans.getRotation( ).x( ), trans.getRotation( ).y( ), trans.getRotation( ).z( ), -trans.getRotation( ).w( ) );
				entity->SetLocalPosition( pos );
				entity->SetLocalRotation( rot );
			}
		}
	}

	mTextBatch->Begin( );
	{
		Enjon::Transform tform( Enjon::Vec3( 0.f, 10.f, -10.f ), Enjon::Quaternion( ), Enjon::Vec3( mFontSize ) );
		Enjon::PrintText( tform, mWorldString, mFont.Get( ), *mTextBatch, Enjon::RGBA32_White( ), 14 );
	}
	mTextBatch->End( );

	return Enjon::Result::PROCESS_RUNNING;
}

//====================================================================================================================

Enjon::Result Game::ProcessInput( f32 dt )
{
	Enjon::Camera* cam = mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >();

	if ( mInput->IsKeyPressed( Enjon::KeyCode::Escape ) )
	{
		return Enjon::Result::SUCCESS;
	}

	if ( mInput->IsKeyPressed( Enjon::KeyCode::T ) )
	{
		mMovementOn = !mMovementOn;
		Enjon::Window* window = const_cast< Enjon::Window* >( mGfx->GetWindow( ) );

		if ( !mMovementOn )
		{
			window->ShowMouseCursor( true );
		}
		else
		{
			window->ShowMouseCursor( false );
		}
	}

	if ( mInput->IsKeyPressed( Enjon::KeyCode::L ) )
	{
		mLockCamera = !mLockCamera;
	}

	if ( mRotateCamera && mLockCamera )
	{
		//Enjon::Camera* camera = mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >();
		//camera->Transform.Position += camera->Right( ).Normalize( ) * mCameraSpeed;
		//camera->LookAt( mSerializedEntity.Get()->GetWorldPosition() );
	}

	if ( mMovementOn )
	{
		Enjon::Camera* camera = mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >();
		Enjon::Vec3 velDir( 0, 0, 0 );

		if ( mInput->IsKeyDown( Enjon::KeyCode::W ) )
		{
			Enjon::Vec3 F = camera->Forward( );
			velDir += F;
		}
		if ( mInput->IsKeyDown( Enjon::KeyCode::S ) )
		{
			Enjon::Vec3 B = camera->Backward( );
			velDir += B;
		}
		if ( mInput->IsKeyDown( Enjon::KeyCode::A ) )
		{
			velDir += camera->Left( );
		}
		if ( mInput->IsKeyDown( Enjon::KeyCode::D ) )
		{
			velDir += camera->Right( );
		}
		if ( mInput->IsKeyDown( Enjon::KeyCode::Space ) )
		{
			if ( mInput->IsKeyDown( Enjon::KeyCode::LeftShift ) )
			{
				velDir -= Enjon::Vec3::YAxis( );
			} 
			else
			{
				velDir += Enjon::Vec3::YAxis( ); 
			}
		} 
		if ( mInput->IsKeyPressed( Enjon::KeyCode::K ) )
		{
			mRotateCamera = !mRotateCamera;
		}

		if ( mInput->IsKeyDown( Enjon::KeyCode::LeftMouseButton ) )
		{
			Enjon::GraphicsScene* scene = mGfx->GetGraphicsScene();
			Enjon::Vec3 pos = cam->GetPosition() + cam->Forward() * 2.0f;
			Enjon::Vec3 vel = cam->Forward() * ballSpeed;

			f32 scalar = 0.2f; 

			btCollisionShape* colShape = new btSphereShape(btScalar(scalar));
			collisionShapes.push_back(colShape);

			// Create dynamic objects
			btTransform startTransform;
			startTransform.setIdentity();

			btScalar mass(1.);

			// Rigid body is dynamic iff mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			btVector3 localInertia(0, 0, 0);
			if (isDynamic) colShape->calculateLocalInertia(mass, localInertia);

			startTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));

			// Using motionstate is recommended
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setRestitution(0.9f);
			body->setFriction(0.8f);
			body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
			body->setDamping(0.7f, 0.7f);

			Enjon::EntityHandle handle = mEntities->Allocate( );
			Enjon::Entity* ent = handle.Get( );
			Enjon::GraphicsComponent* gc = ent->AddComponent<Enjon::GraphicsComponent>();
			ent->SetLocalScale(v3(scalar));
			gc->SetMesh(mSphereMesh);

			u32 roll = Enjon::Random::Roll( 0, 6 );
			switch ( roll )
			{
				case 0: gc->SetMaterial( mFloorMat ); break;
				case 1: gc->SetMaterial( mGunMat ); break;
				case 2: gc->SetMaterial( mRockMat ); break;
				case 3: gc->SetMaterial( mRedMat ); break;
				case 4: gc->SetMaterial( mPlasticMat ); break;
				case 5: gc->SetMaterial( mGreenMat ); break;
				case 6: gc->SetMaterial( mBlueMat ); break;
				default: gc->SetMaterial( mGunMat ); break;
			}

			scene->AddStaticMeshRenderable(gc->GetRenderable());

			mBodies.push_back(body);
			mPhysicsEntities.push_back(ent);

			mDynamicsWorld->addRigidBody(body);
		}

		if (velDir.Length()) velDir = Enjon::Vec3::Normalize(velDir);

		//camera->Transform.Position += mCameraSpeed * dt * velDir; 

		// Set mouse sensitivity
		f32 MouseSensitivity = 10.0f;

		// Get mouse input and change orientation of camera
		Enjon::Vec2 MouseCoords = mInput->GetMouseCoords();

		Enjon::iVec2 viewPort = mGfx->GetViewport();

		// Grab window from graphics subsystem
		Enjon::Window* window = const_cast< Enjon::Window* >( mGfx->GetWindow() ); 

		// Set cursor to not visible
		window->ShowMouseCursor(false);

		// Reset the mouse coords after having gotten the mouse coordinates
		SDL_WarpMouseInWindow(window->GetWindowContext(), (float)viewPort.x / 2.0f, (float)viewPort.y / 2.0f); 

		if ( mLockCamera )
		{ 
			camera->LookAt( mSerializedEntity.Get()->GetWorldPosition() );
		}
		else
		{
			// Offset camera orientation
			f32 xOffset = Math::ToRadians((f32)viewPort.x / 2.0f - MouseCoords.x) * dt * MouseSensitivity;
			f32 yOffset = Math::ToRadians((f32)viewPort.y / 2.0f - MouseCoords.y) * dt * MouseSensitivity;
			camera->OffsetOrientation(xOffset, yOffset); 
		}

	}

	return Enjon::Result::PROCESS_RUNNING;
} 

//==================================================================================================================

Enjon::Result Game::Shutdown()
{
	printf("%d\n", sizeof(Enjon::Entity));
	printf("%d\n", sizeof(Enjon::Transform));
	fmt::print("{}", sizeof(Enjon::Entity));
	printf("Shutting down game...\n");

	return Enjon::Result::SUCCESS;
};

