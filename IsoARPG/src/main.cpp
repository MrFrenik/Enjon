#define REALGAME 0  // Alter this to change back and forth to testing mode
#define PARSER  0
#define TEST_FUNCTIONS 0

// Don't touch these
#if REALGAME 
	#define TESTING 0
#elif TEST_FUNCTIONS
	#define TESTING 0
	#define TESTING_PARSER 0
	#define FUNCTION_TESTS 1
#elif PARSER
	#define TESTING 0
	#define TESTING_PARSER 1 
#else
	#define TESTING 1
#endif

#if TESTING 

#define FULLSCREENMODE   0
#define SECOND_DISPLAY   0

#if FULLSCREENMODE
	#if SECOND_DISPLAY
		#define SCREENWIDTH 1440
		#define SCREENHEIGHT 900
	#else
		#define SCREENWIDTH  1920
		#define SCREENHEIGHT 1080
	#endif
	#define SCREENRES    Graphics::FULLSCREEN
#else
	#define SCREENWIDTH  1024
	#define SCREENHEIGHT 768
	#define SCREENRES Graphics::DEFAULT
#endif 

#include <Enjon.h>
#include <SDL2/SDL.h>
#include <GLEW/glew.h>
#include <ECS/ComponentSystems.h>
#include <ECS/Entity.h>
#include "Level.h"
#include <Utils/FileUtils.h>
#include <Math/Random.h>
#include "Animation.h"
#include "AnimationManager.h"
#include "SpatialHash.h"
#include "Math/Random.h"

#include <stdio.h>
#include <iostream> 
#include <time.h>
#include <stdlib.h>

char buffer[256];
char buffer2[256];
char buffer3[256];
bool isRunning = true;

using namespace Enjon;
using namespace ECS;
using namespace Systems;

void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, struct EntityManager* Manager, ECS::eid32 Entity);

