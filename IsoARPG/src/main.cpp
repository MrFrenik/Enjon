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

#if 0
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
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "SpatialHash.h"
#include "Level.h"
#include "BehaviorTreeManager.h"

/*-- Standard Library includes --*/
#include <stdio.h>
#include <iostream> 
#include <time.h>
#include <stdlib.h>

#define NUM_LIGHTS 	10

typedef struct
{
	EM::Vec3 Position;
	EG::ColorRGBA16 Color;
	float Radius;
	EM::Vec3 Falloff;
} Light;

float LightZ = 0.03f;

typedef struct 
{
	EM::Vec2 Position;
	EM::Vec2 Dimensions; 
	float Angle;
} BeamSegment;

std::vector<BeamSegment> BeamSegments;

char buffer[256];
char buffer2[256];
char buffer3[256];
bool isRunning = true;
bool ShowMap = false;
bool Paused = false;
bool IsDashing = false;

const int LEVELSIZE = 20;

float DashingCounter = 0.0f;

Enjon::uint32 CollisionRunTime = 0;
Enjon::uint32 TransformRunTime = 0;
Enjon::uint32 AIRunTime = 0;
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
void DrawBox(Enjon::Graphics::SpriteBatch* Batch, Enjon::Graphics::SpriteBatch* LightBatch, ECS::Systems::EntityManager* Manager);
void GetLights(EG::Camera2D* Camera, std::vector<Light>* Lights, std::vector<Light*>& LightsToDraw);

SDL_Joystick* Joystick;

