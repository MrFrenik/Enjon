#include "Game.h"
#include "Entity/EntityManager.h"
#include "Entity/Components/GraphicsComponent.h"
#include "IO/ResourceManager.h"
#include "Graphics/DeferredRenderer.h"
#include "Graphics/DirectionalLight.h"
#include "Math/Vec3.h"

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

	auto mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/sphere.obj");
	auto mat = new EG::Material;
	mat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	mat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	mat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	mat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	mat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/green.png"));

	gc->SetMesh(mesh);
	gc->SetMaterial(mat);
	gc->SetPosition(EM::Vec3(0, 0, 0));
	gc->SetScale(10.0f);

	mSun = new EG::DirectionalLight();
	mSun->SetIntensity(10.0f);
	mSun->SetColor(EG::RGBA16_Orange());

	// Get graphics from engine
	auto engine = Enjon::Engine::GetInstance();
	auto graphics = engine->GetGraphics();

	if (graphics)
	{
		auto scene = graphics->GetScene();
		scene->AddRenderable(gc->GetRenderable());
		scene->AddDirectionalLight(mSun);

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