#undef main
int main(int argc, char** argv)
{
	// Seed random 
	srand(time(NULL));

	float FPS = 60;
	int screenWidth = SCREENWIDTH, screenHeight = SCREENHEIGHT;

	// Init Limiter
	Enjon::Utils::FPSLimiter Limiter; 
	Limiter.Init(60.0f); 

	//Init Enjon
	Enjon::Init();

	// Create a window
	Graphics::Window Window;
	Window.Init("Testing Grounds", screenWidth, screenHeight, SCREENRES);

	// Create Camera
	Graphics::Camera2D Camera;
	Camera.Init(screenWidth, screenHeight);
	Camera.SetScale(1.0f); 
	
	// Create HUDCamera
	Graphics::Camera2D HUDCamera;
	HUDCamera.Init(screenWidth, screenHeight);
	HUDCamera.SetScale(1.0f);

	// Init AnimationManager
	AnimationManager::Init(); 
	
	// Init level
	Enjon::Graphics::SpriteBatch TileBatch;
	TileBatch.Init();

	Level level;
	Graphics::GLTexture TileTexture;
	level.Init(Camera.GetPosition().x, Camera.GetPosition().y, 100, 100);
	TileTexture = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledblue.png");
	
	float x = Camera.GetPosition().x;
	float y = Camera.GetPosition().y;
	float currentX = x;
	float currentY = y; 
	float tilewidth = 64.0f;
	float tileheight = tilewidth / 2.0f; 

	// Spatial Hash
	SpatialHash::Grid grid;
	int width = level.GetWidth();
	SpatialHash::Init(&grid, width, width);

	// Make spritesheets for player and enemy and item
	Enjon::Graphics::SpriteSheet PlayerSheet;
	Enjon::Graphics::SpriteSheet EnemySheet;
	Enjon::Graphics::SpriteSheet ItemSheet;
	Enjon::Graphics::SpriteSheet ArrowSheet;
	PlayerSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtestframessplit.png"), Enjon::Math::iVec2(6, 24));
	EnemySheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/enemy.png"), Math::iVec2(1, 1));
	ItemSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Math::iVec2(1, 1));
	ArrowSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"), Math::iVec2(8, 1));
	
	// Creating tiled iso level
	TileBatch.Begin(); 
	level.DrawIsoLevel(TileBatch);
	TileBatch.End();	
 
	// Create PlayerBatch
	Enjon::Graphics::SpriteBatch PlayerBatch;
	PlayerBatch.Init();
	
	// Create QuadrantBatch
	Enjon::Graphics::SpriteBatch EnemyBatch;
	EnemyBatch.Init();

	// Create InputManager
	Input::InputManager Input;

	// Init ShaderManager
	Enjon::Graphics::ShaderManager::Init(); 

	/////////////////
	// Testing ECS //   
	/////////////////

	// Create new EntityManager
	struct EntityManager* World = EntitySystem::NewEntityManager(level.GetWidth(), level.GetWidth());

	Math::Vec2 Pos = Camera.GetPosition() + 50.0f;

	static Math::Vec2 enemydims(222.0f, 200.0f);
	static uint32 AmountDrawn = 1;

	for (int e = 0; e < AmountDrawn; e++)
	{
		float height = 30.0f;
		EntitySystem::CreateAI(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight(), 0))), height),
																enemydims, &EnemySheet, "Enemy", 0.05f); // NOTE(John): The transform system is a bottleneck right now }
	}


	// Create player
	eid32 Player = EntitySystem::CreatePlayer(World, &Input, Math::Vec3(Math::CartesianToIso(Math::Vec2(-level.GetWidth()/2, -level.GetHeight()/2)), 0.0f), Math::Vec2(100.0f, 100.0f), &PlayerSheet, "Player", 0.4f, Math::Vec3(1, 1, 0)); 

	// Create Sword
	eid32 Sword = EntitySystem::CreateItem(World, World->TransformSystem->Transforms[Player].Position, Enjon::Math::Vec2(32.0f, 32.0f), &ItemSheet, 
												(Masks::Type::WEAPON | Masks::GeneralOptions::EQUIPPED), Component::EntityType::WEAPON, "Weapon");

	// Turn off Rendering / Transform Components
	EntitySystem::RemoveComponents(World, Sword, COMPONENT_RENDERER2D | COMPONENT_TRANSFORM3D);

	// Equip player with sword
	World->InventorySystem->Inventories[Player].Items.push_back(Sword);
	World->InventorySystem->Inventories[Player].WeaponEquipped = Sword;

	// Set position to player
	Camera.SetPosition(Math::Vec2(World->TransformSystem->Transforms[Player].Position.x + 100.0f / 2.0f, World->TransformSystem->Transforms[Player].Position.y)); 
	
	while(isRunning)
	{ 
		static float time = 0.0f;
		time+=0.025f;

		/////////////
		// Updates // 
		///////////// 

		// Begin FPSLimiter
		Limiter.Begin();

		// Update Camera
		Camera.Update();
		
		// Update HUDCamera
		HUDCamera.Update();

		// Update Input Manager
		Input.Update();	

		// Update World 
		const Math::Vec2* PlayerStuff = &World->TransformSystem->Transforms[Player].Position.XY();
		static Math::Vec2 quadDimsStuff(50.0f, 50.0f);
		static Math::Vec2 ViewPort;
		static Math::Vec4 CameraDims;
		ViewPort = Math::Vec2(SCREENWIDTH, SCREENHEIGHT) / Camera.GetScale();
		CameraDims = Math::Vec4(*PlayerStuff, quadDimsStuff / Camera.GetScale());

		PlayerController::Update(World->PlayerControllerSystem);
		SpatialHash::ClearCells(World->Grid);
		AIController::Update(World->AIControllerSystem, Player);
		Animation2D::Update(World);
		TransformSystem::Update(World->TransformSystem);
		Collision::Update(World);
		Renderer2D::Update(World); 

		// Check for input
		ProcessInput(&Input, &Camera, World, Player); 

		//LERP camera to center of player position
		static Math::Vec2 m_velocity;
		static float scale = 6.0f; 
		m_velocity.x = Enjon::Math::Lerp(World->TransformSystem->Transforms[Player].Position.x + 100.0f / 2.0f, Camera.GetPosition().x, 8.0f);
		m_velocity.y = Enjon::Math::Lerp(World->TransformSystem->Transforms[Player].Position.y, Camera.GetPosition().y, scale); 
		Camera.SetPosition(m_velocity); 
		////////////////////////////////////////////////

		///////////////
		// RENDERING //
		///////////////
		
		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT);

		static int isLevel;

		// Set up shader
		Math::Mat4 model, view, projection;
		model = Math::Mat4::Identity();
		view = Camera.GetCameraMatrix();
		projection = Math::Mat4::Identity();

		GLint shader = Graphics::ShaderManager::GetShader("Basic")->GetProgramID();
		isLevel = 1;
		glUseProgram(shader);
		{
			glUniform1i(glGetUniformLocation(shader, "tex"),
						 0);
			glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
								1, 0, model.elements);
			glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
								1, 0, view.elements);
			glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
								1, 0, projection.elements);
			glUniform1f(glGetUniformLocation(shader, "time"), time);
			glUniform1i(glGetUniformLocation(shader, "isLevel"), isLevel);
		} 
		
		// Draw map
		TileBatch.RenderBatch(); 


		// Draw Entities
		EnemyBatch.Begin();
		PlayerBatch.Begin(); 

		wchar_t wcstring[10]; 
		glUniform1i(glGetUniformLocation(shader, "isLevel"), 0);
		static uint32 Row = 0;
		static uint32 Col = 0;
		static uint32 i = 0;
		static Math::Vec2 dims(100.0f, 100.0f);
		static Math::Vec2 arrowDims(64.0f, 64.0f);
		static Math::Vec2 itemDims(20.0f, 20.0f);
		static Math::Vec4 uv(0, 0, 1, 1);
		static Math::Vec2 pos(-1000, -1000);
		static Graphics::GLTexture beast = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast.png"); 
		static Graphics::GLTexture playertexture = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtest.png"); 
		static Graphics::GLTexture groundtiletexture = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledblue.png"); 
		static Graphics::GLTexture orb = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"); 
		static Graphics::SpriteSheet ArrowSheet;
		if (!ArrowSheet.IsInit()) ArrowSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"), Enjon::Math::iVec2(8, 1)); 
	
		// Draw enemies
		for (eid32 e = 0; e < World->MaxAvailableID; e++)
		{
			if (e == Player) continue;

			// Don't draw if the entity doesn't exist anymore
			bitmask32 Mask = World->Masks[e];
			if ((Mask & COMPONENT_RENDERER2D) != COMPONENT_RENDERER2D) continue;

			Math::Vec2* EntityPosition; 
			Math::Vec2* Ground;
			char buffer[25];
			
			EntityPosition = &World->TransformSystem->Transforms[e].Position.XY();
			const Enjon::Graphics::ColorRGBA8* Color = &World->Renderer2DSystem->Renderers[e].Color;

			// If AI
			if (Mask & COMPONENT_AICONTROLLER)
			{
				// Don't draw if not in view
				if (Camera.IsBoundBoxInCamView(*EntityPosition, enemydims))
				{
					EnemyBatch.Add(Math::Vec4(*EntityPosition, enemydims), uv, beast.id, *Color);
				}
			}
			else if (World->Types[e] == ECS::Component::EntityType::ITEM)
			{
				if (Camera.IsBoundBoxInCamView(*EntityPosition, itemDims))
				{
					PlayerBatch.Add(Math::Vec4(*EntityPosition, itemDims), ItemSheet.GetUV(0), ItemSheet.texture.id, *Color);
				}

			}
			// If item
			else
			{
				// Don't draw if not in view
				if (Camera.IsBoundBoxInCamView(*EntityPosition, itemDims))
				{
					static int index;

					// Get attack vector and draw arrow based on that
					Enjon::Math::Vec2* AttackVector = &World->TransformSystem->Transforms[Player].AttackVector;
					if		(*AttackVector == EAST)				index = 0;
					else if (*AttackVector == NORTHEAST)		index = 1;
					else if (*AttackVector == NORTH)			index = 2;
					else if (*AttackVector == NORTHWEST)		index = 3;
					else if (*AttackVector == WEST)				index = 4;
					else if (*AttackVector == SOUTHWEST)		index = 5;
					else if (*AttackVector == SOUTH)			index = 6;
					else if (*AttackVector == SOUTHEAST)		index = 7;
					else										index = 0;

					PlayerBatch.Add(Math::Vec4(*EntityPosition, arrowDims), ArrowSheet.GetUV(index), ArrowSheet.texture.id, *Color);
				}
			}

			Ground = &World->TransformSystem->Transforms[e].GroundPosition;
			if (Camera.IsBoundBoxInCamView(*Ground, Math::Vec2(64.0f, 32.0f)))
			{
				PlayerBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
										Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.2f));
			}

			// Draw ground tile on cartesian map			
			PlayerBatch.Add(Math::Vec4(World->TransformSystem->Transforms[e].CartesianPosition, Math::Vec2(32.0f, 32.0f)), Math::Vec4(0, 0, 1, 1), 0, 
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.3f));
		}
	
		// Draw player
		static Graphics::SpriteSheet* Sheet = World->Animation2DSystem->Animations[Player].Sheet; static Enjon::uint32 Frame = World->Animation2DSystem->Animations[Player].CurrentFrame;
		Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
		const Enjon::Graphics::ColorRGBA8* Color = &World->Renderer2DSystem->Renderers[Player].Color;
		PlayerBatch.Add(Math::Vec4(World->TransformSystem->Transforms[Player].Position.XY(), dims), Sheet->GetUV(Frame), Sheet->texture.id, *Color);

		static float AABBHeight = 32.0f, AABBWidth = 64.0f;
		Enjon::Math::Vec2* A = &World->TransformSystem->Transforms[Player].CartesianPosition;
		Enjon::Physics::AABB* AABB = &World->TransformSystem->Transforms[Sword].AABB;
		PlayerBatch.Add(Math::Vec4(AABB->Min.x, AABB->Min.y, abs(AABB->Max.x - AABB->Min.x), abs(AABB->Max.y - AABB->Min.y)), Math::Vec4(0, 0, 1, 1), 0,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.2f));

		// Draw player ground tile 
		const Math::Vec2* GroundPosition = &World->TransformSystem->Transforms[Player].GroundPosition;
		PlayerBatch.Add(Math::Vec4(GroundPosition->x, GroundPosition->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.2f));
	
		// Draw player cartesian position
		PlayerBatch.Add(Math::Vec4(World->TransformSystem->Transforms[Player].CartesianPosition, Math::Vec2(32.0f, 32.0f)), Math::Vec4(0, 0, 1, 1), 0, 
								Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.3f));

		// Draw Cartesian Level
		// NOTE(John): This is a HUGE bottleneck for some reason. Figure it out.
		//level.DrawCartesianLevel(PlayerBatch);
	
		EnemyBatch.End();
		PlayerBatch.End(); 
	
		// Set up shader for rendering
		isLevel = 1;
		shader = Graphics::ShaderManager::GetShader("Basic")->GetProgramID();
		glUseProgram(shader);
		{
			glUniform1i(glGetUniformLocation(shader, "tex"),
						 0);
			glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
								1, 0, model.elements);
			glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
								1, 0, view.elements);
			glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
								1, 0, projection.elements);
			glUniform1f(glGetUniformLocation(shader, "time"), time);
			glUniform1i(glGetUniformLocation(shader, "isLevel"), isLevel);
		} 
		
		PlayerBatch.RenderBatch();
		EnemyBatch.RenderBatch();

		Window.SwapBuffer();
		
		////////////////////////////////////////////////

		// End FPSLimiter
		FPS = Limiter.End();

		// static float counter = 0.0f;
		// counter += 0.1f;
		// if (counter > 1.0f)
		// {
		// 	printf("Player id: %d\n", Player);
		// 	counter = 0.0f;
		// }
	} 

	return 0;
}

