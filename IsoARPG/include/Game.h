#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include <Application.h> 
#include <Asset/Asset.h>
#include <Entity/EntityManager.h>
#include <Graphics/ShaderGraph.h>
#include <Graphics/Shader.h>

#include <Base/MetaClassRegistry.h>

#include <vector> 


namespace Enjon
{
	class Entity;
	class EntityManager; 
	class QuadBatch;
	class DirectionalLight;
	class Material;
	class Mesh;
	class GraphicsSubsystem;
	class Input;
	class UIFont;
}

class Game : public Enjon::Application
{
	public:
		Game();
		~Game();

		virtual Enjon::Result Initialize();  
		virtual Enjon::Result Update(Enjon::f32 dt);
		virtual Enjon::Result Shutdown();

		virtual Enjon::Result ProcessInput(Enjon::f32 dt);

	private:
		void ListEntityChildren(Enjon::Entity* entity, Enjon::u32 indentAmount);

		void TestObjectSerialize( );

	private:
		Enjon::GraphicsSubsystem* mGfx		= nullptr;
		Enjon::Input* mInput				= nullptr;
		Enjon::EntityManager* mEntities		= nullptr;

		Enjon::EntityHandle mGun;
		Enjon::EntityHandle mGreen;
		Enjon::EntityHandle mRed;
		Enjon::EntityHandle mBlue;
		Enjon::EntityHandle mRock;
		Enjon::EntityHandle mRock2;
		Enjon::EntityHandle mSerializedEntity;

		Enjon::DirectionalLight* mSun	= nullptr;
		Enjon::QuadBatch* mBatch 		= nullptr;
		Enjon::QuadBatch* mTextBatch 	= nullptr;

		Enjon::Material* mGunMat 	= nullptr;
		Enjon::Material* mRedMat 	= nullptr;
		Enjon::Material* mGreenMat 	= nullptr;
		Enjon::Material* mBlueMat 	= nullptr;
		Enjon::Material* mFloorMat	= nullptr;
		Enjon::Material* mFontMat	= nullptr;
		Enjon::Material* mRockMat	= nullptr;
		Enjon::Material* mPlasticMat	= nullptr;
		Enjon::Material* mGoldMat	= nullptr; 

		Enjon::AssetHandle< Enjon::Mesh > mSphereMesh;
		Enjon::AssetHandle< Enjon::Mesh > mGunMesh;
		Enjon::AssetHandle< Enjon::Mesh > mBuddhaMesh;
		Enjon::AssetHandle< Enjon::Mesh > mMonkeyMesh; 
		Enjon::AssetHandle< Enjon::UIFont > mFont;
		
		std::vector<Enjon::AssetHandle<Enjon::Mesh>*> mMeshSwitches; 

		std::vector<Enjon::Entity*> mHandles; 

		bool mShowEntities = true;
		bool mMovementOn = false;
		bool mLockCamera = false;
		bool mRotateCamera = false;

		Enjon::f32 mCameraSpeed = 0.12f;

		Enjon::f32 mFontSize = 1.0f;

		Enjon::String mWorldString = ""; 

		Enjon::Texture* mTex = nullptr;

		Enjon::TestNamespace::PointLight mTestObject;
};

#endif