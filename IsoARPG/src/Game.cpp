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
#include <Math/Common.h>
#include <System/Types.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>

#include <Defines.h>
#include <Engine.h>

#include <stdio.h>

#include <iostream>

#include <Bullet/btBulletDynamicsCommon.h>

/*
std::vector<btRigidBody*> mBodies;
btDiscreteDynamicsWorld* mDynamicsWorld;

// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
btAlignedObjectArray<btCollisionShape*> collisionShapes;

// Physics entities to align with rigid bodies
std::vector<Enjon::Entity*> mPhysicsEntities;

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

	mGunMesh 	= EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/cerebus.obj");
	mSphereMesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/sphere.obj");
	mBuddhaMesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/buddha.obj");

	mRedMat 	= new EG::Material;
	mRedMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	mRedMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	mRedMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	mRedMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	mRedMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/red.png"));

	mBlueMat 	= new EG::Material;
	mBlueMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	mBlueMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	mBlueMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	mBlueMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	mBlueMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/blue.png"));

	mGreenMat 	= new EG::Material;
	mGreenMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Albedo.png"));
	mGreenMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Normal.png"));
	mGreenMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Metallic.png"));
	mGreenMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/RustedIron/Roughness.png"));
	mGreenMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../Assets/Textures/green.png"));

	mGunMat 	= new EG::Material;
	mGunMat->SetTexture(EG::TextureSlotType::ALBEDO, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Albedo.png"));
	mGunMat->SetTexture(EG::TextureSlotType::NORMAL, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Normal.png"));
	mGunMat->SetTexture(EG::TextureSlotType::METALLIC, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Metallic.png"));
	mGunMat->SetTexture(EG::TextureSlotType::ROUGHNESS, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Roughness.png"));
	mGunMat->SetTexture(EG::TextureSlotType::EMISSIVE, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Materials/Cerebus/Emissive.png"));

	mGun->SetPosition(v3(0.0f, 3.0f, 0.0f));
	gc->SetMesh(mGunMesh);
	gc->SetMaterial(mGunMat);
	gc->SetPosition(v3(0.0f, 3.0f, 0.0f));

	mGreen->SetPosition(v3(1.0f, 3.0f, -1.0f));
	mGreen->SetScale(v3(0.2f));
	gc2->SetMesh(mSphereMesh);
	gc2->SetMaterial(mGreenMat);
	gc2->SetPosition(mGreen->GetWorldPosition());
	gc2->SetScale(mGreen->GetWorldScale());

	mRed->SetPosition(v3(1.0f, 10.0f, -1.0f));
	mRed->SetScale(v3(0.7f));
	gc3->SetMesh(mBuddhaMesh);
	gc3->SetMaterial(mRedMat);
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

  	// auto plc = mRed->Attach<Enjon::PointLightComponent>();
  	// plc->SetAttenuationRate(0.2f);
  	// plc->SetColor(EG::RGBA16_Red());
  	// plc->SetRadius(300.0f);
  	// plc->SetIntensity(50.0f);

  	// auto plc2 = mGreen->Attach<Enjon::PointLightComponent>();
  	// plc2->SetAttenuationRate(0.2f);
  	// plc2->SetColor(EG::RGBA16_Green());
  	// plc2->SetRadius(300.0f);
  	// plc2->SetIntensity(50.0f);

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
		// scene->AddPointLight(plc->GetLight());
		// scene->AddPointLight(plc2->GetLight());
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

			ImGui::SliderFloat("Camera Speed", &mCameraSpeed, 0.1f, 20.0f);

			ImGui::Text("%d", mEntities->GetActiveEntities().size());
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
		groundTransform.setOrigin(btVector3(0, -0.8, 0));

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

		// mBodies.push_back(body);
	}
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
	// Update movement
	ProcessInput(dt);

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
		mRed->SetScale(EM::Clamp(sin(t) * 3.0f, 0.4f, 3.0f));
		gc3 = mRed->GetComponent<Enjon::GraphicsComponent>();
	}

	// Physics simulation
	mDynamicsWorld->stepSimulation(1.f/60.f, 10);

	// Step through physics bodies and change graphics position/orientation based on those
	for (u32 i = 0; i < (u32)mBodies.size(); ++i)
	{
		btRigidBody* body = mBodies.at(i);
		Enjon::Entity* entity = mPhysicsEntities.at(i);
		btTransform trans;

		if (body && body->getMotionState())
		{
			body->getMotionState()->getWorldTransform(trans);
			if (entity && entity->HasComponent<Enjon::GraphicsComponent>())
			{
				auto gComp = entity->GetComponent<Enjon::GraphicsComponent>();
				EM::Vec3 pos = EM::Vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
				EM::Quaternion rot = EM::Quaternion(trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), -trans.getRotation().w());
				gComp->SetPosition(pos);
				gComp->SetRotation(rot);
				// entity->SetPosition(pos);
				// entity->SetRotation(rot);
			}
		}
	}

	// This is where transform propogation happens
	// mEntities->LateUpdate(dt);
	// for (auto& e : mEntities->GetActiveEntities())
	// {
	// 	e->UpdateComponentTransforms(dt);
	// }
}

//
void Game::ProcessInput(f32 dt)
{
	auto engine = Enjon::Engine::GetInstance();
	auto input = engine->GetInput();
	auto gfx = engine->GetGraphics();

	if (input->IsKeyPressed(SDLK_t))
	    {
	    	mMovementOn = !mMovementOn;
			EG::Window* window = gfx->GetWindow();

			if (!mMovementOn)
			{
				window->ShowMouseCursor(true);
			}
			else
			{
				window->ShowMouseCursor(false);
			}
	    }

	    if (mMovementOn)
	    {
		    EG::Camera* camera = gfx->GetSceneCamera();
		   	EM::Vec3 velDir(0, 0, 0); 

			if (input->IsKeyDown(SDLK_w))
			{
				EM::Vec3 F = camera->Forward();
				velDir += F;
			}
			if (input->IsKeyDown(SDLK_s))
			{
				EM::Vec3 B = camera->Backward();
				velDir += B;
			}
			if (input->IsKeyDown(SDLK_a))
			{
				velDir += camera->Left();
			}
			if (input->IsKeyDown(SDLK_d))
			{
				velDir += camera->Right();
			}

		    if (input->IsKeyDown(SDL_BUTTON_LEFT))
		    {
		    	auto cam = camera;
		    	auto scene = Enjon::Engine::GetInstance()->GetGraphics()->GetScene();
		    	auto pos = cam->GetPosition() + cam->Forward() * 2.0f;
		    	auto vel = cam->Forward() * 30.0f;

				btCollisionShape* colShape = new btSphereShape(btScalar(0.1f));
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
				body->setRestitution(0.02f);
				body->setFriction(10.0f);
				body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
				body->setDamping(0.8f, 0.8f);

				auto ent = mEntities->Allocate();
				auto gc = ent->Attach<Enjon::GraphicsComponent>();
				ent->SetScale(v3(0.1f));
				gc->SetMesh(mSphereMesh);
				gc->SetMaterial(mGunMat);
				gc->SetScale(0.1f);
				scene->AddRenderable(gc->GetRenderable());

				mBodies.push_back(body);
				mPhysicsEntities.push_back(ent);

				mDynamicsWorld->addRigidBody(body);
		    }

			if (velDir.Length()) velDir = EM::Vec3::Normalize(velDir);

			camera->Transform.Position += mCameraSpeed * dt * velDir;

			auto MouseSensitivity = 2.0f;

			// Get mouse input and change orientation of camera
			auto MouseCoords = input->GetMouseCoords();

			auto viewPort = gfx->GetViewport();

			EG::Window* window = gfx->GetWindow();

			window->ShowMouseCursor(false);

			// Reset the mouse coords after having gotten the mouse coordinates
			SDL_WarpMouseInWindow(window->GetWindowContext(), (float)viewPort.x / 2.0f, (float)viewPort.y / 2.0f);

			camera->OffsetOrientation(
										(EM::ToRadians(((float)viewPort.x / 2.0f - MouseCoords.x) * dt) * MouseSensitivity), 
										(EM::ToRadians(((float)viewPort.y / 2.0f - MouseCoords.y) * dt) * MouseSensitivity)
									);

	    }
}

//-------------------------------------------------------------
void Game::Shutdown()
{
	printf("%d\n", sizeof(Enjon::Entity));
	printf("%d\n", sizeof(Enjon::Math::Transform));
	printf("Shutting down game...\n");
}
*/
	    