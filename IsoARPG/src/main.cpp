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
#include <Utils/FileUtils.h>
#include <Math/Random.h>

#include <SDL2/SDL.h>
#include <GLEW/glew.h>

#include <ECS/ComponentSystems.h>
#include <ECS/PlayerControllerSystem.h>
#include <ECS/Transform3DSystem.h>
#include <ECS/CollisionSystem.h>
#include <ECS/Animation2DSystem.h>
#include <ECS/InventorySystem.h> 
#include <ECS/Renderer2DSystem.h>
#include <ECS/AIControllerSystem.h> 
#include <ECS/AttributeSystem.h>
#include <ECS/Entity.h>

#include "Animation.h"
#include "AnimationManager.h"
#include "SpatialHash.h"
#include "Level.h"

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

float DashingCounter = 0.0f;

float CollisionRunTime = 0.0f;
float TransformRunTime = 0.0f;
float ClearEntitiesRunTime = 0.0f;

using namespace Enjon;
using namespace ECS;
using namespace Systems;

void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, struct EntityManager* Manager, ECS::eid32 Entity);
void DrawCursor(Enjon::Graphics::SpriteBatch* Batch, Enjon::Input::InputManager* InputManager);

SDL_Joystick* Joystick;

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

	// Query for available controllers
	printf("%d joysticks were found. \n\n", SDL_NumJoysticks());

	Joystick = SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);

	// Create a window
	Graphics::Window Window;
	Window.Init("Testing Grounds", screenWidth, screenHeight, SCREENRES);

	// Hide mouse
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::HIDE);

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

	Level level;
	Graphics::GLTexture TileTexture;
	level.Init(Camera.GetPosition().x, Camera.GetPosition().y, 100, 100);
	TileTexture = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledblue.png");
	
	float x = Camera.GetPosition().x;
	float y = Camera.GetPosition().y;

	// Spatial Hash
	SpatialHash::Grid grid;
	int width = level.GetWidth();
	SpatialHash::Init(&grid, width, width);

	// Make spritesheets for player and enemy and item
	Enjon::Graphics::SpriteSheet PlayerSheet;
	Enjon::Graphics::SpriteSheet EnemySheet;
	Enjon::Graphics::SpriteSheet EnemySheet2;
	Enjon::Graphics::SpriteSheet ItemSheet;
	Enjon::Graphics::SpriteSheet ArrowSheet;
	Enjon::Graphics::SpriteSheet ReticleSheet;
	Enjon::Graphics::SpriteSheet TargetSheet;
	PlayerSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtestframessplit.png"), Enjon::Math::iVec2(6, 24));
	EnemySheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast.png"), Math::iVec2(1, 1));
	EnemySheet2.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/enemy.png"), Math::iVec2(1, 1));
	ItemSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Math::iVec2(1, 1));
	ArrowSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"), Math::iVec2(8, 1));
	ReticleSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/circle_reticle.png"), Math::iVec2(1, 1));
	TargetSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Target.png"), Math::iVec2(1, 1));
	
	// Creating tiled iso level
	TileBatch.Begin(); 
	level.DrawIsoLevel(TileBatch);
	TileBatch.End();	

	CartesianTileBatch.Begin();
	level.DrawCartesianLevel(CartesianTileBatch);
	CartesianTileBatch.End();

	FrontWallBatch.Begin();
	level.DrawIsoLevelFront(FrontWallBatch);
	FrontWallBatch.End();

	MapBatch.Begin();
	level.DrawMap(MapBatch);
	MapBatch.End();
 
	// Create EntityBatch
	Enjon::Graphics::SpriteBatch EntityBatch;
	EntityBatch.Init();
	
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
	struct EntityManager* World = EntitySystem::NewEntityManager(level.GetWidth(), level.GetWidth(), &Camera);

	Math::Vec2 Pos = Camera.GetPosition() + 50.0f;

	static Math::Vec2 enemydims(222.0f, 200.0f);

	static uint32 AmountDrawn = 2500;
	for (int e = 0; e < AmountDrawn; e++)
	{
		float height = 30.0f;
		EntitySystem::CreateAI(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), height),
																enemydims, &EnemySheet, "Enemy", 0.05f); // NOTE(John): The transform system is a bottleneck right now }
	}

	// Create player
	eid32 Player = EntitySystem::CreatePlayer(World, &Input, Math::Vec3(Math::CartesianToIso(Math::Vec2(-level.GetWidth()/2, -level.GetHeight()/2)), 0.0f), Math::Vec2(100.0f, 100.0f), &PlayerSheet, "Player", 0.3f, Math::Vec3(1, 1, 0)); 

	// Set player for world
	World->Player = Player;

	// Create Sword
	eid32 Sword = EntitySystem::CreateItem(World, World->TransformSystem->Transforms[Player].Position, Enjon::Math::Vec2(32.0f, 32.0f), &ItemSheet, 
												(Masks::Type::WEAPON | Masks::GeneralOptions::EQUIPPED | Masks::GeneralOptions::PICKED_UP), Component::EntityType::WEAPON, "Weapon");

	// Turn off Rendering / Transform Components
	EntitySystem::RemoveComponents(World, Sword, COMPONENT_RENDERER2D | COMPONENT_TRANSFORM3D);

	// Equip player with sword
	World->InventorySystem->Inventories[Player].Items.push_back(Sword);
	World->InventorySystem->Inventories[Player].WeaponEquipped = Sword;

	AmountDrawn = 0;

	for (int e = 0; e < AmountDrawn; e++)
	{
		// Create Sword
		eid32 id = EntitySystem::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
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
			SpatialHash::ClearCells(World->Grid);
			ClearEntitiesRunTime = (SDL_GetTicks() - StartTicks) / 1000.0f; // NOTE(John): As the levels increase, THIS becomes the true bottleneck

			AIController::Update(World->AIControllerSystem, Player);
			Animation2D::Update(World);

			StartTicks = SDL_GetTicks();
			Transform::Update(World->TransformSystem);
			TransformRunTime = (SDL_GetTicks() - StartTicks) / 1000.0f;

			StartTicks = SDL_GetTicks();	
			Collision::Update(World);
			CollisionRunTime = (SDL_GetTicks() - StartTicks) / 1000.0f;

			Renderer2D::Update(World); 
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
			glUniform1f(glGetUniformLocation(shader, "time"), time);
			glUniform1i(glGetUniformLocation(shader, "isLevel"), true);
			glUniform1i(glGetUniformLocation(shader, "isPaused"), Paused);
			glUniform2f(glGetUniformLocation(shader, "resolution"), SCREENWIDTH, SCREENHEIGHT);
			glUniform1i(glGetUniformLocation(shader, "useOverlay"), true);
		} 
		
		// Draw map
		TileBatch.RenderBatch();

		// Draw Entities
		EntityBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT); 
		MapEntityBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT); 
		TextBatch.Begin(); 

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
					EntityBatch.Add(Math::Vec4(*EntityPosition, enemydims), uv, beast.id, *Color, EntityPosition->y - World->TransformSystem->Transforms[e].Position.z);
				}
				// If target
				if (e == World->PlayerControllerSystem->CurrentTarget)
				{
					Math::Vec2 ReticleDims(94.0f, 47.0f);
					Math::Vec2 Position = World->TransformSystem->Transforms[e].GroundPosition - Math::Vec2(15.0f, 5.0f);
					EntityBatch.Add(Math::Vec4(Position.x, Position.y, ReticleDims), Enjon::Math::Vec4(0, 0, 1, 1), TargetSheet.texture.id, Enjon::Graphics::RGBA8_Red(), Position.y);
				}

			}
			else if (World->Types[e] == ECS::Component::EntityType::ITEM)
			{
				if (Camera.IsBoundBoxInCamView(*EntityPosition, itemDims))
				{
					EntityBatch.Add(Math::Vec4(*EntityPosition, itemDims), ItemSheet.GetUV(0), ItemSheet.texture.id, *Color, EntityPosition->y);
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

					EntityBatch.Add(Math::Vec4(*EntityPosition, arrowDims), ArrowSheet.GetUV(index), ArrowSheet.texture.id, *Color, EntityPosition->y);
				}
			}

			Ground = &World->TransformSystem->Transforms[e].GroundPosition;
			if (World->Types[e] != ECS::Component::EntityType::ITEM && Camera.IsBoundBoxInCamView(*Ground, Math::Vec2(64.0f, 32.0f)))
			{
				EntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
										Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.2f), 1.0f);
				MapEntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
										Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.7f), 1.0f);
			}
		}

		// Draw player reticle
		// NOTE(John): In order for this to work the way I'm thinking, I'll need to learn stencil buffering
		// and apply that here to mask out the area I don't want
		// Math::Vec2 ReticleDims(94.0f, 47.0f);
		// Math::Vec2 Position = World->TransformSystem->Transforms[Player].GroundPosition - Math::Vec2(17.0f, 0.0f);
		// EntityBatch.Add(Math::Vec4(Position.x, Position.y + World->TransformSystem->Transforms[Player].Position.z, ReticleDims), Enjon::Math::Vec4(0, 0, 1, 1), ReticleSheet.texture.id, 
		// 							Graphics::RGBA8_White(), Position.y + World->TransformSystem->Transforms[Player].Position.z);	

		// Draw player

		// Dashing state if dashing
		static Graphics::SpriteSheet* Sheet = World->Animation2DSystem->Animations[Player].Sheet; static Enjon::uint32 Frame = World->Animation2DSystem->Animations[Player].CurrentFrame;

		if (IsDashing)
		{
			// Make unable to collide with enemy
			World->AttributeSystem->Masks[Player] &= ~Masks::GeneralOptions::COLLIDABLE;

			float DashAmount = 10.0f;
			World->TransformSystem->Transforms[Player].Position.x += (World->TransformSystem->Transforms[Player].Velocity.x * 2.0f);
			World->TransformSystem->Transforms[Player].Position.y += (World->TransformSystem->Transforms[Player].Velocity.y * 8.0f);
			World->TransformSystem->Transforms[Player].VelocityGoalScale = 0.01f;
			World->TransformSystem->Transforms[Player].Velocity.x *= 1.05f; 
			World->TransformSystem->Transforms[Player].Velocity.y *= 1.05f; 
			// Setting the "alarm"
			DashingCounter += 0.05f;
			if (DashingCounter >= 0.75f) { IsDashing = false; DashingCounter = 0.0f; }
			float Opacity = 0.5f;
			for (int i = 0; i < 5; i++)
			{
				Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
				Enjon::Graphics::ColorRGBA8 DashColor = World->Renderer2DSystem->Renderers[Player].Color;
				Enjon::Math::Vec2 PP = World->TransformSystem->Transforms[Player].Position.XY();
				Enjon::Math::Vec2 PV = World->TransformSystem->Transforms[Player].Velocity.XY();
				PP.x -= (i + i*0.75f) * PV.x;
				PP.y -= (i + i*0.75f) * PV.y;
				EntityBatch.Add(Math::Vec4(PP, dims), Sheet->GetUV(Frame), Sheet->texture.id, Graphics::SetOpacity(DashColor, Opacity), PP.y - World->TransformSystem->Transforms[Player].Position.z);
				Opacity -= 0.05f;
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
		BoxCoords = BoxCoords - boxRadius * Math::CartesianToIso(Math::Vec2(cos(Math::ToRadians(AimAngle -40)), sin(Math::ToRadians(AimAngle - 40))));
		// EntityBatch.Add(Math::Vec4(BoxCoords, 100, 50), Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/vector_reticle.png").id, 
		// 					Graphics::SetOpacity(Graphics::RGBA8_White(), 0.7f), 1.0f, Math::ToRadians(AimAngle + 45), Graphics::CoordinateFormat::ISOMETRIC);


		Enjon::Math::Vec2 AimCoords(World->TransformSystem->Transforms[Player].Position.XY() + Math::Vec2(100.0f, -100.0f));
		float aimRadius = 0.25f;
		AimCoords = AimCoords - aimRadius * Math::CartesianToIso(Math::Vec2(cos(Math::ToRadians(AimAngle)), sin(Math::ToRadians(AimAngle))));

		// Draw player "aim" box for testing purposes
		static float aim_count = 0.0f;
		static float aim_count2 = 0.0f;
		aim_count += 0.5f;
		static Enjon::uint32 aim_index = 0;
		static Graphics::ColorRGBA8 AimColor;
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
				AimColor = Graphics::RGBA8_Red();
				aim_count2 += 0.025f;
				if (aim_count2 >= 1.0f)
				{
					AimColor = Graphics::RGBA8_White();
					aim_index = 0;
					aim_count = 0.0f;
					aim_count2 = 0.0f;
				}

			}
			else
			{
				AimColor = Graphics::RGBA8_White();
				aim_index++;
				aim_count = 0.0f;
			}
		}
		static Graphics::SpriteSheet AimSheet;
		AimSheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/aim_guide.png"), Math::iVec2(8, 1));
		// EntityBatch.Add(Math::Vec4(AimCoords, 200, 300), AimSheet.GetUV(aim_index), AimSheet.texture.id, AimColor, 1.0f, Math::ToRadians(AimAngle + 120.0f), Graphics::CoordinateFormat::ISOMETRIC);

	
		Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
		const Enjon::Graphics::ColorRGBA8* Color = &World->Renderer2DSystem->Renderers[Player].Color;
		Enjon::Math::Vec2* PlayerPosition = &World->TransformSystem->Transforms[Player].Position.XY();
		EntityBatch.Add(Math::Vec4(*PlayerPosition, dims), Sheet->GetUV(Frame), Sheet->texture.id, *Color, PlayerPosition->y - World->TransformSystem->Transforms[Player].Position.z);

		Enjon::Math::Vec2* A = &World->TransformSystem->Transforms[Player].CartesianPosition;
		Enjon::Physics::AABB* AABB = &World->TransformSystem->Transforms[Sword].AABB;
		Enjon::Math::Vec2 AV = World->TransformSystem->Transforms[Player].AttackVector;
		float XDiff = TILE_SIZE;
		Enjon::Math::Vec2 AABBIsomin(Enjon::Math::CartesianToIso(AABB->Min) + Math::Vec2(XDiff, 0.0f));
		Enjon::Math::Vec2 AABBIsomax(Enjon::Math::CartesianToIso(AABB->Max));
		float AABBHeight = AABB->Max.y - AABB->Min.y, AABBWidth = AABB->Max.x - AABB->Min.y;
		EntityBatch.Add(Math::Vec4(AABBIsomin, Math::Vec2(abs(AABB->Max.x - AABB->Min.x), abs(AABB->Max.y - AABB->Min.y))), 
							Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
							Graphics::SetOpacity(Graphics::RGBA8_Red(), 0.2f), 1.0f, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);

		// Draw player ground tile 
		const Math::Vec2* GroundPosition = &World->TransformSystem->Transforms[Player].GroundPosition;
		EntityBatch.Add(Math::Vec4(GroundPosition->x, GroundPosition->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.2f));
		EntityBatch.Add(Math::Vec4(GroundPosition->x - 40.0f, GroundPosition->y - 80.0f, 45.0f, 128.0f), Sheet->GetUV(Frame), Sheet->texture.id,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.3f), 1.0f, Enjon::Math::ToRadians(120.0f));
		MapEntityBatch.Add(Math::Vec4(GroundPosition->x, GroundPosition->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.7f));
		EntityBatch.Add(Math::Vec4(AABB->Min, Math::Vec2(abs(AABB->Max.x - AABB->Min.x), abs(AABB->Max.y - AABB->Min.y))), Math::Vec4(0, 0, 1, 1), 0,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.7f));
		EntityBatch.Add(Math::Vec4(A->x, A->y, TILE_SIZE, TILE_SIZE), Math::Vec4(0, 0, 1, 1), 0,
									Graphics::SetOpacity(Graphics::RGBA8_Black(), 0.7f));


		// Add an overlay to the Map for better viewing
		MapEntityBatch.Add(Math::Vec4(-3100, -3150, 6250, 3100), Math::Vec4(0, 0, 1, 1), Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
								Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA8_White(), 0.7f), 100.0f);

		if (Paused)
		{
			// Draw paused text
			TextBatch.Add(Math::Vec4(Camera.GetPosition().x - 60.0f, Camera.GetPosition().y - 30.0f, 125.0f, 70.0f), Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/paused.png").id, 
									Graphics::RGBA8_Orange(), -50.0f);
		}
	
		EntityBatch.End();
		TextBatch.End(); 
		MapEntityBatch.End(); 
	
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
			glUniform1i(glGetUniformLocation(shader, "isLevel"), false);
			glUniform1i(glGetUniformLocation(shader, "useOverlay"), true);
		} 

		// Draw entities		
		EntityBatch.RenderBatch();

		// Draw front walls
		FrontWallBatch.RenderBatch();

		// Draw Text
		if (Paused)
		{
			glUniform1i(glGetUniformLocation(shader, "useOverlay"), false);
			TextBatch.RenderBatch();
		}

		// Draw Cartesian Map
		model *= Math::Mat4::Translate(Math::Vec3(-SCREENWIDTH / 4.0f - 140.0f, SCREENHEIGHT / 2.0f - 40.0f, 0.0f)) * Math::Mat4::Scale(Math::Vec3(0.08f, 0.08f, 1.0f));
		view = HUDCamera.GetCameraMatrix();

		glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
							1, 0, model.elements);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
							1, 0, view.elements);

		if (ShowMap)
		{
			MapEntityBatch.RenderBatch();
			MapBatch.RenderBatch();
		}


		// Draw Cursor
		DrawCursor(&EntityBatch, &Input);

		Window.SwapBuffer();
		
		////////////////////////////////////////////////

		// End FPSLimiter
		FPS = Limiter.End();

		static float counter = 0.0f;
		counter += 0.025f;
		if (counter > 1.0f)
		{
			printf("FPS: %0.2f\n", FPS);
			printf("ClearEntitiesRunTime: %f s\n", ClearEntitiesRunTime);
			printf("Transform Time: %f s\n", TransformRunTime);
			printf("Collision Time: %f s\n", CollisionRunTime);
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

	static int index = 0;
	static float timer = 0.0f;
	
	// if (m_isclicked)
	// { 
	// 	timer += 0.1f;
	// 	if (timer >= 0.2f)
	// 	{
	// 		index++;
	// 		timer = 0.0f;
	// 	}
	// 	if (index >= 5)
	// 	{
	// 		m_isclicked = false;
	// 		timer = 0.0f;
	// 		index = 0;
	// 	}
	// }

	static Enjon::Graphics::GLTexture MouseTexture = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Cursor24.png");
 
	Batch->Begin();	
	Enjon::Math::Vec4 destRect(InputManager->GetMouseCoords().x, -InputManager->GetMouseCoords().y + SCREENHEIGHT - 16, 16, 16);
	Enjon::Math::Vec4 uvRect(0, 0, 1, 1);
	// if (m_isclicked) m_hudbatch.Add(Math::Vec4(destRect.x - 125, destRect.y - 75, 252, 180), m_groundclick.GetUV(index), m_groundclick.texture.id,
	// 										SetOpacity(Graphics::RGBA8_White(), 0.6f));
	// m_hudbatch.Add(Math::Vec4(destRect.x + 3.0f, destRect.y - 3.0f, destRect.z, destRect.w), uvRect, 
	// 			   m_mousecursortexture.id, SetOpacity(Graphics::RGBA8_Black(), 0.5f));
	Batch->Add(destRect, uvRect, MouseTexture.id);
	Batch->End();
	Batch->RenderBatch();
	Graphics::ShaderManager::UnuseProgram("Basic");
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
			glUniform1i(glGetUniformLocation(shader, "isLevel"), 0);
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









