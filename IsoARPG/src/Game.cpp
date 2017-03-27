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
#include <Asset/Asset.h>
#include <Asset/AssetManager.h>
#include <Asset/TextureAssetLoader.h>
#include <Utils/FileUtils.h>

#include <fmt/printf.h>

#include <Defines.h>
#include <Engine.h>

#include <stdio.h>

#include <iostream>

#include <Bullet/btBulletDynamicsCommon.h>

std::vector<btRigidBody*> mBodies;
btDiscreteDynamicsWorld* mDynamicsWorld;

// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
btAlignedObjectArray<btCollisionShape*> collisionShapes;

// Physics entities to align with rigid bodies
std::vector<Enjon::Entity*> mPhysicsEntities;

Enjon::String mAssetsPath; 
Enjon::AssetManager* mAssetManager;

//-------------------------------------------------------------
Game::Game()
{
}

//-------------------------------------------------------------
Game::~Game()
{
}

//-------------------------------------------------------------
Enjon::Result Game::Initialize()
{ 
	// Set up assets path
	mAssetsPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + Enjon::String("IsoARPG/Assets/"); 
	
	// Create asset manager
	mAssetManager = new Enjon::AssetManager(mAssetsPath); 

	// Paths to resources
	Enjon::String cerebusMeshPath		= Enjon::String("Models/cerebus.obj");
	Enjon::String buddhaMeshPath		= Enjon::String("Models/buddha.obj");
	Enjon::String bunnyMeshPath			= Enjon::String("Models/bunny.obj");
	Enjon::String dragonMeshPath		= Enjon::String("Models/dragon.obj");
	Enjon::String monkeyMeshPath		= Enjon::String("Models/monkey.obj");
	Enjon::String sphereMeshPath		= Enjon::String("Models/unit_sphere.obj");
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

	// Add to asset database
	mAssetManager->AddToDatabase(cerebusAlbedoPath); 
	mAssetManager->AddToDatabase(cerebusNormalPath); 
	mAssetManager->AddToDatabase(cerebusMetallicPath); 
	mAssetManager->AddToDatabase(cerebusRoughnessPath); 
	mAssetManager->AddToDatabase(cerebusEmissivePath); 
	mAssetManager->AddToDatabase(mahogAlbedoPath); 
	mAssetManager->AddToDatabase(mahogNormalPath); 
	mAssetManager->AddToDatabase(mahogMetallicPath); 
	mAssetManager->AddToDatabase(mahogRoughnessPath); 
	mAssetManager->AddToDatabase(mahogEmissivePath); 
	mAssetManager->AddToDatabase(mahogAOPath); 
	mAssetManager->AddToDatabase(cerebusMeshPath);
	mAssetManager->AddToDatabase(sphereMeshPath);
	mAssetManager->AddToDatabase(shaderBallMeshPath); 

	// Create entity manager
	mEntities = new Enjon::EntityManager();

	mEntities->RegisterComponent<Enjon::GraphicsComponent>();
	mEntities->RegisterComponent<Enjon::PointLightComponent>();

	// Allocate handle
	mGun = mEntities->Allocate();
	auto gc = mGun->Attach<Enjon::GraphicsComponent>(); 

	mGunMat 	= new Enjon::Material; 
	mGunMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("materials.cerebus.albedo"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("materials.cerebus.normal"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("materials.cerebus.metallic"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("materials.cerebus.roughness"));
	mGunMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("materials.cerebus.emissive"));
	mGunMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("materials.cerebus.emissive"));

	mGun->SetPosition(Enjon::Vec3(0.0f, 6.0f, 0.0f));
	mGun->SetScale(3.0f);
	gc->SetMesh(mAssetManager->GetAsset<Enjon::Mesh>("models.cerebus"));
	gc->SetMaterial(mGunMat);

	mSun = new Enjon::DirectionalLight();
	mSun->SetIntensity(10.0f);
	mSun->SetColor(Enjon::RGBA16_Orange());

	auto mSun2 = new Enjon::DirectionalLight(Enjon::Vec3(0.5f, 0.5f, -0.75f), Enjon::RGBA16_SkyBlue(), 10.0f); 

	mFloorMat = new Enjon::Material();
	mFloorMat->SetTexture(Enjon::TextureSlotType::Albedo, mAssetManager->GetAsset<Enjon::Texture>("materials.mahogfloor.albedo"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Normal, mAssetManager->GetAsset<Enjon::Texture>("materials.mahogfloor.normal"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Metallic, mAssetManager->GetAsset<Enjon::Texture>("materials.mahogfloor.roughness"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Roughness, mAssetManager->GetAsset<Enjon::Texture>("materials.mahogfloor.roughness"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::Emissive, mAssetManager->GetAsset<Enjon::Texture>("materials.mahogfloor.emissive"));
	mFloorMat->SetTexture(Enjon::TextureSlotType::AO, mAssetManager->GetAsset<Enjon::Texture>("materials.mahogfloor.ao"));
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

	// Get graphics from engine
	auto engine = Enjon::Engine::GetInstance();
	auto graphics = engine->GetGraphics();

	if (graphics)
	{
		auto scene = graphics->GetScene();
		scene->AddDirectionalLight(mSun);
		scene->AddDirectionalLight(mSun2);
		scene->AddRenderable(gc->GetRenderable());
		scene->AddQuadBatch(mBatch);
		scene->SetSun(mSun);
		scene->SetAmbientColor(Enjon::SetOpacity(Enjon::RGBA16_White(), 0.1f));

		// Set graphics camera position
		auto cam = graphics->GetSceneCamera();
		cam->SetPosition(Enjon::Vec3(0, 0, -10));
		cam->LookAt(Enjon::Vec3(0, 0, 0));
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
	}

	return Enjon::Result::SUCCESS; 
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

	if (mGun && mGun->HasComponent<Enjon::GraphicsComponent>())
	{
		mGun->SetRotation(quat::AngleAxis(t * 5.0f, v3(0, 1, 0)));
		gc = mGun->GetComponent<Enjon::GraphicsComponent>(); 
	} 

	// Physics simulation
	mDynamicsWorld->stepSimulation(1.f/60.f, 10);

	// Step through physics bodies and update entity position
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
				Enjon::Vec3 pos = Enjon::Vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
				Enjon::Quaternion rot = Enjon::Quaternion(trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), -trans.getRotation().w());
				entity->SetPosition(pos);
				entity->SetRotation(rot);
			}
		}
	}

	// This is where transform propagation happens
	// mEntities->LateUpdate(dt);
	for (auto& e : mEntities->GetActiveEntities())
	{
		e->UpdateComponentTransforms(dt);
	}

	return Enjon::Result::PROCESS_RUNNING;
}

