#define PARSER  0
#define TEST_FUNCTIONS 0

#define _SECURE_SCL 0

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

/**
* MAIN GAME
*/

#if 1
#define FULLSCREENMODE   1
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

/*-- External/Engine Libraries includes --*/
#include <Enjon.h>

/*-- Entity Component System includes --*/
#include <ECS/ComponentSystems.h>
#include <ECS/PlayerControllerSystem.h>
#include <ECS/Transform3DSystem.h>
#include <ECS/CollisionSystem.h>
#include <ECS/Animation2DSystem.h>
#include <ECS/InventorySystem.h> 
#include <ECS/Renderer2DSystem.h>
#include <ECS/AIControllerSystem.h> 
#include <ECS/AttributeSystem.h>
#include <ECS/EffectSystem.h>
#include <ECS/EntityFactory.h>
#include <ECS/Entity.h>
#include <Loot.h>

/*-- IsoARPG includes --*/
#include "Animation.h"
#include "AnimationManager.h"
#include "SpatialHash.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <stdio.h>
#include <iostream> 
#include <time.h>
#include <stdlib.h>

char buffer[256];
char buffer2[256];
char buffer3[256];
bool isRunning = true;
bool ShowMap = false;
bool Paused = false;
bool IsDashing = false;

const int LEVELSIZE = 100;

float DashingCounter = 0.0f;

Enjon::uint32 CollisionRunTime = 0;
Enjon::uint32 TransformRunTime = 0;
Enjon::uint32 ClearEntitiesRunTime = 0;
Enjon::uint32 RenderTime = 0;
Enjon::uint32 EffectRunTime = 0;
Enjon::uint32 ParticleCount = 0;
Enjon::uint32 TileOverlayRunTime = 0;

using namespace Enjon;
using namespace ECS;
using namespace Systems;

/*-- Function Declarations --*/
void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, struct EntityManager* Manager, ECS::eid32 Entity);
void DrawCursor(Enjon::Graphics::SpriteBatch* Batch, Enjon::Input::InputManager* InputManager);
void DrawSmoke(Enjon::Graphics::Particle2D::ParticleBatch2D* Batch, Enjon::Math::Vec3 Pos);

SDL_Joystick* Joystick;