#undef main
int main(int argc, char** argv)
{
	// Seed random 
	srand(time(NULL));

	float FPS = 100;
	int screenWidth = SCREENWIDTH, screenHeight = SCREENHEIGHT;

	// Profile strings
	std::string FPSString = "60.0";
	std::string RenderTimeString = "0";
	std::string CollisionTimeString = "0";
	std::string TransformTimeString = "0";
	std::string EffectTimeString = "0";
	std::string TileOverlayTimeString = "0";
	std::string AITimeString = "0";

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

	// Hide/Show mouse
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::HIDE);


	// Create Camera
	Graphics::Camera2D Camera;
	Camera.Init(screenWidth, screenHeight);
	Camera.SetScale(1.0f); 
	
	// Create HUDCamera
	Graphics::Camera2D HUDCamera;
	HUDCamera.Init(screenWidth, screenHeight);
	HUDCamera.SetScale(1.0f);

	// Init ShaderManager
	Enjon::Graphics::ShaderManager::Init(); 

	// Init SpriteSheetManager
	EG::SpriteSheetManager::Init();

	// Init AnimationManager
	AnimationManager::Init(); 

	// Init FontManager
	Enjon::Graphics::FontManager::Init();

	// Init BehaviorTreeManager
	BTManager::Init();
	
	// Init level
	Enjon::Graphics::SpriteBatch TileBatch;
	TileBatch.Init();

	Enjon::Graphics::SpriteBatch GroundTileBatch;
	GroundTileBatch.Init();

	Enjon::Graphics::SpriteBatch CartesianTileBatch;
	CartesianTileBatch.Init();

	Enjon::Graphics::SpriteBatch LightBatch;
	LightBatch.Init();

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

	EG::SpriteBatch CubeBatch;
	CubeBatch.Init();

	EG::SpriteBatch DiffuseBatch;
	DiffuseBatch.Init();

	EG::SpriteBatch NormalsBatch;
	NormalsBatch.Init();

	EG::SpriteBatch GroundTileNormalsBatch;
	GroundTileNormalsBatch.Init();

	EG::SpriteBatch DeferredBatch;
	DeferredBatch.Init();

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
	GroundTileNormalsBatch.Begin();
	level.DrawGroundTiles(GroundTileBatch, GroundTileNormalsBatch);
	GroundTileNormalsBatch.End();
	GroundTileBatch.End();

	// GroundTileNormalsBatch.Begin();
	// GroundTileNormalsBatch.Add(
	// 	EM::Vec4(-10000, -10000, 10000, 10000), 
	// 	EM::Vec4(0, 0, 1, 1),
	// 	EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/verticlebar.png").id, 
	// 	EG::RGBA16(0.0f, 0.0f, 1.0f, 1.0f)
	// 	);
	// GroundTileNormalsBatch.End();

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
	EG::Particle2D::ParticleBatch2D* LightParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* TestParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* TextParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* SmokeBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);

	EG::GLSLProgram* DeferredShader = EG::ShaderManager::GetShader("DeferredShader");
	EG::GLSLProgram* DiffuseShader 	= EG::ShaderManager::GetShader("DiffuseShader");
	EG::GLSLProgram* NormalsShader 	= EG::ShaderManager::GetShader("NormalsShader");
	EG::GLSLProgram* ScreenShader 	= EG::ShaderManager::GetShader("NoCameraProjection");

	// FBO
	// float DWidth = SCREENWIDTH * 0.9f;
	float DWidth = SCREENWIDTH;
	float DHeight = DWidth * 0.5625f;
	EG::FrameBufferObject* DiffuseFBO 	= new EG::FrameBufferObject(DWidth, DHeight);
	EG::FrameBufferObject* NormalsFBO 	= new EG::FrameBufferObject(DWidth, DHeight);
	EG::FrameBufferObject* DeferredFBO 	= new EG::FrameBufferObject(SCREENWIDTH, SCREENHEIGHT);

	// Create InputManager
	Input::InputManager Input;


	/////////////////
	// Testing ECS //   
	/////////////////

	// Create new EntityManager
	struct EntityManager* World = EntitySystem::NewEntityManager(level.GetWidth(), level.GetWidth(), &Camera, &level);

	// Init loot system
	Loot::Init();

	// Push back particle batchs into world
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, TestParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, LightParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, TextParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, SmokeBatch);


	Math::Vec2 Pos = Camera.GetPosition() + 50.0f;

	static Math::Vec2 enemydims(222.0f, 200.0f);

	static uint32 AmountDrawn = 20;
	for (int e = 0; e < AmountDrawn; e++)
	{
		float height = 10.0f;
		eid32 ai = Factory::CreateAI(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), height),
																enemydims, &EnemySheet, "Enemy", 0.05f); 
		World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
	}

	// Create random dude to see what he looks like
	{
		float height = 0.0f;
		float h = 300.0f;
		float w = h * 0.707f;
		eid32 ai = Factory::CreateAI(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), height),
																EM::Vec2(w, h), EG::SpriteSheetManager::GetSpriteSheet("Enemy"), "Enemy", 0.05f); 
		World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
	}

	// Create player
	eid32 Player = Factory::CreatePlayer(World, &Input, Math::Vec3(Math::CartesianToIso(Math::Vec2(-level.GetWidth()/2, -level.GetHeight()/2)), 0.0f), Math::Vec2(100.0f, 100.0f), &PlayerSheet, 
		"Player", 0.4f, Math::Vec3(1, 1, 0)); 

	// Set player for world
	World->Player = Player;

	// Create Sword
	eid32 Sword = Factory::CreateWeapon(World, World->TransformSystem->Transforms[Player].Position, Enjon::Math::Vec2(32.0f, 32.0f), &ItemSheet, 
												(Masks::Type::WEAPON | 
												 Masks::WeaponOptions::MELEE | 
												 Masks::GeneralOptions::EQUIPPED | 
												 Masks::GeneralOptions::PICKED_UP), Component::EntityType::WEAPON, "Weapon");

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

	AmountDrawn = 5000;

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(ER::Roll(5, 10), ER::Roll(1, 5)), EG::SpriteSheetManager::GetSpriteSheet("VerticleBar"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
		World->Renderer2DSystem->Renderers[id].Format = EG::CoordinateFormat::ISOMETRIC;
		World->Renderer2DSystem->Renderers[id].Color = EG::RGBA16(0.5f, 0.2f, 0.1f, 1.0f);
		World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
	}

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(ER::Roll(5, 10), ER::Roll(2, 5)), EG::SpriteSheetManager::GetSpriteSheet("Box"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
		World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
		World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(50, 100) / 50.0f;
	}

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(ER::Roll(10, 20), ER::Roll(10, 20)), EG::SpriteSheetManager::GetSpriteSheet("BoxDebris"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
		World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
		World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(50, 100) / 50.0f;
	}

	for (uint32 e = 0; e < 100; e++)
	{
		auto s = ER::Roll(20, 40);
		eid32 Box = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(s, s), EG::SpriteSheetManager::GetSpriteSheet("Box"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[Box].Mass = 2.5f - ER::Roll(1, 50) / 50.0f;
		World->TransformSystem->Transforms[Box].AABBPadding = 15.0f;

	}

	for (uint32 e = 0; e < 50; e++)
	{
		auto s = ER::Roll(50, 80);
		eid32 BiggerBox = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(-level.GetWidth(), 0), Random::Roll(-level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(s, s), EG::SpriteSheetManager::GetSpriteSheet("Box"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[BiggerBox].Mass = 5.0f - ER::Roll(1, 100) / 50.0f;
		World->TransformSystem->Transforms[BiggerBox].AABBPadding = 15.0f;
	}

	// Set position to player
	Camera.SetPosition(Math::Vec2(World->TransformSystem->Transforms[Player].Position.x + 100.0f / 2.0f, World->TransformSystem->Transforms[Player].Position.y)); 

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // Positions   // TexCoords

        -1.0f,  1.0f,  0.0f, 1.0f,   	// TL
        -1.0f, -1.0f,  0.0f, 0.0f,   	// BL
         1.0f, -1.0f,  1.0f, 0.0f,		// BR

        -1.0f,  1.0f,  0.0f, 1.0f,		// TL
         1.0f, -1.0f,  1.0f, 0.0f,		// BR
         1.0f,  1.0f,  1.0f, 1.0f 		// TR
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Vector of lights
	std::vector<Light> Lights;

	const GLfloat constant = 1.0f; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
    const GLfloat linear = 0.1f;
    const GLfloat quadratic = 40.0f;
    // Then calculate radius of light volume/sphere

   	float LevelWidth = level.GetWidth();
   	float LevelHeight = level.GetHeight(); 
	for (GLuint i = 0; i < NUM_LIGHTS; i++)
	{
		EG::ColorRGBA16 Color = EG::RGBA16(ER::Roll(0, 500) / 255.0f, ER::Roll(0, 500) / 255.0f, ER::Roll(0, 500) / 255.0f, 2.5f);
	    GLfloat maxBrightness = std::fmaxf(std::fmaxf(Color.r, Color.g), Color.b);  // max(max(lightcolor.r, lightcolor.g), lightcolor.b)
	    GLfloat Radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2 * quadratic);
		Light L = {
					  EM::Vec3(ER::Roll(0, -LevelWidth), ER::Roll(0, -LevelHeight), LightZ), 
					  Color, 
					  Radius, 
					  EM::Vec3(constant, linear, quadratic)
				  };

		Lights.push_back(L);
	}

	std::vector<Light*> LightsToDraw;


	{
		float BeamSegX = 10.5f, BeamSegY = 2.0f;
		for (Enjon::uint32 i = 0; i < 50; i++)
		{
			BeamSegments.push_back({EM::Vec2(0.0f, 0.0f), EM::Vec2(BeamSegX, BeamSegY), 0.0f});
		}
	}

	
	while(isRunning)
	{ 
		static float t = 0.0f;
		t += 0.025f;

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

		{
			auto L = &Lights.at(0);
			const EM::Vec3* P = &World->TransformSystem->Transforms[Player].Position;
			L->Position = EM::Vec3(P->x, P->y - P->z, LightZ);
			L->Color = EG::RGBA16(6.0f, 4.0f, 6.0f, 1.0f);
		}

		// Clear lights
		LightsToDraw.clear();

    	// Process which lights to actually draw this frame
    	GetLights(&Camera, &Lights, LightsToDraw);

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

			// Clear entities from Renderer system vector
			World->Renderer2DSystem->Entities.clear();

			// Clear entities from collision system vectors
			World->CollisionSystem->Entities.clear();

			// Clear entities from PlayerControllerSystem targets vector
			World->PlayerControllerSystem->Targets.clear();

			// Draw some random assed fire
			EG::Particle2D::DrawFire(LightParticleBatch, EM::Vec3(0.0f, 0.0f, 0.0f));
	
			TileOverlayRunTime = SDL_GetTicks() - StartTicks;		
			StartTicks = SDL_GetTicks();
			SpatialHash::ClearCells(World->Grid);
			ClearEntitiesRunTime = (SDL_GetTicks() - StartTicks); // NOTE(John): As the levels increase, THIS becomes the true bottleneck

			StartTicks = SDL_GetTicks();
			AIController::Update(World->AIControllerSystem, Player);
			AIRunTime = SDL_GetTicks() - StartTicks;

			Animation2D::Update(World);

			StartTicks = SDL_GetTicks();
			Transform::Update(World->TransformSystem, LightParticleBatch);
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
			if (SmokeCount > 10.0f)
			{
				for (Enjon::uint32 i = 0; i < 3; i++)
				{
					DrawSmoke(LightParticleBatch, EM::Vec3(Enjon::Random::Roll(-LvlSize.x, LvlSize.x), Enjon::Random::Roll(-LvlSize.y * 2.0f, LvlSize.y * 2.0f), 0.0f));
				}
				SmokeCount = 0.0f;
			}

			// Updates the world's particle engine
			World->ParticleEngine->Update();
		
			PlayerController::Update(World->PlayerControllerSystem);
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
		
		// Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, Enjon::Graphics::RGBA16(0.05f, 0.05f, 0.05f, 1.0f));

		//Enable alpha blending
		glEnable(GL_BLEND);

		//Set blend function type
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.0, 0.0, 0.0, 0.0));


		// Set up shader
		Math::Mat4 model, view, projection;
		model = Math::Mat4::Identity();
		view = Camera.GetCameraMatrix();
		projection = Math::Mat4::Identity();

		// Draw Entities
		EntityBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT); 
		NormalsBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT);
		MapEntityBatch.Begin(Enjon::Graphics::GlyphSortType::BACK_TO_FRONT); 
		LightBatch.Begin();
		TextBatch.Begin(); 
		HUDBatch.Begin();
		DeferredBatch.Begin();

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
		for (eid32 n = 0; n < World->Renderer2DSystem->Entities.size(); n++)
		{
			ECS::eid32 e = World->Renderer2DSystem->Entities.at(n);

			if (e == Player || e == Sword || e == Bow) continue;

			// Don't draw if the entity doesn't exist anymore
			bitmask32 Mask = World->Masks[e];
			if ((Mask & COMPONENT_RENDERER2D) != COMPONENT_RENDERER2D) continue;

			Math::Vec3* EntityPosition; 
			Math::Vec2* Ground;
			char buffer[25];
		
			EG::SpriteSheet* ESpriteSheet = World->Animation2DSystem->Animations[e].Sheet;	
			EntityPosition = &World->TransformSystem->Transforms[e].Position;
			Ground = &World->TransformSystem->Transforms[e].GroundPosition;
			const Enjon::Graphics::ColorRGBA16* Color = &World->Renderer2DSystem->Renderers[e].Color;
			auto EDims = &World->TransformSystem->Transforms[e].Dimensions;

			// If AI
			if (Mask & COMPONENT_AICONTROLLER)
			{
				EntityBatch.Add(Math::Vec4(EntityPosition->XY(), *EDims), uv, ESpriteSheet->texture.id, *Color, EntityPosition->y - World->TransformSystem->Transforms[e].Position.z);

				// Entity id
				Graphics::Fonts::PrintText(EntityPosition->x + 20.0f, EntityPosition->y - 20.0f, 0.4f, std::string("ID: ") + std::to_string(e), Graphics::FontManager::GetFont(std::string("Bold")), TextBatch, 
															Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

				// Entity Position 
				auto X = EntityPosition->x;
				auto Y = EntityPosition->y;
				auto Z = EntityPosition->z;
				Graphics::Fonts::PrintText(	EntityPosition->x + 20.0f, 
											EntityPosition->y - 40.0f, 
											0.4f, std::string("<") + std::to_string(X) + std::string(", ") + std::to_string(Y) + (", ") + std::to_string(Z) + std::string(">"), 
											Graphics::FontManager::GetFont(std::string("Bold")), TextBatch, 
															Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

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
				EntityBatch.Add(Math::Vec4(EntityPosition->XY(), *EDims), ESpriteSheet->GetUV(0), ESpriteSheet->texture.id, *Color, EntityPosition->y, 
										World->TransformSystem->Transforms[e].Angle, World->Renderer2DSystem->Renderers[e].Format);
			}
			else
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

				EntityBatch.Add(Math::Vec4(EntityPosition->XY(), Dims), Sheet->GetUV(index), Sheet->texture.id, *Color, EntityPosition->y, World->TransformSystem->Transforms[e].Angle, 
									World->Renderer2DSystem->Renderers[e].Format);
			}

			Ground = &World->TransformSystem->Transforms[e].GroundPosition;
			auto EAABB = &World->TransformSystem->Transforms[e].AABB;
			if (World->Types[e] != ECS::Component::EntityType::ITEM)
			{
				EntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
										Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f), 1.0f);
	
				float XDiff = World->TransformSystem->Transforms[e].AABBPadding.x;
				float YDiff = World->TransformSystem->Transforms[e].AABBPadding.y;
				// Enjon::Math::Vec2 EAABBIsoMin(Enjon::Math::CartesianToIso(EAABB->Min) + Math::Vec2(XDiff / 2.0f, XDiff / 2.0f));
				Enjon::Math::Vec2 EAABBIsoMin(Enjon::Math::CartesianToIso(EAABB->Min));
				auto EAABBIsoMax(EM::CartesianToIso(EAABB->Max));
				// float EAABBHeight = EAABB->Max.y - EAABB->Min.y, EAABBWidth = EAABB->Max.x - EAABB->Min.y;
				// EntityBatch.Add(Math::Vec4(EAABBIsoMin, Math::Vec2(abs(EAABB->Max.x - EAABB->Min.x), abs(EAABB->Max.y - EAABB->Min.y))), 
				// 					Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
				// 					Graphics::SetOpacity(Graphics::RGBA16_Red(), 0.2f), EAABBIsoMin.y, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);
				// EntityBatch.Add(Math::Vec4(EAABBIsoMin.x, EAABBIsoMin.y, Math::Vec2(XDiff, YDiff)), 
				// 					Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
				// 					Graphics::SetOpacity(Graphics::RGBA16_Red(), 0.2f), EAABBIsoMin.y, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);
			}
		}


		// Draw player

		// Draw box
		// For a box, need 4 faces, so 4 quads
		// DrawBox(&EntityBatch, &EntityBatch, World);

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
					EG::SetOpacity(DashColor, 0.05f), EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/smoke_1.png").id, 0.025f, LightParticleBatch);
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
		Enjon::Math::Vec2 BoxCoords(Math::CartesianToIso(World->TransformSystem->Transforms[Player].CartesianPosition) + Math::Vec2(45.0f, 35.0f));
		float boxRadius = 10.0f;
		BoxCoords = BoxCoords - boxRadius * Math::CartesianToIso(Math::Vec2(cos(Math::ToRadians(AimAngle - 90)), sin(Math::ToRadians(AimAngle - 90))));
		EntityBatch.Add(Math::Vec4(BoxCoords, 100, 50), Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/vector_reticle.png").id, 
							Graphics::SetOpacity(Graphics::RGBA16_White(), 0.7f), 1.0f, Math::ToRadians(AimAngle + 45), Graphics::CoordinateFormat::ISOMETRIC);


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


		Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
		const Enjon::Graphics::ColorRGBA16* Color = &World->Renderer2DSystem->Renderers[Player].Color;
		Enjon::Math::Vec2* PlayerPosition = &World->TransformSystem->Transforms[Player].Position.XY();
		if (World->Animation2DSystem->Animations[Player].Sheet == EG::SpriteSheetManager::GetSpriteSheet("PlayerSheet2"))
		{
			dims = Math::Vec2(115.0f, 115.0f);
		}
		else dims = Math::Vec2(100.0f, 100.0f);

		// Draw player
		EntityBatch.Add(Math::Vec4(*PlayerPosition, dims), Sheet->GetUV(Frame), Sheet->texture.id, *Color, PlayerPosition->y - World->TransformSystem->Transforms[Player].Position.z);

		Enjon::Math::Vec2* A = &World->TransformSystem->Transforms[Player].CartesianPosition;
		Enjon::Physics::AABB* AABB = &World->TransformSystem->Transforms[Sword].AABB;
		Enjon::Math::Vec2 AV = World->TransformSystem->Transforms[Player].AttackVector;
		float XDiff = TILE_SIZE;
		Enjon::Math::Vec2 AABBIsomin(Enjon::Math::CartesianToIso(AABB->Min) + Math::Vec2(XDiff, 0.0f));
		Enjon::Math::Vec2 AABBIsomax(Enjon::Math::CartesianToIso(AABB->Max));
		float AABBHeight = AABB->Max.y - AABB->Min.y, AABBWidth = AABB->Max.x - AABB->Min.y;

		// Draw player ground tile 
		const Math::Vec2* GroundPosition = &World->TransformSystem->Transforms[Player].GroundPosition;
		EntityBatch.Add(Math::Vec4(GroundPosition->x, GroundPosition->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
									Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f));
		// Draw player shadow
		EntityBatch.Add(Math::Vec4(GroundPosition->x - 20.0f, GroundPosition->y - 20.0f, 45.0f, 128.0f), Sheet->GetUV(Frame), Sheet->texture.id,
									Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.3f), 1.0f, Enjon::Math::ToRadians(120.0f));

	
		///////////////////////////////
		// BEAMS //////////////////////	
		///////////////////////////////

		/*
		EG::ColorRGBA16 C = EG::RGBA16_ZombieGreen();
		EM::Vec2 Norm;

		// First segment
		{
			// Random verticle bar to test rotations
			EM::Vec2 BeamDims = BeamSegments.at(0).Dimensions;
			EM::Vec2 BeamPos = World->TransformSystem->Transforms[Player].Position.XY() + EM::Vec2(BeamDims.y / 2.0f + 60.0f, 40.0f);
			Enjon::Math::Vec2 MousePos = Input.GetMouseCoords();
			Camera.ConvertScreenToWorld(MousePos);
			MousePos = EM::IsoToCartesian(MousePos);
			BeamPos = EM::IsoToCartesian(BeamPos);
			EM::Vec2 R(1,0);
			Norm = EM::Vec2::Normalize(MousePos - BeamPos);
			auto a = acos(Norm.DotProduct(R)) * 180.0f / M_PI;
			if (Norm.y < 0) a *= -1;
			BeamPos = EM::CartesianToIso(BeamPos);
			auto BeamX = BeamPos.x;
			auto BeamY = BeamPos.y;
			auto Rad = 0.5f * BeamDims.x;
			BeamPos = BeamPos + Rad * EM::CartesianToIso(Math::Vec2(cos(EM::ToRadians(a)), sin(EM::ToRadians(a))));
			BeamPos = BeamPos + Rad * EM::CartesianToIso(Math::Vec2(cos(EM::ToRadians(a)), sin(EM::ToRadians(a))));
			BeamSegments.at(0).Position = BeamPos;
			BeamSegments.at(0).Angle = a;

			// Beam 1
			EntityBatch.Add(EM::Vec4(BeamPos, BeamDims), EM::Vec4(0, 0, 1, 1), 
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/VerticleBar.png").id, C, BeamPos.y, EM::ToRadians(a), EG::CoordinateFormat::ISOMETRIC);
		}



		// Rest of the segments
		{
			for (Enjon::uint32 i = 1; i < BeamSegments.size(); i++)
			{
				// Random verticle bar to test rotations
				EM::Vec2 BeamDims = BeamSegments.at(i).Dimensions;
				EM::Vec2 BeamPos = BeamSegments.at(i - 1).Position;
				float O = BeamSegments.at(0).Angle;
				float A = BeamSegments.at(i - 1).Angle;
				float B = BeamSegments.at(i).Angle;
				float Difference = BeamSegments.at(i - 1).Angle - BeamSegments.at(i).Angle;
				// if ((O >= 170 && B <= -180) || (O <= -170 && B >= 170)) { Difference *= -1;}
				float a;
				if (abs(Difference) > 290) a = B + Difference;
				else a = B + Difference * 0.5f;
				// a += ER::Roll(-7, 7);
				auto Rad = 0.5f * BeamDims.x;
				BeamDims.y += i * sin(t);
				BeamPos = BeamPos + Rad * EM::CartesianToIso(Math::Vec2(cos(EM::ToRadians(a)), sin(EM::ToRadians(a))));
				BeamPos = BeamPos + Rad * EM::CartesianToIso(Math::Vec2(cos(EM::ToRadians(a)), sin(EM::ToRadians(a))));
				BeamSegments.at(i).Position = BeamPos;
				BeamSegments.at(i).Angle = a;

				EntityBatch.Add(EM::Vec4(BeamPos, BeamDims), EM::Vec4(0, 0, 1, 1), 
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/VerticleBar.png").id, EG::SetOpacity(C, (sin(t) + 1.1f) / 2.0f), BeamPos.y, EM::ToRadians(a), EG::CoordinateFormat::ISOMETRIC);
			}	
		}
		*/

		/*
		{
			static float SegCounter = 0.0f;
			SegCounter += 0.1f;
			if (SegCounter > 5.0f)
			{
				printf("%.2f, %.2f\n", BeamSegments.at(0).Angle, BeamSegments.at(19).Angle);
				SegCounter = 0.0f;
			}
		}
		*/

		////////////////////////////////////////////////
		////////////////////////////////////////////////

		// Add the health bar for shiggles
		// float X = HUDCamera.GetPosition().x - 190.0f;
		// float Y = HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 60.0f;
		// HUDBatch.Add(EM::Vec4(X, Y, 400.0f, 10.0f),
		// 			 EM::Vec4(0, 0, 1, 1), 
		// 			  HealthSheet.texture.id, 
		// 			  EG::RGBA16_Red());

		auto F = EG::FontManager::GetFont("Bold");

		if (Paused)
		{
			// Draw paused text
			Enjon::Graphics::Fonts::PrintText(Camera.GetPosition().x - 110.0f, Camera.GetPosition().y - 30.0f, 1.0f, "Paused", F, TextBatch);
		}

		// Profiling info
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

		// // Add LightsToDraw
		Enjon::uint32 LightsSize = LightsToDraw.size(); 
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 160.0f, 
										0.4f, "LightsToDraw: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 160.0f, 
										0.4f, std::to_string(LightsSize), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// // LightZ
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 180.0f, 
										0.4f, "LightZ: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 180.0f, 
										0.4f, std::to_string(LightZ), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Entities
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 200.0f, 
										0.4f, "Entities: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 200.0f, 
										0.4f, std::to_string(World->Length), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Renderer size
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 220.0f, 
										0.4f, "Entities Drawn: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 220.0f, 
										0.4f, std::to_string(World->Renderer2DSystem->Entities.size()), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Collisions size
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 240.0f, 
										0.4f, "Collisions size: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 240.0f, 
										0.4f, std::to_string(World->CollisionSystem->Entities.size()), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// Transform run time
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 260.0f, 
										0.4f, "Transforms: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 260.0f, 
										0.4f, TransformTimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// AI run time
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 280.0f, 
										0.4f, "AI: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
		Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 280.0f, 
										0.4f, AITimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

		// // Draw Isometric compass
		// MapEntityBatch.Add(EM::Vec4(HUDCamera.GetPosition() - EM::Vec2(SCREENWIDTH / 2.0f - 30.0f, -SCREENHEIGHT / 2.0f + 250.0f), 150.0f, 75.0f), 
		// 				EM::Vec4(0, 0, 1, 1), EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Coordinates.png").id, EG::RGBA16_White());

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

		/*-- RANDOM DRAWING --*/

		// Add some random polygon
		// std::vector<EM::Vec2> Points;
		// Points.push_back(EM::Vec2(-1500, -1400));	// BL
		// Points.push_back(EM::Vec2(-800, -1400));	// BR
		// Points.push_back(EM::Vec2(-850, -1200));	// TR
		// Points.push_back(EM::Vec2(-900, -1200));	// TL
		// EntityBatch.AddPolygon(Points, EM::Vec4(0, 0, 1, 1), EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/verticlebar.png").id, EG::RGBA16_Orange(), Points.at(0).y, EG::CoordinateFormat::ISOMETRIC);

		// Add a random cube 
		// static EG::SpriteSheet* BoxSheet = EG::SpriteSheetManager::GetSpriteSheet("BoxSheet");
		// EM::Vec2 BoxPos = World->TransformSystem->Transforms[Player].Position.XY();
		// EntityBatch.Add(
		// 	EM::Vec4(BoxPos.x, BoxPos.y + 50.0f, 100, 100), 
		// 	BoxSheet->GetUV(0), 
		// 	BoxSheet->texture.id
		// 	EG::SetOpacity(EG::RGBA16_White(), 0.05f)
		// 	);
		// NormalsBatch.Add(
		// 	EM::Vec4(BoxPos.x, BoxPos.y + 50.0f, 100, 100), 
		// 	BoxSheet->GetUV(1),
		// 	BoxSheet->texture.id,
		// 	EG::SetOpacity(EG::RGBA16_White(), 0.5f)
		// 	);


		EntityBatch.End();
		TextBatch.End(); 
		LightBatch.End();
		MapEntityBatch.End(); 
		HUDBatch.End();
		NormalsBatch.End();
	
		// Diffuse and Position Rendering
		DiffuseFBO->Bind();
		{
			DiffuseShader->Use();
			{
				// Set up uniforms
				DiffuseShader->SetUniformMat4("model", model);
				DiffuseShader->SetUniformMat4("view", view);
				DiffuseShader->SetUniformMat4("projection", projection);

				// Draw ground tiles
				GroundTileBatch.RenderBatch();
				// Draw TileOverlays
				OverlayBatch.RenderBatch();
				// Draw entities		
				EntityBatch.RenderBatch();

				ParticleBatch.RenderBatch();
			}
			DiffuseShader->Unuse();
		}
		DiffuseFBO->Unbind();

		// Normals Rendering
		NormalsFBO->Bind();
		{
			NormalsShader->Use();
			{
				// Set up uniforms
				NormalsShader->SetUniformMat4("model", model);
				NormalsShader->SetUniformMat4("view", view);
				NormalsShader->SetUniformMat4("projection", projection);

				GroundTileNormalsBatch.RenderBatch();
				NormalsBatch.RenderBatch();
			}
			NormalsShader->Unuse();
		}
		NormalsFBO->Unbind();

		// Deferred Render
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE);
		DeferredFBO->Bind();
		{
			DeferredShader->Use();
			{
				static GLuint m_diffuseID 	= glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_diffuse");
				static GLuint m_normalsID  	= glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_normals");
				static GLuint m_positionID  = glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_position");

				EM::Vec3 CP = EM::Vec3(Camera.GetPosition(), 1.0f);

				// Bind diffuse
				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, DiffuseFBO->GetDiffuseTexture());
				glUniform1i(m_diffuseID, 0);

				// Bind normals
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, NormalsFBO->GetDiffuseTexture());
				glUniform1i(m_normalsID, 1);

				// Bind position
				glActiveTexture(GL_TEXTURE2);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, DiffuseFBO->GetPositionTexture());
				glUniform1i(m_positionID, 2);

				glUniform1i(glGetUniformLocation(DeferredShader->GetProgramID(), "NumberOfLights"), LightsToDraw.size());

				auto CameraScale = Camera.GetScale();
				for (GLuint i = 0; i < LightsToDraw.size(); i++)
				{
					auto L = LightsToDraw.at(i);

					glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Position").c_str()), L->Position.x, L->Position.y, L->Position.z + LightZ);
					glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Color").c_str()), L->Color.r, L->Color.g, L->Color.b, L->Color.a);
					glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Radius").c_str()), L->Radius / CameraScale);
					glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Falloff").c_str()), L->Falloff.x, L->Falloff.y, L->Falloff.z);
				}

				// Set uniforms
				glUniform2f(glGetUniformLocation(DeferredShader->GetProgramID(), "Resolution"),
							 SCREENWIDTH, SCREENHEIGHT);
				glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), "AmbientColor"), 0.3f, 0.5f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), "ViewPos"), CP.x, CP.y, CP.z);

				glUniformMatrix4fv(glGetUniformLocation(DeferredShader->GetProgramID(), "InverseCameraMatrix"), 1, 0, 
												Camera.GetCameraMatrix().Invert().elements);
				glUniformMatrix4fv(glGetUniformLocation(DeferredShader->GetProgramID(), "View"), 1, 0, 
												Camera.GetCameraMatrix().elements);
				glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), "Scale"), Camera.GetScale());


				// Render	
				{
					glBindVertexArray(quadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
			DeferredShader->Unuse();
		}
		DeferredFBO->Unbind();

		// Set blend function back to normalized
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Do any post processing here, of course...
		// Bind default buffer and render deferred render texture
		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.0, 0.0, 0.0, 0.0));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ScreenShader->Use();
		{
			DeferredBatch.Begin();
			{
				DeferredBatch.Add(
					EM::Vec4(-1, -1, 2, 2),
					EM::Vec4(0, 0, 1, 1), 
					DeferredFBO->GetDiffuseTexture()
					);
			}
			DeferredBatch.End();
			DeferredBatch.RenderBatch();
		}
		ScreenShader->Unuse();

		// Draw Text
		auto shader = Graphics::ShaderManager::GetShader("Text")->GetProgramID();
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

		// Draw Cursor
		// DrawCursor(&HUDBatch, &Input);



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
			AITimeString = std::to_string(AIRunTime);

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
	if (Input->IsKeyDown(SDLK_UP)){
		LightZ += 0.01f;
	}
	if (Input->IsKeyDown(SDLK_DOWN)){
		LightZ -= 0.01f;
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

void DrawBox(Enjon::Graphics::SpriteBatch* Batch, Enjon::Graphics::SpriteBatch* LightBatch, ECS::Systems::EntityManager* Manager)
{
	/*
		   ------
		 /     /|
		/     /	|
	   -------  /
	   |     | /
	   |_____|/


	*/

	EM::Vec3* PP = &Manager->TransformSystem->Transforms[Manager->Player].Position;
	EM::Vec2 Pos = PP->XY() + EM::Vec2(42.0f, 20.0f - PP->z); 

	float s = 20.0f;
	static EM::Vec2 Dims(s, s);
	static EM::Vec2 LightDims(300.0f, 300.0f);
	static float Height = 40.0f;
	static float t = 0.0f;
	t += 1.0f;
	Height += sin(0.05 * t) * 5.0f;
	float angle = EM::ToRadians(t / 2.0f);
	float angle2 = EM::ToRadians(t / 2.0f + 120.0f);
	float RX = sin(t);
	float RY = sin(0.05f * t) * 1.0f;
	static GLuint TexID = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box.png").id;
	static GLuint LightId = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg-light-small.png").id;
	EG::ColorRGBA16 Color = EG::RGBA16_White();
	float Rad = 80.0f;
	EM::Vec2 BoxPos = Pos + Rad * EM::CartesianToIso(Math::Vec2(cos(angle), sin(angle)));

	Color.r += 10.0f;
	Color.g += 10.0f;
	// Color.r += sin(0.005f * t) * 5.0f;
	// Color.g += sin(0.005f * t) * 5.0f;
	float alpha = Color.r > 0 ? 0.1f : 0.0f;

	// Player shadow
	EG::SpriteSheet* PS = Manager->Animation2DSystem->Animations[Manager->Player].Sheet;
	Enjon::uint32 CurrentFrame = Manager->Animation2DSystem->Animations[Manager->Player].CurrentFrame + Manager->Animation2DSystem->Animations[Manager->Player].BeginningFrame;
	EM::Vec2 BeamDims(100.0f, 150.0f);
	EM::Vec2 BeamPos = Pos + EM::Vec2(10.0f, -10.0f);
	BeamPos = EM::IsoToCartesian(BeamPos);
	EM::Vec2 R(1,0);
	auto Norm = EM::Vec2::Normalize(EM::IsoToCartesian(BoxPos) - BeamPos);
	auto a = acos(Norm.DotProduct(R)) * 180.0f / M_PI;
	// a += 90.0f;
	if (Norm.y < 0) a *= -1;
	BeamPos = EM::CartesianToIso(BeamPos);
	auto PRad = 75.0f + Height / 2.5f + PP->z * 2.0f;
	BeamPos = BeamPos + PRad * EM::CartesianToIso(Math::Vec2(cos(EM::ToRadians(a + 180)), sin(EM::ToRadians(a + 180))));

	// Player Shadow
	Batch->Add(EM::Vec4(BeamPos - Manager->TransformSystem->Transforms[Manager->Player].Position.z, BeamDims + EM::Vec2(Height / 2.0f, Height + PP->z)), 
		PS->GetUV(CurrentFrame), PS->texture.id, EG::SetOpacity(EG::RGBA16_Black(), Color.r / 20.0f - Height / 750.0f),
				BeamPos.y, EM::ToRadians(a + 90.0f), EG::CoordinateFormat::ISOMETRIC);

	// Light box
	Batch->Add(EM::Vec4(BoxPos.x, BoxPos.y + Height, Dims.x, Dims.y + RY), EM::Vec4(0, 0, 1, 1), TexID, EG::RGBA16(Color.r + 100.0f, Color.g + 100.0f, Color.b, Color.a), BoxPos.y, angle);

	// Shadow
	Batch->Add(EM::Vec4(BoxPos.x, BoxPos.y, Dims.x / 100.0f - Height / 2.0f, Dims.y / 100.0f - Height / 2.0f), EM::Vec4(0, 0, 1, 1), TexID, EG::SetOpacity(EG::RGBA16_Black(), 0.2f + Color.r / 30.0f - Height / 450.0f), BoxPos.y, angle, EG::CoordinateFormat::ISOMETRIC);


	// Light haze
	LightBatch->Add(EM::Vec4(BoxPos.x - LightDims.x / 2.0f, BoxPos.y - LightDims.y / 2.0f + Height, LightDims), EM::Vec4(0, 0, 1, 1), LightId, EG::SetOpacity(Color, Color.r / 60.0f), BoxPos.y);

	// Base light
	LightBatch->Add(EM::Vec4(BoxPos.x, BoxPos.y, 500.0f - Height, 500.0f - Height), EM::Vec4(0, 0, 1, 1), LightId, EG::SetOpacity(Color, Color.r / 55.0f - Height / 2500.0f), BoxPos.y, 0.0f, EG::CoordinateFormat::ISOMETRIC);
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
void GetLights(EG::Camera2D* Camera, std::vector<Light>* Lights, std::vector<Light*>& LightsToDraw)
{
	for (auto& L : *Lights)
	{
		auto R = L.Radius;
		auto D = 2 * R;
		EM::Vec2 dimensions(D, D);
		auto P = EM::Vec2(L.Position.x - D, L.Position.y - D);

		 
		Enjon::Math::Vec2 scaledScreenDimensions = Enjon::Math::Vec2((float)SCREENWIDTH, (float)SCREENHEIGHT) / (Camera->GetScale() * 0.37f);
		float MIN_DISTANCE_X = dimensions.x / 2.0f + scaledScreenDimensions.x / 2.0f;
		float MIN_DISTANCE_Y = dimensions.y / 2.0f + scaledScreenDimensions.y / 2.0f;

		//Center position of parameters passed in
		Enjon::Math::Vec2 centerPos = P + dimensions / 2.0f;	
		//Center position of camera
		Enjon::Math::Vec2 centerCameraPos = Camera->GetPosition();
		//Distance vector between two center positions
		Enjon::Math::Vec2 distVec = centerPos - centerCameraPos;

		float xDepth = MIN_DISTANCE_X - abs(distVec.x);
		float yDepth = MIN_DISTANCE_Y - abs(distVec.y);
		
		if (xDepth > yDepth && yDepth > 0) LightsToDraw.push_back(&L);
	}
}


#endif 


#if 0
/**
*  UNIT TESTS
*/

/*-- External/Engine Libraries includes --*/
#include <Enjon.h>

/*-- Standard Library includes --*/
#include <iostream>
#include <sstream>
#include <string>

// Button class
class Button
{
	public:
		EGUI::Signal<> on_click;
};

// Slider class
class Slider
{
	public:
		EGUI::Signal<> on_slide_right;
		EGUI::Signal<> on_slide_left;
};

// TextBox
class TextBox
{
	public:
		TextBox(std::string n) : Name(n) 
		{ 
			text = ""; 

			this->text.on_change().connect([&](std::string S)
			{
				std::cout << this->Name << " changed to: " << S << std::endl;
			});
		}

		void SetText(std::string S) { this->text = S; }

	public:
		EGUI::Property<std::string> text;

	private:
		std::string Name;
};


#undef main
int main(int argc, char** argv)
{
	// Create button
	Button UpButton;
	Button DownButton;

	// Create slider
	Slider slider;

	// Create TextBox
	TextBox TB("Delays");
	TextBox TB2("Other Delays");

	std::vector<TextBox*> TextBoxes;
	TextBoxes.push_back(&TB);
	TextBoxes.push_back(&TB2);

	// Create Property
	EGUI::Property<float> DelayValue;

	// Connect value of text to delay value
	DelayValue.connect_from(::atof(TB.text.get().c_str()));

	// Connect button click to setting textbox
	UpButton.on_click.connect([&]()
	{
		for (auto T : TextBoxes)
		{
			// Get value of string
			float V = ::atof(T->text.get().c_str());

			// Increment by 1
			V += 1.0f;

			// Set back to text
			T->text = std::to_string(V);
		}
	});


	UpButton.on_click.emit();

	return 0;
}




#endif

/**
* SYSTEMS TEST
*/

#if 1

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
	#define SCREENRES    EG::FULLSCREEN
#else
	#define SCREENWIDTH  1024
	#define SCREENHEIGHT 768
	#define SCREENRES EG::DEFAULT
#endif 

/*-- External/Engine Libraries includes --*/
#include <Enjon.h>
#include <sajson/sajson.h>

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
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "SpatialHash.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <stdio.h>
#include <iostream> 
#include <time.h>
#include <stdlib.h>
#include <vector>

using namespace ECS;
using namespace Systems;
using namespace spine;

namespace Enjon { namespace GUI {

	// Something like this eventually for global gui references...
	namespace ButtonManager
	{
		std::unordered_map<std::string, GUIButton*> Buttons;

		void Add(std::string S, GUIButton* B)
		{
			Buttons[S] = B;
		}

		GUIButton* Get(const std::string S)
		{
			auto search = Buttons.find(S);
			if (search != Buttons.end())
			{
				return search->second;
			}	

			return nullptr;
		}
	};

	// This is stupid, but it's for testing...
	namespace TextBoxManager
	{
		std::unordered_map<std::string, GUITextBox*> TextBoxes;

		void Add(std::string S, GUITextBox* T)
		{
			TextBoxes[S] = T;
		}

		GUITextBox* Get(const std::string S)
		{
			auto search = TextBoxes.find(S);
			if (search != TextBoxes.end())
			{
				return search->second;
			}
			return nullptr;
		}
	};

	namespace GUIManager
	{
		std::unordered_map<std::string, GUIElementBase*> Elements;

		void Add(std::string S, GUIElementBase* E)
		{
			Elements[S] = E;
		}

		GUIElementBase* Get(const std::string S)
		{
			auto search = Elements.find(S);
			if (search != Elements.end())
			{
				return search->second;
			}

			return nullptr;
		}
	}
}}

namespace CameraManager
{
	std::unordered_map<std::string, EG::Camera2D*> Cameras;

	void AddCamera(std::string S, EG::Camera2D* C)
	{
		Cameras[S] = C;
	}

	EG::Camera2D* GetCamera(const std::string S)
	{
		auto search = Cameras.find(S);
		if (search != Cameras.end())
		{
			return search->second;
		}
		return nullptr;
	}
};

struct SceneNode
{

};

class SceneGraph
{
	public:

	private:
};

namespace CursorManager
{
	std::unordered_map<std::string, SDL_Cursor*> Cursors;

	void Init()
	{
		Cursors["Arrow"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		Cursors["IBeam"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	}

	SDL_Cursor* Get(const std::string S)
	{
		auto search = Cursors.find(S);
		if (search != Cursors.end())
		{
			return search->second;
		}
		return nullptr;
	}

};

// Just need to get where I can group together GUIElements, transform them together, and then
// access individual GUIElements with the mouse
using namespace Enjon;
using namespace GUI;

/* Function Declarations */
bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera);
ImageFrame GetImageFrame(const sajson::value& Frame, const std::string Name);
void DrawFrame(const ImageFrame& Image, EM::Vec2 Position, const Atlas& A, EG::SpriteBatch* Batch, const EG::ColorRGBA16& Color = EG::RGBA16_White());
Anim CreateAnimation(const std::string& AnimName, const sajson::value& FramesDoc);
void CalculateAABBWithParent(EP::AABB* A, GUIButton* Button);
void DrawCursor(Enjon::Graphics::SpriteBatch* Batch, Enjon::Input::InputManager* InputManager);
bool IsModifier(unsigned int Key);


const std::string AnimTextureDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.png");
const std::string AnimTextureJSONDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.json");
const std::string AnimationDir("../IsoARPG/Profiles/Animations/Player/PlayerAttackOHLSEAnimation.json");

GUIElementBase* SelectedGUIElement = nullptr;
GUIElementBase* KeyboardFocus = nullptr;
GUIElementBase* MouseFocus = nullptr;

EG::GLTexture MouseTexture;

#undef main
int main(int argc, char** argv) {

	Enjon::Init();

	float t = 0.0f;
	float FPS = 0.0f;
	float TimeIncrement = 0.0f;

	// Create a window
	EG::Window Window;
	Window.Init("Unit Test", SCREENWIDTH, SCREENHEIGHT, SCREENRES);
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::SHOW);

	// Init cursor manager
	CursorManager::Init();

	// Set to default cursor
	SDL_SetCursor(CursorManager::Get("Arrow"));

	EU::FPSLimiter Limiter;
	Limiter.Init(60);

	// Init ShaderManager
	EG::ShaderManager::Init(); 

	// Init FontManager
	EG::FontManager::Init();

	// Shader for frame buffer
	EG::GLSLProgram* BasicShader	= EG::ShaderManager::GetShader("Basic");
	EG::GLSLProgram* TextShader		= EG::ShaderManager::GetShader("Text");  

	// UI Batch
	EG::SpriteBatch* UIBatch = new EG::SpriteBatch();
	UIBatch->Init();

	EG::SpriteBatch* SceneBatch = new EG::SpriteBatch();
	SceneBatch->Init();

	EG::SpriteBatch* BGBatch = new EG::SpriteBatch();
	BGBatch->Init();

	const float W = SCREENWIDTH;
	const float H = SCREENHEIGHT;

	// Create Camera
	EG::Camera2D* Camera = new EG::Camera2D;
	Camera->Init(W, H);

	// Create HUDCamera
	EG::Camera2D* HUDCamera = new EG::Camera2D;
	HUDCamera->Init(W, H);

	// Register cameras with manager
	CameraManager::AddCamera("HUDCamera", HUDCamera);
	CameraManager::AddCamera("SceneCamera", Camera);

	// InputManager
	EI::InputManager Input = EI::InputManager();

	// Matricies for shaders
	EM::Mat4 Model, View, Projection;

    using sajson::literal;
    std::string json = EU::read_file_sstream(AnimTextureJSONDir.c_str());
    const sajson::document& doc = sajson::parse(sajson::string(json.c_str(), json.length()));

    if (!doc.is_valid())
    {
        std::cout << "Invalid json: " << doc.get_error_message() << std::endl;;
    }


    // Get root and length of json file
    const auto& root = doc.get_root();
    const auto len = root.get_length();

    // // Get handle to meta deta
    const auto meta = root.find_object_key(literal("meta"));
    assert(meta < len);
    const auto& Meta = root.get_object_value(meta);

    // // Get handle to frame data
    const auto frames = root.find_object_key(literal("frames"));
    assert(frames < len);
    const auto& Frames = root.get_object_value(frames);

    // // Get image size
    auto ISize = Meta.get_value_of_key(literal("size"));
    float AWidth = ISize.get_value_of_key(literal("w")).get_safe_float_value();
    float AHeight = ISize.get_value_of_key(literal("h")).get_safe_float_value();

    Atlas atlas = {	EM::Vec2(AWidth, AHeight), 
    				EI::ResourceManager::GetTexture(AnimTextureDir.c_str())
    			  };

	// Create animation
	Anim Test = CreateAnimation(std::string("Player_Attack_OH_L_SE"), Frames);

	// Set up mouse texture to default
	MouseTexture = EI::ResourceManager::GetTexture("../assets/Textures/mouse_cursor_20.png");

	////////////////////////////////
	// ANIMATION EDITOR ////////////

	GUIButton 			PlayButton;
	GUIButton 			NextFrame;
	GUIButton 			PreviousFrame;
	GUIButton 			OffsetUp;
	GUIButton 			OffsetDown;
	GUIButton 			OffsetLeft;
	GUIButton 			OffsetRight;
	GUIButton 			DelayUp;
	GUIButton 			DelayDown;
	GUIButton			ToggleOnionSkin;
	GUITextBox 			InputText;
	GUIAnimationElement SceneAnimation;

	// Set up ToggleOnionSkin
	ToggleOnionSkin.Type = GUIType::BUTTON;
	ToggleOnionSkin.State = ButtonState::INACTIVE;

	// Set up Scene Animtation
	SceneAnimation.CurrentAnimation = &Test;
	SceneAnimation.CurrentIndex = 0;
	SceneAnimation.Position = EM::Vec2(0.0f);
	SceneAnimation.State = ButtonState::INACTIVE;
	SceneAnimation.HoverState = HoveredState::OFF_HOVER;
	SceneAnimation.Type = GUIType::SCENE_ANIMATION;

	// Set up text box's text
	InputText.Text = std::string("");
	InputText.CursorIndex = 0;

	EG::ColorRGBA16 PlayButtonColor = EG::RGBA16_White();
	EG::ColorRGBA16 InputTextColor = EG::RGBA16(0.05f, 0.05f, 0.05f, 0.4f);

	PlayButton.Type = GUIType::BUTTON;
	InputText.Type = GUIType::TEXTBOX;

	GUIGroup Group;
	Group.Position = EM::Vec2(0.0f, -200.0f);

	// Add PlayButton to Group
	GUI::AddToGroup(&Group, &PlayButton);
	GUI::AddToGroup(&Group, &NextFrame);
	GUI::AddToGroup(&Group, &PreviousFrame);
	GUI::AddToGroup(&Group, &InputText);

	// Set up play button image frames
	PlayButton.Frames.push_back(GetImageFrame(Frames, "playbuttonup"));
	PlayButton.Frames.push_back(GetImageFrame(Frames, "playbuttondown"));

	// Set up PlayButton offsets
	{
		auto xo = 0.0f;
		auto yo = 0.0f;
		PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.x = xo;
		PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.y = yo;
		PlayButton.Frames.at(ButtonState::ACTIVE).Offsets.x = xo;
		PlayButton.Frames.at(ButtonState::ACTIVE).Offsets.y = yo;
	}


	// Set up PlayButton position within the group
	PlayButton.Position = EM::Vec2(10.0f, 20.0f);

	// Set state to inactive
	PlayButton.State = ButtonState::INACTIVE;
	PlayButton.HoverState = HoveredState::OFF_HOVER;

	// Set up Scaling Factor
	PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor = 1.0f;
	PlayButton.Frames.at(ButtonState::ACTIVE).ScalingFactor = 1.0f;

	// Set up PlayButton AABB
	PlayButton.AABB.Min =  EM::Vec2(PlayButton.Position.x + Group.Position.x + PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.x * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor,
									PlayButton.Position.y + Group.Position.y + PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.y * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor); 
	PlayButton.AABB.Max = PlayButton.AABB.Min + EM::Vec2(PlayButton.Frames.at(ButtonState::INACTIVE).SourceSize.x * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor, 
										     PlayButton.Frames.at(ButtonState::INACTIVE).SourceSize.y * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor);

	// Set up InputText position within the group
	InputText.Position = EM::Vec2(100.0f, 60.0f);

	// Set states to inactive
	InputText.State = ButtonState::INACTIVE;
	InputText.HoverState = HoveredState::OFF_HOVER;

	// Set up InputText AABB
	// This will be dependent on the size of the text, or it will be static, or it will be dependent on some image frame
	InputText.AABB.Min = InputText.Position + Group.Position;
	InputText.AABB.Max = InputText.AABB.Min + EM::Vec2(200.0f, 20.0f);

	// Calculate Group's AABB by its children's AABBs 
	Group.AABB.Min = Group.Position;
	// Figure out height
	auto GroupHeight = InputText.AABB.Max.y - Group.AABB.Min.y;
	auto GroupWidth = InputText.AABB.Max.x - Group.AABB.Min.x;
	Group.AABB.Max = Group.AABB.Min + EM::Vec2(GroupWidth, GroupHeight);

	// Set up ToggleOnionSkin's on_click signal
	ToggleOnionSkin.on_click.connect([&]()
	{
		std::cout << "Emiting onion skin..." << std::endl;

		ToggleOnionSkin.State = ToggleOnionSkin.State == ButtonState::INACTIVE ? ButtonState::ACTIVE : ButtonState::INACTIVE;
	});

	// Set up SceneAnimation's on_hover signal
	SceneAnimation.on_hover.connect([&]()
	{
		SceneAnimation.HoverState = HoveredState::ON_HOVER;
	});

	// Set up SceneAnimation's off_hover signal
	SceneAnimation.off_hover.connect([&]()
	{
		SceneAnimation.HoverState = HoveredState::OFF_HOVER;
	});

	// Set up InputText's on_click signal
	InputText.on_click.connect([&]()
	{
		// For now, just change the text to empty string
		// Will need to eventually set this box to the selected component, then hot-swap in input manager controls for this particular gui-element
		// Find where the cursor is by the mouse
	});

	InputText.on_backspace.connect([&]()
	{
		auto str_len = InputText.Text.length();
		auto cursor_index = InputText.CursorIndex;

		// erase from string
		if (str_len > 0)
		{
			InputText.Text.erase(cursor_index - 1);
			InputText.CursorIndex--;
		}
	});

	InputText.on_keyboard.connect([&](std::string c)
	{
		auto str_len = InputText.Text.length();
		auto cursor_index = InputText.CursorIndex;

		// std::cout << cursor_index << std::endl;

		// End of string
		if (cursor_index >= str_len)
		{
			InputText.Text += c;
			InputText.CursorIndex = str_len + 1;
		}
		// Cursor somewhere in the middle of the string
		else if (cursor_index > 0)
		{
			auto FirstHalf = InputText.Text.substr(0, cursor_index);
			auto SecondHalf = InputText.Text.substr(cursor_index, str_len);

			FirstHalf += c; 
			InputText.Text = FirstHalf + SecondHalf;
			InputText.CursorIndex++;
		}
		// Beginning of string
		else
		{
			InputText.Text = c + InputText.Text;
			InputText.CursorIndex++;
		}
	});

	// Set up InputText's on_hover signal
	InputText.on_hover.connect([&]()
	{
		// Change the mouse cursor
		SDL_SetCursor(CursorManager::Get("IBeam"));

		InputText.HoverState = HoveredState::ON_HOVER;

		// Change color of Box
		InputTextColor = EG::SetOpacity(EG::RGBA16_LightGrey(), 0.3f);

	});

	// Set up InputText's off_hover signal
	InputText.off_hover.connect([&]()
	{
		// Change mouse cursor back to defaul
		SDL_SetCursor(CursorManager::Get("Arrow"));

		InputText.HoverState = HoveredState::OFF_HOVER;
	
		// Change color of Box
		InputTextColor = EG::RGBA16(0.05f, 0.05f, 0.05f, 0.4f);
	});

	// Set up PlayButton's on_hover signal
	PlayButton.on_hover.connect([&]()
	{
		// We'll just change a color for now
		PlayButtonColor = EG::RGBA16_White();

		// Set state to active
		PlayButton.HoverState = HoveredState::ON_HOVER;
	});

	// Set up PlayButton's off_hover signal
	PlayButton.off_hover.connect([&]()
	{
		PlayButtonColor = EG::RGBA16_LightGrey();

		// Set state to inactive
		PlayButton.HoverState = HoveredState::OFF_HOVER;
	});

	// Set up PlayButton's signal
	PlayButton.on_click.connect([&]()
	{
		if (TimeIncrement <= 0.0f) 
		{
			TimeIncrement = 0.15f;
			PlayButton.State = ButtonState::ACTIVE;
		}

		else 
		{
			TimeIncrement = 0.0f;
			PlayButton.State = ButtonState::INACTIVE;
		}
	});

	// Set up NextFrame's signal
	NextFrame.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Animation, which in this case is just Test
		auto CurrentIndex = SceneAnimation.CurrentIndex;
		SceneAnimation.CurrentIndex = (CurrentIndex + 1) % SceneAnimation.CurrentAnimation->TotalFrames;

		// And set t = -1.0f for safety
		t = -1.0f;
	});

	// Set up PreviousFrame's signal
	PreviousFrame.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Animation, which in this case is just Test
		auto CurrentIndex = SceneAnimation.CurrentIndex;
		if (CurrentIndex > 0) SceneAnimation.CurrentIndex -= 1;

		// Bounds check
		else SceneAnimation.CurrentIndex = SceneAnimation.CurrentAnimation->TotalFrames - 1;

		// And set t = -1.0f for safety
		t = -1.0f;
	});

	// Set up OffsetUp's signal
	OffsetUp.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Frame
		auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

		// Get CurrentFrame's YOffset
		auto YOffset = CurrentFrame->Offsets.y;

		// Increment by arbitrary amount...
		YOffset += 1.0f;

		// Reset offset
		CurrentFrame->Offsets.y = YOffset;
	});

	// Set up OffsetDown's signal
	OffsetDown.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Frame
		auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

		// Get CurrentFrame's YOffset
		auto YOffset = CurrentFrame->Offsets.y;

		// Increment by arbitrary amount...
		YOffset -= 1.0f;

		// Reset offset
		CurrentFrame->Offsets.y = YOffset;
	});

	// Set up OffsetLeft's signal
	OffsetLeft.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Frame
		auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

		// Get CurrentFrame's YOffset
		auto XOffset = CurrentFrame->Offsets.x;

		// Increment by arbitrary amount...
		XOffset -= 1.0f;

		// Reset offset
		CurrentFrame->Offsets.x = XOffset;
	});

	// Set up OffsetRight's signal
	OffsetRight.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Frame
		auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

		// Get CurrentFrame's YOffset
		auto XOffset = CurrentFrame->Offsets.x;

		// Increment by arbitrary amount...
		XOffset += 1.0f;

		// Reset offset
		CurrentFrame->Offsets.x = XOffset;
	});

	// Set up DelayUp's signal
	DelayUp.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Frame
		auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

		// Get CurrentFrame's Delay
		auto Delay = CurrentFrame->Delay;

		// Increment by arbitrary amount...
		Delay += 0.1f;

		// Reset Delay
		CurrentFrame->Delay = Delay;
	});

	// Set up DelayUp's signal
	DelayDown.on_click.connect([&]()
	{
		// If playing, then stop the time
		if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

		PlayButton.State = ButtonState::INACTIVE;

		// Get Current Frame
		auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

		// Get CurrentFrame's Delay
		auto Delay = CurrentFrame->Delay;

		// Increment by arbitrary amount...
		if (Delay > 0.0f) Delay -= 0.1f;

		// Reset Delay
		CurrentFrame->Delay = Delay;
	});

	// Put into button manager map
	ButtonManager::Add("PlayButton", &PlayButton);
	ButtonManager::Add("NextFrame", &NextFrame);
	ButtonManager::Add("PreviousFrame", &PreviousFrame);
	ButtonManager::Add("OffsetUp", &OffsetUp);
	ButtonManager::Add("OffsetDown", &OffsetDown);
	ButtonManager::Add("OffsetLeft", &OffsetLeft);
	ButtonManager::Add("OffsetRight", &OffsetRight);
	ButtonManager::Add("DelayUp", &DelayUp);
	ButtonManager::Add("DelayDown", &DelayDown);


	// Put into textbox manager
	TextBoxManager::Add("InputText", &InputText);

	GUIManager::Add("PlayButton", &PlayButton);
	GUIManager::Add("NextFrame", &NextFrame);
	GUIManager::Add("PreviousFrame", &PreviousFrame);
	GUIManager::Add("OffsetUp", &OffsetUp);
	GUIManager::Add("OffsetDown", &OffsetDown);
	GUIManager::Add("OffsetLeft", &OffsetLeft);
	GUIManager::Add("OffsetRight", &OffsetRight);
	GUIManager::Add("DelayUp", &DelayUp);
	GUIManager::Add("DelayDown", &DelayDown);
	GUIManager::Add("InputText", &InputText);
	GUIManager::Add("SceneAnimation", &SceneAnimation);
	GUIManager::Add("ToggleOnionSkin", &ToggleOnionSkin);

	// Draw BG
	BGBatch->Begin();
	BGBatch->Add(
					EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
					EM::Vec4(0, 0, 1, 1),
					EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg.png").id,
					EG::SetOpacity(EG::RGBA16_SkyBlue(), 0.4f)
				);
	BGBatch->Add(
					EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
					EM::Vec4(0, 0, 1, 1),
					EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg.png").id,
					EG::SetOpacity(EG::RGBA16_White(), 0.3f)
				);
	BGBatch->Add(
					EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
					EM::Vec4(0, 0, 1, 1),
					EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg_cross.png").id,
					EG::SetOpacity(EG::RGBA16_White(), 0.1f)
				);
	BGBatch->End();


	// Main loop
	bool running = true;
	while (running)
	{
		Limiter.Begin();

		// Keep track of animation delays
		t += TimeIncrement;

		// Check for quit condition
		running = ProcessInput(&Input, Camera);

		// Update cameras
		Camera->Update();
		HUDCamera->Update();

		// Set up AABB of Scene Animation
		EGUI::AnimationElement::AABBSetup(&SceneAnimation);

		// Update input
		Input.Update();

		// Set up necessary matricies
    	Model = EM::Mat4::Identity();	
    	View = HUDCamera->GetCameraMatrix();
    	Projection = EM::Mat4::Identity();

		/////////////////////////////////
		// RENDERING ////////////////////
		/////////////////////////////////

		//Enable alpha blending
		glEnable(GL_BLEND);

		//Set blend function type
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0));
		// Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.02f, 0.0f, 0.05f, 1.0f));

		// Basic shader for UI
		BasicShader->Use();
		{
			BasicShader->SetUniformMat4("model", Model);
			BasicShader->SetUniformMat4("projection", Projection);
			BasicShader->SetUniformMat4("view", View);

			// Draw BG
			BGBatch->RenderBatch();

			UIBatch->Begin();
			{
				// Draw Parent
				auto Parent = PlayButton.Parent;

				UIBatch->Add(
								EM::Vec4(Parent->AABB.Min, Parent->AABB.Max - Parent->AABB.Min),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::SetOpacity(EG::RGBA16_Blue(), 0.05f)
							);

				// Draw Play button
				auto PBF = PlayButton.Frames.at(PlayButton.State);
				// Calculate these offsets
				DrawFrame(PBF, PlayButton.Position + Parent->Position, atlas, UIBatch, PlayButtonColor);

				// Draw PlayButton AABB
				// Calculate the AABB (this could be set up to another signal and only done when necessary)
				// std::cout << PlayButton.AABB.Max - PlayButton.AABB.Min << std::endl;
				CalculateAABBWithParent(&PlayButton.AABB, &PlayButton);
				UIBatch->Add(
								EM::Vec4(PlayButton.AABB.Min, PlayButton.AABB.Max - PlayButton.AABB.Min), 
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::SetOpacity(EG::RGBA16_Red(), 0.3f)
							);

				// CalculateAABBWithParent(&InputText.AABB, &InputText);
				UIBatch->Add(
								EM::Vec4(InputText.AABB.Min, InputText.AABB.Max - InputText.AABB.Min), 
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								InputTextColor
							);
			}
			UIBatch->End();
			UIBatch->RenderBatch();

			View = Camera->GetCameraMatrix();
			BasicShader->SetUniformMat4("view", View);

			SceneBatch->Begin();
			{
				// Draw AABB of current frame
				auto CurrentAnimation = SceneAnimation.CurrentAnimation;
				auto CurrentIndex = SceneAnimation.CurrentIndex;
				auto Frame = &CurrentAnimation->Frames.at(CurrentIndex);
				auto TotalFrames = CurrentAnimation->TotalFrames;
				auto& Position = SceneAnimation.Position;

				if (SceneAnimation.HoverState == HoveredState::ON_HOVER)
				{
					auto AABB_SA = &SceneAnimation.AABB;
					SceneBatch->Add(
								EM::Vec4(AABB_SA->Min, AABB_SA->Max - AABB_SA->Min), 
								EM::Vec4(0,0,1,1), 
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/selection_box.png").id,
								EG::RGBA16_Red()
							  );
				}

				if (t >= Frame->Delay)
				{
					CurrentIndex = (CurrentIndex + 1) % TotalFrames;
					SceneAnimation.CurrentIndex = CurrentIndex;
					Frame = &CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);
					t = 0.0f;
				}

				// Check for onion skin being on
				if (ToggleOnionSkin.State == ButtonState::ACTIVE)
				{
					auto PreviousIndex = 0;

					// Draw the scene previous and after
					if (CurrentIndex > 0)
					{
						PreviousIndex = CurrentIndex - 1;
					}	
					else
					{
						PreviousIndex = TotalFrames - 1;
					}

					auto NextFrame = &CurrentAnimation->Frames.at((SceneAnimation.CurrentIndex + 1) % TotalFrames);
					auto PreviousFrame = &CurrentAnimation->Frames.at(PreviousIndex);

					DrawFrame(*PreviousFrame, Position, atlas, SceneBatch, EG::SetOpacity(EG::RGBA16_Blue(), 0.3f));
					DrawFrame(*NextFrame, Position, atlas, SceneBatch, EG::SetOpacity(EG::RGBA16_Red(), 0.3f));
				}

				// Draw Scene animation
				DrawFrame(*Frame, Position,	atlas, SceneBatch);

			}
			SceneBatch->End();
			SceneBatch->RenderBatch();

		}
		BasicShader->Unuse();

		// Shader for text
		TextShader->Use();
		{
			View = HUDCamera->GetCameraMatrix();

			TextShader->SetUniformMat4("model", Model);
			TextShader->SetUniformMat4("projection", Projection);
			TextShader->SetUniformMat4("view", View);

			UIBatch->Begin();
			{
				// Get font for use
				auto CurrentFont = EG::FontManager::GetFont("WeblySleek");
				auto XOffset = 110.0f;
				auto scale = 1.0f;

				EG::Fonts::PrintText(HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 30.0f, scale, std::string("FPS: "), CurrentFont, *UIBatch, 
												EG::SetOpacity(EG::RGBA16_White(), 0.8f));
				EG::Fonts::PrintText(HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 50.0f, HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 30.0f, scale, std::to_string((uint32_t)FPS), CurrentFont, *UIBatch, 
												EG::SetOpacity(EG::RGBA16_White(), 0.8f));

				auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

				// Display current frame information
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 70.0f, scale, 
										std::string("Animation: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				// Display current frame information
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 70.0f, scale, 
										Test.Name, 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				// Current Frame Name
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 90.0f, scale, 
										std::string("Frame: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 90.0f, scale, 
										std::to_string(SceneAnimation.CurrentIndex), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				// Current Frame Delay
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 110.0f, scale, 
										std::string("Delay: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 110.0f, scale, 
										std::to_string(CurrentFrame->Delay), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				// Current Frame Y offset
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 130.0f, scale, 
										std::string("Y Offset: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 130.0f, scale, 
										std::to_string(static_cast<int32_t>(CurrentFrame->Offsets.y)), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				// Current Frame X Offset
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 150.0f, scale, 
										std::string("X Offset: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 150.0f, scale, 
										std::to_string(static_cast<int32_t>(CurrentFrame->Offsets.x)), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);

				// Print out text box's text w/ shadow
				// Could totally load these styles from JSON, which would be a cool way to add themes to the editor
				auto Padding = EM::Vec2(5.0f, 5.0f);
				auto ITextHeight = InputText.AABB.Max.y - InputText.AABB.Min.y; // InputTextHeight
				auto TextHeight = ITextHeight - 20.0f;
				EG::Fonts::PrintText(	
										InputText.Position.x + InputText.Parent->Position.x + Padding.x, 
										InputText.Position.y + InputText.Parent->Position.y + Padding.y + TextHeight, 1.0f, 
										InputText.Text, 
										EG::FontManager::GetFont("WeblySleek"), 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				static float caret_count = 0.0f;
				caret_count += 0.1f;
				static bool caret_on = true;

				if (caret_count >= 4.0f)
				{
					caret_count = 0.0f;
					caret_on = !caret_on;	
				}

				if (caret_on)
				{
					// Print out caret, make it a yellow line
					// Need to get text from InputText
					auto Text = InputText.Text;
					auto XAdvance = InputText.Position.x + InputText.Parent->Position.x + Padding.x;

					// Get xadvance of all characters
					for (auto& c : Text)
					{
						XAdvance += EG::Fonts::GetAdvance(c, CurrentFont, scale);
					}
					UIBatch->Add(
									EM::Vec4(XAdvance + 1.0f, InputText.Position.y + InputText.Parent->Position.y + Padding.y + TextHeight, 1.0f, 10.0f),
									EM::Vec4(0, 0, 1, 1),
									EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
									EG::RGBA16_LightGrey()
								);
				}


			}
			UIBatch->End();
			UIBatch->RenderBatch();
		}
		TextShader->Unuse();

		Window.SwapBuffer();

		FPS = Limiter.End();
	}

	return 0;
}

bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
{
	static bool WasHovered = false;
	static EM::Vec2 MouseFrameOffset(0.0f);
	unsigned int CurrentKey = 0;
	static std::string str = "";
	char CurrentChar = 0;

	if (KeyboardFocus)
	{
		SDL_StartTextInput();
	}
	else 
	{
		SDL_StopTextInput();
		CurrentChar = 0;
	}

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
                break;
			case SDL_KEYUP:
				Input->ReleaseKey(event.key.keysym.sym); 
				CurrentKey = 0;
				break;
			case SDL_KEYDOWN:
				Input->PressKey(event.key.keysym.sym);
				CurrentKey = event.key.keysym.sym;
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
			case SDL_MOUSEWHEEL:
				Camera->SetScale(Camera->GetScale() + (event.wheel.y) * 0.05f);
				if (Camera->GetScale() < 0.1f) Camera->SetScale(0.1f);
			case SDL_TEXTINPUT:
				str = event.text.text;
				CurrentChar = event.text.text[0];
				std::cout << str << std::endl;
			default:
				break;
		}
    }

    if (SelectedGUIElement)
    {
    	switch (SelectedGUIElement->Type)
    	{
    		case GUIType::BUTTON:
    			// std::cout << "Selected Button!" << std::endl;
    			break;
    		case GUIType::TEXTBOX:
    			// std::cout << "Selected TextBox!" << std::endl;
    			break;
    		default:
    			break;
    	}
    }

	// Get Mouse Position
	auto MousePos = Input->GetMouseCoords();
	CameraManager::GetCamera("HUDCamera")->ConvertScreenToWorld(MousePos);

	// Get play button
	auto PlayButton = static_cast<GUIButton*>(GUIManager::Get("PlayButton"));
	auto AABB_PB = &PlayButton->AABB;

	// Check whether the mouse is hovered over the play button
	auto MouseOverButton = EP::AABBvsPoint(AABB_PB, MousePos);

	if (MouseOverButton)
	{
		if (PlayButton->HoverState == HoveredState::OFF_HOVER)
		{
			std::cout << "Entering Hover..." << std::endl;

			// Emit on hover action
			PlayButton->on_hover.emit();
		}
	}

	// If the mouse was hovering and has now left
	else if (PlayButton->HoverState == HoveredState::ON_HOVER)
	{
		std::cout << "Exiting Hover..." << std::endl;

		// Emit off hover action
		PlayButton->off_hover.emit();
	}

	// Get InputText
	auto InputText = static_cast<GUITextBox*>(GUIManager::Get("InputText"));
	auto AABB_IT = &InputText->AABB;

	// Check whether mouse is over the input text
	auto MouseOverText = EP::AABBvsPoint(AABB_IT, MousePos);

	if (MouseOverText)
	{
		if (InputText->HoverState == HoveredState::OFF_HOVER)
		{
			std::cout << "Entering Hover..." << std::endl;

			// Emit on_hover action
			InputText->on_hover.emit();
		}
	}

	// If the mouse was hovering and has now left
	else if (InputText->HoverState == HoveredState::ON_HOVER)
	{
		std::cout << "Exiting Hover..." << std::endl;

		// Emit off hover action
		InputText->off_hover.emit();
	}

	// Get SceneAnimation
	auto SceneAnimation = static_cast<GUIAnimationElement*>(GUIManager::Get("SceneAnimation"));
	auto SceneMousePos = Input->GetMouseCoords();
	CameraManager::GetCamera("SceneCamera")->ConvertScreenToWorld(SceneMousePos);
	auto AABB_SA = &SceneAnimation->AABB;

	// Check whether mouse is over scene animation
	auto MouseOverAnimation = EP::AABBvsPoint(AABB_SA, SceneMousePos);
	if (MouseOverAnimation)
	{
		if (SceneAnimation->HoverState == HoveredState::OFF_HOVER)
		{
			std::cout << "Entering Hover..." << std::endl;

			// Emit on hover action
			SceneAnimation->on_hover.emit();
		}
	}

	// If mouse was hovering nad has now left
	else if (SceneAnimation->HoverState == HoveredState::ON_HOVER)
	{
		std::cout << "Exiting Hover..." << std::endl;

		// Emit off hover action
			SceneAnimation->off_hover.emit();
	}


    // Basic check for click
    // These events need to be captured and passed to the GUI manager as signals
    if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
    {
    	auto X = MousePos.x;
    	auto Y = MousePos.y;

    	std::cout << "Mouse Pos: " << MousePos << std::endl;

    	// Do AABB test with PlayButton
    	if (MouseOverButton)
    	{
    		SelectedGUIElement = PlayButton;
    		PlayButton->on_click.emit();
    		MouseFocus = nullptr;
    	}

    	else if (MouseOverText)
    	{
    		SelectedGUIElement = InputText;
    		KeyboardFocus = InputText;
    		MouseFocus = nullptr; 			// The way to do this eventually is set all of these focuses here to this element but define whether or not it can move
    		InputText->on_click.emit();
    	}

    	else if (MouseOverAnimation)
    	{
    		SelectedGUIElement = SceneAnimation;
    		MouseFocus = SceneAnimation;
    		MouseFrameOffset = EM::Vec2(SceneMousePos.x - SceneAnimation->AABB.Min.x, SceneMousePos.y - SceneAnimation->AABB.Min.y);
    	}

    	else
    	{
    		// This is incredibly not thought out at all...
    		SelectedGUIElement = nullptr;
    		MouseFocus = nullptr;
    		KeyboardFocus = nullptr;
    	}
    }

    // NOTE(John): Again, these input manager states will be hot loaded in, so this will be cleaned up eventaully...
	if (MouseFocus)
	{
		if (Input->IsKeyDown(SDL_BUTTON_LEFT))
		{
			auto X = SceneMousePos.x;
			auto Y = SceneMousePos.y;

			// Turn off the play button
			if (PlayButton->State == ButtonState::ACTIVE) PlayButton->on_click.emit();

			if (MouseFocus->Type == GUIType::SCENE_ANIMATION)
			{
				auto Anim = static_cast<GUIAnimationElement*>(MouseFocus);
				auto CurrentAnimation = Anim->CurrentAnimation;	

				// Find bottom corner of current frame
				auto BottomCorner = Anim->AABB.Min;

				// Update offsets
				CurrentAnimation->Frames.at(Anim->CurrentIndex).Offsets = EM::Vec2(X - MouseFrameOffset.x, Y - MouseFrameOffset.y);
			}
		}
	}

	if (Input->IsKeyPressed(SDLK_ESCAPE))
	{
		return false;	
	}

	if (KeyboardFocus && CurrentKey != 0)
	{
		// Check for modifiers first
		if (!IsModifier(CurrentKey))
		{
			if (CurrentKey == SDLK_BACKSPACE) InputText->on_backspace.emit();
			else InputText->on_keyboard.emit(str);
		}
	}


	else if (KeyboardFocus == nullptr)
	{
		if (Input->IsKeyDown(SDLK_e))
		{
			Camera->SetScale(Camera->GetScale() + 0.05f);
		}
		if (Input->IsKeyDown(SDLK_q))
		{
			auto S = Camera->GetScale();
			if (S > 0.1f) Camera->SetScale(S - 0.05f);
		}
		if (Input->IsKeyPressed(SDLK_SPACE))
		{
			// Get button from button manager
			auto PlayButton = ButtonManager::Get("PlayButton");

			// Press play
			PlayButton->on_click.emit();
		}
		if (Input->IsKeyDown(SDLK_RIGHT))
		{
			// Get button from button manager
			auto OffsetRight = ButtonManager::Get("OffsetRight");

			// Press next frame
			OffsetRight->on_click.emit();	
		}
		if (Input->IsKeyDown(SDLK_LEFT))
		{
			// Get button from button manager
			auto OffsetLeft = ButtonManager::Get("OffsetLeft");

			// Press next frame
			OffsetLeft->on_click.emit();
		}
		if (Input->IsKeyDown(SDLK_UP))
		{
			// Get button from button manager
			auto OffsetUp = ButtonManager::Get("OffsetUp");

			// Press offset up
			OffsetUp->on_click.emit();
		}
		if (Input->IsKeyDown(SDLK_DOWN))
		{
			// Get button from button manager
			auto OffsetDown = ButtonManager::Get("OffsetDown");

			// Press offset Down
			if (OffsetDown)
			{
				OffsetDown->on_click.emit();
			}
		}
		if (Input->IsKeyPressed(SDLK_m))
		{
			// Get button from button manager
			auto NextFrame = ButtonManager::Get("NextFrame");

			// Press offset Down
			if (NextFrame)
			{
				NextFrame->on_click.emit();
			}
		}
		if (Input->IsKeyPressed(SDLK_n))
		{
			// Get button from button manager
			auto PreviousFrame = ButtonManager::Get("PreviousFrame");

			// Press offset Down
			if (PreviousFrame)
			{
				PreviousFrame->on_click.emit();
			}

		}
		if (Input->IsKeyPressed(SDLK_LEFTBRACKET))
		{
			// Get button from button manager
			auto DelayDown = ButtonManager::Get("DelayDown");

			// Emit
			if (DelayDown)
			{
				DelayDown->on_click.emit();	
			}	
		}
		if (Input->IsKeyPressed(SDLK_RIGHTBRACKET))
		{
			// Get button from button manager
			auto DelayUp = ButtonManager::Get("DelayUp");

			// Emit
			if (DelayUp)
			{
				DelayUp->on_click.emit();
			}
		}
		if (Input->IsKeyPressed(SDLK_o))
		{
			// Get button from button manager
			auto ToggleOnionSkin = static_cast<GUIButton*>(GUIManager::Get("ToggleOnionSkin"));

			// Emit
			if (ToggleOnionSkin)
			{
				ToggleOnionSkin->on_click.emit();
			}
		}
	}



	return true;
}

ImageFrame GetImageFrame(const sajson::value& Frames, const std::string Name)
{
	using namespace sajson;

    // Get an image for testing
    auto image = Frames.find_object_key(literal(Name.c_str()));
    assert(image < Frames.get_length());
    const auto& Image = Frames.get_object_value(image);
   
   	// Get sub objects 
    const auto imageframe = Image.find_object_key(literal("frame"));
    const auto& imageFrame = Image.get_object_value(imageframe);
	const auto sss = Image.find_object_key(literal("spriteSourceSize"));
	const auto& SSS = Image.get_object_value(sss);

	// UV information
	float x = imageFrame.get_value_of_key(literal("x")).get_safe_float_value();
	float y = imageFrame.get_value_of_key(literal("y")).get_safe_float_value();
	float z = imageFrame.get_value_of_key(literal("w")).get_safe_float_value();
	float w = imageFrame.get_value_of_key(literal("h")).get_safe_float_value();

	// Size information
	EM::Vec2 SourceSize(SSS.get_value_of_key(literal("w")).get_safe_float_value(), 
						SSS.get_value_of_key(literal("h")).get_safe_float_value());

	// Return frame
	ImageFrame IF = {	
						EM::Vec4(x, y, z, w), 
					  	EM::Vec2(0.0f),
					  	SourceSize,
					  	0.0f,
					  	1.0f, 
					  	Name, 
					  	0
					};


	// Need to read this value from .json 
	// Also, the TextureDir needs to be formatted at beginning of program, since it's OS specific 
	// 	in the way that forward or backslashes are read
	IF.TextureID = EI::ResourceManager::GetTexture(AnimTextureDir).id;

	// ImageFrame IF;
	return IF;
}

void DrawFrame(const ImageFrame& Image, EM::Vec2 Position, const Atlas& A, EG::SpriteBatch* Batch, const EG::ColorRGBA16& Color)
{
	float ScalingFactor = 1.0f;
	auto& Dims = Image.UVs;
	auto& SSize = Image.SourceSize;
	auto& Offsets = Image.Offsets;

	auto AWidth = A.AtlasSize.x;
	auto AHeight = A.AtlasSize.y;

	Batch->Add(
				EM::Vec4(Position.x + Offsets.x * ScalingFactor, 
						Position.y + Offsets.y * ScalingFactor, 
						EM::Vec2(SSize.x, SSize.y) * ScalingFactor), 
				EM::Vec4(Dims.x / AWidth, 
						(AHeight - Dims.y - Dims.w) / AHeight, 
						 Dims.z / AWidth, 
						 Dims.w / AHeight), 
				A.Texture.id, 
				Color
			  );
}

Anim CreateAnimation(const std::string& AnimName, const sajson::value& FramesDoc)
{
	// need to parse the file for a specific animatioa
	Anim A;

	// This doc will need to be passed in, but for now, just lazy load it...
    using sajson::literal;
    std::string json = EU::read_file(AnimationDir.c_str());
    const sajson::document& doc = sajson::parse(sajson::string(json.c_str(), json.length()));

    if (!doc.is_valid())
    {
        std::cout << "Invalid json: " << doc.get_error_message() << std::endl;
    }

    // Get root and length of animation json file
    const auto& anim_root = doc.get_root();
    const auto len = anim_root.get_length();

    // Get handle to animation
    const auto anim = anim_root.find_object_key(literal(AnimName.c_str()));
    assert(anim < len);
    const auto& Anim = anim_root.get_object_value(anim);
    const auto anim_len = Anim.get_length();

    // Get frames array and delays array
    std::vector<std::string> frames;

    // Frames
    const auto fr = Anim.find_object_key(literal("frames"));
    assert(fr < anim_len);
    const auto& Frames = Anim.get_object_value(fr);
    const auto frames_len = Frames.get_length();
    frames.reserve(frames_len);

    // Delays
    const auto de = Anim.find_object_key(literal("delays"));
    assert(de < anim_len);
    const auto& Delays = Anim.get_object_value(de);

    // YOffset
    const auto yos = Anim.find_object_key(literal("yoffsets"));
    assert(yos < anim_len);
    const auto& YOffset = Anim.get_object_value(yos);

    // XOffset
    const auto xos = Anim.find_object_key(literal("xoffsets"));
    assert(xos < anim_len);
    const auto& XOffset = Anim.get_object_value(xos);

    // Get iframe, get its delay and offsets, push into A.frames
    for (auto i = 0; i < frames_len; i++)
    {
    	auto IF = GetImageFrame(FramesDoc, Frames.get_array_element(i).get_string_value());
    	IF.Delay = Delays.get_array_element(i).get_safe_float_value();
    	IF.Offsets.x = XOffset.get_array_element(i).get_safe_float_value();
    	IF.Offsets.y = YOffset.get_array_element(i).get_safe_float_value();

    	// push back into A.frames
    	A.Frames.push_back(IF);
    }

    // Get total number of frames in vector
    A.TotalFrames = A.Frames.size();

    // Set animation name
    A.Name = AnimName;

	return A;
}

void CalculateAABBWithParent(EP::AABB* A, GUIButton* Button)
{
	auto Parent = Button->Parent;
	auto PPos = &Parent->Position;

	// Set up PlayButton AABB
	Button->AABB.Min =  EM::Vec2(Button->Position.x + PPos->x + Button->Frames.at(ButtonState::INACTIVE).Offsets.x * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor,
									Button->Position.y + PPos->y + Button->Frames.at(ButtonState::INACTIVE).Offsets.y * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor); 
	Button->AABB.Max = Button->AABB.Min + EM::Vec2(Button->Frames.at(ButtonState::INACTIVE).SourceSize.x * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor, 
										     Button->Frames.at(ButtonState::INACTIVE).SourceSize.y * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor); 
}

void DrawCursor(Enjon::Graphics::SpriteBatch* Batch, Enjon::Input::InputManager* InputManager)
{
	float size = 16.0f; 
	std::cout << InputManager->GetMouseCoords() << std::endl;
	auto MouseCoords = InputManager->GetMouseCoords();
	CameraManager::GetCamera("HUDCamera")->ConvertScreenToWorld(MouseCoords);
	Enjon::Math::Vec4 destRect(MouseCoords.x, MouseCoords.y - size, size, size);
	Enjon::Math::Vec4 uvRect(0, 0, 1, 1);
	Batch->Add(destRect, uvRect, MouseTexture.id);
}

bool IsModifier(unsigned int Key)
{
	if (Key == SDLK_LSHIFT || 
		Key == SDLK_RSHIFT || 
		Key == SDLK_LCTRL  ||
		Key == SDLK_RCTRL  ||
		Key == SDLK_CAPSLOCK)
	return true;

	else return false; 
}


#endif


