void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, struct EntityManager* World, ECS::eid32 Entity)
{
    SDL_Event event;
//
//    //Will keep looping until there are no more events to process
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
			case SDL_KEYUP:
				Input->ReleaseKey(event.key.keysym.sym); 
				break;
			case SDL_KEYDOWN:
				Input->PressKey(event.key.keysym.sym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				Input->PressKey(event.button.button);
				break;
			case SDL_MOUSEBUTTONUP:
				Input->ReleaseKey(event.button.button);
				break;
			case SDL_MOUSEMOTION:
				Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
				break;
			default:
				break;
		}
    } 
	
	if (Input->IsKeyPressed(SDLK_ESCAPE))
	{
		isRunning = false;	
	}
	if (Input->IsKeyDown(SDLK_q)){
		if (Camera->GetScale() > 0.1f) Camera->SetScale(Camera->GetScale() - 0.01f);	
	}
	if (Input->IsKeyDown(SDLK_e)){
		Camera->SetScale(Camera->GetScale() + 0.01f);	
	}
}


#endif 


#if TESTING_PARSER

#include <stdio.h>
#include <Graphics/SpriteSheet.h>
#include <IO/ResourceManager.h>
#include <Math/Maths.h>
#include <Enjon.h>
#include <Graphics/Window.h>
#include <Graphics/SpriteBatch.h>
#include <Graphics/Camera2D.h>
#include <Graphics/ShaderManager.h>