//
Enjon::Result Game::ProcessInput(f32 dt)
{
	Enjon::Engine* engine = Enjon::Engine::GetInstance();
	Enjon::Input* input = engine->GetInput();
	Enjon::DeferredRenderer* gfx = engine->GetGraphics();

	if (input->IsKeyPressed(SDLK_ESCAPE))
	{
		return Enjon::Result::SUCCESS;
	}

	if (input->IsKeyPressed(SDLK_t))
	    {
	    	mMovementOn = !mMovementOn;
			Enjon::Window* window = gfx->GetWindow();

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
		    Enjon::Camera* camera = gfx->GetSceneCamera();
		   	Enjon::Vec3 velDir(0, 0, 0); 

			if (input->IsKeyDown(SDLK_w))
			{
				Enjon::Vec3 F = camera->Forward();
				velDir += F;
			}
			if (input->IsKeyDown(SDLK_s))
			{
				Enjon::Vec3 B = camera->Backward();
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

			/*
		    if (input->IsKeyDown(SDL_BUTTON_LEFT))
		    {
		    	auto cam = camera;
		    	auto scene = Enjon::Engine::GetInstance()->GetGraphics()->GetScene();
		    	auto pos = cam->GetPosition() + cam->Forward() * 2.0f;
		    	auto vel = cam->Forward() * 30.0f;

		    	f32 scalar = 0.3f; 

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
				body->setRestitution(0.2f);
				body->setFriction(2.0f);
				body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
				body->setDamping(0.7f, 0.7f);

				auto ent = mEntities->Allocate();
				auto gc = ent->Attach<Enjon::GraphicsComponent>();
				ent->SetScale(v3(scalar));
				gc->SetMesh(mSphereMesh);
				gc->SetMaterial(mGunMat);
				scene->AddRenderable(gc->GetRenderable());

				mBodies.push_back(body);
				mPhysicsEntities.push_back(ent);

				mDynamicsWorld->addRigidBody(body);
		    }
			*/

			if (velDir.Length()) velDir = Enjon::Vec3::Normalize(velDir);

			camera->Transform.Position += mCameraSpeed * dt * velDir;

			// Set mouse sensitivity
			f32 MouseSensitivity = 2.0f;

			// Get mouse input and change orientation of camera
			Enjon::Vec2 MouseCoords = input->GetMouseCoords();

			Enjon::iVec2 viewPort = gfx->GetViewport();

			// Grab window from graphics subsystem
			Enjon::Window* window = gfx->GetWindow(); 

			// Set cursor to not visible
			window->ShowMouseCursor(false);

			// Reset the mouse coords after having gotten the mouse coordinates
			SDL_WarpMouseInWindow(window->GetWindowContext(), (float)viewPort.x / 2.0f, (float)viewPort.y / 2.0f);

			// Offset camera orientation
			f32 xOffset = Enjon::ToRadians((f32)viewPort.x / 2.0f - MouseCoords.x) * dt * MouseSensitivity;
			f32 yOffset = Enjon::ToRadians((f32)viewPort.y / 2.0f - MouseCoords.y) * dt * MouseSensitivity;
			camera->OffsetOrientation(xOffset, yOffset); 
	    }

		return Enjon::Result::PROCESS_RUNNING;
}

//-------------------------------------------------------------
Enjon::Result Game::Shutdown()
{
	printf("%d\n", sizeof(Enjon::Entity));
	printf("%d\n", sizeof(Enjon::Transform));

	fmt::print("{}", sizeof(Enjon::Entity));
	
	printf("Shutting down game...\n");

	return Enjon::Result::SUCCESS;
}