#undef main
int main(int argc, char** argv)
{
	// Seed random 
	srand(time(NULL));

	float FPS = 60;
	int screenWidth = SCREENWIDTH, screenHeight = SCREENHEIGHT;

	// Profile strings
	std::string FPSString = "60.0";
	std::string RenderTimeString = "0";
	std::string CollisionTimeString = "0";
	std::string TransformTimeString = "0";
	std::string EffectTimeString = "0";
	std::string TileOverlayTimeString = "0";

	// Init Limiter
	Enjon::Utils::FPSLimiter Limiter; 
	Limiter.Init(70.0f); 

	//Init Enjon
	Enjon::Init();

	// Query for available controllers
	printf("%d joysticks were found. \n\n", SDL_NumJoysticks());

	Joystick = SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);

	// Create a window
	Graphics::Window Window;
	Window.Init("Testing Grounds", screenWidth, screenHeight, SCREENRES);

	// Hide/Show mouse
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::SHOW);

	// Create Camera
	Graphics::Camera2D Camera;
	Camera.Init(screenWidth, screenHeight);
	Camera.SetScale(1.0f); 
	
	// Create HUDCamera
	Graphics::Camera2D HUDCamera;
	HUDCamera.Init(screenWidth, screenHeight);
	HUDCamera.SetScale(1.0f);

	// Init SpriteSheetManager
	EG::SpriteSheetManager::Init();

	// Init AnimationManager
	AnimationManager::Init(); 

	// Init FontManager
	Enjon::Graphics::FontManager::Init();
	
	// Init level
	Enjon::Graphics::SpriteBatch TileBatch;
	TileBatch.Init();

	Enjon::Graphics::SpriteBatch GroundTileBatch;
	GroundTileBatch.Init();

	Enjon::Graphics::SpriteBatch CartesianTileBatch;
	CartesianTileBatch.Init();

	Enjon::Graphics::SpriteBatch CartesianEntityBatch;
	CartesianEntityBatch.Init();

	Enjon::Graphics::SpriteBatch FrontWallBatch;
	FrontWallBatch.Init();

	Enjon::Graphics::SpriteBatch MapBatch;
	MapBatch.Init();

	Enjon::Graphics::SpriteBatch MapEntityBatch;
	MapEntityBatch.Init();

	Enjon::Graphics::SpriteBatch TextBatch;
	TextBatch.Init();

	Enjon::Graphics::SpriteBatch HUDBatch;
	HUDBatch.Init();

	Enjon::Graphics::SpriteBatch OverlayBatch;
	OverlayBatch.Init();

	Level level;
	Graphics::GLTexture TileTexture;
	level.Init(Camera.GetPosition().x, Camera.GetPosition().y, LEVELSIZE, LEVELSIZE);
	TileTexture = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledblue.png");
	
	float x = Camera.GetPosition().x;
	float y = Camera.GetPosition().y;

	// Spatial Hash
	SpatialHash::Grid grid;
	int width = level.GetWidth();
	SpatialHash::Init(&grid, width, width);

	// Make spritesheets for player and enemy and item
	EG::SpriteSheet PlayerSheet;
	EG::SpriteSheet EnemySheet;
	EG::SpriteSheet EnemySheet2;
	EG::SpriteSheet ItemSheet;
	EG::SpriteSheet ArrowSheet;
	EG::SpriteSheet ReticleSheet;
	EG::SpriteSheet TargetSheet;
	EG::SpriteSheet HealthSheet;
	PlayerSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtestframessplit.png"), Enjon::Math::iVec2(6, 24));
	EnemySheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast.png"), Math::iVec2(1, 1));
	EnemySheet2.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/enemy.png"), Math::iVec2(1, 1));
	ItemSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Math::iVec2(1, 1));
	ArrowSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"), Math::iVec2(8, 1));
	ReticleSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/circle_reticle.png"), Math::iVec2(1, 1));
	TargetSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Target.png"), Math::iVec2(1, 1));
	HealthSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBar.png"), EM::iVec2(1, 1));

	
	// Creating tiled iso level
	TileBatch.Begin(); 
	level.DrawIsoLevel(TileBatch);
	TileBatch.End();	

	GroundTileBatch.Begin();
	level.DrawGroundTiles(GroundTileBatch);
	GroundTileBatch.End();

	CartesianTileBatch.Begin();
	level.DrawCartesianLevel(CartesianTileBatch);
	CartesianTileBatch.End();

	FrontWallBatch.Begin();
	level.DrawIsoLevelFront(FrontWallBatch);
	FrontWallBatch.End();

	MapBatch.Begin();
	level.DrawMap(MapBatch);
	MapBatch.End();

	// Add Overlays 
	OverlayBatch.Begin();
	level.DrawTileOverlays(OverlayBatch);
	OverlayBatch.End();
 
	// Create EntityBatch
	Enjon::Graphics::SpriteBatch EntityBatch;
	EntityBatch.Init();
	
	// Create Particle Batch
	Enjon::Graphics::SpriteBatch ParticleBatch;
	ParticleBatch.Init();

	// Create particle batchs to be used by World
	EG::Particle2D::ParticleBatch2D* TestParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* TextParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* SmokeBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);

	// Create InputManager
	Input::InputManager Input;

	// Init ShaderManager
	Enjon::Graphics::ShaderManager::Init(); 

	/////////////////
	// Testing ECS //   
	/////////////////

	// Create new EntityManager
	struct EntityManager* World = EntitySystem::NewEntityManager(level.GetWidth(), level.GetWidth(), &Camera, &level);

	// Init loot system
	Loot::Init();

	// Push back particle batchs into world
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, TestParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, TextParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, SmokeBatch);


	Math::Vec2 Pos = Camera.GetPosition() + 50.0f;

	static Math::Vec2 enemydims(222.0f, 200.0f);

	static uint32 AmountDrawn = 100;
	for (int e = 0; e < AmountDrawn; e++)
	{
		float height = 10.0f;
		eid32 ai = Factory::CreateAI(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), height),
																enemydims, &EnemySheet, "Enemy", 0.05f); 
		World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
	}

	// Create player
	eid32 Player = Factory::CreatePlayer(World, &Input, Math::Vec3(Math::CartesianToIso(Math::Vec2(-level.GetWidth()/2, -level.GetHeight()/2)), 0.0f), Math::Vec2(100.0f, 100.0f), &PlayerSheet, 
		"Player", 0.4f, Math::Vec3(1, 1, 0)); 

	// Set player for world
	World->Player = Player;

	// Create Sword
	eid32 Sword = Factory::CreateWeapon(World, World->TransformSystem->Transforms[Player].Position, Enjon::Math::Vec2(32.0f, 32.0f), &ItemSheet, 
												(Masks::Type::WEAPON | Masks::GeneralOptions::EQUIPPED | Masks::GeneralOptions::PICKED_UP), Component::EntityType::WEAPON, "Weapon");

	// Create Bow
	eid32 Bow = Factory::CreateWeapon(World, World->TransformSystem->Transforms[Player].Position, Enjon::Math::Vec2(32.0f, 32.0f), &ItemSheet, 
												(Masks::Type::WEAPON | Masks::GeneralOptions::PICKED_UP), Component::EntityType::WEAPON, "Weapon");

	// Turn off Rendering / Transform Components
	EntitySystem::RemoveComponents(World, Sword, COMPONENT_RENDERER2D | COMPONENT_TRANSFORM3D);

	// Add weapons to player inventory
	World->InventorySystem->Inventories[Player].Items.push_back(Sword);
	World->InventorySystem->Inventories[Player].Items.push_back(Bow);
	// Equip sword
	World->InventorySystem->Inventories[Player].WeaponEquipped = Sword;

	AmountDrawn = 0;

	for (int e = 0; e < AmountDrawn; e++)
	{
		// Create Sword
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(32.0f, 32.0f), &ItemSheet, Masks::Type::ITEM, Component::EntityType::ITEM, "Weapon");
	}

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
		static float StartTicks = 0.0f;
		ViewPort = Math::Vec2(SCREENWIDTH, SCREENHEIGHT) / Camera.GetScale();
		CameraDims = Math::Vec4(*PlayerStuff, quadDimsStuff / Camera.GetScale());

		if (!Paused)
		{
			StartTicks = SDL_GetTicks();
			// Check whether or not overlays are dirty and then reset overlay batch if needed
			if (World->Lvl->GetOverlaysDirty())
			{
				OverlayBatch.Begin();
				World->Lvl->DrawTileOverlays(OverlayBatch);
				OverlayBatch.End();
			}

			// Draw some random assed fire
			// EG::Particle2D::DrawFire(TestParticleBatch, EM::Vec3(0.0f, 0.0f, 0.0f));
	
			TileOverlayRunTime = SDL_GetTicks() - StartTicks;		
			StartTicks = SDL_GetTicks();
			SpatialHash::ClearCells(World->Grid);
			ClearEntitiesRunTime = (SDL_GetTicks() - StartTicks); // NOTE(John): As the levels increase, THIS becomes the true bottleneck

			AIController::Update(World->AIControllerSystem, Player);
			Animation2D::Update(World);

			StartTicks = SDL_GetTicks();
			Transform::Update(World->TransformSystem, TestParticleBatch);
			TransformRunTime = (SDL_GetTicks() - StartTicks);

			StartTicks = SDL_GetTicks();	
			Collision::Update(World);
			CollisionRunTime = (SDL_GetTicks() - StartTicks);

			// Apply effects
			StartTicks = SDL_GetTicks();	
			Effect::Update(World);
			EffectRunTime = (SDL_GetTicks() - StartTicks);

			Renderer2D::Update(World); 

			auto LvlSize = level.GetDims();
			EM::IsoToCartesian(LvlSize);
			static float SmokeCount = 0.0f;
			SmokeCount += 0.005f;
			Enjon::uint32 SR = Enjon::Random::Roll(0, 3);
			if (SmokeCount > 1.0f)
			{
				for (Enjon::uint32 i = 0; i < 10; i++)
				{
					DrawSmoke(SmokeBatch, EM::Vec3(Enjon::Random::Roll(-LvlSize.x, LvlSize.x), Enjon::Random::Roll(-LvlSize.y * 2.0f, LvlSize.y * 2.0f), 0.0f));
				}
				SmokeCount = 0.0f;
			}

			// Updates the world's particle engine
			World->ParticleEngine->Update();
		
			PlayerController::Update(World->PlayerControllerSystem);
	
			// Clear entities from collision system vectors
			World->CollisionSystem->Entities.clear();

			// Clear entities from PlayerControllerSystem targets vector
			World->PlayerControllerSystem->Targets.clear();
		}


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

		StartTicks = SDL_GetTicks();
		
		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, Enjon::Graphics::RGBA16(0.05f, 0.05f, 0.05f, 1.0f));

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
			glUniform1i(glGetUniformLocation(shader, "isLevel"), true);
			glUniform1i(glGetUniformLocation(shader, "isPaused"), Paused);
			glUniform1i(glGetUniformLocation(shader, "useOverlay"), true);
		} 

		// Draw ground tiles
		GroundTileBatch.RenderBatch();

		// Draw TileOverlays
		OverlayBatch.RenderBatch();

		// Draw map
		// TileBatch.RenderBatch();

		// Draw Entities
		EntityBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT); 
		MapEntityBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT); 
		TextBatch.Begin(); 
		HUDBatch.Begin();

		wchar_t wcstring[10]; 
		glUniform1i(glGetUniformLocation(shader, "isLevel"), false);
		static uint32 Row = 0;
		static uint32 Col = 0;
		static uint32 i = 0;
		static Math::Vec2 enemydims2(180.0f, 222.0f);
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

		EM::Vec2 PC = World->TransformSystem->Transforms[Player].Position.XY();
	
		// Draw enemies
		for (eid32 e = 0; e < World->MaxAvailableID; e++)
		{
			if (e == Player || e == Sword || e == Bow) continue;

			// Don't draw if the entity doesn't exist anymore
			bitmask32 Mask = World->Masks[e];
			if ((Mask & COMPONENT_RENDERER2D) != COMPONENT_RENDERER2D) continue;

			Math::Vec2* EntityPosition; 
			Math::Vec2* Ground;
			char buffer[25];
			
			EntityPosition = &World->TransformSystem->Transforms[e].Position.XY();
			Ground = &World->TransformSystem->Transforms[e].GroundPosition;
			const Enjon::Graphics::ColorRGBA16* Color = &World->Renderer2DSystem->Renderers[e].Color;

			// If AI
			if (Mask & COMPONENT_AICONTROLLER)
			{
				// Don't draw if not in view
				if (Camera.IsBoundBoxInCamView(*EntityPosition, enemydims))
				{
					EntityBatch.Add(Math::Vec4(*EntityPosition, enemydims), uv, beast.id, *Color, EntityPosition->y - World->TransformSystem->Transforms[e].Position.z);
					Graphics::Fonts::PrintText(EntityPosition->x + 100.0f, EntityPosition->y + 220.0f, 0.4f, std::to_string(e), Graphics::FontManager::GetFont(std::string("Bold")), TextBatch, 
															Graphics::SetOpacity(Graphics::RGBA16_Green(), 0.8f));
				}
				// If target
				if (e == World->PlayerControllerSystem->CurrentTarget)
				{
					Math::Vec2 ReticleDims(94.0f, 47.0f);
					Math::Vec2 Position = World->TransformSystem->Transforms[e].GroundPosition - Math::Vec2(15.0f, 5.0f);
					EntityBatch.Add(Math::Vec4(Position.x, Position.y, ReticleDims), Enjon::Math::Vec4(0, 0, 1, 1), TargetSheet.texture.id, Enjon::Graphics::RGBA16_Red(), Position.y);
				}

			}
			else if (World->Types[e] == ECS::Component::EntityType::ITEM)
			{
				if (Camera.IsBoundBoxInCamView(*EntityPosition, itemDims))
				{
					EntityBatch.Add(Math::Vec4(*EntityPosition, itemDims), ItemSheet.GetUV(0), ItemSheet.texture.id, *Color, EntityPosition->y);
				}

			}
			else
			{
				// Don't draw if not in view
				if (Camera.IsBoundBoxInCamView(*EntityPosition, itemDims))
				{
					auto Sheet = World->Animation2DSystem->Animations[e].Sheet;
					auto Dims = World->TransformSystem->Transforms[e].Dimensions;
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

					EntityBatch.Add(Math::Vec4(*EntityPosition, Dims), Sheet->GetUV(index), Sheet->texture.id, *Color, EntityPosition->y);
				}
			}

			Ground = &World->TransformSystem->Transforms[e].GroundPosition;
			auto EAABB = &World->TransformSystem->Transforms[e].AABB;
			if (World->Types[e] != ECS::Component::EntityType::ITEM && Camera.IsBoundBoxInCamView(*Ground, Math::Vec2(64.0f, 32.0f)))
			{
				EntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
										Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f), 1.0f);
				// MapEntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
				// 						Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.7f), 1.0f);
	
				float XDiff = World->TransformSystem->Transforms[e].AABBPadding.x;
				Enjon::Math::Vec2 EAABBIsoMin(Enjon::Math::CartesianToIso(EAABB->Min) + Math::Vec2(TILE_SIZE + XDiff / 2.0f, XDiff + TILE_SIZE / 2.0f));
				float EAABBHeight = EAABB->Max.y - EAABB->Min.y, EAABBWidth = EAABB->Max.x - EAABB->Min.y;
				EntityBatch.Add(Math::Vec4(EAABBIsoMin, Math::Vec2(abs(EAABB->Max.x - EAABB->Min.x), abs(EAABB->Max.y - EAABB->Min.y))), 
									Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
									Graphics::SetOpacity(Graphics::RGBA16_Red(), 0.2f), EAABBIsoMin.y, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);
			}
		}

		// Draw player

		// Dashing state if dashing
		Graphics::SpriteSheet* Sheet = World->Animation2DSystem->Animations[Player].Sheet; 
		Enjon::uint32 Frame = World->Animation2DSystem->Animations[Player].CurrentFrame;

		auto Vel = &World->TransformSystem->Transforms[Player].Velocity;
		if (IsDashing && !(Vel->x == 0 && Vel->y == 0))
		{
			// Make unable to collide with enemy
			World->AttributeSystem->Masks[Player] &= ~Masks::GeneralOptions::COLLIDABLE;

			float DashAmount = 10.0f;
			auto Pos = &World->TransformSystem->Transforms[Player].Position;
			auto V = EM::Vec2::Normalize(Vel->XY());
			V *= 5.0f;
			Pos->x += V.x;
			Pos->y += V.y;

			// Setting the "alarm"
			DashingCounter += 0.05f;
			if (DashingCounter >= 1.0f) { IsDashing = false; DashingCounter = 0.0f; }
			float Opacity = 0.5f;
			for (int i = 0; i < 5; i++)
			{
				Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
				Enjon::Graphics::ColorRGBA16 DashColor = World->Renderer2DSystem->Renderers[Player].Color;
				// DashColor.r += (i + i*2.9f);
				DashColor.g += (i + i*20.75f);
				DashColor.b += (i*5.25f);
				Enjon::Math::Vec2 PP = World->TransformSystem->Transforms[Player].Position.XY();
				Enjon::Math::Vec2 PV = World->TransformSystem->Transforms[Player].Velocity.XY();
				PP.x -= (i + i*2.75f) * PV.x;
				PP.y -= (i + i*2.75f) * PV.y;
				EntityBatch.Add(Math::Vec4(PP, dims), Sheet->GetUV(Frame - i), Sheet->texture.id, Graphics::SetOpacity(DashColor, Opacity), PP.y - World->TransformSystem->Transforms[Player].Position.z);
				Opacity -= 0.05f;

				// Add a particle pos, vel, size, color, texture, decay, batch
				EG::Particle2D::AddParticle(EM::Vec3(PP + EM::Vec2(40.0f, 50.0f), 0.0f), EM::Vec3(PV * -1.0f, 0.0f), EM::Vec2(2.0f, 2.0f), DashColor, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png").id, 0.05f, World->ParticleEngine->ParticleBatches[0]);
				// Add a particle pos, vel, size, color, texture, decay, batch
				EG::Particle2D::AddParticle(EM::Vec3(PP + EM::Vec2(40.0f, 30.0f), 0.0f), EM::Vec3(PV * -1.0f, 0.0f), EM::Vec2(Random::Roll(25, 100), Random::Roll(25, 100)), 
					EG::SetOpacity(DashColor, 0.025f), EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/smoke_1.png").id, 0.025f, World->ParticleEngine->ParticleBatches[0]);
			}
		}

		else 
		{
			static float slide_counter = 0.0f;
			slide_counter += 0.05f;
			if (slide_counter >= 1.0f)
			{
				World->TransformSystem->Transforms[Player].VelocityGoalScale = 0.3f;
				slide_counter = 0.0f;
				// Make able to collide with enemy again
				World->AttributeSystem->Masks[Player] |= Masks::GeneralOptions::COLLIDABLE;
			}

		}
		// Get angle from mouse to coordinates
		Math::Vec2 MouseCoords = Input.GetMouseCoords();
		Camera.ConvertScreenToWorld(MouseCoords);
		static Math::Vec2 right(1.0f, 0.0f);
		Math::Vec2 Diff = Math::Vec2::Normalize(World->TransformSystem->Transforms[Player].Position.XY() - MouseCoords);
		float DotProduct = Diff.DotProduct(right);
		float AimAngle = acos(DotProduct) * 180.0f / M_PI;
		if (Diff.y < 0.0f) AimAngle *= -1; 
		// printf("Aim Angle: %.2f\n", AimAngle);


		// Draw player "spell" box for testing purposes
		// Get the box coords in cartesian view, then translate to iso coords
		static float rotation_count = 0.0f;
		rotation_count += 1.25f;
		Enjon::Math::Vec2 BoxCoords(Math::CartesianToIso(World->TransformSystem->Transforms[Player].CartesianPosition) + Math::Vec2(5.0f, 0.0f));
		float boxRadius = 50.0f;
		BoxCoords = BoxCoords - boxRadius * Math::CartesianToIso(Math::Vec2(cos(Math::ToRadians(AimAngle - 90)), sin(Math::ToRadians(AimAngle - 90))));
		// EntityBatch.Add(Math::Vec4(BoxCoords, 100, 50), Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/vector_reticle.png").id, 
		// 					Graphics::SetOpacity(Graphics::RGBA16_White(), 0.7f), 1.0f, Math::ToRadians(AimAngle + 45), Graphics::CoordinateFormat::ISOMETRIC);


		Enjon::Math::Vec2 AimCoords(World->TransformSystem->Transforms[Player].Position.XY() + Math::Vec2(100.0f, -100.0f));
		float aimRadius = 0.25f;
		AimCoords = AimCoords - aimRadius * Math::CartesianToIso(Math::Vec2(cos(Math::ToRadians(AimAngle)), sin(Math::ToRadians(AimAngle))));

		// Draw player "aim" box for testing purposes
		static float aim_count = 0.0f;
		static float aim_count2 = 0.0f;
		aim_count += 0.5f;
		static Enjon::uint32 aim_index = 0;
		static Graphics::ColorRGBA16 AimColor;
		if (aim_count >= 1.0f)
		{
			if (aim_index == 0)
			{
				aim_count2 += 0.05f;
				if (aim_count2 >= 1.0f)
				{
					aim_count2 = 0.0f;
					aim_count = 0.0f;
					aim_index++;
				}	
			}
			else if (aim_index == 7)
			{
				AimColor = Graphics::RGBA16_Red();
				aim_count2 += 0.025f;
				if (aim_count2 >= 1.0f)
				{
					AimColor = Graphics::RGBA16_White();
					aim_index = 0;
					aim_count = 0.0f;
					aim_count2 = 0.0f;
				}

			}
			else
			{
				AimColor = Graphics::RGBA16_White();
				aim_index++;
				aim_count = 0.0f;
			}
		}
		static Graphics::SpriteSheet AimSheet;
		AimSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/vector_reticle.png"), Math::iVec2(1, 1));
		// EntityBatch.Add(Math::Vec4(AimCoords, 300, 100), AimSheet.GetUV(aim_index), AimSheet.texture.id, AimColor, 1.0f, Math::ToRadians(AimAngle + 90.0f), Graphics::CoordinateFormat::ISOMETRIC);


		Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
		const Enjon::Graphics::ColorRGBA16* Color = &World->Renderer2DSystem->Renderers[Player].Color;
		Enjon::Math::Vec2* PlayerPosition = &World->TransformSystem->Transforms[Player].Position.XY();
		if (World->Animation2DSystem->Animations[Player].Sheet == EG::SpriteSheetManager::GetSpriteSheet("PlayerSheet2"))
		{
			dims = Math::Vec2(115.0f, 115.0f);
			// printf("Yes, fucker!\n");
		}
		else dims = Math::Vec2(100.0f, 100.0f);

		EntityBatch.Add(Math::Vec4(*PlayerPosition, dims), Sheet->GetUV(Frame), Sheet->texture.id, *Color, PlayerPosition->y - World->TransformSystem->Transforms[Player].Position.z);

		Enjon::Math::Vec2* A = &World->TransformSystem->Transforms[Player].CartesianPosition;
		Enjon::Physics::AABB* AABB = &World->TransformSystem->Transforms[Sword].AABB;
		Enjon::Math::Vec2 AV = World->TransformSystem->Transforms[Player].AttackVector;
		float XDiff = TILE_SIZE;
		Enjon::Math::Vec2 AABBIsomin(Enjon::Math::CartesianToIso(AABB->Min) + Math::Vec2(XDiff, 0.0f));
		Enjon::Math::Vec2 AABBIsomax(Enjon::Math::CartesianToIso(AABB->Max));
		float AABBHeight = AABB->Max.y - AABB->Min.y, AABBWidth = AABB->Max.x - AABB->Min.y;
		// EntityBatch.Add(Math::Vec4(AABBIsomin, Math::Vec2(abs(AABB->Max.x - AABB->Min.x), abs(AABB->Max.y - AABB->Min.y))), 
		// 					Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
		// 					Graphics::SetOpacity(Graphics::RGBA16_Red(), 0.2f), AABBIsomin.y, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);

		// Draw player ground tile 
		const Math::Vec2* GroundPosition = &World->TransformSystem->Transforms[Player].GroundPosition;
		EntityBatch.Add(Math::Vec4(GroundPosition->x, GroundPosition->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
									Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f));
		// Draw player shadow
		EntityBatch.Add(Math::Vec4(GroundPosition->x - 20.0f, GroundPosition->y - 20.0f, 45.0f, 128.0f), Sheet->GetUV(Frame), Sheet->texture.id,
									Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.3f), 1.0f, Enjon::Math::ToRadians(120.0f));
		// MapEntityBatch.Add(Math::Vec4(GroundPosition->x, GroundPosition->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
		// 							Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.7f));

		// Cartesian AABB overlay
		// EntityBatch.Add(Math::Vec4(AABB->Min, Math::Vec2(abs(AABB->Max.x - AABB->Min.x), abs(AABB->Max.y - AABB->Min.y))), Math::Vec4(0, 0, 1, 1), 0,
		// 							Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.7f));
		// EntityBatch.Add(Math::Vec4(A->x, A->y, TILE_SIZE, TILE_SIZE), Math::Vec4(0, 0, 1, 1), 0,
		// 							Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.7f));

		// Add Health display
		Graphics::Fonts::Font* F = Graphics::FontManager::GetFont(std::string("Bold")); 

		Enjon::Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - 40.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 40.0f, 0.4f, 
											"Enemy Health", EG::FontManager::GetFont("Bold"), 
											HUDBatch, EG::RGBA16_Orange());

		// Rotate the thing
		{
			// Random verticle bar to test rotations
			EM::Vec2 BeamDims(500.0f, 2.0f);
			EM::Vec2 BeamPos = World->TransformSystem->Transforms[Player].Position.XY() + EM::Vec2(BeamDims.y / 2.0f + 60.0f, 40.0f);
			Enjon::Math::Vec2 MousePos = Input.GetMouseCoords();
			Camera.ConvertScreenToWorld(MousePos);
			MousePos = EM::IsoToCartesian(MousePos);
			BeamPos = EM::IsoToCartesian(BeamPos);
			EM::Vec2 R(1,0);
			auto Norm = EM::Vec2::Normalize(MousePos - BeamPos);
			auto a = acos(Norm.DotProduct(R)) * 180.0f / M_PI;
			if (Norm.y < 0) a *= -1;
			// a += 90;
			BeamPos = EM::CartesianToIso(BeamPos);
			auto BeamX = BeamPos.x;
			auto BeamY = BeamPos.y;
			auto Rad = 250.0f;
			BeamPos = BeamPos + Rad * EM::CartesianToIso(Math::Vec2(cos(EM::ToRadians(a)), sin(EM::ToRadians(a))));
			// BeamPos = 2 * EM::Vec2(BeamX + cos(a), BeamY + sin(a));
			// auto Radius = 10.0f;
			// BeamPos = Radius * EM::Vec2(BeamX + cos(a), BeamY + sin(a));
			// BeamPos = EM::Vec2(BeamX * cos(a) - BeamY * sin(a), BeamX * sin(a) + BeamY * cos(a));

			EntityBatch.Add(EM::Vec4(BeamPos, BeamDims), EM::Vec4(0, 0, 1, 1), 
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/verticlebar.png").id, EG::SetOpacity(EG::RGBA16_Green(), 0.5f), BeamPos.y, EM::ToRadians(a), EG::CoordinateFormat::ISOMETRIC);
		}	

		float X = HUDCamera.GetPosition().x - 190.0f;
		float Y = HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 60.0f;
		HUDBatch.Add(EM::Vec4(X, Y, 400.0f, 10.0f),
					 EM::Vec4(0, 0, 1, 1), 
					  HealthSheet.texture.id, 
					  EG::RGBA16_Red());

		// Add an overlay to the Map for better viewing
		// MapEntityBatch.Add(Math::Vec4(-3100, -3150, 6250, 3100), Math::Vec4(0, 0, 1, 1), Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
		// 						Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA16_White(), 0.7f), 100.0f);

		if (Paused)
		{
			// Draw paused text
			Enjon::Graphics::Fonts::PrintText(Camera.GetPosition().x - 110.0f, Camera.GetPosition().y - 30.0f, 1.0f, "Paused", F, TextBatch);
		}

		// Add FPS
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 60.0f, 
										0.4f, "FPS: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 60.0f, 
										0.4f, FPSString, F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Add CollisionTime
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 80.0f, 
										0.4f, "Collisions: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 80.0f, 
										0.4f, CollisionTimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Add RenderTime
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 100.0f, 
										0.4f, "Rendering: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 100.0f, 
										0.4f, RenderTimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Add EffectTime
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 120.0f, 
										0.4f, "Effects: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 120.0f, 
										0.4f, EffectTimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Add TileOverlayTime
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 140.0f, 
										0.4f, "TileOverlay: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 140.0f, 
										0.4f, TileOverlayTimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Draw Isometric compass
		MapEntityBatch.Add(EM::Vec4(HUDCamera.GetPosition() - EM::Vec2(SCREENWIDTH / 2.0f - 30.0f, -SCREENHEIGHT / 2.0f + 250.0f), 150.0f, 75.0f), 
						EM::Vec4(0, 0, 1, 1), EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Coordinates.png").id, EG::RGBA16_White());

		// Add particles to entity batch
		EG::Particle2D::Draw(World->ParticleEngine, &Camera);

		// Draw only the world that surrounds the player
		{
			// auto TW = 256.0f / 2.0f;
			// EM::Vec2* PGP = &World->TransformSystem->Transforms[Player].CartesianPosition;
			// uint32 R = -PGP->x / TW;
			// uint32 C = -PGP->y / TW;

			// // 12 block radius might be the smallest I'd like to go
			// uint32 Radius = 12;
			// uint32 Padding = 2;

			// uint32 MinR = R - Radius;
			// uint32 MaxR = R + Radius + Padding;
			// uint32 MinC = C - Radius;
			// uint32 MaxC = C + Radius + Padding;


			// if (C <= Radius) MinC = 0;
			// if (R <= Radius) MinR = 0;
			// if (C >= LEVELSIZE - Radius - Padding) MaxC = LEVELSIZE;
			// if (R >= LEVELSIZE - Radius - Padding) MaxR = LEVELSIZE;

			// auto IsoTiles = level.GetIsoTiles();
			// for (uint32 i = MinR; i < MaxR; i++)
			// {
			// 	for (uint32 j = MinC; j < MaxC; j++)
			// 	{
			// 		auto T = IsoTiles[LEVELSIZE * i + j];

			// 		// If front wall, then lower opacity
			// 		EG::ColorRGBA16 Color = EG::RGBA16_White();
			// 		if (i == 0 || i >= LEVELSIZE - 1 || j == 0 || j >= LEVELSIZE - 1) Color = EG::SetOpacity(Color, 0.5f);
			// 		EntityBatch.Add(Enjon::Math::Vec4(T->pos, T->dims), T->Sheet->GetUV(T->index), T->Sheet->texture.id, Color, T->depth);
			// 	}
			// }
		}


		EntityBatch.End();
		TextBatch.End(); 
		MapEntityBatch.End(); 
		HUDBatch.End();
	
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
			glUniform1i(glGetUniformLocation(shader, "isLevel"), false);
			glUniform1i(glGetUniformLocation(shader, "useOverlay"), true);
			glUniform1i(glGetUniformLocation(shader, "isPaused"), Paused);
		} 

		// Draw entities		
		EntityBatch.RenderBatch();


		// Draw Text
		shader = Graphics::ShaderManager::GetShader("Text")->GetProgramID();
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
		} 


		TextBatch.RenderBatch();
		ParticleBatch.RenderBatch();	


		shader = Graphics::ShaderManager::GetShader("Basic")->GetProgramID();
		glUseProgram(shader);
		view = Camera.GetCameraMatrix();
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
							1, 0, view.elements);

		// Draw front walls
		// FrontWallBatch.RenderBatch();

		// Draw Text
		shader = Graphics::ShaderManager::GetShader("Text")->GetProgramID();
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
		} 

		// Render HUD camera	
		view = HUDCamera.GetCameraMatrix();
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
							1, 0, view.elements);
		HUDBatch.RenderBatch();

		shader = Graphics::ShaderManager::GetShader("Basic")->GetProgramID();
		glUseProgram(shader);
		// Draw Cartesian Map
		// model *= Math::Mat4::Translate(Math::Vec3(-SCREENWIDTH / 4.0f - 140.0f, SCREENHEIGHT / 2.0f - 40.0f, 0.0f)) * Math::Mat4::Scale(Math::Vec3(0.08f, 0.08f, 1.0f));
		view = HUDCamera.GetCameraMatrix();

		glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
							1, 0, model.elements);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
							1, 0, view.elements);

		if (ShowMap)
		{
			MapEntityBatch.RenderBatch();
			// MapBatch.RenderBatch();
		}


		// Draw Cursor
		DrawCursor(&EntityBatch, &Input);

		Window.SwapBuffer();

		RenderTime = SDL_GetTicks() - StartTicks;
		
		////////////////////////////////////////////////

		// End FPSLimiter
		FPS = Limiter.End();

		static float counter = 0.0f;
		counter += 0.025f;
		if (counter > 1.0f)
		{
			FPSString = std::to_string(FPS);
			CollisionTimeString = std::to_string(CollisionRunTime);
			TransformTimeString = std::to_string(TransformRunTime);
			RenderTimeString = std::to_string(RenderTime);
			EffectTimeString = std::to_string(EffectRunTime);
			TileOverlayTimeString = std::to_string(TileOverlayRunTime);

			Loot::PrintCounts();

			counter = 0.0f;
		}
	} 

	return 0;
}

