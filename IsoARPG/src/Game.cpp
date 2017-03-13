#include "Game.h"

#include <Entity/EntityManager.h>
#include <Entity/Components/GraphicsComponent.h>
#include <Entity/Components/PointLightComponent.h>
#include <IO/ResourceManager.h>
#include <Graphics/DeferredRenderer.h>
#include <Graphics/DirectionalLight.h>
#include <Graphics/QuadBatch.h>
#include <Math/Vec3.h>
#include <Math/Random.h>
#include <System/Types.h>
#include <ImGui/ImGuiManager.h>

#include <Defines.h>
#include <Engine.h>

#include <stdio.h>

#include <iostream>

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
	mEntities->RegisterComponent<Enjon::PointLightComponent>();

	// Allocate handle
	mGun = mEntities->Allocate();
	mGreen = mEntities->Allocate();
	mRed = mEntities->Allocate();
	auto gc = mGun->Attach<Enjon::GraphicsComponent>();
	auto gc2 = mGreen->Attach<Enjon::GraphicsComponent>();
	auto gc3 = mRed->Attach<Enjon::GraphicsComponent>();

	// Set up child heirarchy
	mGun->AddChild(mGreen);
	mGreen->AddChild(mRed);

	auto gunMesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/cerebus.obj");
	auto sphereMesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/sphere.obj");

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

	mGun->SetPosition(v3(0.0f, 3.0f, 0.0f));
	gc->SetMesh(gunMesh);
	gc->SetMaterial(gunMat);
	gc->SetPosition(v3(0.0f, 3.0f, 0.0f));

	mGreen->SetPosition(v3(1.0f, 3.0f, -1.0f));
	mGreen->SetScale(v3(0.2f));
	gc2->SetMesh(sphereMesh);
	gc2->SetMaterial(greenMat);
	gc2->SetPosition(mGreen->GetWorldPosition());
	gc2->SetScale(mGreen->GetWorldScale());

	mRed->SetPosition(v3(1.0f, 10.0f, -1.0f));
	mRed->SetScale(v3(1.0f));
	gc3->SetMesh(sphereMesh);
	gc3->SetMaterial(redMat);
	gc3->SetPosition(mRed->GetWorldPosition());
	gc3->SetScale(mRed->GetWorldScale());

	mSun = new EG::DirectionalLight();
	mSun->SetIntensity(10.0f);
	mSun->SetColor(EG::RGBA16_Orange());

	auto mSun2 = new EG::DirectionalLight(EM::Vec3(0.5f, 0.5f, -0.75f), EG::RGBA16_SkyBlue(), 10.0f);

	// Ground
	mBatch = new EG::QuadBatch();
	mBatch->Init();
	EG::Material* floorMat = new EG::Material();
 	floorMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/OakFloor/Albedo.png"));
 	floorMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/OakFloor/Normal.png"));
 	floorMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/OakFloor/Roughness.png"));
 	floorMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/OakFloor/Roughness.png"));
 	floorMat->SetTexture(EG::TextureSlotType::AO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/OakFloor/AO.png"));
  	mBatch->SetMaterial(floorMat);

  	auto plc = mRed->Attach<Enjon::PointLightComponent>();
  	plc->SetAttenuationRate(0.2f);
  	plc->SetColor(EG::RGBA16_Red());
  	plc->SetRadius(300.0f);
  	plc->SetIntensity(50.0f);

  	auto plc2 = mGreen->Attach<Enjon::PointLightComponent>();
  	plc2->SetAttenuationRate(0.2f);
  	plc2->SetColor(EG::RGBA16_Green());
  	plc2->SetRadius(300.0f);
  	plc2->SetIntensity(50.0f);

  	// for (auto i = 1; i < 100; ++i)
  	// {
  	// 	auto h = mEntities->Allocate();
  	// 	mHandles.push_back(h);
  	// 	mHandles.at(i - 1)->AddChild(h);
  	// 	h->SetPosition(v3(0.0f, 2.0f, 0.0f));
  	// 	auto gfx = h->Attach<Enjon::GraphicsComponent>();
  	// 	gfx->SetMesh(sphereMesh);
  	// 	gfx->SetMaterial(blueMat);
  	// 	gfx->SetPosition(h->GetWorldPosition());
  	// 	Enjon::Engine::GetInstance()->GetGraphics()->GetScene()->AddRenderable(gfx->GetRenderable());
  	// }

	mBatch->Begin();
  	{
 		for (auto i = -10; i <10; i++)
 		{
 			for (auto j = -10; j <10; j++)
 			{
 				EM::Transform t(EM::Vec3(j * 2, 0, i * 2), quat::AngleAxis(EM::ToRadians(90), EM::Vec3(1, 0, 0)), EM::Vec3(1, 1, 1));
 				mBatch->Add(
 							t, 
 							EM::Vec4(0, 0, 1, 1),
 							0
 						);
 			}
 		}
 	}
  	mBatch->End();

	// Get graphics from engine
	auto engine = Enjon::Engine::GetInstance();
	auto graphics = engine->GetGraphics();

	if (graphics)
	{
		auto scene = graphics->GetScene();
		scene->AddDirectionalLight(mSun);
		scene->AddDirectionalLight(mSun2);
		scene->AddRenderable(gc->GetRenderable());
		scene->AddRenderable(gc2->GetRenderable());
		scene->AddRenderable(gc3->GetRenderable());
		scene->AddPointLight(plc->GetLight());
		scene->AddPointLight(plc2->GetLight());
		scene->AddQuadBatch(mBatch);
		scene->SetSun(mSun);
		scene->SetAmbientColor(EG::SetOpacity(EG::RGBA16_White(), 0.1f));

		// Set graphics camera position
		auto cam = graphics->GetSceneCamera();
		cam->SetPosition(EM::Vec3(0, 0, -10));
		cam->LookAt(EM::Vec3(0, 0, 0));
	}

	// Set up ImGui window
	mShowEntities = true;
	auto showEntities = [&]()
	{
		// Docking windows
		if (ImGui::BeginDock("Entities", &mShowEntities))
		{
			auto position 	= mGun->GetLocalPosition();
			auto scale 		= mGun->GetLocalScale();
			auto rotation 	= mGun->GetLocalRotation();

			f32 pos[] = {position.x, position.y, position.z}; 
			f32 scl[] = {scale.x, scale.y, scale.z}; 
			f32 rot[] = {rotation.x, rotation.y, rotation.z}; 

			ImGui::InputFloat3("Position", pos);
			ImGui::InputFloat3("Scale", scl);
			ImGui::InputFloat3("Rotation", rot);

			mGun->SetPosition(v3(pos[0], pos[1], pos[2]));
			mGun->SetScale(v3(scl[0], scl[1], scl[2]));
			mGun->SetRotation(quat(rot[0], rot[1], rot[2], rotation.w));
		}
		ImGui::EndDock();
	};

	// Set up way to dock these initially at start up of all systems
	Enjon::ImGuiManager::RegisterWindow(showEntities);

	// Set up docking layout
	Enjon::ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout("Entities", "Game View", ImGui::DockSlotType::Slot_Left, 0.1f));
}

