#include "Game.h"

#include <Entity/EntityManager.h>
#include <Entity/Components/GraphicsComponent.h>
#include <Entity/Components/PointLightComponent.h>
#include <IO/ResourceManager.h>
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

#define ADD_ASSET_TO_PROJECT 1

Enjon::Texture* mNewTexture = nullptr;
Enjon::Mesh* mMesh = nullptr;
Enjon::PointLight* mPointLight = nullptr;

void Game::TestObjectSerialize( )
{
	using namespace Enjon;

	AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
	TestNamespace::PointLight writeTestObject; 
	ObjectArchiver archiver; 
	FontAssetLoader assetLoader;

	// Set value and texture
	writeTestObject.mFloatValue = 1.0f;
	writeTestObject.mUintValue = 3;
	writeTestObject.mTexture = am->GetAsset< Texture >( "isoarpg.materials.scuffedplastic.albedo" );
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

	String outputPath = am->GetAssetsPath( ) + "/Cache/testObject.easset";

	// Write to file
	archiver.WriteToFile( outputPath ); 
 
	// Cool, now de-serialize all objects in file
	Vector<Object*> objects;
	archiver.Deserialize( outputPath, objects ); 

	// Test serializing/deserializing directional lights in a scene
	Enjon::u32 i = 0;
	for ( auto& dl : mGfx->GetScene()->GetDirectionalLights() )
	{ 
		String outputAssetPath = am->GetAssetsPath( ) + "/Cache/directionalLight" + std::to_string(i) +".easset";
		ObjectArchiver oa;
		oa.Serialize( dl );
		oa.WriteToFile( outputAssetPath ); 
		oa.Deserialize( outputAssetPath, objects ); 
		i++;
	} 

	// Try deserializing one of the textures that have been cached
	{
		ObjectArchiver textureArchiver; 
		Enjon::String texPath = am->GetCachedAssetsPath( ) + "isoarpg.materials.woodframe.albedo.easset";
		mNewTexture = static_cast< Enjon::Texture* > ( textureArchiver.Deserialize( texPath ) ); 
		mGunMat->SetTexture( Enjon::TextureSlotType::Albedo, Enjon::AssetHandle< Enjon::Texture >( mNewTexture ) );
	} 

	// Try deserializing one of the textures that have been cached
	{
		ObjectArchiver meshArchiver; 
		Enjon::String meshPath = am->GetCachedAssetsPath( ) + "isoarpg.models.buddha.easset";
		mMesh = static_cast< Enjon::Mesh* > ( meshArchiver.Deserialize( meshPath ) ); 
		mGun.Get( )->GetComponent< GraphicsComponent >( )->SetMesh( AssetHandle< Mesh >( mMesh ) );
	} 

	{ 
		// Will not serialize if not needed!
		Enjon::String assetPath( "/Textures/beast.png" );
		am->AddToDatabase( assetPath );

		// Will not serialize if not needed!
		assetPath = Enjon::String( "/Models/bb8.obj" );
		am->AddToDatabase( assetPath );

		Enjon::AssetHandle< Enjon::Texture > cachedTexture = am->GetAsset< Enjon::Texture >( "isoarpg.textures.beast" );
		mGun.Get( )->GetComponent< GraphicsComponent >( )->GetMaterial( )->SetTexture( Enjon::TextureSlotType::Albedo, cachedTexture );

		Enjon::AssetHandle< Enjon::Mesh > cachedMesh = am->GetAsset< Enjon::Mesh >( "isoarpg.models.unit_cube" );
		mGun.Get( )->GetComponent< GraphicsComponent >( )->SetMesh( cachedMesh );
	} 
}

std::vector<btRigidBody*> mBodies;
btDiscreteDynamicsWorld* mDynamicsWorld;

// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
btAlignedObjectArray<btCollisionShape*> collisionShapes;

// Physics entities to align with rigid bodies
std::vector<Enjon::Entity*> mPhysicsEntities;

Enjon::String mAssetsPath; 
Enjon::AssetManager* mAssetManager;

f32 ballSpeed = 10.0f;

Enjon::Signal<f32> testSignal;
Enjon::Property<f32> testProperty;

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