void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, struct EntityManager* World, ECS::eid32 Entity)
{
    SDL_Event event;
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

	if (Input->IsKeyPressed(SDLK_c)) {
		printf("Dashing\n");
		IsDashing = true;
	}

	// Stupid, but use it for now...
	if (Input->IsKeyPressed(SDLK_m)) {
		ShowMap = !ShowMap;
	}

	if (Input->IsKeyPressed(SDLK_p)) {
		Paused = !Paused;
	}
}

void DrawCursor(Enjon::Graphics::SpriteBatch* Batch, Enjon::Input::InputManager* InputManager)
{ 
	Enjon::Math::Mat4 model, view, projection;
	model = Math::Mat4::Identity();
	view = Math::Mat4::Orthographic(0, (float)SCREENWIDTH, 0, (float)SCREENHEIGHT, -1, 1);
	projection = Math::Mat4::Identity();
	
	GLuint shader = Graphics::ShaderManager::GetShader("Basic")->GetProgramID(); 
	glUseProgram(shader);
	{
		glUniform1i(glGetUniformLocation(shader, "texture"),
					 0);
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
							1, 0, model.elements);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
							1, 0, view.elements);
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
							1, 0, projection.elements);
	}

	static Enjon::Graphics::GLTexture MouseTexture = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/crosshair.png");

	float size = 32.0f; 
	Batch->Begin();	
	Enjon::Math::Vec4 destRect(InputManager->GetMouseCoords().x - size / 2.0f, -InputManager->GetMouseCoords().y + SCREENHEIGHT - size + size / 2.0f, size, size);
	Enjon::Math::Vec4 uvRect(0, 0, 1, 1);
	Batch->Add(destRect, uvRect, MouseTexture.id);
	Batch->End();
	Batch->RenderBatch();
	Graphics::ShaderManager::UnuseProgram("Basic");
}