//-------------------------------------------------------------
void Game::ListEntityChildren(Enjon::Entity* entity, u32 indentAmount)
{
	for (auto& c : entity->GetChildren())
	{
		// Formatting
		for (u32 i = 0; i < indentAmount; ++i)
		{
			ImGui::Text("\t");
		}

		ImGui::SameLine();
		ImGui::Text("%d", c->GetID());

		ImGui::SameLine();
    	if (ImGui::Button("-"))
    	{
    		mEntities->Destroy(c);
    	}

		// List all children recursively
		ListEntityChildren(c, indentAmount + 1);
	}

}

//-------------------------------------------------------------
void Game::Update(Enjon::f32 dt)
{
	// Update entity manager
	mEntities->Update(dt);

	static float t = 0.0f;
	t += 0.01f * dt;

	Enjon::GraphicsComponent* gc 	= nullptr;
	Enjon::GraphicsComponent* gc2 	= nullptr;
	Enjon::GraphicsComponent* gc3 	= nullptr;

	if (mGun && mGun->HasComponent<Enjon::GraphicsComponent>())
	{
		mGun->SetRotation(quat::AngleAxis(t * 5.0f, v3(0, 1, 0)));
		gc = mGun->GetComponent<Enjon::GraphicsComponent>();
	}

	if (mGreen && mGreen->HasComponent<Enjon::GraphicsComponent>())
	{
		// Update local position of parent
		mGreen->SetPosition(v3(cos(t) * 3.0f, 3.0f, sin(t) * 3.0f));
		gc2 = mGreen->GetComponent<Enjon::GraphicsComponent>();
	}

	if (mRed && mRed->HasComponent<Enjon::GraphicsComponent>())
	{
		mRed->SetPosition(v3(3.0f, -20.0f + sin(t * 30.0f) * 3.0f, 3.0f));
		gc3 = mRed->GetComponent<Enjon::GraphicsComponent>();
	}

	// Don't delete these or this shit will fail
	// for (u32 i = 0; i < (u32)mHandles.size(); ++i)
	// {
	// 	auto h = mHandles.at(i);

	// 	if (h->IsValid())
	// 	{
	// 		h->SetPosition(v3(cos(t * i) * i, i, sin(t * i) * i));
	// 	}
	// }

	// This is where transform propogation happens
	mEntities->LateUpdate(dt);
}

//-------------------------------------------------------------
void Game::Shutdown()
{
	printf("%d\n", sizeof(Enjon::Entity));
	printf("%d\n", sizeof(Enjon::Math::Transform));
	printf("Shutting down game...\n");
}
