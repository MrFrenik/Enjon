#include "Game.h"
#include "Entity/EntityManager.h"
#include "Entity/Components/GraphicsComponent.h"
#include "IO/ResourceManager.h"
#include "Graphics/DeferredRenderer.h"
#include "Graphics/DirectionalLight.h"
#include "Math/Vec3.h"
#include "Math/Random.h"

#include "Defines.h"
#include "Engine.h"

#include <stdio.h>

//-------------------------------------------------------------
Game::Game()
{

}

//-------------------------------------------------------------
Game::~Game()
{

}

//-------------------------------------------------------------
void Game::Initialize()
{
	// Create entity manager
	mEntities = new Enjon::EntityManager();

	mEntities->RegisterComponent<Enjon::GraphicsComponent>();

	// Allocate handle
	mHandle = mEntities->Allocate();
	auto gc = mHandle->Attach<Enjon::GraphicsComponent>();

	auto gunMesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/cerebus.obj");
	auto mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/sphere.obj");
	auto redMat 	= new EG::Material;
	redMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	redMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	redMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	redMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	redMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/red.png"));

	auto blueMat 	= new EG::Material;
	blueMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	blueMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	blueMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	blueMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	blueMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/blue.png"));

	auto greenMat 	= new EG::Material;
	greenMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	greenMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	greenMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	greenMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	greenMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/green.png"));

	auto gunMat 	= new EG::Material;
	gunMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Albedo.png"));
	gunMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Normal.png"));
	gunMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Metallic.png"));
	gunMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Roughness.png"));
	gunMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Emissive.png"));

	gc->SetMesh(gunMesh);
	gc->SetMaterial(gunMat);

	mSun = new EG::DirectionalLight();
	mSun->SetIntensity(10.0f);
	mSun->SetColor(EG::RGBA16_Orange());

	auto mSun2 = new EG::DirectionalLight(EM::Vec3(0.5f, 0.5f, -0.75f), EG::RGBA16_SkyBlue(), 10.0f);

	// Get graphics from engine
	auto engine = Enjon::Engine::GetInstance();
	auto graphics = engine->GetGraphics();

	if (graphics)
	{
		auto scene = graphics->GetScene();
		scene->AddDirectionalLight(mSun);
		scene->AddDirectionalLight(mSun2);
		scene->SetSun(mSun);
		scene->AddRenderable(gc->GetRenderable());
		scene->SetAmbientColor(EG::SetOpacity(EG::RGBA16_White(), 0.1f));

		/*
		for (auto i = 0; i < 100; ++i)
		{
			auto handle = mEntities->Allocate();
			auto gcomp = handle->Attach<Enjon::GraphicsComponent>();
			gcomp->SetMesh(mesh);
			gcomp->SetScale(10.0f);

			Enjon::f32 x = (Enjon::f32)ER::Roll(-200, 200);
			Enjon::f32 y = (Enjon::f32)ER::Roll(-200, 200);
			Enjon::f32 z = (Enjon::f32)ER::Roll(-200, 200);

			auto roll = ER::Roll(0, 2);

			switch(roll)
			{
				case 0:	gcomp->SetMaterial(redMat); break;
				case 1: gcomp->SetMaterial(greenMat); break;
				case 2: gcomp->SetMaterial(blueMat); break;
				default: gcomp->SetMaterial(greenMat); break;
			}

			gcomp->SetPosition(EM::Vec3(x, y, z));
			scene->AddRenderable(gcomp->GetRenderable());
		}
		*/

		// Set graphics camera position
		auto cam = graphics->GetSceneCamera();
		cam->SetPosition(EM::Vec3(0, 0, -10));
		cam->LookAt(EM::Vec3(0, 0, 0));
	}
}

//-------------------------------------------------------------
void Game::Update(Enjon::f32 dt)
{
}

//-------------------------------------------------------------
void Game::Shutdown()
{
	printf("%d\n", sizeof(Enjon::EntityHandle));
	printf("%d\n", sizeof(Enjon::Math::Transform));
	printf("Shutting down game...\n");
}