void DrawSmoke(Enjon::Graphics::Particle2D::ParticleBatch2D* Batch, Enjon::Math::Vec3 Pos)
{
	static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_1.png").id;
	static GLuint PTex2 = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_2.png").id;
	static GLuint PTex3 = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_3.png").id;
	static EG::ColorRGBA16 Gray = EG::RGBA16(0.3f, 0.3f, 0.3f, 1.0f);

	for (int i = 0; i < 100; i++)
	{
		float XPos = Random::Roll(-2000, 2000), YPos = Random::Roll(-2000, 2000), ZVel = Random::Roll(-1, 1), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
						YSize = Random::Roll(100, 500), XSize = Random::Roll(200, 700);
		int Roll = Random::Roll(1, 3);
		GLuint tex;
		if (Roll == 1) tex = PTex;
		else if (Roll == 2) tex = PTex2;
		else tex = PTex3; 

		// int RedAmount = Random::Roll(0, 50);
		// int Alpha = Random::Roll(995, 1000) / 1000.0f;


		EG::Particle2D::AddParticle(Pos + Math::Vec3(XPos, YPos, 0.0f), Math::Vec3(XVel, YVel, ZVel), 
			Math::Vec2(XSize, YSize), EG::RGBA16(Gray.r, Gray.g, Gray.b + 0.1f, 0.185f), tex, 0.00075f, Batch);
	}
}