Enjon::Result Game::Initialize()
{ 
	// Set up assets path
	// This needs to be done in a project settings config file or in the cmake, not in source 
	mAssetsPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("/IsoARPG/Assets"); 
	Enjon::String cachePath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("/IsoARPG/Assets/Cache/"); 
	Enjon::String projectDirectory = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + "/IsoARPG/";
	
	// Get asset manager and set its properties ( I don't like this )
	mAssetManager = Enjon::Engine::GetInstance()->GetSubsystemCatalog()->Get<Enjon::AssetManager>(); 
	// This also needs to be done through a config file or cmake
	mAssetManager->SetAssetsPath( mAssetsPath );
	mAssetManager->SetCachedAssetsPath( cachePath );
	mAssetManager->SetDatabaseName( GetApplicationName( ) ); 
	mAssetManager->Initialize( );

	for ( auto& p : std::experimental::filesystem::recursive_directory_iterator( projectDirectory ) )
	{
		if ( Enjon::AssetManager::HasFileExtension( p.path().string(), "easset" ) )
		{
			std::cout << "Asset: " << p << "\n";
		} 
	}

	{
		// This works for flat arrays
		Enjon::AssetHandle< Enjon::Texture > textures[ (u32)Enjon::TextureSlotType::Count ];
		u32 size = *( &textures + 1 ) - textures;

		std::array< Enjon::AssetHandle< Enjon::Texture >, ( u32 )Enjon::TextureSlotType::Count > texArray;
		size = sizeof( texArray ) / sizeof( texArray[ 0 ] );

		// What about for vectors( not so much )
		std::vector< Enjon::AssetHandle< Enjon::Texture > > vecTextures;
		vecTextures.push_back( Enjon::AssetHandle< Enjon::Texture >( ) );
		vecTextures.push_back( Enjon::AssetHandle< Enjon::Texture >( ) );
		vecTextures.push_back( Enjon::AssetHandle< Enjon::Texture >( ) );

		size = sizeof( vecTextures ) / sizeof(vecTextures[ 0 ]);
	}

	// Get Subsystems from engine
	Enjon::Engine* engine = Enjon::Engine::GetInstance();
	Enjon::SubsystemCatalog* subSysCatalog = engine->GetSubsystemCatalog();
	mGfx = subSysCatalog->Get<Enjon::GraphicsSubsystem>();
	mInput = subSysCatalog->Get<Enjon::Input>();

	// Paths to resources
	Enjon::String toyBoxDispPath		= Enjon::String("/Textures/toy_box_disp.png");
	Enjon::String toyBoxNormalPath		= Enjon::String("/Textures/toy_box_normal.png");
	Enjon::String eyePath				= Enjon::String("/Models/eye.obj");
	Enjon::String noisePath				= Enjon::String("/Textures/worleyNoise.png");
	Enjon::String grassTexturePath		= Enjon::String("/Textures/grass.png");
	Enjon::String quadPath				= Enjon::String("/Models/quad.obj");
	Enjon::String rockPath				= Enjon::String("/Models/rock.obj");
	Enjon::String cerebusMeshPath		= Enjon::String("/Models/cerebus.obj");
	Enjon::String buddhaMeshPath		= Enjon::String("/Models/buddha.obj");
	Enjon::String bunnyMeshPath			= Enjon::String("/Models/bunny.obj");
	Enjon::String dragonMeshPath		= Enjon::String("/Models/dragon.obj");
	Enjon::String monkeyMeshPath		= Enjon::String("/Models/monkey.obj");
	Enjon::String sphereMeshPath		= Enjon::String("/Models/unit_sphere.obj");
	Enjon::String cubeMeshPath			= Enjon::String("/Models/unit_cube.obj");
	Enjon::String shaderballPath		= Enjon::String("/Models/shaderball.obj");
	Enjon::String unitSpherePath		= Enjon::String("/Models/unit_sphere.obj");
	Enjon::String unrealShaderBallPath	= Enjon::String("/Models/unreal_shaderball.obj");
	Enjon::String unitShaderBallPath	= Enjon::String("/Models/unit_shaderball.obj");
	Enjon::String catMeshPath			= Enjon::String("/Models/cat.obj");
	Enjon::String dudeMeshPath			= Enjon::String("/Models/dude.obj");
	Enjon::String shaderBallMeshPath	= Enjon::String("/Models/shaderball.obj");
	Enjon::String cerebusAlbedoPath		= Enjon::String("/Materials/Cerebus/Albedo.png"); 
	Enjon::String cerebusNormalPath		= Enjon::String("/Materials/Cerebus/Normal.png"); 
	Enjon::String cerebusMetallicPath	= Enjon::String("/Materials/Cerebus/Metallic.png"); 
	Enjon::String cerebusRoughnessPath	= Enjon::String("/Materials/Cerebus/Roughness.png"); 
	Enjon::String cerebusEmissivePath	= Enjon::String("/Materials/Cerebus/Emissive.png"); 
	Enjon::String mahogAlbedoPath		= Enjon::String("/Materials/MahogFloor/Albedo.png"); 
	Enjon::String mahogNormalPath		= Enjon::String("/Materials/MahogFloor/Normal.png"); 
	Enjon::String mahogMetallicPath		= Enjon::String("/Materials/MahogFloor/Roughness.png"); 
	Enjon::String mahogRoughnessPath	= Enjon::String("/Materials/MahogFloor/Roughness.png"); 
	Enjon::String mahogEmissivePath		= Enjon::String("/Materials/MahogFloor/Emissive.png"); 
	Enjon::String mahogAOPath			= Enjon::String("/Materials/MahogFloor/AO.png"); 
	Enjon::String woodAlbedoPath		= Enjon::String("/Materials/WoodFrame/Albedo.png"); 
	Enjon::String woodNormalPath		= Enjon::String("/Materials/WoodFrame/Normal.png"); 
	Enjon::String woodRoughnessPath		= Enjon::String("/Materials/WoodFrame/Roughness.png"); 
	Enjon::String woodMetallicPath		= Enjon::String("/Materials/WoodFrame/Metallic.png"); 
	Enjon::String plasticAlbedoPath		= Enjon::String("/Materials/ScuffedPlastic/Albedo.png"); 
	Enjon::String plasticNormalPath		= Enjon::String("/Materials/ScuffedPlastic/Normal.png"); 
	Enjon::String plasticRoughnessPath	= Enjon::String("/Materials/ScuffedPlastic/Roughness.png"); 
	Enjon::String plasticMetallicPath	= Enjon::String("/Materials/ScuffedPlastic/Metallic.png"); 
	Enjon::String plasticAOPath			= Enjon::String("/Materials/ScuffedPlastic/AO.png"); 
	Enjon::String wornRedAlbedoPath		= Enjon::String("/Materials/RustedIron/Albedo.png"); 
	Enjon::String wornRedNormalPath		= Enjon::String("/Materials/RustedIron/Normal.png"); 
	Enjon::String wornRedRoughnessPath	= Enjon::String("/Materials/RustedIron/Roughness.png"); 
	Enjon::String wornRedMetallicPath	= Enjon::String("/Materials/RustedIron/Metallic.png"); 
	Enjon::String scuffedGoldAlbedoPath		= Enjon::String("/Materials/ScuffedGold/Albedo.png"); 
	Enjon::String scuffedGoldNormalPath		= Enjon::String("/Materials/ScuffedGold/Normal.png"); 
	Enjon::String scuffedGoldMetallicPath	= Enjon::String("/Materials/ScuffedGold/Metallic.png"); 
	Enjon::String scuffedGoldRoughnessPath	= Enjon::String("/Materials/ScuffedGold/Roughness.png"); 
	Enjon::String rockAlbedoPath		= Enjon::String("/Materials/CopperRock/Albedo.png"); 
	Enjon::String rockNormalPath		= Enjon::String("/Materials/CopperRock/Normal.png"); 
	Enjon::String rockRoughnessPath		= Enjon::String("/Materials/CopperRock/Roughness.png"); 
	Enjon::String rockMetallicPath		= Enjon::String("/Materials/CopperRock/Metallic.png"); 
	Enjon::String rockEmissivePath		= Enjon::String("/Materials/CopperRock/Emissive.png"); 
	Enjon::String rockAOPath			= Enjon::String("/Materials/CopperRock/AO.png"); 
	Enjon::String frontNormalPath		= Enjon::String("/Textures/front_normal.png"); 
	Enjon::String brdfPath				= Enjon::String("/Textures/brdf.png"); 
	Enjon::String waterPath				= Enjon::String("/Textures/water.png"); 
	Enjon::String greenPath				= Enjon::String("/Textures/green.png"); 
	Enjon::String redPath				= Enjon::String("/Textures/red.png"); 
	Enjon::String bluePath				= Enjon::String("/Textures/blue.png"); 
	Enjon::String blackPath				= Enjon::String("/Textures/black.png"); 
	Enjon::String midGreyPath			= Enjon::String("/Textures/grey.png"); 
	Enjon::String lightGreyPath			= Enjon::String("/Textures/light_grey.png"); 
	Enjon::String whitePath				= Enjon::String("/Textures/white.png"); 
	Enjon::String teapotPath			= Enjon::String( "/Models/teapot.obj" );
	Enjon::String swordPath				= Enjon::String( "/Models/sword.obj" );
	Enjon::String eyeNormal				= Enjon::String( "/Textures/eye_NORMAL.png" );
	Enjon::String eyeAlbedo				= Enjon::String( "/Textures/eyeball_COLOR1.png" );
	Enjon::String shaderGraphPath		= Enjon::String( "/Shaders/ShaderGraphs/testGraph.sg" );

	// Try loading font
	Enjon::String rootPath = engine->GetConfig( ).GetRoot( );
	Enjon::String fontPath = rootPath + "/Assets/Fonts/WeblySleek/weblysleekuisb.ttf"; 

	// Add to asset database
	mAssetManager->AddToDatabase( toyBoxDispPath );
	mAssetManager->AddToDatabase( toyBoxNormalPath );
	mAssetManager->AddToDatabase( unitSpherePath );
	mAssetManager->AddToDatabase( shaderGraphPath, false );
	mAssetManager->AddToDatabase( eyePath );
	mAssetManager->AddToDatabase( eyeAlbedo );
	mAssetManager->AddToDatabase( eyeNormal );
	mAssetManager->AddToDatabase( dragonMeshPath );
	mAssetManager->AddToDatabase( swordPath );
	mAssetManager->AddToDatabase( noisePath );
	mAssetManager->AddToDatabase( grassTexturePath );
	mAssetManager->AddToDatabase( quadPath );
	mAssetManager->AddToDatabase( rockPath );
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
	mSphereMesh = mAssetManager->GetAsset<Enjon::Mesh>( "isoarpg.models.unit_sphere" );

	// Create entity manager
	mEntities = new Enjon::EntityManager();

	mEntities->RegisterComponent<Enjon::GraphicsComponent>();
	mEntities->RegisterComponent<Enjon::PointLightComponent>();

	// Allocate handle
	mGreen = mEntities->Allocate( );
	mRed = mEntities->Allocate( );
	mBlue = mEntities->Allocate( );
	mGun = mEntities->Allocate( );
	mRock = mEntities->Allocate( );
	mRock2 = mEntities->Allocate( );
	auto rgc = mRock.Get( )->Attach< Enjon::GraphicsComponent >( );
	auto rgc2 = mRock2.Get( )->Attach< Enjon::GraphicsComponent >( );
	auto gc = mGun.Get()->Attach<Enjon::GraphicsComponent>(); 
	auto pc = mGun.Get()->Attach<Enjon::PointLightComponent>(); 

	mRed.Get()->Attach< Enjon::GraphicsComponent >( );
	mGreen.Get()->Attach< Enjon::GraphicsComponent >( );
	mBlue.Get()->Attach< Enjon::GraphicsComponent >( );

	pc->GetLight( )->SetPosition( Enjon::Vec3( 10.0f, 2.0f, 4.0f ) );
	pc->GetLight( )->SetIntensity( 20.0f );
	pc->GetLight( )->SetAttenuationRate( 0.2f );
	pc->GetLight( )->SetRadius( 100.0f );
	pc->GetLight( )->SetColor( Enjon::RGBA32_Orange( ) );

	mPlasticMat = new Enjon::Material;
	mPlasticMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedplastic.albedo"));
	mPlasticMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedplastic.normal"));
	mPlasticMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedplastic.metallic"));
	mPlasticMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedplastic.roughness"));
	mPlasticMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.black"));
	mPlasticMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedplastic.ao"));
	
	mGoldMat = new Enjon::Material;
	mGoldMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedgold.albedo"));
	mGoldMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedgold.normal"));
	mGoldMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedgold.metallic"));
	mGoldMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.scuffedgold.roughness"));
	mGoldMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.black"));
	mGoldMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.white")); 
	
	mRockMat 	= new Enjon::Material; 
	mRockMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.rustediron.albedo"));
	mRockMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.rustediron.normal"));
	mRockMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.rustediron.metallic"));
	mRockMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.rustediron.roughness"));
	mRockMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.black"));
	mRockMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.white"));

	mGunMat 	= new Enjon::Material; 
	mGunMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.cerebus.albedo"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.cerebus.normal"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.cerebus.metallic"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.cerebus.roughness"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.cerebus.emissive"));
	mGunMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.white"));

	mGun.Get()->SetPosition(Enjon::Vec3(0.0f, 0.0f, 0.0f));
	mGun.Get()->SetRotation( Enjon::Quaternion::AngleAxis( 45.0f, Enjon::Vec3::ZAxis() ) );
	gc->SetMesh(mAssetManager->GetAsset<Enjon::Mesh>("isoarpg.models.cat"));
	gc->SetMaterial(mGoldMat);

	mSun = new Enjon::DirectionalLight();
	mSun->SetIntensity(1.5f);
	mSun->SetColor(Enjon::RGBA32_White());

	auto mSun2 = new Enjon::DirectionalLight(Enjon::Vec3(0.5f, 0.5f, -0.75f), Enjon::RGBA32_SkyBlue(), 10.0f); 

	mFloorMat = new Enjon::Material();
	mFloorMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.white"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.mahogfloor.normal"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.textures.black"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.mahogfloor.roughness"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.mahogfloor.emissive"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("isoarpg.materials.mahogfloor.ao"));
	mFloorMat->TwoSided( true );

	mBlueMat = new Enjon::Material( );
	mBlueMat->SetTexture( Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.blue") );
	mBlueMat->SetTexture( Enjon::TextureSlotType::Normal, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.normal") );
	mBlueMat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.metallic") );
	mBlueMat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.roughness") );
	mBlueMat->SetTexture( Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.blue") );
	mBlueMat->SetTexture( Enjon::TextureSlotType::AO, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.white") );
	
	mRedMat = new Enjon::Material( );
	mRedMat->SetTexture( Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.red") );
	mRedMat->SetTexture( Enjon::TextureSlotType::Normal, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.normal") );
	mRedMat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.metallic") );
	mRedMat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.roughness") );
	mRedMat->SetTexture( Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.red") );
	mRedMat->SetTexture( Enjon::TextureSlotType::AO, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.white") );
	
	mGreenMat = new Enjon::Material( );
	mGreenMat->SetTexture( Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.green") );
	mGreenMat->SetTexture( Enjon::TextureSlotType::Normal, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.normal") );
	mGreenMat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.metallic") );
	mGreenMat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.roughness") );
	mGreenMat->SetTexture( Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.green") );
	mGreenMat->SetTexture( Enjon::TextureSlotType::AO, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.white") );

	mFontMat = new Enjon::Material( );
	auto f = mFont.Get( );
	f->GetAtlas( 14 );
	mFontMat->SetTexture( Enjon::TextureSlotType::Albedo, mFont.Get( )->GetAtlas( 14 )->GetAtlasTexture( ) );
	mFontMat->SetTexture( Enjon::TextureSlotType::Normal, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.normal") );
	mFontMat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.metallic") );
	mFontMat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.materials.cerebus.roughness") );
	mFontMat->SetTexture( Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.green") );
	mFontMat->SetTexture( Enjon::TextureSlotType::AO, mAssetManager->GetAsset< Enjon::Texture >("isoarpg.textures.white") );
	mFontMat->TwoSided( true );

	mGreen.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMaterial( mGreenMat );
	mGreen.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >("isoarpg.models.unit_cube" ) );
	mRed.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMaterial( mRedMat ); 
	mRed.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >("isoarpg.models.unit_cube" ) );
	mBlue.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMaterial( mBlueMat );
	mBlue.Get()->GetComponent< Enjon::GraphicsComponent >( )->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >("isoarpg.models.unit_cube" ) );

	mGun.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 0, 5, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 1.5f ) ) );
	mGreen.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 0, 7, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.5f ) ) );
	mRed.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( 5, 7, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.25f ) ) );
	mBlue.Get()->SetLocalTransform( Enjon::Transform( Enjon::Vec3( -5, 7, 0 ), Enjon::Quaternion( 0, 0, 0, 1 ), Enjon::Vec3( 0.25f ) ) );

	mGun.Get()->AddChild( mGreen ); 
	mGreen.Get()->AddChild( mRed );
	mGreen.Get()->AddChild( mBlue ); 

	rgc2->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >( "isoarpg.models.buddha" ) );
	rgc->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >( "isoarpg.models.cerebus" ) );
	rgc->SetMaterial( mGunMat );
	rgc2->SetMaterial( mPlasticMat );
	mRock.Get( )->SetPosition( Enjon::Vec3( 5.0f, 5.0f, 20.0f ) );
	mRock.Get( )->SetScale( Enjon::Vec3( 5.0f ) );
	
	mRock2.Get( )->SetPosition( Enjon::Vec3( 10.0f, 0.0f, 20.0f ) );
	//mRock2.Get( )->SetScale( Enjon::Vec3( 0.01f ) );

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
				Enjon::Quaternion rot = Enjon::Quaternion::AngleAxis(Enjon::ToRadians(90.0f), Enjon::Vec3::XAxis());
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

	// Set up shader ball scene
	for ( u32 i = 0; i < (u32)GreyScale::Count; ++i )  // Metallic
	{
		for ( u32 j = 0; j < ( u32 )GreyScale::Count; ++j )  // Roughnes
		{ 
			// Make new entity
			Enjon::EntityHandle eh = mEntities->Allocate( );
			auto gfxcmp = eh.Get( )->Attach< Enjon::GraphicsComponent >( ); 
			Enjon::Material* mat = new Enjon::Material( );
			mat->SetTexture( Enjon::TextureSlotType::Albedo, mAssetManager->GetDefaultAsset< Enjon::Texture >( ) );
			mat->SetTexture( Enjon::TextureSlotType::Normal, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.materials.scuffedgold.normal" ) );
			mat->SetTexture( Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.black" ) );
			mat->SetTexture( Enjon::TextureSlotType::AO, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.materials.mahogfloor.ao" ) );
			gfxcmp->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >( "isoarpg.models.shaderball" ) ); 
			gfxcmp->SetMaterial( mat );

			eh.Get( )->SetScale( Enjon::Vec3( 0.009f ) );
			eh.Get( )->SetPosition( Enjon::Vec3( j * 3.0f, 0.0f, i * 3.0f ) + Enjon::Vec3( 15, 0, 15 ) );

			switch ( GreyScale( i ) )
			{
				case GreyScale::Black:
				{
					mat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.black" ) );

					switch ( GreyScale( j ) )
					{
						case GreyScale::Black:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.black" ) );
						} break;
						case GreyScale::Grey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.grey" ) );
						} break;
						case GreyScale::LightGrey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.light_grey" ) );
						} break;
						case GreyScale::White:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.white" ) );
						} break;
					} 
				} break;
				case GreyScale::Grey:
				{
					mat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.grey" ) );

					switch ( GreyScale( j ) )
					{
						case GreyScale::Black:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.black" ) );
						} break;
						case GreyScale::Grey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.grey" ) );
						} break;
						case GreyScale::LightGrey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.light_grey" ) );
						} break;
						case GreyScale::White:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.white" ) );
						} break;
					} 
				} break;
				case GreyScale::LightGrey:
				{
					mat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.light_grey" ) );

					switch ( GreyScale( j ) )
					{
						case GreyScale::Black:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.black" ) );
						} break;
						case GreyScale::Grey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.grey" ) );
						} break;
						case GreyScale::LightGrey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.light_grey" ) );
						} break;
						case GreyScale::White:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.white" ) );
						} break;
					} 
				} break;
				case GreyScale::White:
				{
					mat->SetTexture( Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.white" ) );

					switch ( GreyScale( j ) )
					{
						case GreyScale::Black:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.black" ) );
						} break;
						case GreyScale::Grey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.grey" ) );
						} break;
						case GreyScale::LightGrey:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.light_grey" ) );
						} break;
						case GreyScale::White:
						{
							mat->SetTexture( Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset< Enjon::Texture >( "isoarpg.textures.white" ) );
						} break;
					} 
				} break;
			}
			// Add to scene
			auto scene = mGfx->GetScene();
			scene->AddRenderable( eh.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );
		} 
	}

	if (mGfx)
	{
		auto scene = mGfx->GetScene();
		scene->AddDirectionalLight( mSun );
		scene->AddDirectionalLight( mSun2 );
		scene->AddRenderable(gc->GetRenderable());
		scene->AddPointLight( pc->GetLight( ) );
		scene->AddRenderable( mGreen.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );
		scene->AddRenderable( mRed.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) );
		scene->AddRenderable( mBlue.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) ); 
		scene->AddRenderable( mRock.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) ); 
		scene->AddRenderable( mRock2.Get( )->GetComponent< Enjon::GraphicsComponent >( )->GetRenderable( ) ); 
		scene->AddQuadBatch(mBatch);
		scene->AddQuadBatch(mTextBatch);
		scene->SetSun(mSun);
		scene->SetAmbientColor(Enjon::SetOpacity(Enjon::RGBA32_White(), 0.1f));

		// Set graphics camera position
		auto cam = mGfx->GetSceneCamera();
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

			mGun.Get( )->SetPosition(v3(pos[0], pos[1], pos[2]));
			mGun.Get( )->SetScale(v3(scl[0], scl[1], scl[2]));
			mGun.Get( )->SetRotation(quat(rot[0], rot[1], rot[2], rotation.w));

			for ( auto c : mGun.Get( )->GetChildren( ) )
			{
				ImGui::Text( "Child Local:" );
				Enjon::Transform trans = c.Get( )->GetLocalTransform( );
				auto position 	= trans.Position;
				auto scale 		= trans.Scale;
				auto rotation 	= trans.Rotation;

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
				auto position 	= trans.Position;
				auto scale 		= trans.Scale;
				auto rotation 	= trans.Rotation;

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

			auto cam = mGfx->GetSceneCamera( );
			ImGui::InputFloat3( "Cam Position", ( float* )&cam->GetPosition( ) );
			ImGui::InputFloat4( "Cam Rotation", ( float* )&cam->GetRotation( ) );

			f32 ar = cam->GetAspectRatio( );
			ImGui::InputFloat( "AspectRatio", &ar );

			auto vp = mGfx->GetViewport( );
			ImGui::InputInt2( "View Dimensions", ( s32* )&vp );
			
			ImGui::Text( "32 bit prop size: %d", sizeof( Enjon::Property<f32> ) );
			ImGui::Text( "32 bit signal size: %d", sizeof( Enjon::Signal<f32> ) ); 

			if ( ImGui::CollapsingHeader( "Test Object" ) )
			{
				Enjon::ImGuiManager::DebugDumpObject( &mTestObject );
			}

			if ( ImGui::CollapsingHeader( "Scene" ) )
			{
				Enjon::ImGuiManager::DebugDumpObject( mGfx->GetScene( ) );
			}

			if ( ImGui::CollapsingHeader( "Sun" ) )
			{
				Enjon::ImGuiManager::DebugDumpObject( mSun ); 
			}

			// Testing meta functions
			if ( ImGui::CollapsingHeader( "Entity" ) )
			{ 
				Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* >( mRed.Get( )->Class( ) );

				// Debug dump object
				Enjon::ImGuiManager::DebugDumpObject( mRed.Get( ) );
				Enjon::MetaFunction* getWPFunc = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "GetWorldPosition" ) );
				Enjon::MetaFunction* setWPFunc = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "SetPosition" ) );
				if ( getWPFunc )
				{
					// TODO(): Need to make this type safe - detect that this return type does not match the function signture
					auto wp = getWPFunc->Invoke< Enjon::Vec3 >( mRed.Get( ) );
					if ( ImGui::SliderFloat3( "Entity WP", ( float* )&wp, 0.0f, 1.0f ) )
					{
						if ( setWPFunc )
						{
							setWPFunc->Invoke< void >( mRed.Get( ), wp );
						}
					}
				}
			} 

			if ( ImGui::CollapsingHeader( "PointLight" ) )
			{
				auto plc = mGun.Get( )->GetComponent< Enjon::PointLightComponent >( );
				auto light = plc->GetLight( );
				Enjon::ImGuiManager::DebugDumpObject( light );

				Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* > ( light->Class( ) ); 
				Enjon::MetaFunction* getWPFunc = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "GetPosition" ) );
				Enjon::MetaFunction* setWPFunc = const_cast< Enjon::MetaFunction* > ( cls->GetFunction( "SetPosition" ) );
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
				auto am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
				auto textures = am->GetAssets< Enjon::Texture >( );
				for ( auto& t : *textures ) 
				{
					if ( t.second.GetAsset() )
					{
						const Enjon::Texture* tex = t.second.GetAsset()->Cast< Enjon::Texture >( );
						Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* > ( tex->Class( ) ); 

						if ( ImGui::TreeNode( tex->GetName( ).c_str( ) ) )
						{
							Enjon::ImGuiManager::DebugDumpObject( tex ); 

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
					Enjon::ImGuiManager::DebugDumpObject( mat );
				} 
			}
			if ( ImGui::CollapsingHeader( "GraphicsComponent" ) )
			{
				auto gc = mGun.Get( )->GetComponent< Enjon::GraphicsComponent >( );
				Enjon::ImGuiManager::DebugDumpObject( gc ); 
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

	// Set up way to dock these initially at start up of all systems
	Enjon::ImGuiManager::RegisterWindow(showEntities);

	// Set up docking layout
	Enjon::ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout("Entities", "Game View", ImGui::DockSlotType::Slot_Left, 0.1f));

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

	fmt::print( "Entity Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::Entity >() ); 
	fmt::print( "Instance of Entity ID: {}\n", mGun.Get( )->GetTypeId() ); 

	fmt::print( "GraphicsComponent Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::GraphicsComponent >() ); 
	fmt::print( "Instance of GraphicsComponent ID: {}\n", mGun.Get( )->GetComponent<Enjon::GraphicsComponent>()->GetTypeId() ); 
	
	fmt::print( "PointlightComponent Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::PointLightComponent >() ); 
	fmt::print( "Instance of PointlightComponent ID: {}\n", mGun.Get( )->GetComponent<Enjon::PointLightComponent>()->GetTypeId() ); 
	
	fmt::print( "Material Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::Material >() ); 
	fmt::print( "Instance of Material ID: {}\n", mGun.Get( )->GetComponent<Enjon::GraphicsComponent>()->GetMaterial()->GetTypeId() ); 
	
	fmt::print( "Mesh Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::Mesh >() ); 
	fmt::print( "Instance of Mesh ID: {}\n", mGun.Get( )->GetComponent<Enjon::GraphicsComponent>()->GetMesh().Get()->GetTypeId() ); 
	
	fmt::print( "Texture Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::Texture >() ); 
	fmt::print( "Instance of Texture ID: {}\n", mGun.Get( )->GetComponent<Enjon::GraphicsComponent>()->GetMaterial()->GetTexture( Enjon::TextureSlotType::Albedo ).Get()->GetTypeId() ); 
	
	fmt::print( "Texture Class Type ID: {}\n", Enjon::Object::GetTypeId< Enjon::Texture >() ); 
	fmt::print( "Instance of Texture ID: {}\n", mGun.Get( )->GetComponent<Enjon::GraphicsComponent>()->GetMaterial()->GetTexture( Enjon::TextureSlotType::Normal ).Get()->GetTypeId() ); 

	fmt::print( "Same: {}\n", Enjon::Object::GetTypeId< Enjon::Texture >( ) == mGun.Get( )->GetComponent<Enjon::GraphicsComponent>( )->GetMaterial( )->GetTexture( Enjon::TextureSlotType::Albedo ).Get( )->GetTypeId( ) ); 

	/*
	Enjon::ByteBuffer writeBuffer;
	u32 texID;
	s32 width, height, nComps;
	s32 widthtga, heighttga, nCompstga;
	s32 len;
	stbi_set_flip_vertically_on_load( false );
	u8* data = stbi_load( ( rootPath + "/IsoARPG/Assets/" + "Materials/CopperRock/Albedo.png" ).c_str( ), &width, &height, &nComps, STBI_rgb_alpha );
	u8* tgaData = stbi_load( ( rootPath + "/IsoARPG/Assets/Textures/cerebusAlbedo.tga" ).c_str( ), &widthtga, &heighttga, &nCompstga, STBI_rgb_alpha );

	auto saveData = stbi_write_png_to_mem( data, 0, width, height, 4, &len );
	stbi_write_tga( ( rootPath + "/testTGA" ).c_str( ), widthtga, heighttga, 4, tgaData );
	Enjon::ByteBuffer readBuffer;
	readBuffer.ReadFromFile( rootPath + "/testTGA" );
	u32 size = readBuffer.GetSize( );
	u8* loadBufferData = ( u8* )malloc( size );
	s32 loadWidth, loadHeight, loadComps;
	for ( usize i = 0; i < size; ++i )
	{
		loadBufferData[ i ] = readBuffer.Read< char >( );
	}

	u32 srcSize = size;
	u32 maxSize = LZ4_compressBound( srcSize );
	char* compressedData = (char*)malloc( maxSize );
	s32 compressed_data_size = LZ4_compress_default( (char*)loadBufferData, compressedData, srcSize, maxSize );

	// Write out
	Enjon::ByteBuffer compressBuffer;
	compressBuffer.Write( compressed_data_size );
	for ( usize i = 0; i < compressed_data_size; ++i )
	{
		compressBuffer.Write( compressedData[ i ] );
	}
	compressBuffer.WriteToFile( rootPath + "/compressedTexture" );

	writeBuffer.Write( width );
	writeBuffer.Write( height );
	writeBuffer.Write( 4 );
	writeBuffer.Write( len );
	for ( usize i = 0; i < len; ++i )
	{
		writeBuffer.Write( saveData[ i ] );
	}
	stbi_image_free( data );
	writeBuffer.WriteToFile( rootPath + "/testTexture" );

	writeBuffer.ReadFromFile( rootPath + "/testTexture" );

	width = writeBuffer.Read< s32 >( );
	height = writeBuffer.Read< s32 >( );
	nComps = writeBuffer.Read< s32 >( );
	len = writeBuffer.Read< s32 >( );
	unsigned char* loadData = ( unsigned char* )malloc( len );
	for ( usize i = 0; i < len; ++i )
	{
		loadData[ i ] = writeBuffer.Read< char >( );
	}

	s32 comps;
	unsigned char* loadedData = stbi_load_from_memory( loadData, len, &width, &height, &comps, 4 );
	u8* tgaDataLoad = stbi_load_from_memory( loadBufferData, size, &loadWidth, &loadHeight, &loadComps, 4 );

	// Generate texture
	// TODO(): Make this API generalized to work with DirectX as well as OpenGL
	glGenTextures( 1, &( texID ) );

	glBindTexture( GL_TEXTURE_2D, texID );

	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		loadWidth,
		loadHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		tgaDataLoad
	);

	// No longer need data, so free
	stbi_image_free( loadedData );

	s32 MAG_PARAM = GL_LINEAR;
	s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
	b8 genMips = true;

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

	if ( genMips )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	glBindTexture( GL_TEXTURE_2D, 0 );

	mTex = new Enjon::Texture( width, height, texID );
	*/
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
	Enjon::Mat4 ViewMatrix = Enjon::Mat4::LookAt( CamPosition, CamPosition + CamForward, CamUp ); 

	// Find perspective matrix
	f32 FOV = 60.0f;
	f32 Near = 0.01f;
	f32 Far = 1000.0f;
	f32 SW = 1400.0f;
	f32 SH = 900.0f;
	f32 AspectRatio = 1.837563f; 
	Enjon::Mat4 ProjectionMatrix = Enjon::Mat4::Perspective( FOV, AspectRatio, Near, Far );


	// Calculate model matrix
	Enjon::Mat4 ModelMatrix = Enjon::Mat4::Identity( );
	ModelMatrix *= Enjon::Mat4::Translate( ObjectPosition );
	ModelMatrix *= Enjon::QuaternionToMat4( ObjectRotation );
	ModelMatrix *= Enjon::Mat4::Scale( ObjectScale );

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
	Enjon::Quaternion q = Enjon::Quaternion::AngleAxis( Enjon::ToRadians( 45.0f ), Enjon::Vec3::YAxis( ) ) *
						  Enjon::Quaternion::AngleAxis( Enjon::ToRadians( -36.0f ), Enjon::Vec3::ZAxis( ) );
	Enjon::Mat4 mat = Enjon::QuaternionToMat4( q );
	std::cout << mat << "\n";


	{
		Enjon::Vec2 numbers[ ] = {
			Enjon::Vec2( .4016187639621184, 0.4485978731822172 ),
			Enjon::Vec2( 0.6818915804271153, 0.3622686499035348 ),
			Enjon::Vec2( 0.6683032288439311, 0.5233702444624347 ),
			Enjon::Vec2( 0.4113190311141083, 0.6554002622737832 ),
			Enjon::Vec2( 0.3677100875763688, 0.3897306382940081 ),
			Enjon::Vec2( 0.6162426043508087, 0.329379447817739 ),
			Enjon::Vec2( 0.6086891976986508, 0.4712739931592119 ),
			Enjon::Vec2( 0.3785668792045406, 0.5663520525797013 )
		};

		Enjon::Vec2 screenDims = Enjon::Vec2( 640, 480 );

		for ( usize i = 0; i < 8; ++i ) 
		{
			auto res = numbers[ i ] * screenDims;
			std::cout << res << "\n";
		}
	} 

	{
		auto q = Enjon::QuaternionToMat4( Enjon::Quaternion::AngleAxis( Enjon::ToRadians( 90.0f ), Enjon::Vec3::ZAxis( ) ) );
		std::cout << q << '\n';
		//Enjon::Mat4 model = Enjon::Mat4::Identity( );
		//model *= Enjon::Mat4::Translate( Enjon::Vec3( 0.0f ) );
		//model *= Enjon::QuaternionToMat4( Enjon::Quaternion::AngleAxis( Enjon::ToRadians( 90.0f ), Enjon::Vec3::ZAxis( ) ) );
		//model *= Enjon::Mat4::Scale( Enjon::Vec3( 1.0f ) );
		//std::cout << model << '\n';
	} 

	{
		Enjon::Vec3 p( 5, 2, -3 );
		std::cout << Enjon::Quaternion::AngleAxis( Enjon::ToRadians( 90.0f ), Enjon::Vec3::ZAxis( ) ) * p << "\n";
	}

	TestObjectSerialize( );
	
	return Enjon::Result::SUCCESS; 
} 

//-------------------------------------------------------------
Enjon::Result Game::Update(Enjon::f32 dt)
{ 
	// Update movement and check for success/failure of update
	Enjon::Result res = ProcessInput(dt);
	if (res != Enjon::Result::PROCESS_RUNNING)
	{
		return res;
	}

	// Update entity manager
	mEntities->Update(dt);

	static Enjon::f32 t = 0.0f;
	t += 0.01f * dt;

	Enjon::GraphicsComponent* gc 	= nullptr;
	Enjon::GraphicsComponent* gc2 	= nullptr;
	Enjon::GraphicsComponent* gc3 	= nullptr; 

	if ( mGun.Get( ) ) 
	{
		mGun.Get( )->SetRotation( Enjon::Quaternion::AngleAxis( t * 5.0f, Enjon::Vec3::ZAxis( ) ) 
							* Enjon::Quaternion::AngleAxis( t * 5.0f, Enjon::Vec3::YAxis() ) );
	} 
	
	if ( mRock.Get( ) ) 
	{
		mRock.Get( )->SetRotation( Enjon::Quaternion::AngleAxis( t * 2.0f, Enjon::Vec3::YAxis() ) );
	} 

	mGreen.Get( )->SetRotation( Enjon::Quaternion::AngleAxis( t * 10.0f, Enjon::Vec3::YAxis( ) ) );
mRed.Get( )->SetRotation( Enjon::Quaternion::AngleAxis( t * 10.0f, Enjon::Vec3::XAxis( ) ) );
mBlue.Get( )->SetRotation( Enjon::Quaternion::AngleAxis( t * 10.0f, Enjon::Vec3::XAxis( ) ) );

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
			entity->SetPosition( pos );
			entity->SetRotation( rot );
		}
	}
}

mTextBatch->Begin( );
{
	Enjon::Transform tform( Enjon::Vec3( 0.f, 10.f, -10.f ), Enjon::Quaternion( ), Enjon::Vec3( mFontSize ) );
	Enjon::PrintText( tform, mWorldString, mFont.Get( ), *mTextBatch, Enjon::RGBA32_White( ), 14 );
}
mTextBatch->End( );

// This is where transform propagation happens
// mEntities->LateUpdate(dt);
for ( auto& e : mEntities->GetActiveEntities( ) )
{
	if ( e->HasComponent< Enjon::GraphicsComponent >( ) )
	{
		auto gfx = e->GetComponent< Enjon::GraphicsComponent >( );
		gfx->SetTransform( e->GetWorldTransform( ) );
	}
}

return Enjon::Result::PROCESS_RUNNING;
}

//====================================================================================================================

Enjon::Result Game::ProcessInput( f32 dt )
{
	Enjon::Camera* cam = mGfx->GetSceneCamera( );

	if ( mInput->IsKeyPressed( Enjon::KeyCode::Escape ) )
	{
		return Enjon::Result::SUCCESS;
	}

	if ( mInput->IsKeyPressed( Enjon::KeyCode::T ) )
	{
		mMovementOn = !mMovementOn;
		Enjon::Window* window = mGfx->GetWindow( );

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

	if ( mMovementOn && cam->GetProjectionType( ) == Enjon::ProjectionType::Perspective )
	{
		Enjon::Camera* camera = mGfx->GetSceneCamera( );
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

		if ( mInput->IsKeyDown( Enjon::KeyCode::LeftMouseButton ) )
		{
			Enjon::Scene* scene = mGfx->GetScene();
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
			Enjon::GraphicsComponent* gc = ent->Attach<Enjon::GraphicsComponent>();
			ent->SetScale(v3(scalar));
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

			scene->AddRenderable(gc->GetRenderable());

			mBodies.push_back(body);
			mPhysicsEntities.push_back(ent);

			mDynamicsWorld->addRigidBody(body);
		}

		if (velDir.Length()) velDir = Enjon::Vec3::Normalize(velDir);

		camera->Transform.Position += mCameraSpeed * dt * velDir;

		// Set mouse sensitivity
		f32 MouseSensitivity = 2.0f;

		// Get mouse input and change orientation of camera
		Enjon::Vec2 MouseCoords = mInput->GetMouseCoords();

		Enjon::iVec2 viewPort = mGfx->GetViewport();

		// Grab window from graphics subsystem
		Enjon::Window* window = mGfx->GetWindow(); 

		// Set cursor to not visible
		window->ShowMouseCursor(false);

		// Reset the mouse coords after having gotten the mouse coordinates
		SDL_WarpMouseInWindow(window->GetWindowContext(), (float)viewPort.x / 2.0f, (float)viewPort.y / 2.0f); 

		if ( mLockCamera )
		{ 
			camera->LookAt( mGun.Get()->GetWorldPosition() );
		}
		else
		{
			// Offset camera orientation
			f32 xOffset = Enjon::ToRadians((f32)viewPort.x / 2.0f - MouseCoords.x) * dt * MouseSensitivity;
			f32 yOffset = Enjon::ToRadians((f32)viewPort.y / 2.0f - MouseCoords.y) * dt * MouseSensitivity;
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