#include "Animation.h"

#undef main
int main(int argc, char** argv)
{
	#define WINDOWRUNNING 1
		
	struct AnimationProfile* player_walk = CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "walk"); 
	struct AnimationProfile* player_attack_dagger = CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_dagger"); 
	struct AnimationProfile* player_attack_bow = CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_bow"); 

	struct Animation Player_Walk = CreateAnimation(player_walk, 0.1f, Orientation::NE);
	struct Animation Player_Attack_Dagger = CreateAnimation(player_attack_dagger, 0.075f); 
	struct Animation Player_Attack_Bow = CreateAnimation(player_attack_bow, 0.075f); 


#if WINDOWRUNNING
	Enjon::Init();
	Enjon::Graphics::Window Window;
	Window.Init("Test", 800, 600); 

	Enjon::Graphics::Camera2D Camera;
	Camera.Init(800, 600);

	Enjon::Utils::FPSLimiter Limiter;
	Limiter.Init(60.0f);

	Enjon::Graphics::SpriteBatch Batch;
	Enjon::Graphics::SpriteSheet Sheet; 

	Batch.Init();
	Sheet.Init(Enjon::Input::ResourceManager::GetTexture(player_walk->FilePath), Enjon::Math::iVec2(6, 24)); 

	Enjon::Graphics::ShaderManager::Init();


	// Game loop
	while (true)
	{ 
		Limiter.Begin();

		static struct Animation* CurrentAnimation;
		static float increment = 0.1f;
		static float size = 200.0f;

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT); 
		Camera.Update(); 

		Enjon::Math::Mat4 model, view, projection;
		model = Enjon::Math::Mat4::Identity();
		view = Camera.GetCameraMatrix();
		projection = Enjon::Math::Mat4::Identity();

		GLint shader = Enjon::Graphics::ShaderManager::GetShader("Basic")->GetProgramID();
		glUseProgram(shader);
		{
			glUniform1i(glGetUniformLocation(shader, "tex"),
						 0);
			glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
								1, 0, model.elements);
			glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
								1, 0, view.elements);
			glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
								1, 0, projection.elements);
			glUniform1i(glGetUniformLocation(shader, "isLevel"), 1);
		} 
	
		// Begin batch
		Batch.Begin();

		////////// 
		// WALK //
		//////////
		CurrentAnimation = &Player_Walk; 

		CurrentAnimation->AnimationTimer += CurrentAnimation->AnimationTimerIncrement;
		if (CurrentAnimation->AnimationTimer >= CurrentAnimation->Profile->Delays[CurrentAnimation->CurrentFrame % CurrentAnimation->Profile->FrameCount])
		{
			CurrentAnimation->CurrentFrame++;
			CurrentAnimation->AnimationTimer = 0.0f;
			if (CurrentAnimation->CurrentFrame >= CurrentAnimation->BeginningFrame + CurrentAnimation->Profile->FrameCount) 
					CurrentAnimation->CurrentFrame -= CurrentAnimation->Profile->FrameCount;
		} 
	
		// Draw Walk
		Batch.Add(Enjon::Math::Vec4(-50, 0, size, size), Sheet.GetUV(CurrentAnimation->CurrentFrame), Sheet.texture.id);
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
		/////////////////// 
		// ATTACK DAGGER //
		///////////////////
		CurrentAnimation = &Player_Attack_Dagger; 
		
		CurrentAnimation->AnimationTimer += CurrentAnimation->AnimationTimerIncrement;
		if (CurrentAnimation->AnimationTimer >= CurrentAnimation->Profile->Delays[CurrentAnimation->CurrentFrame % CurrentAnimation->Profile->FrameCount])
		{
			CurrentAnimation->CurrentFrame++;
			CurrentAnimation->AnimationTimer = 0.0f;
			if (CurrentAnimation->CurrentFrame >= CurrentAnimation->BeginningFrame + CurrentAnimation->Profile->FrameCount) CurrentAnimation->CurrentFrame -= CurrentAnimation->Profile->FrameCount; 
		}
		
		// Draw Attack Dagger
		Batch.Add(Enjon::Math::Vec4(-300, 0, size, size), Sheet.GetUV(CurrentAnimation->CurrentFrame), Sheet.texture.id);
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		//////////////// 
		// ATTACK BOW //
		////////////////
		CurrentAnimation = &Player_Attack_Bow; 
		
		CurrentAnimation->AnimationTimer += CurrentAnimation->AnimationTimerIncrement;
		if (CurrentAnimation->AnimationTimer >= CurrentAnimation->Profile->Delays[CurrentAnimation->CurrentFrame % CurrentAnimation->Profile->FrameCount])
		{
			CurrentAnimation->CurrentFrame++;
			CurrentAnimation->AnimationTimer = 0.0f;
			if (CurrentAnimation->CurrentFrame >= CurrentAnimation->BeginningFrame + CurrentAnimation->Profile->FrameCount) CurrentAnimation->CurrentFrame -= CurrentAnimation->Profile->FrameCount; 
		}
		
		// Draw Attack Bow
		Batch.Add(Enjon::Math::Vec4(200, 0, size, size), Sheet.GetUV(CurrentAnimation->CurrentFrame), Sheet.texture.id);

		// End and render
		Batch.End();
		Batch.RenderBatch();

		Limiter.End();
		Window.SwapBuffer();
	}

	#endif 

	// Delete pointers in animation
	// free(player_walk.Delays);
	// free(player_walk.FilePath);

	char ch;
	scanf("%c", &ch);

	return 0;
} 

#endif

#if FUNCTION_TESTS

#include "SpatialHash.h"
#include <Level.h>

int main(int argc, char** argv) {

	// Need to make sure that the level size is normalized from (0, 0) to (width, height)
	
	SpatialHash::Grid grid;

	SpatialHash::Init(&grid, 10, 10);

	ECS::eid32 id = 5;
	V2 position = V2(127, 127); 
	SpatialHash::FindCell(&grid, id, &position);
	position = V2(220, 0); 
	SpatialHash::FindCell(&grid, id, &position);

	while(true);

	return 0;
}

#endif