#endif 

/**
* SYSTEMS TEST
*/

#if 0
#include <stdio.h>
#include <map>

#include <Enjon.h>
#include <Graphics/Font.h>

bool ProcessInput(Enjon::Input::InputManager* Input);
// void InitFont(void);
// void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Enjon::Graphics::SpriteBatch& Batch, 
// 						Enjon::Graphics::ColorRGBA16 Color = Enjon::Graphics::RGBA16_White());

// typedef struct 
// {
// 	GLuint TextureID;
// 	Enjon::Math::iVec2 Size;
// 	Enjon::Math::iVec2 Bearing;
// 	GLuint Advance;
// } Character;

// std::map<GLchar, Character> Characters;

#undef main
int main(int argc, char** argv)
{
	#define WINDOWRUNNING 1

#if WINDOWRUNNING
	Enjon::Init();
	Enjon::Graphics::Window Window;
	Window.Init("Test", 800, 600); 

	Enjon::Graphics::Camera2D Camera;
	Camera.Init(800, 600);

	Enjon::Utils::FPSLimiter Limiter;
	Limiter.Init(60.0f);
	
	Enjon::Input::InputManager Input;

	Enjon::Graphics::SpriteBatch Batch;

	// InitFont();

	Enjon::Graphics::Fonts::Font Zombie_32;
	Enjon::Graphics::Fonts::Init("../assets/fonts/Zombie/Zombie.ttf", 32, &Zombie_32);

	Batch.Init();

	Enjon::Graphics::ShaderManager::Init();

	bool Running = true;
	// Game loop
	while (Running)
	{ 
		Limiter.Begin();

		// Process input
		Running = ProcessInput(&Input);

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT); 
		Camera.Update(); 

		Enjon::Math::Mat4 model, view, projection;
		model = Enjon::Math::Mat4::Identity();
		view = Camera.GetCameraMatrix();
		projection = Enjon::Math::Mat4::Identity();

		GLint shader = Enjon::Graphics::ShaderManager::GetShader("Text")->GetProgramID();
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
		} 
	
		// Begin batch
		Batch.Begin();

		// Draw text
		Enjon::Graphics::Fonts::PrintText(0.0f, 0.0f, 0.8f, "I have text!", &Zombie_32, Batch, Enjon::Graphics::RGBA16_Orange());

		// End and render
		Batch.End();
		Batch.RenderBatch();


		Limiter.End();
		Window.SwapBuffer();
	}

	#endif

	return 0;
} 

bool ProcessInput(Enjon::Input::InputManager* Input)
{
    SDL_Event event;
//    //Will keep looping until there are no more events to process
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
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
		return false;
	}

	return true;
}

// void InitFont(void)
// {
// 	// FreeType
//     FT_Library ft;
//     // All functions return a value different than 0 whenever an error occurred
//     if (FT_Init_FreeType(&ft))
//         std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

//     // Load font as face
//     FT_Face face;
//     if (FT_New_Face(ft, "../assets/fonts/Zombie/Zombie.ttf", 0, &face))
//         std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

//     // Set size to load glyphs as
//     FT_Set_Pixel_Sizes(face, 0, 48);

//     // Disable byte-alignment restriction
//     glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

//     // Load first 128 characters of ASCII set
//     for (GLubyte c = 0; c < 128; c++)
//     {
//         // Load character glyph 
//         if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//         {
//             std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//             continue;
//         }
//         // Generate texture
//         GLuint texture;
//         glGenTextures(1, &texture);
//         glBindTexture(GL_TEXTURE_2D, texture);
//         glTexImage2D(
//             GL_TEXTURE_2D,
//             0,
//             GL_RED,
//             face->glyph->bitmap.width,
//             face->glyph->bitmap.rows,
//             0,
//             GL_RED,
//             GL_UNSIGNED_BYTE,
//             face->glyph->bitmap.buffer
//         );
//         // Set texture options
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//         // Now store character for later use
//         Character character = {
//             texture,
//             Enjon::Math::iVec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//             Enjon::Math::iVec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//             face->glyph->advance.x
//         };
//         Characters.insert(std::pair<GLchar, Character>(c, character));
//     }
//     glBindTexture(GL_TEXTURE_2D, 0);
//     // Destroy FreeType once we're finished
//     FT_Done_Face(face);
//     FT_Done_FreeType(ft);
// }

// void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Enjon::Graphics::SpriteBatch& Batch, 
// 						Enjon::Graphics::ColorRGBA16 Color)
// {
// 	// Iterate through all characters
//     std::string::const_iterator c;
//     for (c = text.begin(); c != text.end(); c++) 
//     {
//         Character ch = Characters[*c];

//         GLfloat xpos = x + ch.Bearing.x * scale;
//         GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

//         GLfloat w = ch.Size.x * scale;
//         GLfloat h = ch.Size.y * scale;

//         Enjon::Math::Vec4 DestRect(xpos, ypos, w, h);
//         Enjon::Math::Vec4 UV(0, 0, 1, 1);

//         // Add to batch
//         Batch.Add(DestRect, UV, ch.TextureID, Color);

//         // Advance to next character
//         x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
//     }
// }

#endif


/**
*  UNIT TESTS
*/

#if 0

#include <Enjon.h>

#include <unordered_map>
#include <iostream>

#undef main
int main(int argc, char** argv) {

	std::unordered_map<std::string, int> Map;

	Map["BLACK"] = 5;
	Map["Bob"] = 6;

	for (auto e : Map)
	{
		std::cout << "Key: " << e.first << ", Value: " << e.second << std::endl;	
	}

	return 0;
}
#endif









