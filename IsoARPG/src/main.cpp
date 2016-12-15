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
#include <Editor/AnimationEditor.h>
#include <Editor/BehaviorTreeEditor.h>
#include <System/Internals.h>
#include <BehaviorTree/BehaviorTreeManager.h>
#include <Utils/Functions.h>
#include <Scripting/ScriptingNode.h>
#include <CVarsSystem.h>

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


/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
#include "Level.h"
#include "Loot.h"

/*-- Standard Library includes --*/
#include <stdio.h>
#include <iostream> 
#include <time.h>
#include <stdlib.h>
#include <queue>
#include <unordered_set>
#include <functional>
#include <algorithm>

#define NUM_LIGHTS 	1

typedef struct
{
	EM::Vec3 Position;
	EG::ColorRGBA16 Color;
	float Radius;
	EM::Vec3 Falloff;
} Light;

float LightZ = -0.35f;
float LightIntensity = 5.0f;

// Vector of lights
std::vector<Light> Lights;

typedef struct 
{	
	EM::Vec2 Position;
	EM::Vec2 Dimensions; 
	float Angle;
} BeamSegment;

std::vector<BeamSegment> BeamSegments;

// Console stuff
std::vector<std::string> ConsoleOutput;
EGUI::GUITextBox ConsoleInputTextBox;

char buffer[256];
char buffer2[256];
char buffer3[256];
bool isRunning 				= true;
bool ShowMap 				= false;
bool Paused 				= false;
bool IsDashing 				= false;
bool DebugInfo 				= false;
bool DebugEntityInfo 		= false;
bool AnimationEditorOn 		= false;
bool BehaviorTreeEditorOn 	= false;
bool ParticleEditorOn 		= false;
bool DeferredRenderingOn 	= true;
bool AIControllerEnabled 	= false;
bool DrawSplineOn 			= false;
bool ShowConsole 			= false;
bool LimiterEnabled			= true;

const int LEVELSIZE = 300;

float DashingCounter = 0.0f;

Enjon::f32 StartTicks = 0.0f;

Enjon::uint32 CollisionRunTime = 0;
Enjon::uint32 TransformRunTime = 0;
Enjon::uint32 AIRunTime = 0;
Enjon::uint32 ClearEntitiesRunTime = 0;
Enjon::uint32 RenderTime = 0;
Enjon::uint32 EffectRunTime = 0;
Enjon::uint32 ParticleCount = 0;
Enjon::uint32 TileOverlayRunTime = 0;
Enjon::uint32 PlayerControllerTime = 0;

using namespace Enjon;
using namespace ECS;
using namespace Systems;

/*-- Function Declarations --*/
void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, ECS::Systems::EntityManager* Manager, ECS::eid32 Entity);
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
	std::string PlayerControllerTimeString = "0";

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

	// Create InputManager
	Input::InputManager Input;

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

	// Init AnimManager
	AnimManager::Init();

	// Init FontManager
	Enjon::Graphics::FontManager::Init();

	// Init BehaviorTreeManager
	BTManager::Init();

	// Init AnimationEditor
	Enjon::AnimationEditor::Init(&Input, SCREENWIDTH, SCREENHEIGHT);

	// Init BehaviorTreeEditor
	Enjon::BehaviorTreeEditor::Init(&Input, SCREENWIDTH, SCREENHEIGHT);


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

	EG::SpriteBatch DepthBatch;
	DepthBatch.Init();

	EG::SpriteBatch GroundTileNormalsBatch;
	GroundTileNormalsBatch.Init();

	EG::SpriteBatch GroundTileDebugBatch;
	GroundTileDebugBatch.Init();

	EG::SpriteBatch DebugActiveTileBatch;
	DebugActiveTileBatch.Init();

	EG::SpriteBatch DeferredBatch;
	DeferredBatch.Init();

	EG::SpriteBatch DebugSpatialBatch;
	DebugSpatialBatch.Init();


	Level level;
	Graphics::GLTexture TileTexture;
	level.Init(0, 0, LEVELSIZE, LEVELSIZE);
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
	EnemySheet.Init(Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast2.png"), Math::iVec2(1, 1));
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
	GroundTileDebugBatch.Begin();
	level.DrawGroundTiles(GroundTileBatch, GroundTileNormalsBatch);
	level.DrawDebugTiles(GroundTileDebugBatch);
	GroundTileNormalsBatch.End();
	GroundTileDebugBatch.End();
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
	EG::Particle2D::ParticleBatch2D* LightParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* TestParticleBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* SmokeBatch = EG::Particle2D::NewParticleBatch(&EntityBatch);
	EG::Particle2D::ParticleBatch2D* TextParticleBatch = EG::Particle2D::NewParticleBatch(&TextBatch);

	EG::GLSLProgram* DeferredShader 	= EG::ShaderManager::GetShader("DeferredShader");
	EG::GLSLProgram* DiffuseShader 		= EG::ShaderManager::GetShader("DiffuseShader");
	EG::GLSLProgram* NormalsShader 		= EG::ShaderManager::GetShader("NormalsShader");
	EG::GLSLProgram* DepthShader 		= EG::ShaderManager::GetShader("DepthShader");
	EG::GLSLProgram* ScreenShader 		= EG::ShaderManager::GetShader("NoCameraProjection");
	EG::GLSLProgram* BasicShader 		= EG::ShaderManager::GetShader("Basic");
	EG::GLSLProgram* BasicLightShader 	= EG::ShaderManager::GetShader("BasicLighting");

	// FBO
	float DWidth = SCREENWIDTH;
	float DHeight = SCREENHEIGHT;
	EG::FrameBufferObject* DiffuseFBO 	= new EG::FrameBufferObject(SCREENWIDTH, SCREENHEIGHT);
	EG::FrameBufferObject* NormalsFBO 	= new EG::FrameBufferObject(SCREENWIDTH, SCREENHEIGHT);
	EG::FrameBufferObject* DepthFBO 	= new EG::FrameBufferObject(SCREENWIDTH, SCREENHEIGHT);
	EG::FrameBufferObject* DeferredFBO 	= new EG::FrameBufferObject(SCREENWIDTH, SCREENHEIGHT);

	/////////////////
	// Testing ECS //   
	/////////////////

	// Create new EntityManager
	ECS::Systems::EntityManager* World = EntitySystem::NewEntityManager(level.GetWidth(), level.GetWidth(), &Camera, &level);

	// Draw debug spatial grid
	DebugSpatialBatch.Begin();
	SpatialHash::DrawGrid(World->Grid, &DebugSpatialBatch);
	DebugSpatialBatch.End();

	// Init loot system
	Loot::Init();

	// Push back particle batchs into world
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, TestParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, LightParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, TextParticleBatch);
	EG::Particle2D::AddParticleBatch(World->ParticleEngine, SmokeBatch);


	Math::Vec2 Pos = Camera.GetPosition() + 50.0f;

	static Math::Vec2 enemydims(222.0f, 200.0f);

	static uint32 AmountDrawn = 40000;
	for (int e = 0; e < AmountDrawn; e++)
	{
		float height = -50.0f;
		eid32 ai = Factory::CreateAI(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(level.GetWidth(), 0), Random::Roll(level.GetHeight() * 2, 0))), height),
																enemydims, &EnemySheet, "Enemy", 0.05f); 
		World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
	}

	eid32 Player = Factory::CreatePlayer(World, &Input, Math::Vec3(Math::CartesianToIso(Math::Vec2(level.GetWidth()/2, level.GetHeight()/2)), 0.0f), Math::Vec2(222.0f, 200.0f), &PlayerSheet, 
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

	{
		eid32 Debris = Factory::CreateBoxDebris(World, EM::Vec3(1000, 1000, 0), EM::Vec3(2, 2, 0));
	}

	AmountDrawn = 0;

	for (uint32 e = 0; e < AmountDrawn * 500; e++)
	{
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(level.GetWidth(), 0), Random::Roll(level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(ER::Roll(5, 10), ER::Roll(1, 5)), EG::SpriteSheetManager::GetSpriteSheet("VerticleBar"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
		World->Renderer2DSystem->Renderers[id].Format = EG::CoordinateFormat::ISOMETRIC;
		World->Renderer2DSystem->Renderers[id].Color = EG::RGBA16(0.5f, 0.2f, 0.1f, 1.0f);
		World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
	}

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(level.GetWidth(), 0), Random::Roll(level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(ER::Roll(5, 10), ER::Roll(2, 5)), EG::SpriteSheetManager::GetSpriteSheet("Box"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
		World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
		World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(50, 100) / 50.0f;
	}

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		eid32 id = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(level.GetWidth(), 0), Random::Roll(level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(ER::Roll(10, 20), ER::Roll(10, 20)), EG::SpriteSheetManager::GetSpriteSheet("BoxDebris"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
		World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
		World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(50, 100) / 50.0f;
	}

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		auto s = ER::Roll(20, 40);
		eid32 Box = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(level.GetWidth(), 0), Random::Roll(level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(s, s), EG::SpriteSheetManager::GetSpriteSheet("Box"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[Box].Mass = 2.5f - ER::Roll(1, 50) / 50.0f;
		World->TransformSystem->Transforms[Box].AABBPadding = 15.0f;

	}

	for (uint32 e = 0; e < AmountDrawn; e++)
	{
		auto s = ER::Roll(50, 80);
		eid32 BiggerBox = Factory::CreateItem(World, Math::Vec3(Math::CartesianToIso(Math::Vec2(Random::Roll(level.GetWidth(), 0), Random::Roll(level.GetHeight() * 2, 0))), 0.0f), 
										Enjon::Math::Vec2(s, s), EG::SpriteSheetManager::GetSpriteSheet("Box"), Masks::Type::ITEM, Component::EntityType::ITEM);
		World->TransformSystem->Transforms[BiggerBox].Mass = 5.0f - ER::Roll(1, 100) / 50.0f;
		World->TransformSystem->Transforms[BiggerBox].AABBPadding = 15.0f;
	}

	// Set position to player
	Camera.SetPosition(Math::Vec2(World->TransformSystem->Transforms[Player].Position.x, World->TransformSystem->Transforms[Player].Position.y)); 

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


	const GLfloat constant = 1.0f; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
    const GLfloat linear = 0.1f;
    const GLfloat quadratic = 5.0f;
    // Then calculate radius of light volume/sphere

   	float LevelWidth = level.GetWidth();
   	float LevelHeight = level.GetHeight(); 
	for (GLuint i = 0; i < NUM_LIGHTS; i++)
	{
		// EG::ColorRGBA16 Color = EG::RGBA16(ER::Roll(0, 500) / 255.0f, ER::Roll(0, 500) / 255.0f, ER::Roll(0, 500) / 255.0f, 2.5f);
		EG::ColorRGBA16 Color = EG::RGBA16(0.6f, 0.7f, ER::Roll(0, 255) / 255.0f, 1.0f);
	    GLfloat maxBrightness = std::fmaxf(std::fmaxf(Color.r, Color.g), Color.b);  // max(max(lightcolor.r, lightcolor.g), lightcolor.b)
	    // GLfloat maxBrightness = 100.0f;  // max(max(lightcolor.r, lightcolor.g), lightcolor.b)
	    GLfloat Radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2 * quadratic);
		Light L = {
					  // EM::Vec3(ER::Roll(0, -LevelWidth), ER::Roll(0, -LevelHeight), LightZ), 
					  EM::Vec3(ER::Roll(0, LevelWidth), ER::Roll(0, LevelHeight), LightZ), 
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

	// Console text box initialization
	ConsoleInputTextBox.Position = EM::Vec2(-SCREENWIDTH * 0.5f + 5.0f, -SCREENHEIGHT * 0.5f + 20.0f);
	ConsoleInputTextBox.Dimensions = EM::Vec2(SCREENWIDTH - 10.0f, ConsoleInputTextBox.Dimensions.y);
	ConsoleInputTextBox.MaxStringLength = 100;
	ConsoleInputTextBox.KeyboardInFocus = true;

	ConsoleInputTextBox.on_enter.connect([&]()
	{
		// Parse the string
		auto& C = ConsoleInputTextBox.Text;

		// Split the string into a vector on spaces
		std::vector<std::string> Elements = EU::split(C, ' ');

		if (!Elements.size()) return;

		// Check first element for function call (totally a test)
		if (Elements.at(0).compare("mul") == 0) 
		{
			// Error
			if (Elements.size() < 3) ConsoleOutput.push_back("mul requires 2 arguments");

			// Calculate the result
			else
			{
				if (!EU::is_numeric(Elements.at(1)) || !EU::is_numeric(Elements.at(2))) 
				{
					ConsoleOutput.push_back("Cannot multiply non numeric terms: " + Elements.at(1) + ", " + Elements.at(2));
				}

				// Otherwise, multiply and push back result
				else
				{
					auto Result = std::atof(Elements.at(1).c_str()) * std::atof(Elements.at(2).c_str());
					ConsoleOutput.push_back(std::to_string(Result));
				}
			}
		}
		// Clear the elements
		else if (Elements.at(0).compare("clear") == 0 || Elements.at(0).compare("cls") == 0)
		{
			ConsoleOutput.clear();
		}
		else if (Elements.at(0).compare("spawn") == 0)
		{
			if (Elements.size() < 2) ConsoleOutput.push_back("spawn requires argument");

			else
			{
				if (Elements.at(1).compare("ai") == 0)
				{
					auto CamPos = Camera.GetPosition();
					auto MouseCoords = Input.GetMouseCoords();	
					Camera.ConvertScreenToWorld(MouseCoords);
					float height = 0.0f;
					Math::Vec2 enemydims(100.0f, 100.0f);

					if (Elements.size() > 2)
					{
						if (EU::is_numeric(Elements.at(2)))
						{
							auto Amount = std::atoi(Elements.at(2).c_str());

							if (Amount > 100) ConsoleOutput.push_back("spawn: cannot spawn amount: " + Elements.at(2));
							else
							{
								for (auto i = 0; i < Amount; i++)
								{
									auto ai = Factory::CreateAI(World, Math::Vec3(MouseCoords.x, MouseCoords.y, height), enemydims, EG::SpriteSheetManager::GetSpriteSheet("Beast"), "Enemy", 0.05f);
									World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
								}
							}
						}
						else
						{
							auto ai = Factory::CreateAI(World, Math::Vec3(MouseCoords.x, MouseCoords.y, height), enemydims, EG::SpriteSheetManager::GetSpriteSheet("Beast"), "Enemy", 0.05f);
							World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
						}
					}
					else
					{
						auto ai = Factory::CreateAI(World, Math::Vec3(MouseCoords.x, MouseCoords.y, height), enemydims, EG::SpriteSheetManager::GetSpriteSheet("Beast"), "Enemy", 0.05f);
						World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
					}
				}
				else ConsoleOutput.push_back("spawn: cannot recognize argument: " + Elements.at(1));
			}

		}
		else if (Elements.at(0).compare("limiter") == 0)
		{
			if (Elements.size() < 2) ConsoleOutput.push_back("Limiter requires argument");
			else
			{
				if (Elements.at(1).compare("enabled") == 0)
				{
					LimiterEnabled = true;
				}
				else if (Elements.at(1).compare("disabled") == 0)
				{
					LimiterEnabled = false;
				}
				else
				{
					ConsoleOutput.push_back("Cannot find limiter operation: " + Elements.at(1));
				}
			}
		}
		else if (Elements.at(0).compare("ai") == 0)
		{
			if (Elements.size() < 2) ConsoleOutput.push_back("ai requires argument");
			else
			{
				if (Elements.at(1).compare("enabled") == 0)
				{
					AIControllerEnabled = true;
				}
				else if (Elements.at(1).compare("disabled") == 0)
				{
					AIControllerEnabled = false;
				}
				else
				{
					ConsoleOutput.push_back("Cannot find ai operation: " + Elements.at(1));
				}
			}
		}
		else if (Elements.at(0).compare("cvarlist") == 0)
		{
			auto registeredCommands = CVarsSystem::GetRegisteredCommands();
			ConsoleOutput.push_back("Console Variables Available:");
			for (auto& c : registeredCommands) ConsoleOutput.push_back(c);
		}
		else
		{
			// Register command with CVar System
			if (Elements.size() < 2) ConsoleOutput.push_back("need argument for cvar");
			else
			{
				if (!CVAR_SET(Elements.at(0), std::atof(Elements.at(1).c_str())))
				{
					ConsoleOutput.push_back("Cvar does not exist: " + Elements.at(0));
				}
				else
				{
					ConsoleOutput.push_back("Set " + Elements.at(0) + ": " + Elements.at(1));
				}
			}
		}
		
		// Clear the console text
		ConsoleInputTextBox.Clear();	

		// Reset focus
		ConsoleInputTextBox.KeyboardInFocus = true;
	});



	///////////////////////////////////////////////////////
	// Script Nodes ///////////////////////////////////////
	///////////////////////////////////////////////////////

	EScript::EUintNode 							PlayerIDReference(Player);
	EScript::RenderComponentGetComponentNode	GetRenderComponent;
	EScript::RenderComponentSetColorNode 		SetColor;
	EScript::CosineNode 						Cos;
	EScript::SinNode 							Sin;
	EScript::TanNode 							Tan;
	EScript::EVec4Node 							NewColor;
	EScript::WorldTimeNode 						WorldTime;
	EScript::EFloatNode 						One(1.0f);

	// Get render component
	GetRenderComponent.SetInputs(&PlayerIDReference);

	// Set Color
	SetColor.SetInputs(&GetRenderComponent, &NewColor);

	// Get new color
	NewColor.SetInputs(&Cos, &Sin, &Tan, &One);

	// Get cosine
	Cos.SetInputs(&WorldTime);

	// Get Sin
	Sin.SetInputs(&WorldTime);

	// Set the entry point for script
	auto EntryPoint = &SetColor;

	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
	
	// Main game loop
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

		StartTicks = SDL_GetTicks();


		{
			auto L = &Lights.at(0);
			const EM::Vec3* P = &World->TransformSystem->Transforms[Player].Position;
			EM::Vec2 AddOn = EM::CartesianToIso(EM::Vec2(150 * cos(t), 150 * sin(t)));
			L->Position = EM::Vec3(P->x + AddOn.x, P->y - P->z + AddOn.y, 0.0f);
			L->Color = EG::RGBA16(0.3f, 0.2f, 1.0f, 1.0f);
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
		ViewPort = Math::Vec2(SCREENWIDTH, SCREENHEIGHT) / Camera.GetScale();
		CameraDims = Math::Vec4(*PlayerStuff, quadDimsStuff / Camera.GetScale());

		// AnimationEditor
		if (AnimationEditorOn)
		{
			// Update editor
			AnimationEditorOn = Enjon::AnimationEditor::Update();	
		}

		else if (BehaviorTreeEditorOn)
		{
			// Update editor
			BehaviorTreeEditorOn = Enjon::BehaviorTreeEditor::Update();
		}

		// Game
		else
		{
			// Update Input Manager
			Input.Update();	

			if (ShowConsole)
			{
				ConsoleInputTextBox.Update();
			}

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

				// Update world object
				ECSS::EntitySystem::Update();

				// Draw some random assed fire
				// EG::Particle2D::DrawFire(LightParticleBatch, EM::Vec3(0.0f, 0.0f, 0.0f));

				// Execute script
				EntryPoint->Execute();
		
				TileOverlayRunTime = SDL_GetTicks() - StartTicks;		

				StartTicks = SDL_GetTicks();
				SpatialHash::ClearCells(World->Grid);
				ClearEntitiesRunTime = (SDL_GetTicks() - StartTicks); // NOTE(John): Only clearing cells that are marked as dirty

				StartTicks = SDL_GetTicks();
				Transform::Update(World->TransformSystem, LightParticleBatch);
				TransformRunTime = (SDL_GetTicks() - StartTicks);

				Animation2D::Update(World);

				StartTicks = SDL_GetTicks();	
				Collision::Update(World);
				CollisionRunTime = (SDL_GetTicks() - StartTicks);

				StartTicks = SDL_GetTicks();
				if (AIControllerEnabled) AIController::Update(World->AIControllerSystem, Player);
				AIRunTime = SDL_GetTicks() - StartTicks;

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

				if (!ShowConsole)
				{
					StartTicks = SDL_GetTicks();	
					PlayerController::Update(World->PlayerControllerSystem);
					PlayerControllerTime = (SDL_GetTicks() - StartTicks);
				}
			}

			// Check for input
			ProcessInput(&Input, &Camera, World, Player); 


			//LERP camera to center of player position
			static Math::Vec2 m_velocity;
			static float scale = 6.0f; 
			m_velocity.x = Enjon::Math::Lerp(World->TransformSystem->Transforms[Player].Position.x, Camera.GetPosition().x, 8.0f);
			m_velocity.y = Enjon::Math::Lerp(World->TransformSystem->Transforms[Player].Position.y, Camera.GetPosition().y, scale); 
			Camera.SetPosition(m_velocity);
		}

		////////////////////////////////////////////////

		///////////////
		// RENDERING //
		///////////////

		StartTicks = SDL_GetTicks();

		//Enable alpha blending
		glEnable(GL_BLEND);

		//Set blend function type
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		if (AnimationEditorOn)
		{
			Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.16f, 0.16f, 0.16f, 1.0));
	
			// Show mouse
			Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::SHOW);
	
			// Render AnimationEditor scene
			Enjon::AnimationEditor::Draw();
		}

		else if (BehaviorTreeEditorOn)
		{
			Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.16f, 0.16f, 0.16f, 1.0));
	
			// Show mouse
			Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::SHOW);
	
			// Render AnimationEditor scene
			Enjon::BehaviorTreeEditor::Draw();
		}

		// Render game scene
		else
		{
			Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.0, 0.0, 0.0, 0.0));

			// Hide mouse
			Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::HIDE);

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
			DepthBatch.Begin();

			static uint32 Row = 0;
			static uint32 Col = 0;
			static uint32 i = 0;
			static Math::Vec2 enemydims2(180.0f, 222.0f);
			static Math::Vec2 dims(100.0f, 100.0f);
			static Math::Vec2 arrowDims(64.0f, 64.0f);
			static Math::Vec2 itemDims(20.0f, 20.0f);
			static Math::Vec4 uv(0, 0, 1, 1);
			static Math::Vec2 pos(-1000, -1000);
			static Graphics::GLTexture beast = Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast2.png"); 
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

				Ground = &World->TransformSystem->Transforms[e].GroundPosition;
				auto EAABB = &World->TransformSystem->Transforms[e].AABB;
				// EntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), EG::SpriteSheetManager::GetSpriteSheet("Orb2")->texture.id,
				// 						Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f), 1.0f);

				// If AI
				if (Mask & COMPONENT_AICONTROLLER)
				{

					// Print Entity info if debug info is on
					if (DebugEntityInfo)
					{
						auto CF = EG::FontManager::GetFont("WeblySleek_32");

						// Entity id
						Graphics::Fonts::PrintText(	EntityPosition->x + 20.0f, 
													EntityPosition->y - 20.0f, 
													0.4f, 
													std::string("ID: ") + std::to_string(e), 
													CF, 
													TextBatch, 
													EG::SetOpacity(Graphics::RGBA16_White(), 0.8f)
												);
						// Entity Type
						std::string Type;
						if (World->AttributeSystem->Masks[e] & Masks::Type::AI) Type = "AI";
						else Type = "Unknown";


						// Entity Position 
						auto X = EntityPosition->x;
						auto Y = EntityPosition->y;
						auto Z = EntityPosition->z;


						Graphics::Fonts::PrintText(	EntityPosition->x + 20.0f, 
													EntityPosition->y - 40.0f, 
													0.4f, std::string("<") + std::to_string(X) + std::string(", ") + std::to_string(Y) + (", ") + std::to_string(Z) + std::string(">"), 
													CF, TextBatch, 
																	Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

						Graphics::Fonts::PrintText( EntityPosition->x + 20.0f, 
													EntityPosition->y - 60.0f, 
													0.4f, std::string("Type: ") + Type, 
													CF, TextBatch, 
																	Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

						auto EntityHealth = World->AttributeSystem->HealthComponents[e].Health;

						Graphics::Fonts::PrintText( EntityPosition->x + 20.0f, 
													EntityPosition->y - 80.0f, 
													0.4f, std::string("Health: ") + std::to_string(EntityHealth), 
													CF, TextBatch, 
																	Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

						auto Cells =  SpatialHash::FindGridCoordinates(World->Grid, World->TransformSystem->Transforms[e].CartesianPosition);

						Graphics::Fonts::PrintText( EntityPosition->x + 20.0f, 
													EntityPosition->y - 100.0f, 
													0.4f, std::string("Grid Cell: ") + "(" + std::to_string(Cells.x) + ", " + std::to_string(Cells.y) + ")", 
													CF, TextBatch, 
													Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

						auto Node = World->AIControllerSystem->AIControllers[e].BB.SO.CurrentNode;
						if (Node != nullptr)
						{
							Graphics::Fonts::PrintText( EntityPosition->x + 20.0f, 
														EntityPosition->y - 120.0f, 
														0.4f, std::string("RUNNING: ") + Node->String(), 
														CF, TextBatch, 
														Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));
						}
					}

					// If target
					if (e == World->PlayerControllerSystem->CurrentTarget)
					{
						Math::Vec2 ReticleDims(94.0f, 47.0f);
						Math::Vec2 Position = World->TransformSystem->Transforms[e].GroundPosition - Math::Vec2(15.0f, 5.0f);
						EntityBatch.Add(Math::Vec4(Position.x, Position.y, ReticleDims), Enjon::Math::Vec4(0, 0, 1, 1), TargetSheet.texture.id, Enjon::Graphics::RGBA16_Red(), 1000);
					}

					// EntityBatch.Add(Math::Vec4(EntityPosition->XY(), *EDims), uv, ESpriteSheet->texture.id, *Color, EntityPosition->y - World->TransformSystem->Transforms[e].Position.z);

					auto CurrentIndex = World->Animation2DSystem->AnimComponents[e].CurrentIndex;
					auto Image = &World->Animation2DSystem->AnimComponents[e].CurrentAnimation->Frames.at(CurrentIndex);
					auto Position = EntityPosition->XY();

					EA::DrawFrame(*Image, Position, &EntityBatch, *Color, 1.8f, Position.y - World->TransformSystem->Transforms[e].Position.z);

					// EntityBatch.Add(
					// 				EM::Vec4(Position, 300.0f, 300.0f * 0.87f),
					// 				EM::Vec4(0, 0, 1, 1),
					// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Enemy_Diffuse.png").id,
					// 				*Color,
					// 				Position.y - World->TransformSystem->Transforms[e].Position.z
					// 				);

					// NormalsBatch.Add(
					// 				EM::Vec4(Position, 300.0f, 300.0f * 0.87f),
					// 				EM::Vec4(0, 0, 1, 1),
					// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Enemy_NormalDepth.png").id,
					// 				EG::RGBA16_White(),
					// 				Position.y - World->TransformSystem->Transforms[e].Position.z
					// 				);

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

				// Ground = &World->TransformSystem->Transforms[e].GroundPosition;
				// auto EAABB = &World->TransformSystem->Transforms[e].AABB;
				// if (World->Types[e] != ECS::Component::EntityType::ITEM)
				// {
				// 	EntityBatch.Add(Math::Vec4(Ground->x, Ground->y, 64.0f, 32.0f), Math::Vec4(0, 0, 1, 1), groundtiletexture.id,
				// 							Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f), 1.0f);
		
				// 	float XDiff = World->TransformSystem->Transforms[e].AABBPadding.x;
				// 	float YDiff = World->TransformSystem->Transforms[e].AABBPadding.y;
				// 	// Enjon::Math::Vec2 EAABBIsoMin(Enjon::Math::CartesianToIso(EAABB->Min) + Math::Vec2(XDiff / 2.0f, XDiff / 2.0f));
				// 	Enjon::Math::Vec2 EAABBIsoMin(Enjon::Math::CartesianToIso(EAABB->Min));
				// 	auto EAABBIsoMax(EM::CartesianToIso(EAABB->Max));
				// 	// float EAABBHeight = EAABB->Max.y - EAABB->Min.y, EAABBWidth = EAABB->Max.x - EAABB->Min.y;
				// 	// EntityBatch.Add(Math::Vec4(EAABBIsoMin, Math::Vec2(abs(EAABB->Max.x - EAABB->Min.x), abs(EAABB->Max.y - EAABB->Min.y))), 
				// 	// 					Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
				// 	// 					Graphics::SetOpacity(Graphics::RGBA16_Red(), 0.2f), EAABBIsoMin.y, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);
				// 	// EntityBatch.Add(Math::Vec4(EAABBIsoMin.x, EAABBIsoMin.y, Math::Vec2(XDiff, YDiff)), 
				// 	// 					Math::Vec4(0, 0, 1, 1), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2dmaptile.png").id, 
				// 	// 					Graphics::SetOpacity(Graphics::RGBA16_Red(), 0.2f), EAABBIsoMin.y, Math::ToRadians(0.0f), Graphics::CoordinateFormat::ISOMETRIC);
				// }
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


			Frame = World->Animation2DSystem->Animations[Player].CurrentFrame + World->Animation2DSystem->Animations[Player].BeginningFrame;
			const Enjon::Graphics::ColorRGBA16* Color = &World->Renderer2DSystem->Renderers[Player].Color;
			Enjon::Math::Vec2* PlayerPosition = &World->TransformSystem->Transforms[Player].Position.XY();
			if (World->Animation2DSystem->Animations[Player].Sheet == EG::SpriteSheetManager::GetSpriteSheet("PlayerSheet2"))
			{
				dims = Math::Vec2(115.0f, 115.0f);
			}
			else dims = Math::Vec2(100.0f, 100.0f);

			//////////////////////////////////////////
			// DRAW PLAYER ///////////////////////////
			//////////////////////////////////////////

			{
				// void DrawFrame(const ImageFrame& Image, EM::Vec2 Position, EG::SpriteBatch* Batch, const EG::ColorRGBA16& Color = EG::RGBA16_White(), float ScalingFactor = 1.0f);
				// Get handle to image frame
				auto CurrentIndex = World->Animation2DSystem->AnimComponents[Player].CurrentIndex;
				auto Image = &World->Animation2DSystem->AnimComponents[Player].CurrentAnimation->Frames.at(CurrentIndex);
				auto PlayerColor = &World->Renderer2DSystem->Renderers[Player].Color;

				EA::DrawFrame(*Image, *PlayerPosition, &EntityBatch, *PlayerColor, 1.5f, PlayerPosition->y - World->TransformSystem->Transforms[Player].Position.z);

				// Print Entity info if debug info is on
				if (DebugEntityInfo)
				{
					auto CF = EG::FontManager::GetFont("WeblySleek_32");

					// Entity id
					Graphics::Fonts::PrintText(	PlayerPosition->x + 20.0f, 
												PlayerPosition->y - 20.0f, 
												0.4f, 
												std::string("ID: ") + std::to_string(Player), 
												CF, 
												TextBatch, 
												EG::SetOpacity(Graphics::RGBA16_White(), 0.8f)
											);
					// Entity Type
					std::string Type;
					if (World->AttributeSystem->Masks[Player] & Masks::Type::AI) Type = "Player";
					else Type = "Unknown";


					// Entity Position 
					auto X = PlayerPosition->x;
					auto Y = PlayerPosition->y;
					auto Z = World->TransformSystem->Transforms[Player].Position.z;


					Graphics::Fonts::PrintText(	PlayerPosition->x + 20.0f, 
												PlayerPosition->y - 40.0f, 
												0.4f, std::string("<") + std::to_string(X) + std::string(", ") + std::to_string(Y) + (", ") + std::to_string(Z) + std::string(">"), 
												CF, TextBatch, 
																Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

					Graphics::Fonts::PrintText( PlayerPosition->x + 20.0f, 
												PlayerPosition->y - 60.0f, 
												0.4f, std::string("Type: ") + Type, 
												CF, TextBatch, 
																Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

					auto EntityHealth = World->AttributeSystem->HealthComponents[Player].Health;

					Graphics::Fonts::PrintText( PlayerPosition->x + 20.0f, 
												PlayerPosition->y - 80.0f, 
												0.4f, std::string("Health: ") + std::to_string(EntityHealth), 
												CF, TextBatch, 
																Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

					auto Cells =  SpatialHash::FindGridCoordinates(World->Grid, World->TransformSystem->Transforms[Player].CartesianPosition);

					Graphics::Fonts::PrintText( PlayerPosition->x + 20.0f, 
												PlayerPosition->y - 100.0f, 
												0.4f, std::string("Grid Cell: ") + "(" + std::to_string(Cells.x) + ", " + std::to_string(Cells.y) + ")", 
												CF, TextBatch, 
												Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

				}
			}

			// Draw some player 1 shit...
			Graphics::Fonts::PrintText( PlayerPosition->x, 
										PlayerPosition->y + 170.0f, 
										1.0f, "P1", 
										EG::FontManager::GetFont("8Bit_32"), TextBatch, 
										Graphics::SetOpacity(Graphics::RGBA16_Orange(), 0.8f));

			//////////////////////////////////////////

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
										Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.2f), 10000000.0f);
			// Draw player shadow
			EntityBatch.Add(Math::Vec4(GroundPosition->x - 20.0f, GroundPosition->y - 20.0f, 200.0f, 300.0f), EM::Vec4(0, 0, 1, 1), 
										EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/mainDudeSmall.png").id,
										Graphics::SetOpacity(Graphics::RGBA16_Black(), 0.3f), 1000000.0f, Enjon::Math::ToRadians(120.0f));

		
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

			auto F = EG::FontManager::GetFont("WeblySleek_32");

			if (Paused)
			{
				// Draw paused text
				Enjon::Graphics::Fonts::PrintText(Camera.GetPosition().x - 110.0f, Camera.GetPosition().y - 30.0f, 1.0f, "Paused", F, TextBatch);
			}

			if (DebugInfo)
			{
				static auto NumberOfDebugEntries = 15;
				// Draw box around area
				HUDBatch.Add(	
								EM::Vec4(
											HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 10.0f, 
											HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 60.0f - 20.0f * NumberOfDebugEntries,
											340.0f, 
											20.0f + 20.0f * NumberOfDebugEntries
										), 
								EM::Vec4(), 
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::SetOpacity(EG::RGBA16_LightPurple(), 0.3f)
							);
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
												0.4f, std::to_string(World->Entities.size()), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

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

				// Player Controller run time
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 300.0f, 
												0.4f, "PlayerController: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 300.0f, 
												0.4f, PlayerControllerTimeString + " ms", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

				// Camera scale
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 320.0f, 
												0.4f, "Camera Scale: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 320.0f, 
												0.4f, std::to_string(Camera.GetScale()), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

				// Spatial Hash
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 30.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 340.0f, 
												0.4f, "Spatial Grid: ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.5f));
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 200.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 340.0f, 
												0.4f, "Rows: " + std::to_string(World->Grid->rows) + ", ", F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));
				Graphics::Fonts::PrintText(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 270.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 340.0f, 
												0.4f, "Cols: " + std::to_string(World->Grid->cols), F, HUDBatch, Graphics::SetOpacity(Graphics::RGBA16_White(), 0.8f));

			}

			// Draw random particle for sub image splitting
			// Get a texture of a text character

			// // Draw Isometric compass
			// MapEntityBatch.Add(EM::Vec4(HUDCamera.GetPosition() - EM::Vec2(SCREENWIDTH / 2.0f - 30.0f, -SCREENHEIGHT / 2.0f + 250.0f), 150.0f, 75.0f), 
			// 				EM::Vec4(0, 0, 1, 1), EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/Coordinates.png").id, EG::RGBA16_White());

			// Add particles to entity batch
			EG::Particle2D::Draw(World->ParticleEngine, &Camera);

			/*
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
			{
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
			}
			*/

			if (level.IsDrawDebugEnabled())
			{
				DebugActiveTileBatch.Begin();
				{
					level.DrawDebugActiveTile(DebugActiveTileBatch, World->TransformSystem->Transforms[World->Player].Position.XY());
				}
				DebugActiveTileBatch.End();

				DebugSpatialBatch.Begin();
				{
					auto MP = Input.GetMouseCoords();
					Camera.ConvertScreenToWorld(MP);
					MP = EM::IsoToCartesian(MP - EM::Vec2(50.0f, 0.0f));
					SpatialHash::DrawActiveCell(World->Grid, &DebugSpatialBatch, MP);

					// Get index of MP
					auto MPGridCoords = SpatialHash::FindGridCoordinates(World->Grid, MP);
					auto Index = MPGridCoords.y * World->Grid->cols + MPGridCoords.x;

					// std::cout << "Grid Coords: <" << MPGridCoords.x << ", " << MPGridCoords.y << ">, ObstructionValue: " << World->Grid->cells.at(Index).ObstructionValue << std::endl;

					SpatialHash::DrawGrid(World->Grid, &DebugSpatialBatch, World->TransformSystem->Transforms[World->Player].CartesianPosition);

					/*
					static float t = 0.0f;
					static auto index = 0;
					static auto MaxI = World->Grid->rows * World->Grid->cols;
					t += 0.1f;

					if (t > 0.2f)
					{
						index++;
						if (index > MaxI - 1) index = 0;	
						t = 0.0f;
					}

					auto CellCoordinates = SpatialHash::FindGridCoordinatesFromIndex(World->Grid, index);
					auto CellDimensions = SpatialHash::GetCellDimensions(World->Grid, CellCoordinates);
					auto Color = EG::RGBA16_Yellow();

					if (World->Grid->cells.at(index).entities.size())
					{
						Color = EG::RGBA16_Red();

						for (auto& e : World->Grid->cells.at(index).entities)
						{
							auto Damage = Enjon::Random::Roll(5, 10);

							// Get health and color of entity
							Component::HealthComponent* HealthComponent = &World->AttributeSystem->HealthComponents[e];
							Enjon::Graphics::ColorRGBA16* Color = &World->Renderer2DSystem->Renderers[e].Color;

							// Set option to damaged
							World->AttributeSystem->Masks[e] |= Masks::GeneralOptions::DAMAGED;
					
							// Decrement by some arbitrary amount for now	
							HealthComponent->Health -= Damage;
						}
					}

					DebugSpatialBatch.Add(
											EM::Vec4(CellDimensions.x, CellDimensions.y, CellDimensions.z - 5.0f, CellDimensions.w - 5.0f), 
											EM::Vec4(0, 0, 1, 1), 
											EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png").id,
											Color
										);
					*/


					// Let's do some pathfinding here, queer!
					// auto Path = PathFinding::FindPath(World->Grid, World->TransformSystem->Transforms[World->Player].CartesianPosition, MP);

					for (auto e = 0; e < World->MaxAvailableID; e++)
					{
						if (World->Masks[e] & COMPONENT_AICONTROLLER)
						{
							auto i = 0.1f;
							// Draw each node in path
							for (auto& N : World->AIControllerSystem->PathFindingComponents[e].Path)
							{
								auto Index = N.Index;

								// Get parent of index from spatial hash
								auto Coords = SpatialHash::FindGridCoordinatesFromIndex(World->Grid, World->Grid->cells.at(Index).ParentIndex);

								auto Dims = SpatialHash::GetCellDimensions(World->Grid, Coords);

								DebugSpatialBatch.Add(
														EM::Vec4(Dims.x, Dims.y, Dims.z - 5.0f, Dims.w - 5.0f), 
														EM::Vec4(0, 0, 1, 1), 
														EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png").id,
														EG::SetOpacity(EG::RGBA16_Black(), i)
													);
								i += 0.1f;
							}

						}	
					}


					// auto AIPath = AIController::GetPath();

					// // Draw each node in path
					// for (auto& N : *AIPath)
					// {
					// 	auto Index = N.Index;

					// 	// Get parent of index from spatial hash
					// 	auto Coords = SpatialHash::FindGridCoordinatesFromIndex(World->Grid, World->Grid->cells.at(Index).ParentIndex);

					// 	auto Dims = SpatialHash::GetCellDimensions(World->Grid, Coords);

					// 	DebugSpatialBatch.Add(
					// 							EM::Vec4(Dims.x, Dims.y, Dims.z - 5.0f, Dims.w - 5.0f), 
					// 							EM::Vec4(0, 0, 1, 1), 
					// 							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png").id,
					// 							EG::RGBA16_Black()
					// 						);
					// }
				}
				DebugSpatialBatch.End();
			}


			// Other shit

			// Draw player again, this time transparent, so he doesn't get "occluded" behind anything
			{
				auto CurrentIndex = World->Animation2DSystem->AnimComponents[Player].CurrentIndex;
				auto Image = &World->Animation2DSystem->AnimComponents[Player].CurrentAnimation->Frames.at(CurrentIndex);

				// EA::DrawFrame(*Image, *PlayerPosition, &EntityBatch, EG::SetOpacity(EG::RGBA16_White(), 0.1f), 1.5f, -10000.0f);
			}

			// Draw spline
			if (DrawSplineOn)
			{
				static auto offset = 0.0f;
				offset += 0.01f;
				auto MP = Input.GetMouseCoords();
				Camera.ConvertScreenToWorld(MP);
				auto CO = cos(offset);
				auto SO = sin(offset);
				EG::Shapes::DrawSpline(
										&EntityBatch,
										EM::Vec4(200.0f, 300.0f + 40 * SO, MP),
										EM::Vec4(300.0f + 30 * CO, 600.0f + 30 * SO, 500.0f + 50 * CO, 100.0f + 20 * SO),
										30.0f + 25 * SO,
										200,
										EG::RGBA16_LightPurple()
									);
			}


			EntityBatch.End();
			TextBatch.End(); 
			LightBatch.End();
			MapEntityBatch.End(); 
			HUDBatch.End();
			NormalsBatch.End();
			DepthBatch.End();

			// Deferred rendering pass if enabled		
			if (DeferredRenderingOn)
			{
				DiffuseFBO->Bind();
				{
					DiffuseShader->Use();
					{
						// Set up uniforms
						DiffuseShader->SetUniform("model", model);
						DiffuseShader->SetUniform("view", view);
						DiffuseShader->SetUniform("projection", projection);

						GroundTileBatch.RenderBatch();

						// Draw debug ground tiles
						if (level.IsDrawDebugEnabled()) 
						{
							GroundTileDebugBatch.RenderBatch();	
							DebugActiveTileBatch.RenderBatch();
							DebugSpatialBatch.RenderBatch();
						}
						// Draw TileOverlays
						OverlayBatch.RenderBatch();
						// Draw entities		
						EntityBatch.RenderBatch();
						// Draw particles 	
						ParticleBatch.RenderBatch();
					}
					DiffuseShader->Unuse();
				}
				DiffuseFBO->Unbind();

				// Normals Rendering
				// NormalsFBO->Bind();
				// {
				// 	NormalsShader->Use();
				// 	{
				// 		// Set up uniforms
				// 		NormalsShader->SetUniform("model", model);
				// 		NormalsShader->SetUniform("view", view);
				// 		NormalsShader->SetUniform("projection", projection);

				// 		GroundTileNormalsBatch.RenderBatch();
				// 		NormalsBatch.RenderBatch();
				// 	}
				// 	NormalsShader->Unuse();
				// }
				// NormalsFBO->Unbind();

				// Deferred Render
				glDisable(GL_DEPTH_TEST);
				glBlendFunc(GL_ONE, GL_ONE);
				DeferredFBO->Bind();
				{
					DeferredShader->Use();
					{
						GLuint m_diffuseID 	= glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_diffuse");
						GLuint m_normalsID  = glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_normals");
						GLuint m_positionID = glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_position");
						// GLuint m_depthID  	= glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_depth");

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

						// Bind depth
						// glActiveTexture(GL_TEXTURE3);
						// glEnable(GL_TEXTURE_2D);
						// glBindTexture(GL_TEXTURE_2D, DepthFBO->GetDiffuseTexture());
						// glUniform1i(m_depthID, 3);

						glUniform1i(glGetUniformLocation(DeferredShader->GetProgramID(), "NumberOfLights"), LightsToDraw.size());

						auto CameraScale = Camera.GetScale();
						for (GLuint i = 0; i < LightsToDraw.size(); i++)
						{
							auto L = LightsToDraw.at(i);

							glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Position").c_str()), L->Position.x, L->Position.y, L->Position.z + LightZ);
							glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Color").c_str()), L->Color.r, L->Color.g, L->Color.b, L->Color.a);
							glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Radius").c_str()), L->Radius / CameraScale);
							glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Falloff").c_str()), L->Falloff.x, L->Falloff.y, L->Falloff.z);
							glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Depth").c_str()), L->Position.y);
							glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Intensity").c_str()), LightIntensity);
						}

						// Set uniforms
						glUniform2f(glGetUniformLocation(DeferredShader->GetProgramID(), "Resolution"),
									 SCREENWIDTH, SCREENHEIGHT);
						// glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), "AmbientColor"), 0.3f, 0.5f, 0.8f, 0.8f);
						glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), "AmbientColor"), 0.4f, 0.4f, 0.4f, 1.0f);
						// glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), "AmbientColor"), 0.0f, 0.0f, 0.0f, 1.0f);
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
			}

			// Standard pass if deferred disabled
			else
			{

				BasicShader->Use();
				{
					// Set up uniforms
					BasicShader->SetUniform("model", model);
					BasicShader->SetUniform("view", view);
					BasicShader->SetUniform("projection", projection);

					// Draw ground tiles
					GroundTileBatch.RenderBatch();
					// Draw debug ground tiles
					if (level.IsDrawDebugEnabled()) 
					{
						GroundTileDebugBatch.RenderBatch();	
						DebugActiveTileBatch.RenderBatch();
						DebugSpatialBatch.RenderBatch();
					}
					// Draw TileOverlays
					OverlayBatch.RenderBatch();
					// Draw entities		
					EntityBatch.RenderBatch();
					// Draw particles
					ParticleBatch.RenderBatch();
				}
				BasicShader->Unuse();
			}

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
			DrawCursor(&HUDBatch, &Input);
		}

		//////////////////////////////////////////////////////////////////////////////////
		// RENDERING CONSOLE /////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////

		// Draw console if on
		if (ShowConsole)
		{
			auto Shader = EG::ShaderManager::GetShader("Text");
			Shader->Use();
			{
				Shader->SetUniform("view", HUDCamera.GetCameraMatrix());

				HUDBatch.Begin();
				{
					HUDBatch.Add(
									EM::Vec4(-SCREENWIDTH * 0.5f, -SCREENHEIGHT * 0.5f, SCREENWIDTH, SCREENHEIGHT),
									EM::Vec4(0, 0, 1, 1),
									EI::ResourceManager::GetTexture("../Assets/Textures/Default.png").id,
									EG::SetOpacity(EG::RGBA16_LightPurple(), 0.4f)
								);
				}
				HUDBatch.End();
				HUDBatch.RenderBatch();

				TextBatch.Begin(EG::GlyphSortType::FRONT_TO_BACK);
				{
					// Render the console input text box
					ConsoleInputTextBox.Draw(&TextBatch);

					// Render the text in the console output
					auto StartPosition = EM::Vec2(-SCREENWIDTH * 0.5f + 9.0f, -SCREENHEIGHT * 0.5f + ConsoleInputTextBox.Dimensions.y);
					auto YOffset = 30.0f;
					for (auto C = ConsoleOutput.rbegin(); C != ConsoleOutput.rend(); C++)
					{
						 EG::Fonts::PrintText(StartPosition.x, StartPosition.y + YOffset, 1.0f, *C, ConsoleInputTextBox.TextFont, TextBatch, EG::RGBA16_LightGrey());
						 YOffset += 20.0f;
					}

				}
				TextBatch.End();
				TextBatch.RenderBatch();
			}
			Shader->Unuse();
		}


		///////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////

		Window.SwapBuffer();

		RenderTime = SDL_GetTicks() - StartTicks;
		
		////////////////////////////////////////////////

		// End FPSLimiter if limiter is enabled
		if (LimiterEnabled)
		{
			FPS = Limiter.End();
		}

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
			PlayerControllerTimeString = std::to_string(PlayerControllerTime);

			// Print loot counts for debugging
			// Loot::PrintCounts();

			counter = 0.0f;
		}
	} 

	return 0;
}

void ProcessInput(Enjon::Input::InputManager* Input, Enjon::Graphics::Camera2D* Camera, ECS::Systems::EntityManager* World, ECS::eid32 Entity)
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

    if (!ShowConsole)
    {
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
		if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyDown(SDLK_LEFT))
		{
			Lights.at(0).Falloff.z -= 0.1f;
		}
		else if (Input->IsKeyDown(SDLK_LEFT))
		{
			LightIntensity -= 0.1f;	
		}
		if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyDown(SDLK_RIGHT))
		{
			Lights.at(0).Falloff.z += 0.1f;
		}
		else if (Input->IsKeyDown(SDLK_RIGHT))
		{
			LightIntensity += 0.1f;	
		}

		if (Input->IsKeyPressed(SDLK_c)) {
			printf("Dashing\n");
			IsDashing = true;
		}

		// Stupid, but use it for now...
		if (Input->IsKeyPressed(SDLK_m)) {
			ShowMap = !ShowMap;
		}

		if (Input->IsKeyPressed(SDLK_0))
		{
			BehaviorTreeEditorOn = !BehaviorTreeEditorOn;
		}

		if (Input->IsKeyPressed(SDLK_p)) {

			AnimationEditorOn = !AnimationEditorOn;
		}

		if (Input->IsKeyPressed(SDLK_i)) 
		{
			// Get camera position
			auto CamPos = Camera->GetPosition();
			auto MouseCoords = Input->GetMouseCoords();	
			Camera->ConvertScreenToWorld(MouseCoords);
			
			// HERE AND QUEER!	
			float height = 0.0f;
			static Math::Vec2 enemydims(100.0f, 100.0f);
			eid32 ai = Factory::CreateAI(World, Math::Vec3(MouseCoords.x, MouseCoords.y, height),
																	enemydims, EG::SpriteSheetManager::GetSpriteSheet("Beast"), "Enemy", 0.05f); 
			World->TransformSystem->Transforms[ai].AABBPadding = EM::Vec2(15);
		}
		if (Input->IsKeyPressed(SDLK_h))
		{
			// Switch debug info
			DebugInfo = !DebugInfo;
		}
		if (Input->IsKeyPressed(SDLK_o))
		{
			DeferredRenderingOn = !DeferredRenderingOn;
		}

		if (Input->IsKeyPressed(SDLK_u))
		{
			World->Lvl->DrawDebugEnabled();
		}

		if (Input->IsKeyPressed(SDLK_g))
		{
			DebugEntityInfo = !DebugEntityInfo;
		}

		if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyDown(SDLK_b))
		{
			// Get camera position
			auto CamPos = Camera->GetPosition();
			auto MouseCoords = Input->GetMouseCoords();	
			Camera->ConvertScreenToWorld(MouseCoords);
			MouseCoords = EM::IsoToCartesian(MouseCoords + EM::Vec2(0.0f, 10.0f));

			// Get grid coordinate
			// Snap to grid
			auto GridCoord = SpatialHash::FindGridCoordinates(World->Grid, MouseCoords);

			// Find Isometric coordinates of GridCoords
			auto CellDimensions = SpatialHash::GetCellDimensions(World->Grid, GridCoord);

			eid32 id = Factory::CreateItem(
											World, 
											Math::Vec3(CellDimensions.x, CellDimensions.y, 0.0f), 
											Enjon::Math::Vec2(700.0f, 700.0f), 
											EG::SpriteSheetManager::GetSpriteSheet("Box"), 
											Masks::Type::ITEM, 
											Component::EntityType::ITEM
										  );

			// World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
			World->CollisionSystem->CollisionComponents[id].ObstructionValue = 1.0f;
			World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
			World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(2000, 2500) / 50.0f;
			World->TransformSystem->Transforms[id].AABBPadding = EM::Vec2(0, 0);
			World->TransformSystem->Transforms[id].GroundPositionOffset = EM::Vec2(-5.0f, -5.0f);
			World->Renderer2DSystem->Renderers[id].Color = EG::SetOpacity(EG::RGBA16_White(), 0.3f);

		}

		else if (Input->IsKeyDown(SDLK_LCTRL) && Input->IsKeyDown(SDLK_b))
		{
			// Get camera position
			auto CamPos = Camera->GetPosition();
			auto MouseCoords = Input->GetMouseCoords();	
			Camera->ConvertScreenToWorld(MouseCoords);
			MouseCoords = EM::IsoToCartesian(MouseCoords + EM::Vec2(0.0f, 10.0f));

			// Get grid coordinate
			// Snap to grid
			auto GridCoord = SpatialHash::FindGridCoordinates(World->Grid, MouseCoords);

			// Find Isometric coordinates of GridCoords
			auto CellDimensions = SpatialHash::GetCellDimensions(World->Grid, GridCoord);


			eid32 id = Factory::CreateItem(
											World, 
											Math::Vec3(CellDimensions.x, CellDimensions.y, 0.0f), 
											Enjon::Math::Vec2(CellDimensions.z - 5.0f, CellDimensions.w - 5.0f), 
											EG::SpriteSheetManager::GetSpriteSheet("DiagonalTileDown"), 
											Masks::Type::ITEM, 
											Component::EntityType::ITEM
										  );

			// World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
			World->CollisionSystem->CollisionComponents[id].ObstructionValue = 1.0f;
			World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
			World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(2000, 2500) / 50.0f;
			World->TransformSystem->Transforms[id].AABBPadding = EM::Vec2(0, 0);
			World->TransformSystem->Transforms[id].GroundPositionOffset = EM::Vec2(-5.0f, -5.0f);
			World->Renderer2DSystem->Renderers[id].Color = EG::SetOpacity(EG::RGBA16_White(), 0.3f);
		}

		else if (Input->IsKeyDown(SDLK_b))
		{
			// Get camera position
			auto CamPos = Camera->GetPosition();
			auto MouseCoords = Input->GetMouseCoords();	
			Camera->ConvertScreenToWorld(MouseCoords);
			MouseCoords = EM::IsoToCartesian(MouseCoords + EM::Vec2(0.0f, 10.0f));

			// Get grid coordinate
			// Snap to grid
			auto GridCoord = SpatialHash::FindGridCoordinates(World->Grid, MouseCoords);

			// Find Isometric coordinates of GridCoords
			auto CellDimensions = SpatialHash::GetCellDimensions(World->Grid, GridCoord);


			eid32 id = Factory::CreateItem(
											World, 
											Math::Vec3(CellDimensions.x, CellDimensions.y, 0.0f), 
											Enjon::Math::Vec2(CellDimensions.z - 5.0f, CellDimensions.w - 5.0f), 
											EG::SpriteSheetManager::GetSpriteSheet("Tile"), 
											Masks::Type::ITEM, 
											Component::EntityType::ITEM
										  );

			// World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
			World->CollisionSystem->CollisionComponents[id].ObstructionValue = 1.0f;
			World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
			World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(2000, 2500) / 50.0f;
			World->TransformSystem->Transforms[id].AABBPadding = EM::Vec2(0, 0);
			World->TransformSystem->Transforms[id].GroundPositionOffset = EM::Vec2(-5.0f, -5.0f);
			World->Renderer2DSystem->Renderers[id].Color = EG::SetOpacity(EG::RGBA16_White(), 0.3f);
		}

		if (Input->IsKeyPressed(SDLK_x))
		{
			// Get camera position
			auto CamPos = Camera->GetPosition();
			auto MouseCoords = Input->GetMouseCoords();	
			Camera->ConvertScreenToWorld(MouseCoords);
			MouseCoords = EM::IsoToCartesian(MouseCoords + EM::Vec2(0.0f, 10.0f));

			// Get grid coordinate
			// Snap to grid
			auto GridCoord = SpatialHash::FindGridCoordinates(World->Grid, MouseCoords);

			auto Index = GridCoord.y * World->Grid->cols + GridCoord.x;

			for (auto& e : World->Grid->cells.at(Index).entities)
			{
				if (World->Types[e] != Component::EntityType::ENEMY && World->Types[e] != Component::EntityType::PLAYER)
				{
					EntitySystem::RemoveEntity(World, e);
				}
			}
		}


		if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyPressed(SDLK_n))
		{
			auto PlayerPosition = &World->TransformSystem->Transforms[World->Player].Position;
			auto Position = EM::Vec2(PlayerPosition->x, PlayerPosition->y + 200.0f);
			
			// auto x_advance = 0.0f;
			// auto F = EG::FontManager::GetFont("8Bit_32");
			// auto c = 'H';
			// auto CS = EG::Fonts::GetCharacterAttributes(EM::Vec2(PlayerPosition->x, PlayerPosition->y + 50.0f), 1.0f, F, c, &x_advance);
			// auto TextureID = CS.TextureID;
			// auto Width = EG::Fonts::GetAdvance(c, F);
			// auto Height = EG::Fonts::GetHeight(c, F);

			auto TextureID = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast.png").id;
			auto Width = 200.0f;
			auto Height = 200.0f;
			auto Center = EM::Vec2(Position.x + Width / 2.0f, Position.y + Height / 2.0f);
			auto Row = 10;
			auto Col = Row;
			auto CurrentUAdvance = 0.0f;
			auto CurrentVAdvance = 0.0f;
			auto UAdvance = (1.0f / static_cast<float>(Col));
			auto VAdvance = (1.0f / static_cast<float>(Row));

			for (auto r = 0; r < Row; r++)
			{
				for (auto c = 0; c < Col; c++)
				{
					// Calculate velocity based off of center

					auto Velocity = EM::Vec3(EM::CartesianToIso(EM::Vec2(ER::Roll(-3, 3), ER::Roll(-3, 3))), -1.0f);
					EG::Particle2D::AddParticle(
													EM::Vec3(Position, 0.0f), 
													Velocity, 
													EM::Vec2(
																Width / Col,
																Height / Row
															), 
													EG::RGBA16_White(),
													TextureID, 
													0.0005f, 
													World->ParticleEngine->ParticleBatches.at(0),
													EM::Vec4(CurrentUAdvance, CurrentVAdvance, CurrentUAdvance + UAdvance, CurrentVAdvance + VAdvance)
												);

					CurrentUAdvance += UAdvance;

				}

				CurrentVAdvance += VAdvance;
				CurrentUAdvance = 0.0f;
			}

			// EG::Particle2D::AddParticle(
			// 								EM::Vec3(Position, 0.0f), 
			// 								EM::Vec3(-1.0f, 0.0f, 0.0f), 
			// 								EM::Vec2(
			// 											Width / 2.0f,
			// 											Height
			// 										), 
			// 								EG::RGBA16_Red(),
			// 								CS.TextureID, 
			// 								0.0005f, 
			// 								World->ParticleEngine->ParticleBatches.at(2),
			// 								EM::Vec4(0.0f, 0.0f, 0.5f, 1.0f)
			// 							);

			// EG::Particle2D::AddParticle(
			// 								EM::Vec3(Position, 0.0f), 
			// 								EM::Vec3(1.0f, 0.0f, 0.0f), 
			// 								EM::Vec2(
			// 											Width / 2.0f,
			// 											Height
			// 										), 
			// 								EG::RGBA16_Red(),
			// 								CS.TextureID, 
			// 								0.0005f, 
			// 								World->ParticleEngine->ParticleBatches.at(2),
			// 								EM::Vec4(0.5f, 0.0f, 1.0f, 1.0f)
			// 							);

		}

		else if (Input->IsKeyPressed(SDLK_n))
		{
			// Get camera position
			auto CamPos = Camera->GetPosition();
			auto MouseCoords = Input->GetMouseCoords();	
			Camera->ConvertScreenToWorld(MouseCoords);

			eid32 id = Factory::CreateItem(
											World, 
											Math::Vec3(MouseCoords.x, MouseCoords.y, 0.0f), 
											Enjon::Math::Vec2(ER::Roll(30, 45), ER::Roll(30, 45)), 
											EG::SpriteSheetManager::GetSpriteSheet("BoxDebris"), 
											Masks::Type::ITEM, 
											Component::EntityType::ITEM
										  );

			World->TransformSystem->Transforms[id].Angle = ER::Roll(0, 360);
			World->AttributeSystem->Masks[id] |= Masks::GeneralOptions::DEBRIS;
			World->TransformSystem->Transforms[id].Mass = (float)ER::Roll(50, 100) / 50.0f;
			World->CollisionSystem->CollisionComponents[id].ObstructionValue = 0.1f;
			World->TransformSystem->Transforms[id].AABBPadding = EM::Vec2(-10, -10);
		}

		if (Input->IsKeyPressed(SDLK_j))
		{
			AIControllerEnabled = !AIControllerEnabled;
		}

		if (Input->IsKeyDown(SDLK_l))
		{
			// Paint tile as obstructed
			auto MP = Input->GetMouseCoords();
			Camera->ConvertScreenToWorld(MP);
			MP = EM::IsoToCartesian(MP - EM::Vec2(50.0f, 0.0f));

			auto GridCoords = SpatialHash::FindGridCoordinates(World->Grid, MP);

			auto Index = GridCoords.y * World->Grid->cols + GridCoords.x;

			// Turn tile obstructed
			World->Grid->cells.at(Index).ObstructionValue = 1.0f;
		}

		if (Input->IsKeyDown(SDLK_SEMICOLON))
		{
			// Paint tile as obstructed
			auto MP = Input->GetMouseCoords();
			Camera->ConvertScreenToWorld(MP);
			MP = EM::IsoToCartesian(MP - EM::Vec2(50.0f, 0.0f));

			auto GridCoords = SpatialHash::FindGridCoordinates(World->Grid, MP);

			auto Index = GridCoords.y * World->Grid->cols + GridCoords.x;

			// Turn tile obstructed
			// World->Grid->cells.at(Index).ObstructionValue += 0.1f;
			World->Grid->cells.at(Index).ObstructionValue = 0.5f;
		}

		if (Input->IsKeyPressed(SDLK_QUOTE))
		{
			// Paint tile as obstructed
			auto MP = Input->GetMouseCoords();
			Camera->ConvertScreenToWorld(MP);
			MP = EM::IsoToCartesian(MP - EM::Vec2(50.0f, 0.0f));

			auto GridCoords = SpatialHash::FindGridCoordinates(World->Grid, MP);

			auto Index = GridCoords.y * World->Grid->cols + GridCoords.x;

			// Turn tile obstructed
			if (Input->IsKeyDown(SDLK_LSHIFT) && World->Grid->cells.at(Index).ObstructionValue < 1.0f) 	World->Grid->cells.at(Index).ObstructionValue += 0.1f;
			else if (World->Grid->cells.at(Index).ObstructionValue < 0.0f) 								World->Grid->cells.at(Index).ObstructionValue -= 0.1f;
		}

		if (Input->IsKeyDown(SDLK_k))
		{
			// Paint tile as unobstructed
			auto MP = Input->GetMouseCoords();
			Camera->ConvertScreenToWorld(MP);
			MP = EM::IsoToCartesian(MP - EM::Vec2(50.0f, 0.0f));

			auto GridCoords = SpatialHash::FindGridCoordinates(World->Grid, MP);

			auto Index = GridCoords.y * World->Grid->cols + GridCoords.x;

			// Turn tile obstructed
			World->Grid->cells.at(Index).ObstructionValue = 0.0f;
		}

		if (Input->IsKeyPressed(SDLK_9))
		{
			DrawSplineOn = !DrawSplineOn;
		}
    }

    else if (ShowConsole)
    {
    	ShowConsole = ConsoleInputTextBox.ProcessInput(Input, Camera);
    }

	// Console
	if (Input->IsKeyPressed(SDLK_BACKQUOTE))
	{
		ShowConsole = !ShowConsole;
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

		 
		Enjon::Math::Vec2 scaledScreenDimensions = Enjon::Math::Vec2((float)SCREENWIDTH, (float)SCREENHEIGHT) / ((Camera->GetScale() * 0.2f));
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
		
		if (xDepth > yDepth && yDepth > 0) 
		{
			L.Radius = std::min(1.0f - distVec.x / SCREENWIDTH / 2.0f, 1.0f - distVec.y / SCREENHEIGHT / 2.0f);
			LightsToDraw.push_back(&L);
		}
	}
}

#endif 


#if 0
/**
*  UNIT TESTS
*/

/*-- External/Engine Libraries includes --*/
#include <Enjon.h>

/*-- IsoARPG Includes --*/
#include "AnimManager.h"

/*-- Standard Library includes --*/
#include <iostream>
#include <sstream>
#include <string>

using json = nlohmann::json;
const int SCREENWIDTH = 1024;
const int SCREENHEIGHT = 768;

#undef main
int main(int argc, char** argv)
{

	Enjon::Init();

	float t = 0.0f;
	float FPS = 0.0f;
	float TimeIncrement = 0.0f;

	// Create a window
	EG::Window Window;
	Window.Init("Unit Test", SCREENWIDTH, SCREENHEIGHT);
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::SHOW);

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

	// InputManager
	EI::InputManager Input = EI::InputManager();

	// Matricies for shaders
	EM::Mat4 Model, View, Projection;

	AnimManager::Init();


	return 0;
}




#endif

/**
* SYSTEMS TEST
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
	#define SCREENRES    EG::FULLSCREEN
#else
	#define SCREENWIDTH  1024
	#define SCREENHEIGHT 768
	#define SCREENRES EG::DEFAULT
#endif 

/*-- External/Engine Libraries includes --*/
#include <Enjon.h>
#include <Editor/AnimationEditor.h>
#include <GUI/GUIAnimationElement.h>
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
#include "AnimManager.h"
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
using namespace EA;

using json = nlohmann::json;

// Just need to get where I can group together GUIElements, transform them together, and then
// access individual GUIElements with the mouse
using namespace Enjon;
using namespace GUI;

/* Function Declarations */
bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera);
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

float caret_count = 0.0f;
bool caret_on = true;

float TimeScale = 1.0f;

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
	EI::InputManager Input;

	// Init AnimationEditor
	Enjon::AnimationEditor::Init();

	// Matricies for shaders
	EM::Mat4 Model, View, Projection;

	auto Json = EU::read_file_sstream(AnimTextureJSONDir.c_str());
    
   	// parse and serialize JSON
   	json j_complete = json::parse(Json);

   	// Get handle to frames data
   	auto Frames = j_complete.at("frames");

    // Get handle to meta deta
    const auto Meta = j_complete.at("meta");

    // Get image size
    auto ISize = Meta.at("size");
    float AWidth = ISize.at("w");
    float AHeight = ISize.at("h");

    Atlas atlas = {	
    			EM::Vec2(AWidth, AHeight), 
				EI::ResourceManager::GetTexture(AnimTextureDir.c_str())
		  	};

    // Init animation manager
    AnimManager::Init();

	// Create animation
	Anim* Test = AnimManager::GetAnimation("Player_Attack_OH_L_SE");

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
	SceneAnimation.CurrentAnimation = Test;
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
	PlayButton.Frames.push_back(EA::GetImageFrame(Frames, "playbuttonup", AnimTextureDir));
	PlayButton.Frames.push_back(EA::GetImageFrame(Frames, "playbuttondown", AnimTextureDir));

	PlayButton.Frames.at(ButtonState::INACTIVE).TextureAtlas = atlas;
	PlayButton.Frames.at(ButtonState::ACTIVE).TextureAtlas = atlas;

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
		// Naive way first - get mouse position in UI space
		EM::Vec2 MouseCoords = Input.GetMouseCoords();
		CameraManager::GetCamera("HUDCamera")->ConvertScreenToWorld(MouseCoords);

		std::string& Text = InputText.Text;
		auto XAdvance = InputText.Position.x;
		uint32_t index = 0;

		std::cout << "Mouse x: " << MouseCoords.x << std::endl;

		// Get advance
		for (auto& c : Text)
		{
			float Advance = EG::Fonts::GetAdvance(c, EG::FontManager::GetFont("WeblySleek"), 1.0f);
			if (XAdvance + Advance < MouseCoords.x) 
			{
				XAdvance += Advance;
				index++;
				std::cout << "XAdvance: " << XAdvance << std::endl;
				std::cout << "Index: " << index << std::endl;
			}
			else break;
		}

		InputText.CursorIndex = index;
		std::cout << "Cursor Index: " << InputText.CursorIndex << std::endl;

		// set caret on to true and count to 0
		caret_count = 0.0f;
		caret_on = true;
	});

	InputText.on_backspace.connect([&]()
	{
		auto str_len = InputText.Text.length();
		auto cursor_index = InputText.CursorIndex;

		// erase from string
		if (str_len > 0 && cursor_index > 0)
		{
			auto S1 = InputText.Text.substr(0, cursor_index - 1);
			std::string S2;

			if (cursor_index + 1 < str_len) S2 = InputText.Text.substr(cursor_index, str_len);

			S1.erase(cursor_index - 1);
			InputText.Text = S1 + S2;
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

	// Main Animation Editor Loop



	// Main loop
	bool running = true;
	while (running)
	{
		Limiter.Begin();

		// Keep track of animation delays
		t += TimeIncrement * TimeScale;

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
			BasicShader->SetUniform("model", Model);
			BasicShader->SetUniform("projection", Projection);
			BasicShader->SetUniform("view", View);

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
				DrawFrame(PBF, PlayButton.Position + Parent->Position, UIBatch, PlayButtonColor);

				// Draw PlayButton AABB
				// Calculate the AABB (this could be set up to another signal and only done when necessary)
				// std::cout << PlayButton.AABB.Max - PlayButton.AABB.Min << std::endl;
				CalculateAABBWithParent(&PlayButton.AABB, &PlayButton);
				// UIBatch->Add(
				// 				EM::Vec4(PlayButton.AABB.Min, PlayButton.AABB.Max - PlayButton.AABB.Min), 
				// 				EM::Vec4(0, 0, 1, 1),
				// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
				// 				EG::SetOpacity(EG::RGBA16_Red(), 0.3f)
				// 			);

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
			BasicShader->SetUniform("view", View);

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

					DrawFrame(*PreviousFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_Blue(), 0.3f));
					DrawFrame(*NextFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_Red(), 0.3f));
				}

				// Draw Scene animation
				DrawFrame(*Frame, Position,	SceneBatch);

			}
			SceneBatch->End();
			SceneBatch->RenderBatch();

		}
		BasicShader->Unuse();

		// Shader for text
		TextShader->Use();
		{
			View = HUDCamera->GetCameraMatrix();

			TextShader->SetUniform("model", Model);
			TextShader->SetUniform("projection", Projection);
			TextShader->SetUniform("view", View);

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
										SceneAnimation.CurrentAnimation->Name, 
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
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 170.0f, scale, 
										std::string("Onion Skin: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				auto OnionString = ToggleOnionSkin.State == ButtonState::ACTIVE ? std::string("On") : std::string("Off");
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 170.0f, scale, 
										OnionString, 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);

				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 190.0f, scale, 
										std::string("Time Scale: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_LightGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera->GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera->GetPosition().y + SCREENHEIGHT / 2.0f - 190.0f, scale, 
										std::to_string(TimeScale), 
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

				caret_count += 0.1f;

				if (caret_count >= 4.0f)
				{
					caret_count = 0.0f;
					caret_on = !caret_on;	
				}

				if (KeyboardFocus && caret_on)
				{
					// Print out caret, make it a yellow line
					// Need to get text from InputText
					auto Text = InputText.Text;
					auto XAdvance = InputText.Position.x + InputText.Parent->Position.x + Padding.x;

					// Get xadvance of all characters
					for (auto i = 0; i < InputText.CursorIndex; ++i)
					{
						XAdvance += EG::Fonts::GetAdvance(InputText.Text[i], CurrentFont, scale);
					}
					UIBatch->Add(
									EM::Vec4(XAdvance + 0.2f, InputText.Position.y + InputText.Parent->Position.y + Padding.y + TextHeight, 1.0f, 10.0f),
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
			else if (CurrentKey == SDLK_LEFT)
			{
				if (InputText->CursorIndex > 0) InputText->CursorIndex--;
			}
			else if (CurrentKey == SDLK_RIGHT)
			{
				if (InputText->CursorIndex < InputText->Text.length()) InputText->CursorIndex++;
			}
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

		if (Input->IsKeyDown(SDLK_t))
		{
			if (TimeScale > 0.0f) TimeScale -= 0.01f;
		}

		if (Input->IsKeyDown(SDLK_y))
		{
			if (TimeScale < 1.0f) TimeScale += 0.01f;
		}
	}

	return true;
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



/**
* SYSTEMS TEST
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
	#define SCREENRES    EG::FULLSCREEN
#else
	#define SCREENWIDTH  1024
	#define SCREENHEIGHT 768
	#define SCREENRES EG::DEFAULT
#endif 

/*-- External/Engine Libraries includes --*/
#include <Enjon.h>
#include <Editor/AnimationEditor.h>
#include <GUI/GUIAnimationElement.h>
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
#include "AnimManager.h"
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
using namespace EA;

using json = nlohmann::json;

// Just need to get where I can group together GUIElements, transform them together, and then
// access individual GUIElements with the mouse
using namespace Enjon;
using namespace GUI;

const std::string AnimTextureDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.png");
const std::string AnimTextureJSONDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.json");
const std::string AnimationDir("../IsoARPG/Profiles/Animations/Player/PlayerAttackOHLSEAnimation.json");

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

	EU::FPSLimiter Limiter;
	Limiter.Init(60);

	// Init ShaderManager
	EG::ShaderManager::Init(); 

	// Init FontManager
	EG::FontManager::Init();

	// InputManager
	EI::InputManager Input;

    // Init animation manager
    AnimManager::Init();

	// Init AnimationEditor
	Enjon::AnimationEditor::Init(&Input, SCREENWIDTH, SCREENHEIGHT);

	// Main loop Animation Editor Loop
	bool running = true;
	while (running)
	{
		Limiter.Begin();

		// Update Animation Editor
		running = Enjon::AnimationEditor::Update();

		//Enable alpha blending
		glEnable(GL_BLEND);

		//Set blend function type
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0));

		// Render scene
		Enjon::AnimationEditor::Draw();

		Window.SwapBuffer();

		FPS = Limiter.End();
	}

	return 0;
}



#endif


#if 0
#include "Enjon.h"

#include <iostream>

namespace PathFinding {
	
	struct Node 
	{
		EM::Vec2 GridCoordinates;
		float GCost;
		float HCost;
		float WCost;
		float FCost;
		Enjon::uint32 Index;
		Node* Parent;
	};

}

template <typename T, typename K>
T PopHeap(std::priority_queue<T, std::vector<T>, K>& Q)
{
	T Val = Q.top();
	Q.pop();
	return Val;
}

template <typename T>
bool SetFind(std::unordered_set<T>& S, T Val)
{
	auto it = S.find(Val);
	if (it != S.end()) return true;
	return false;
}


using namespace Enjon;
using namespace PathFinding;

typedef HeapItem<Node, float> HeapNode;

int32 CompareNode(HeapNode* A, HeapNode* B)
{
	if (A->PriorityValue == B->PriorityValue)  	return 0;
	if (A->PriorityValue >  B->PriorityValue) 	return 1;
	if (A->PriorityValue <  B->PriorityValue) 	return -1;
}

struct CompareInt
{
	bool operator() (int& A, int& B)
	{
		return A > B;
	}
};

#undef main
int main(int argc, char** argv)
{
	Heap<Node, float> OpenSet(100, CompareNode);

	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 30));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 3));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 4));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 25));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 125));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 23));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 9));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 9));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 1));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 2));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 5));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 23));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 9));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 9));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 1));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 2));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 5));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 234));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 12412));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 235));
	OpenSet.Add(HeapItem<Node, float>({EM::Vec2(0.0f), 0, 0, 0, 0, 0, nullptr}, 0));

	while (OpenSet.Count())
	{
		auto N = OpenSet.Pop();

		std::cout << "Value: " << N.PriorityValue << std::endl;
	}

	std::cout << "\n\n\n";

	std::priority_queue<int, std::vector<int>, CompareInt> Ints;	

	Ints.push(4);
	Ints.push(2);
	Ints.push(9);
	Ints.push(123);
	Ints.push(5);
	Ints.push(2345);
	Ints.push(0);
	Ints.push(7);
	Ints.push(45);
	Ints.push(98);
	Ints.push(435);

	while (!Ints.empty())
	{
		auto N = PopHeap(Ints);

		std::cout << "Value: " << N << std::endl;
	}



	return 0;
}

#endif

// NEWBEGIN

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
	#define SCREENRES    EG::WindowFlags::FULLSCREEN
#else
	#define SCREENWIDTH  1024
	#define SCREENHEIGHT 768
	#define SCREENRES EG::WindowFlags::DEFAULT
#endif 

#define PROFILE(profiled) 			\
	static float time##profiled = 0.0f; \
	ticks = SDL_GetTicks();

#define ENDPROFILE(profiled) 			\
	time##profiled += 0.01f;			\
	if (##profiled >= 1.0f) printf("%s: %d\n", profiled, SDL_GetTicks() - ticks);

#include <iostream>

#include <Enjon.h>
#include <System/Types.h>
#include <Graphics/Camera3D.h>
#include <Graphics/ModelAsset.h>
#include <Graphics/Camera.h>
#include <Entity/EntityManager.h>
#include <Graphics/RenderTarget.h>
#include <Graphics/PointLight.h>
#include <Graphics/SpotLight.h>
#include <Graphics/GBuffer.h>
#include <Console.h>
#include <CVarsSystem.h>
#include <Graphics/QuadBatch.h>

using u32 = uint32_t;

EG::ModelAsset GlobalModel;
EG::ModelAsset Floor;
EG::ModelAsset NormalFloor;
EG::ModelAsset Wall;
EG::ModelAsset Cube;
EG::ModelAsset SpriteWithNormal;
EG::ModelAsset MonkeyHead;
EG::ModelAsset OtherCube;
EG::ModelAsset CubeSprite;
EG::ModelAsset UVAnimatedAsset;
std::vector<EG::ModelInstance> Instances;
std::vector<EG::ModelInstance> Animations;
std::vector<EG::ModelInstance> UVAnimations;
EG::Camera FPSCamera;
EG::Camera2D HUDCamera;

const uint8_t RENDERING = 0;

bool DebugDrawingEnabled 	= false;
bool ShowConsole 			= false;
float Tick = 0.0f;

enum class DrawFrameType
{
	FINAL, 
	DIFFUSE, 
	NORMAL, 
	POSITION, 
	BLUR,
	DEPTH,
	LIGHTS
};

DrawFrameType DrawFrame = DrawFrameType::FINAL;

enum class BlurType
{
	SMALL, 
	MEDIUM, 
	LARGE
};

BlurType BlurArrayType = BlurType::SMALL;

EG::SpotLight Spot;

float RotationSpeed = 20.0f;
float VXOffset = 0.01f;
float TextScale = 0.01f;
float TextSpacing = 0.0f;
EM::Vec2 UVScalar = EM::Vec2(1.0f, 0.1f);
EM::Vec2 UVAdditive = EM::Vec2(0, 0);
float DistanceRadius = 0.0f;
float LineWidth = 1.0f;
float Exposure = 1.0f;
float Gamma = 1.0f;
EM::Vec3 BlurWeights(0.38f, 0.32f, 0.39f);
EM::Vec3 BlurIterations(20, 10, 40);

struct ToneMapSettings
{
	float Exposure;
	float Gamma;
	float BloomScalar;
	float Scale;
	float Threshold;
};

struct FXAASettings
{
	float SpanMax;
	float ReduceMin;
	float ReduceMul;
};

struct FXAASettings FXAASettings{8.0f, 1.0/128.0f, 0.5f};
struct ToneMapSettings ToneMapSettings{1.0f, 1.0f, 10.0f, 1.0f, 0.0f};

void LoadUVAnimatedAsset()
{
	// Get mesh
	UVAnimatedAsset.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/dragon.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("UVAnimation");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    UVAnimatedAsset.Shader = Shader;
    // Textures
	UVAnimatedAsset.Material.Textures[EG::TextureSlotType::DIFFUSE] = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield.png");
	UVAnimatedAsset.Material.Textures[EG::TextureSlotType::NORMAL] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/front_normal.png");
	UVAnimatedAsset.Material.Shininess = 100.0f;
}

void LoadMonkeyHeadAsset()
{
	// Get mesh
	MonkeyHead.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/shaderball.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("GBuffer");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    MonkeyHead.Shader = Shader;
    // Textures
	MonkeyHead.Material.Textures[EG::TextureSlotType::DIFFUSE] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_pixel.png");
	MonkeyHead.Material.Textures[EG::TextureSlotType::NORMAL] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall_normal.png");
	MonkeyHead.Material.Shininess = 100.0f;
}

void LoadOtherCubeAsset()
{
	// Get mesh
	OtherCube.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/basic_cube.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("GBuffer");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    OtherCube.Shader = Shader;
    // Textures
	OtherCube.Material.Textures[EG::TextureSlotType::DIFFUSE] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall.png");
	OtherCube.Material.Textures[EG::TextureSlotType::NORMAL] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall_normal.png");
	OtherCube.Material.Shininess = 20.0f;
}

void LoadCubeAsset()
{
	// Cube.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/cube.obj");
	Cube.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/cube.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("GBuffer");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    Cube.Shader = Shader;
    // Set texture
    Cube.Material.Textures[EG::TextureSlotType::DIFFUSE] = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall.png");
    Cube.Material.Textures[EG::TextureSlotType::NORMAL] = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall_normal.png");
    Cube.Material.Shininess = 20.0f;
}

void LoadNormalFloorAsset()
{
	// Get mesh
	NormalFloor.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/quad.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("GBuffer");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    NormalFloor.Shader = Shader;
    // Textures
	NormalFloor.Material.Textures[EG::TextureSlotType::DIFFUSE] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall.png");
	NormalFloor.Material.Textures[EG::TextureSlotType::NORMAL] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall_normal.png");
	NormalFloor.Material.Shininess = 20.0f;
}

void LoadNormalMappedSpriteAsset()
{
	// Get mesh
	SpriteWithNormal.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/quad.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("AnimatedMaterial");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    SpriteWithNormal.Shader = Shader;
    // Textures
	SpriteWithNormal.Material.Textures[EG::TextureSlotType::DIFFUSE] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/TexturePackerTest/test.png");
	SpriteWithNormal.Material.Textures[EG::TextureSlotType::NORMAL] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/TexturePackerTest/test_normal.png");
	SpriteWithNormal.Material.Shininess = 20.0f;
}

void LoadCubeSprite()
{
	// Get mesh
	CubeSprite.Mesh = EI::ResourceManager::GetMesh("../IsoARPG/Assets/Models/quad.obj");

    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("DefaultLighting");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    CubeSprite.Shader = Shader;
    // Textures
	CubeSprite.Material.Textures[EG::TextureSlotType::DIFFUSE] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box.png");
	CubeSprite.Material.Textures[EG::TextureSlotType::NORMAL] 	= EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_normal.png");
	CubeSprite.Material.Shininess = 20.0f;
}

void LoadInstances()
{
	EG::ModelInstance D;
	D.Asset = &SpriteWithNormal;
	D.Transform.Position 	= EM::Vec3(4, 0, 3);
    D.Transform.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(-45), EM::Vec3(0, 1, 0)); 
    D.Transform.Scale 		= EM::Vec3(1.395f, 1.0f, 1.0f);
	Animations.push_back(D);

	EG::ModelInstance U;
	U.Asset = &MonkeyHead;
	U.Transform.Position 	= EM::Vec3(0, 0, 0);
	U.Transform.Scale 		= EM::Vec3(1.0f, 1.0f, 1.0f) * 0.005f;
	Instances.push_back(U);

	EG::ModelInstance B;
	B.Asset = &UVAnimatedAsset;
	B.Transform.Position 	= EM::Vec3(5, 0, 5);
	// B.Transform.Scale 		= EM::Vec3(1.0f, 1.0f, 1.0f) * 0.005f;
	B.Transform.Scale 		= EM::Vec3(1.0f, 1.0f, 1.0f) * 0.75f;
	UVAnimations.push_back(B);

	// for (u32 i = 0; i < 50; ++i)
	// {
	// 	EG::ModelInstance M;
	// 	M.Asset = &MonkeyHead;
	// 	M.Transform.Position 	= EM::Vec3(ER::Roll(-20, 20), ER::Roll(0, 20), ER::Roll(-20, 20));
	// 	float Scale = (float)ER::Roll(1, 10) / 10.0f;
	// 	M.Transform.Scale 		= EM::Vec3(1.0f, 1.0f, 1.0f) * 0.005;
	// 	Instances.push_back(M);
	// }

	/*
	for (u32 i = 0; i < 10000; i++)
	{
		EG::ModelInstance C;
		C.Asset = &SpriteWithNormal;
		C.Transform.Position 	= EM::Vec3(ER::Roll(-50, 50), ER::Roll(0, 50), ER::Roll(-50, 50));
	    C.Transform.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(-45), EM::Vec3(0, 1, 0)); 
	    C.Transform.Scale 		= EM::Vec3(1.395f, 1.0f, 1.0f);
		Animations.push_back(C);
	}
	*/


	// EG::ModelInstance C;
	// C.Asset = &CubeSprite;
	// C.Transform.Position 	= EM::Vec3(10, 0, 10);
	// C.Transform.Scale 		= EM::Vec3(1.0f, 1.0f, 1.0f) * 0.5f;
 //    C.Transform.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(-135), EM::Vec3(0, 1, 0)) * 
 //    							EM::Quaternion::AngleAxis(EM::ToRadians(180), EM::Vec3(0, 0, 1));
 //    Instances.push_back(C);
}

void RenderInstance(const EG::ModelInstance& Instance)
{
	// Get reference to asset pointer
	auto Asset = Instance.Asset;
	auto& Transform = Instance.Transform;

	static GLint CurrentTextureID = 0;

	glBindVertexArray(Asset->Mesh->VAO);
	{
        // Bind instance texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Asset->Material.Textures[EG::TextureSlotType::DIFFUSE].id);

		// Bind normal
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Asset->Material.Textures[EG::TextureSlotType::NORMAL].id);

		// glActiveTexture(GL_TEXTURE2);
		// glBindTexture(GL_TEXTURE_2D, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id);

		EM::Mat4 Model;
		// L = T*R*S
		Model *= EM::Mat4::Translate(Transform.Position);
		Model *= EM::QuaternionToMat4(Transform.Orientation);
		Model *= EM::Mat4::Scale(Transform.Scale);

		static float t = 0.0f;
		t += 0.001f;

		auto& Position = Instances.at(0).Transform.Position;

		Asset->Shader->SetUniform("model", Model);
		Asset->Shader->SetUniform("Near", 0.1f);
		Asset->Shader->SetUniform("Far", 100.0f);
		glDrawArrays(Asset->Mesh->DrawType, 0, Asset->Mesh->DrawCount);

	}
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

struct Frame
{
	float TextureWidth;
	float TextureHeight;
	float FrameWidth; 
	float FrameHeight;
	EM::Vec4 SpriteFrame;
};

std::vector<struct Frame> SpriteFrames;
float frameTimer = 0.0f;
u32 CurrentFrameIndex = 0;

void RenderAnimation(EG::ModelInstance& Instance)
{
	// Get reference to asset pointer
	auto Asset = Instance.Asset;
	auto& Transform = Instance.Transform;

	static GLint CurrentTextureID = 0;


	glBindVertexArray(Asset->Mesh->VAO);
	{
        // Bind instance texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Asset->Material.Textures[EG::TextureSlotType::DIFFUSE].id);

		// Bind normal
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Asset->Material.Textures[EG::TextureSlotType::NORMAL].id);

		// glActiveTexture(GL_TEXTURE2);
		// glBindTexture(GL_TEXTURE_2D, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id);


		auto& CurrentFrame = SpriteFrames.at(CurrentFrameIndex);
		float Scale = CurrentFrame.FrameWidth / CurrentFrame.FrameHeight;
		EM::Vec4 SpriteFrame = CurrentFrame.SpriteFrame;

		Transform.Scale = EM::Vec3(Scale, 1.0f, 1.0f) * 0.8f;

		EM::Mat4 Model;
		// L = T*R*S
		Model *= EM::Mat4::Translate(Transform.Position);
		Model *= EM::QuaternionToMat4(Transform.Orientation);
		Model *= EM::Mat4::Scale(Transform.Scale);

		auto CamPos = FPSCamera.Transform.Position;


		auto& Position = Instances.at(0).Transform.Position;


		Asset->Shader->SetUniform("model", Model);
		Asset->Shader->SetUniform("SpriteFrame", SpriteFrame);
		Asset->Shader->SetUniform("Near", 0.1f);
		Asset->Shader->SetUniform("Far", 100.0f);
		glDrawArrays(Asset->Mesh->DrawType, 0, Asset->Mesh->DrawCount);

	}

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void RenderUVAnimation(EG::ModelInstance& Instance)
{
	// Get reference to asset pointer
	auto Asset = Instance.Asset;
	auto& Transform = Instance.Transform;

	static GLint CurrentTextureID = 0;


	glBindVertexArray(Asset->Mesh->VAO);
	{
        // Bind instance texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Asset->Material.Textures[EG::TextureSlotType::DIFFUSE].id);

		// Bind normal
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Asset->Material.Textures[EG::TextureSlotType::NORMAL].id);

		// glActiveTexture(GL_TEXTURE2);
		// glBindTexture(GL_TEXTURE_2D, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id);

		EM::Mat4 Model;
		// L = T*R*S
		Model *= EM::Mat4::Translate(Transform.Position);
		Model *= EM::QuaternionToMat4(Transform.Orientation);
		Model *= EM::Mat4::Scale(Transform.Scale);

		Asset->Shader->SetUniform("model", Model);
		Asset->Shader->SetUniform("Near", 0.1f);
		Asset->Shader->SetUniform("Far", 100.0f);
		// Asset->Shader->SetUniform("UVAdditive", UVAdditive);
		Asset->Shader->SetUniform("UVScalar", UVScalar);
		glDrawArrays(Asset->Mesh->DrawType, 0, Asset->Mesh->DrawCount);
	}

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void LoadFrames()
{
	struct Frame One;
	One.TextureWidth = 688;
	One.TextureHeight = 493;
	One.FrameWidth = 200;
	One.FrameHeight = One.TextureHeight - 296;
	One.SpriteFrame.x = 0;
	One.SpriteFrame.y = 0;
	One.SpriteFrame.z = One.FrameWidth / One.TextureWidth;
	One.SpriteFrame.w = One.FrameHeight / One.TextureHeight; 

	struct Frame Two;
	Two.TextureWidth = 688;
	Two.TextureHeight = 493;
	Two.FrameWidth = 136;
	Two.FrameHeight = 192;
	Two.SpriteFrame.x = 555 / Two.TextureWidth;
	Two.SpriteFrame.y = (Two.TextureHeight - 302) / Two.TextureHeight;
	Two.SpriteFrame.z = Two.FrameWidth / Two.TextureWidth;
	Two.SpriteFrame.w = Two.FrameHeight / Two.TextureHeight; 

	SpriteFrames.push_back(One);
	SpriteFrames.push_back(Two);
}

// bool ProcessInput(Enjon::Input::InputManager* Input, EG::Camera3D* Camera);
bool ProcessInput(Enjon::Input::InputManager* Input, EG::Camera* Camera);

double SmallGaussianCurve[16];
double MediumGaussianCurve[16];
double LargeGaussianCurve[16];

// Main window
EG::Window Window;

EntityManager* EManager;

void DrawFullScreenQuad(GLuint VAO)
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

double NormalPDF(double x, double s, double m = 0.0f)
{
	static const double inv_sqrt_2pi = 0.3989422804014327;
	double a = (x - m) / s;

	return inv_sqrt_2pi / s * std::exp(-0.5 * a * a);
}

// The MAIN function, from here we start the application and run the game loop
#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{
	Enjon::Init();

	float t = 0.0f;
	float FPS = 0.0f;
	float TimeIncrement = 0.0f;
	u32 ticks = 0;

	double weight;
	double start = -3.0;
	double end = 3.0;
	double denom = 2.0 * end + 1.0;
	double num_samples = 15.0;
	double range = end * 2.0;
	double step = range / num_samples;
	uint32_t i = 0;

	weight = 1.74;
	printf("Small Curve:\n");
	for (double x = start; x <= end; x += step)
	{
		double pdf = NormalPDF(x, 0.23);
		std::cout << x << ": " << pdf << '\n';
		SmallGaussianCurve[i++] = pdf;
	}

	i = 0;
	weight = 3.9f;
	printf("Medium Curve:\n");
	for (double x = start; x <= end; x += step)
	{
		double pdf = NormalPDF(x, 0.775);
		std::cout << x << ": " << pdf << '\n';
		MediumGaussianCurve[i++]= pdf;
	}

	i = 0;
	weight = 2.53f;
	printf("Large Curve:\n");
	for (double x = start; x <= end; x += step)
	{
		double pdf = NormalPDF(x, 1.0);
		std::cout << x << ": " << pdf << '\n';
		LargeGaussianCurve[i++] = pdf;
	}

	// TODO(John): Make an InitSubsystems call in the engine

	// Initialize window
	Window.Init(
				"3D Test", 
				SCREENWIDTH, 
				SCREENHEIGHT, 
				EG::WindowFlagsMask((u32)SCREENRES)
			);
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::HIDE);

	// Init ShaderManager
	EG::ShaderManager::Init(); 

	// Init FontManager
	EG::FontManager::Init();

	FPSCamera = EG::Camera((Enjon::uint32)SCREENWIDTH, (Enjon::uint32)SCREENHEIGHT);
	HUDCamera.Init((Enjon::uint32)SCREENWIDTH, (Enjon::uint32)SCREENHEIGHT);

	// Init Console
	Enjon::Console::Init(SCREENWIDTH, SCREENHEIGHT);

	EG::FrameBufferObject 	FBO((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT);
	EG::RenderTarget 		DebugTarget((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT);
	EG::RenderTarget 		SmallBlurHorizontal((Enjon::u32)SCREENWIDTH / 4, (Enjon::u32)SCREENHEIGHT / 4);
	EG::RenderTarget 		SmallBlurVertical((Enjon::u32)SCREENWIDTH / 4, (Enjon::u32)SCREENHEIGHT / 4);
	EG::RenderTarget 		MediumBlurHorizontal((Enjon::u32)SCREENWIDTH  / 16, (Enjon::u32)SCREENHEIGHT  / 16);
	EG::RenderTarget 		MediumBlurVertical((Enjon::u32)SCREENWIDTH  / 16, (Enjon::u32)SCREENHEIGHT  / 16);
	EG::RenderTarget 		LargeBlurHorizontal((Enjon::u32)SCREENWIDTH / 64, (Enjon::u32)SCREENHEIGHT / 64);
	EG::RenderTarget 		LargeBlurVertical((Enjon::u32)SCREENWIDTH / 64, (Enjon::u32)SCREENHEIGHT / 64);
	EG::RenderTarget 		Composite((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT);
	EG::GBuffer 	 		GBuffer((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT);
	EG::RenderTarget 		DeferredLight((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT);
	EG::RenderTarget 		BrightTarget((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT);
	EG::RenderTarget		FXAATarget((Enjon::u32)SCREENWIDTH, (Enjon::u32)SCREENHEIGHT); 			

	EG::SpriteBatch CompositeBatch;
	CompositeBatch.Init();

	EG::SpriteBatch Batch;
	Batch.Init();

	EG::QuadBatch QBatch;
	EG::QuadBatch FloorBatch;

	QBatch.Init();
	FloorBatch.Init();

	EG::GLSLProgram* CompositeProgram 			= EG::ShaderManager::GetShader("Composite");
	EG::GLSLProgram* HorizontalBlurProgram 		= EG::ShaderManager::GetShader("HorizontalBlur");
	EG::GLSLProgram* VerticalBlurProgram 		= EG::ShaderManager::GetShader("VerticalBlur");
	EG::GLSLProgram* GBufferProgram 			= EG::ShaderManager::GetShader("GBuffer");
	EG::GLSLProgram* DeferredLightProgram 		= EG::ShaderManager::GetShader("DeferredLight");
	EG::GLSLProgram* DirectionalLightProgram 	= EG::ShaderManager::GetShader("DirectionalLight");
	EG::GLSLProgram* PointLightProgram 			= EG::ShaderManager::GetShader("PointLight");
	EG::GLSLProgram* SpotLightProgram 			= EG::ShaderManager::GetShader("SpotLight");
	EG::GLSLProgram* UIProgram					= EG::ShaderManager::GetShader("Text");
	EG::GLSLProgram* FXAAProgram 				= EG::ShaderManager::GetShader("FXAA");
	EG::GLSLProgram* QuadBatchProgram 			= EG::ShaderManager::GetShader("QuadBatch");
	EG::GLSLProgram* WorldTextProgram 			= EG::ShaderManager::GetShader("WorldText");
	EG::GLSLProgram* BrightProgram 				= EG::ShaderManager::GetShader("Bright");

	// Load model data
	LoadCubeAsset();
	LoadCubeSprite();
	LoadNormalMappedSpriteAsset();
	LoadNormalFloorAsset();
	LoadOtherCubeAsset();
	LoadMonkeyHeadAsset();
	LoadUVAnimatedAsset();
	LoadInstances();

	LoadFrames();

	EU::FPSLimiter Limiter;
	Limiter.Init(60);

	// InputManager
	EI::InputManager Input;

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glEnable( GL_MULTISAMPLE );
    // glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    // Initialize FPSCamera
	FPSCamera.Transform.Position = EM::Vec3(7, 2, 7);
	FPSCamera.LookAt(EM::Vec3(0, 0, 0));
	FPSCamera.ProjType = EG::ProjectionType::Orthographic;
	FPSCamera.FieldOfView = 50.0f;
	FPSCamera.ViewPortAspectRatio = (Enjon::f32)SCREENWIDTH / (Enjon::f32)SCREENHEIGHT;
	FPSCamera.OrthographicScale = 4.5f;

	// Initialize entity manager
	EManager = new EntityManager;
	EManager->RegisterComponent<PositionComponent>();
	EManager->RegisterComponent<VelocityComponent>();
	// EManager->RegisterComponent<MovementComponent>();

	// for (auto i = 0; i < MAX_ENTITIES; i++)
	// {
	// 	auto Handle = EManager->CreateEntity();
	// 	Handle->Attach<PositionComponent>();
	// 	Handle->Attach<VelocityComponent>();
	// 	Handle->Attach<MovementComponent>();
	// }

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

    std::vector<EG::PointLight> PointLights;
    for (u32 i = 0; i < 20; i++)
    {
    	EG::PointLight L;
    	L.Position = EM::Vec3(ER::Roll(-50, 50), ER::Roll(0, 10), ER::Roll(-50, 50));
    	L.Parameters = EG::PointLightParameters(1.0f, 0.0f, 0.03f);

    	float R = (float)ER::Roll(0, 255) / 255.0f;
    	float G = (float)ER::Roll(0, 255) / 255.0f;
    	float B = (float)ER::Roll(0, 255) / 255.0f;

    	L.Color = EG::RGBA16(R, G, B, 1.0f);

    	L.Intensity = 1.0f;

    	PointLights.push_back(L);
    }

	Spot = EG::SpotLight(
						FPSCamera.Transform.Position, 
						EG::SpotLightParameters(
													1.0f, 
													0.0f, 
													0.03f, 
													EM::Vec3(7, 0, 7),
													std::cos(EM::ToRadians(12.5f)),
													std::cos(EM::ToRadians(17.5))
												), 
						EG::RGBA16_SkyBlue(),
						10.0f
					);

	float SunlightIntensity = 0.2f;

    // Attach components
    auto Entity = EManager->CreateEntity();
    {
	    auto Position = EManager->Attach<PositionComponent>(Entity);
	    auto Velocity = EManager->Attach<VelocityComponent>(Entity);

	    Position->x = Instances.at(0).Transform.Position.x;
	    Position->y = Instances.at(0).Transform.Position.y;
	    Position->z = Instances.at(0).Transform.Position.z;

	    Velocity->x = 0.0f;
	    Velocity->y = 0.0f;
	    Velocity->z = 0.0f;
    }

    std::vector<EM::Transform> Transforms;
    for (auto i = 0; i < 30000; i++)
    {
    	EM::Transform t;
		t.Position 	= EM::Vec3(ER::Roll(-100, 100), ER::Roll(0, 100), ER::Roll(-100, 100));
	    t.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(-45), EM::Vec3(0, 1, 0)); 
	    t.Scale 		= EM::Vec3(1.395f, 1.0f, 1.0f);
	    Transforms.push_back(t);
    }

    // Set up floor
    FloorBatch.Begin();
    {
		auto map_size = 20;
		for (auto i = 0; i < map_size; i++)
		{
			for (auto j = 0; j < map_size; j++)
			{
				EM::Transform T;
				T.Position = EM::Vec3((Enjon::f32)i * 2.0f, -1.0f, (Enjon::f32)j * 2.0f);
				T.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(90), EM::Vec3(1, 0, 0));
				FloorBatch.Add(
								T, 
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall.png").id
							);
			}
		}
    }
    FloorBatch.End();

	// Register local cvars
	Enjon::CVarsSystem::Register("rotation_speed", &RotationSpeed, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("vxoffset", &VXOffset, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("fxaa_span_max", &FXAASettings.SpanMax, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("fxaa_reduce_min", &FXAASettings.ReduceMin, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("fxaa_reduce_mul", &FXAASettings.ReduceMul, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("sunlight_intensity", &SunlightIntensity, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("text_scale", &TextScale, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("text_spacing", &TextSpacing, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("uv_scalar_x", &UVScalar.x, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("uv_scalar_y", &UVScalar.y, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("uv_additive_x", &UVAdditive.x, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("uv_additive_y", &UVAdditive.y, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("distance_radius", &DistanceRadius, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("line_width", &LineWidth, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("exposure", &ToneMapSettings.Exposure, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("gamma", &ToneMapSettings.Gamma, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("bloom_scalar", &ToneMapSettings.BloomScalar, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("blur_weight_small", &BlurWeights.x, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("blur_weight_medium", &BlurWeights.y, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("blur_weight_large", &BlurWeights.z, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("blur_iter_small", &BlurIterations.x, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("blur_iter_medium", &BlurIterations.y, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("blur_iter_large", &BlurIterations.z, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("threshold", &ToneMapSettings.Threshold, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("scale", &ToneMapSettings.Scale, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("spot_r", &Spot.Color.r, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("spot_g", &Spot.Color.g, Enjon::CVarType::TYPE_FLOAT);
	Enjon::CVarsSystem::Register("spot_b", &Spot.Color.b, Enjon::CVarType::TYPE_FLOAT);

    // Game loop
    bool running = true;
    while (running)
    {
    	Limiter.Begin();

    	Tick += 0.001f;

    	static float t = 0.0f;
    	t += 0.001f;
    	if (t > 50000.0f) t = 0.0f;

		frameTimer += 0.01f;
		if (frameTimer >= 10.0f)
		{
			CurrentFrameIndex = (CurrentFrameIndex + 1) % SpriteFrames.size();
			frameTimer = 0.0f;
		}

    	static float timer = 0.0f;
    	timer += 0.01f;

    	Input.Update();

    	HUDCamera.Update();

    	// Console update
    	Enjon::Console::Update(timer);

    	// Processing input
    	if (ShowConsole) 
    	{
    		ShowConsole = Enjon::Console::ProcessInput(&Input);
    	}

    	else
    	{
	    	running = ProcessInput(&Input, &FPSCamera);
    	}

    	// Camera.Update();
    	auto MouseCoords = Input.GetMouseCoords();

    	// Update the FPS camera
    	EM::Vec3& CamPos = FPSCamera.Transform.Position;

    	// Attach light to "player" position
    	float speed = 2.6f;
    	PointLights.at(0).Position = Animations.at(0).Transform.Position + EM::Vec3(cos(timer * speed), sin(timer * speed), sin(timer * speed));
    	PointLights.at(0).Color = EG::RGBA16_Orange();
    	PointLights.at(1).Position = Animations.at(0).Transform.Position - EM::Vec3(cos(timer * speed), sin(timer * speed), sin(timer * speed));
    	PointLights.at(1).Color = EG::RGBA16_ZombieGreen();

    	PointLights.at(2).Position = Instances.at(0).Transform.Position - EM::Vec3(cos(timer * speed), sin(timer * speed), sin(timer * speed));
    	PointLights.at(2).Color = EG::RGBA16_Orange();
    	PointLights.at(3).Position = Instances.at(0).Transform.Position + EM::Vec3(cos(timer * speed), sin(timer * speed), sin(timer * speed));
    	PointLights.at(3).Color = EG::RGBA16_SkyBlue();
    	PointLights.at(4).Position = Instances.at(0).Transform.Position + EM::Vec3(0.0f, 5.0f, 0.0f);
    	PointLights.at(4).Color = EG::RGBA16_Red();
    	PointLights.at(4).Intensity = EM::Clamp(cos(timer) * 5.0f, 0.0f, 5.0f);

    	PointLights.at(5).Position = UVAnimations.at(0).Transform.Position + EM::Vec3(cos(timer * speed), 0.0f, sin(timer * speed));
    	PointLights.at(5).Color = EG::RGBA16_White();

    	// Set up spot light
    	Spot.Parameters.Direction = EM::Vec3(cos(timer) * speed, 0.0f, sin(timer) * speed);

    	// Update components
    	auto Position = Entity->GetComponent<PositionComponent>();
    	auto Velocity = Entity->GetComponent<VelocityComponent>();

    	Velocity->x = cos(t) * 10.0f;
    	Velocity->z = sin(t) * 10.0f;

    	Position->x = cos(timer);
    	Position->y = sin(timer);
    	Position->z = sin(timer);
    	EM::Vec3 Pos(Position->x, Position->y, Position->z);
    	Instances.at(0).Transform.Position = Pos;

        //////////////////////////////////////////////////////////////////////////
    	// Rendering /////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

		// Rotate one of the instances over time
		auto& InstanceTransform = Instances.at(0).Transform;
		InstanceTransform.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(timer * RotationSpeed), EM::Vec3(0, 1, 0));

		// Rotate one of the instances over time
		auto& InstanceTransform2 = UVAnimations.at(0).Transform;
		InstanceTransform2.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(timer * RotationSpeed), EM::Vec3(0, 1, 0));

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, EG::RGBA16_Black());

    	// Bind FBO
    	GBuffer.Bind();
    	{
	        // Create transformations
	        EM::Mat4 CameraMatrix;
	    	CameraMatrix = FPSCamera.GetViewProjectionMatrix();

	        PROFILE(RENDERING)
	        for (auto& c : Instances)
	        {
	        	auto Shader = c.Asset->Shader;
	        	Shader->Use();
		        	Shader->SetUniform("camera", CameraMatrix);
			        RenderInstance(c);
		        Shader->Unuse();
	        } 

	        for (auto& c : Animations)
	        {
	        	auto Shader = c.Asset->Shader;
	        	Shader->Use();
	        		Shader->SetUniform("camera", CameraMatrix);
	        		RenderAnimation(c);
	        	Shader->Unuse();
	        }

	        for (auto& c : UVAnimations)
	        {
	        	auto Shader = c.Asset->Shader;
	        	Shader->Use();
	        		Shader->SetUniform("camera", CameraMatrix);
	        		Shader->SetUniform("Tick", Tick);
	        		RenderUVAnimation(c);
	        	Shader->Unuse();
	        }

			QuadBatchProgram->Use();
			{
				QuadBatchProgram->SetUniform("camera", CameraMatrix);
				QuadBatchProgram->SetUniform("NearFar", FPSCamera.GetNearFar());

				// Render floor
				QuadBatchProgram->BindTexture("normalMap", EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall_normal.png").id, 1);
				FloorBatch.RenderBatch();

				QBatch.Begin();
				{
					for (uint32_t i = 0; i < Transforms.size(); i++)
					{
						// Check if in range of camera to be drawn or not
						if (EM::Vec3::DistanceSquared(Transforms.at(i).Position, FPSCamera.Transform.Position) <= DistanceRadius)
						{
							Transforms.at(i).Position = Transforms.at(i).Position;
							Transforms.at(i).Scale = EM::Vec3(1, 1, 1) * float(i) / (float)Transforms.size();
							QBatch.Add(
											Transforms.at(i) ,
											EM::Vec4(0, 0, 1, 1), 
											EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/TexturePackerTest/test.png").id
										);
						}
					}

					static float uv_add = 0.0f;
					uv_add += 0.001f;
					QBatch.Add(
								EM::Transform(
												EM::Vec3(0, 2, 13),
												EM::Quaternion::AngleAxis(EM::ToRadians(-90), EM::Vec3(0, 1, 0)),
												EM::Vec3(10, 1, 1)
											),
								EM::Vec4(0.0f + uv_add * UVScalar.x, 0.0f, 1.0f + uv_add * UVScalar.y, 1.0f),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png", GL_LINEAR).id
						);

					// Line rendering?
					QBatch.Add(
								EM::Transform(
												EM::Vec3(10, -0.9, 13),
												EM::Quaternion::AngleAxis(EM::ToRadians(-90), EM::Vec3(0, 1, 0)) * 
												EM::Quaternion::AngleAxis(EM::ToRadians(90), EM::Vec3(0, 0, 1)),
												EM::Vec3(5, 0.1 * LineWidth, 1.0)
											),
								EM::Vec4(0.0f + uv_add * UVScalar.x, 0.0f, 1.0f, 1.0f),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield.png", GL_LINEAR).id,
								EG::RGBA16_ZombieGreen()
						);
				}
				QBatch.End();
				QBatch.RenderBatch();
			}

			QuadBatchProgram->Unuse();

			WorldTextProgram->Use();
			{
				WorldTextProgram->SetUniform("camera", CameraMatrix);
				WorldTextProgram->SetUniform("NearFar", FPSCamera.GetNearFar());
				QBatch.Begin();
				{
						EG::Fonts::PrintText(
												EM::Transform(
																Instances.at(0).Transform.Position + EM::Vec3(-2, 2, 0),
																EM::Quaternion(0, 0, 0, 1), 
																EM::Vec3(TextScale, TextScale, 1)
															),
												"Testing this shit",
												EG::FontManager::GetFont("8Bit_32"),
												QBatch,
												EG::RGBA16_Orange(),
												TextSpacing
										);
						// for (auto i = 0; i < 10; i++)
						// {
						// 	EG::Fonts::PrintText(
						// 							EM::Transform(
						// 											EM::Vec3(-i * 2, i * 2, i * 2),
						// 											EM::Quaternion(0, 0, 0, 1), 
						// 											EM::Vec3(TextScale, TextScale, 1)
						// 										),
						// 							"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
						// 							EG::FontManager::GetFont("8Bit_32"),
						// 							QBatch,
						// 							EG::RGBA16_Orange(),
						// 							TextSpacing
						// 					);
						// }

				}
				QBatch.End();
				WorldTextProgram->BindTexture("normalMap", EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/front_normal.png").id, 1);
				QBatch.RenderBatch();
			}
			WorldTextProgram->Unuse();

	        ENDPROFILE(RENDERING)
    	}
    	GBuffer.Unbind();

		// Light pass
		DeferredLight.Bind();
		{
			// Bind VAO
			glBindVertexArray(quadVAO);

			Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16_Black());

			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_ONE, GL_ONE);

			// Directional lights
			DirectionalLightProgram->Use();
			{
				glEnable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glBlendFunc(GL_ONE, GL_ONE);


				DirectionalLightProgram->BindTexture("DiffuseMap", GBuffer.GetTexture(EG::GBufferTextureType::DIFFUSE), 0);
				DirectionalLightProgram->BindTexture("NormalMap", GBuffer.GetTexture(EG::GBufferTextureType::NORMAL), 1);
				DirectionalLightProgram->BindTexture("PositionMap", GBuffer.GetTexture(EG::GBufferTextureType::POSITION), 2);
				DirectionalLightProgram->SetUniform("Resolution", GBuffer.GetResolution());
				DirectionalLightProgram->SetUniform("CamPos", FPSCamera.Transform.Position);			
				DirectionalLightProgram->SetUniform("CameraForward", FPSCamera.Forward());


				// Direcitonal light
				// NOTE: Will be faster to cache uniforms rather than find them every frame
				DirectionalLightProgram->SetUniform("LightPos", EM::Vec3(	
																		-0.3f, 
																		0.8f, 
																		1.0f)
																	);
				DirectionalLightProgram->SetUniform("LightColor", EM::Vec3(0.6f, 0.3f, 0.1f));
				DirectionalLightProgram->SetUniform("LightIntensity", SunlightIntensity);


				// Render	
				{
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}

				DirectionalLightProgram->SetUniform("LightPos", EM::Vec3(	
																		0.5f, 
																		0.2f, 
																		-0.8f)
																	);
				DirectionalLightProgram->SetUniform("LightColor", EM::Vec3(0.2f, 0.8f, 0.4f));
				DirectionalLightProgram->SetUniform("LightIntensity", 0.2f);

				// Render	
				{
					// glDrawArrays(GL_TRIANGLES, 0, 6);
				}
			}
			DirectionalLightProgram->Unuse();

			// Point lights
			PointLightProgram->Use();
			{
				PointLightProgram->BindTexture("DiffuseMap", GBuffer.GetTexture(EG::GBufferTextureType::DIFFUSE), 0);
				PointLightProgram->BindTexture("NormalMap", GBuffer.GetTexture(EG::GBufferTextureType::NORMAL), 1);
				PointLightProgram->BindTexture("PositionMap", GBuffer.GetTexture(EG::GBufferTextureType::POSITION), 2);
				PointLightProgram->SetUniform("Resolution", GBuffer.GetResolution());
				PointLightProgram->SetUniform("CamPos", FPSCamera.Transform.Position);			
				PointLightProgram->SetUniform("CameraForward", FPSCamera.Forward());

				for (auto& L : PointLights)
				{
					PointLightProgram->SetUniform("LightPos", L.Position);
					PointLightProgram->SetUniform("LightColor", EM::Vec3(L.Color.r, L.Color.g, L.Color.b));
					PointLightProgram->SetUniform("Falloff", L.Parameters.Falloff);
					// PointLightProgram->SetUniform("Radius", L.Parameters.Radius);
					PointLightProgram->SetUniform("LightIntensity", L.Intensity);

					// Render Light to screen
					{
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
				}

			}
			PointLightProgram->Unuse();

			// Do spot lights
			SpotLightProgram->Use();
			{
				SpotLightProgram->BindTexture("DiffuseMap", 	GBuffer.GetTexture(EG::GBufferTextureType::DIFFUSE), 0);
				SpotLightProgram->BindTexture("NormalMap", 		GBuffer.GetTexture(EG::GBufferTextureType::NORMAL), 1);
				SpotLightProgram->BindTexture("PositionMap", 	GBuffer.GetTexture(EG::GBufferTextureType::POSITION), 2);
				SpotLightProgram->SetUniform("Resolution", 		GBuffer.GetResolution());
				SpotLightProgram->SetUniform("CamPos", 			FPSCamera.Transform.Position);			
				SpotLightProgram->SetUniform("CameraForward", 	FPSCamera.Forward());
				SpotLightProgram->SetUniform("Falloff", 		Spot.Parameters.Falloff);
				SpotLightProgram->SetUniform("LightColor", 		EM::Vec3(Spot.Color.r, Spot.Color.g, Spot.Color.b));
				// SpotLightProgram->SetUniform("LightPos", 		Spot.Position);
				SpotLightProgram->SetUniform("LightPos", 		FPSCamera.Transform.Position);
				SpotLightProgram->SetUniform("LightIntensity", 	Spot.Intensity);
				// SpotLightProgram->SetUniform("LightDirection", 	Spot.Parameters.Direction);
				SpotLightProgram->SetUniform("LightDirection", 	FPSCamera.Forward());
				SpotLightProgram->SetUniform("InnerCutoff", 	Spot.Parameters.InnerCutoff);
				SpotLightProgram->SetUniform("OuterCutoff", 	Spot.Parameters.OuterCutoff);

				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			SpotLightProgram->Unuse();
		
			// Unbind VAO
			glBindVertexArray(0);
		}
		DeferredLight.Unbind();

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		// Get bright target
		BrightTarget.Bind();
		{
			Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16_Black());

			BrightProgram->Use();
			{
				BrightProgram->BindTexture("light_tex", DeferredLight.GetTexture(), 0);
				BrightProgram->SetUniform("scale", ToneMapSettings.Scale);
				BrightProgram->SetUniform("threshold", ToneMapSettings.Threshold);

				CompositeBatch.Begin();
				{
		    		CompositeBatch.Add(
								EM::Vec4(-1, -1, 2, 2),
								EM::Vec4(0, 0, 1, 1), 
								DeferredLight.GetTexture()
								);
				}
				CompositeBatch.End();
				CompositeBatch.RenderBatch();
			}
			BrightProgram->Unuse();
		}
		BrightTarget.Unbind();

    	// Small blur
    	for (Enjon::u32 i = 0; i < (Enjon::u32)BlurIterations.x * 2; i++)
    	{
    		bool IsEven = (i % 2 == 0);
    		EG::RenderTarget* Target = IsEven ? &SmallBlurHorizontal : &SmallBlurVertical;
    		EG::GLSLProgram* Program = IsEven ? HorizontalBlurProgram : VerticalBlurProgram;

			Target->Bind();
			{
				Program->Use();
				{
					for (uint32_t j = 0; j < 16; j++)
					{
						std::string Uniform = "blurWeights[" + std::to_string(j) + "]";
						Program->SetUniform(Uniform, SmallGaussianCurve[j]);
					}

					Program->SetUniform("weight", BlurWeights.x);
					GLuint TextureID = i == 0 ? BrightTarget.GetTexture() : IsEven ? SmallBlurVertical.GetTexture() : SmallBlurHorizontal.GetTexture();
					CompositeBatch.Begin();
					{
			    		CompositeBatch.Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1), 
									TextureID
									);
					}
					CompositeBatch.End();
					CompositeBatch.RenderBatch();
				}
				Program->Unuse();
			}	
			Target->Unbind();
    	}

    	// Medium blur iterations
    	for (Enjon::u32 i = 0; i < (Enjon::u32)BlurIterations.y * 2; i++)
    	{
    		bool IsEven = (i % 2 == 0);
    		EG::RenderTarget* Target = IsEven ? &MediumBlurHorizontal : &MediumBlurVertical;
    		EG::GLSLProgram* Program = IsEven ? HorizontalBlurProgram : VerticalBlurProgram;

			Target->Bind();
			{
				Program->Use();
				{
					for (uint32_t j = 0; j < 16; j++)
					{
						std::string Uniform = "blurWeights[" + std::to_string(j) + "]";
						Program->SetUniform(Uniform, MediumGaussianCurve[j]);
					}

					Program->SetUniform("weight", BlurWeights.y);
					GLuint TextureID = i == 0 ? BrightTarget.GetTexture() : IsEven ? MediumBlurVertical.GetTexture() : MediumBlurHorizontal.GetTexture();
					CompositeBatch.Begin();
					{
			    		CompositeBatch.Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1), 
									TextureID
									);
					}
					CompositeBatch.End();
					CompositeBatch.RenderBatch();
				}
				Program->Unuse();
			}	
			Target->Unbind();
    	}

    	// Large blur iterations
    	for (Enjon::u32 i = 0; i < (Enjon::u32)BlurIterations.z * 2; i++)
    	{
    		bool IsEven = (i % 2 == 0);
    		EG::RenderTarget* Target = IsEven ? &LargeBlurHorizontal : &LargeBlurVertical;
    		EG::GLSLProgram* Program = IsEven ? HorizontalBlurProgram : VerticalBlurProgram;

			Target->Bind();
			{
				Program->Use();
				{
					for (uint32_t j = 0; j < 16; j++)
					{
						std::string Uniform = "blurWeights[" + std::to_string(j) + "]";
						Program->SetUniform(Uniform, LargeGaussianCurve[j]);
					}

					Program->SetUniform("weight", BlurWeights.z);
					GLuint TextureID = i == 0 ? BrightTarget.GetTexture() : IsEven ? LargeBlurVertical.GetTexture() : LargeBlurHorizontal.GetTexture();
					CompositeBatch.Begin();
					{
			    		CompositeBatch.Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1), 
									TextureID
									);
					}
					CompositeBatch.End();
					CompositeBatch.RenderBatch();
				}
				Program->Unuse();
			}	
			Target->Unbind();
    	}

		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16_Black());

		FXAATarget.Bind();
		{
			FXAAProgram->Use();
			{
				Window.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16_Black());
				FXAAProgram->SetUniform("resolution", EM::Vec2(SCREENWIDTH, SCREENHEIGHT));
				FXAAProgram->SetUniform("FXAASettings", EM::Vec3(FXAASettings.SpanMax, FXAASettings.ReduceMul, FXAASettings.ReduceMin));
				CompositeBatch.Begin();
				{
					CompositeBatch.Add(
										EM::Vec4(-1, -1, 2, 2),
										EM::Vec4(0, 0, 1, 1),
										DeferredLight.GetTexture()
									);
				}
				CompositeBatch.End();
				CompositeBatch.RenderBatch();
			}
			FXAAProgram->Unuse();
		}
		FXAATarget.Unbind();

		// Final Composite pass
    	CompositeProgram->Use();
    	{
	    	CompositeBatch.Begin();
	    	{
	    		if (!DebugDrawingEnabled)
	    		{
	    			switch(DrawFrame)
	    			{
	    				case DrawFrameType::FINAL:
	    				{
							CompositeProgram->BindTexture("blurTexSmall", SmallBlurVertical.GetTexture(), 1);
							CompositeProgram->BindTexture("blurTexMedium", MediumBlurVertical.GetTexture(), 2);
							CompositeProgram->BindTexture("blurTexLarge", LargeBlurVertical.GetTexture(), 3);
							CompositeProgram->SetUniform("exposure", ToneMapSettings.Exposure);
							CompositeProgram->SetUniform("gamma", ToneMapSettings.Gamma);
							CompositeProgram->SetUniform("bloomScalar", ToneMapSettings.BloomScalar);
							CompositeBatch.Begin();
							{
								CompositeBatch.Add(
													EM::Vec4(-1, -1, 2, 2),
													EM::Vec4(0, 0, 1, 1),
													FXAATarget.GetTexture()
												);
							}
							CompositeBatch.End();
							CompositeBatch.RenderBatch();

	    				} break;	

	    				/*
	    				case DrawFrameType::LIGHTS:
	    				{
							DeferredLight.Bind(EG::RenderTarget::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;

	    				case DrawFrameType::DIFFUSE:
	    				{
							GBuffer.Bind(EG::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0 + (Enjon::u32)EG::GBufferTextureType::DIFFUSE);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;	

	    				case DrawFrameType::NORMAL:
	    				{
							GBuffer.Bind(EG::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0 + (Enjon::u32)EG::GBufferTextureType::NORMAL);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;	

	    				case DrawFrameType::POSITION:
	    				{
							GBuffer.Bind(EG::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0 + (Enjon::u32)EG::GBufferTextureType::POSITION);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;	

	    				case DrawFrameType::BLUR:
	    				{
							BlurVertical.Bind(EG::RenderTarget::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;	

	    				case DrawFrameType::DEPTH:
	    				{
							GBuffer.Bind(EG::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0 + (Enjon::u32)EG::GBufferTextureType::DEPTH);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;	

	    				default:
	    				{
							DeferredLight.Bind(EG::RenderTarget::BindType::READ);
							glReadBuffer(GL_COLOR_ATTACHMENT0);
							glBlitFramebuffer(
												0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, 
												SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
											);

	    				} break;
	    				*/
	    			}
	    		}
	    		else
	    		{
		    		// CompositeBatch.Add(
								// EM::Vec4(-1, 0, 1, 1),
								// EM::Vec4(0, 0, 1, 1), 
								// GBuffer.GetTexture(EG::GBufferTextureType::DIFFUSE)
								// );
		    		// CompositeBatch.Add(
								// EM::Vec4(0, 0, 1, 1),
								// EM::Vec4(0, 0, 1, 1), 
								// GBuffer.GetTexture(EG::GBufferTextureType::NORMAL)
								// );
		    		// CompositeBatch.Add(
								// EM::Vec4(-1, -1, 1, 1),
								// EM::Vec4(0, 0, 1, 1), 
								// GBuffer.GetTexture(EG::GBufferTextureType::POSITION)
								// );
		    		// CompositeBatch.Add(
								// EM::Vec4(-1, -1, 2, 2),
								// EM::Vec4(0, 0, 1, 1), 
								// BlurVertical.GetTexture()
								// );
					// BlurVertical.Bind(EG::RenderTarget::BindType::READ);
					// glReadBuffer(GL_COLOR_ATTACHMENT0);
					// glBlitFramebuffer(
					// 					0, 0, SCREENWIDTH * 4, SCREENHEIGHT * 4, 0, 0, 
					// 					SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR
					// 				);
	    		}

	    	}
		   	CompositeBatch.End();
		   	CompositeBatch.RenderBatch();
		}   	
		CompositeProgram->Unuse();


		/*
		UIProgram->Use();
		{
			CompositeBatch.Begin();
			{
	    		// Add menu bar
	    		CompositeBatch.Add(
	    							EM::Vec4(-1, -1, 0.010f, 2),
	    							EM::Vec4(0, 0, 1, 1),
	    							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/menu_bar.png").id
	    						);

	    		// Add menu bar
	    		CompositeBatch.Add(
	    							EM::Vec4(0.990f, -1, 0.011f, 2),
	    							EM::Vec4(0, 0, 1, 1),
	    							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/menu_bar.png").id
	    						);

	    		// Add menu bar
	    		CompositeBatch.Add(
	    							EM::Vec4(-1, 0.95f, 2, 0.052f),
	    							EM::Vec4(0, 0, 1, 1),
	    							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/menu_bar.png").id
	    						);


	    		// Add menu bar
	    		CompositeBatch.Add(
	    							EM::Vec4(-1, -1, 2, 0.01f),
	    							EM::Vec4(0, 0, 1, 1),
	    							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/menu_bar.png").id
	    						);
			}
			CompositeBatch.End();
			CompositeBatch.RenderBatch();
		}
		UIProgram->Unuse();
		*/

		static float FPS = 0.0f;

		glDisable(GL_DEPTH_TEST);
		UIProgram->Use();
		{
			CompositeBatch.Begin();
			{
				EG::Fonts::Font* F = EG::FontManager::GetFont("Reduction_14");
				UIProgram->SetUniform("view", HUDCamera.GetCameraMatrix());	
				EG::Fonts::PrintText(	
										(float)(-SCREENWIDTH) / 2.0f + 10.0f, 
										(float)SCREENHEIGHT / 2.0f - 40.0f, 
										1.0f, 
										"FPS: ", 
										F, 
										CompositeBatch, 
										EG::RGBA16_White()
									);

				EG::Fonts::PrintText(	
										(float)(-SCREENWIDTH) / 2.0f + EG::Fonts::GetStringAdvance("FPS:F", F), 
										(float)SCREENHEIGHT / 2.0f - 40.0f, 
										1.0f, 
										std::to_string(FPS), 
										EG::FontManager::GetFont("Reduction_14"), 
										CompositeBatch, 
										EG::RGBA16_ZombieGreen()
									);

				std::string DrawCallLabel("Draw Calls: ");
				EG::Fonts::PrintText(	
										(float)(-SCREENWIDTH) / 2.0f + 10.0f, 
										(float)SCREENHEIGHT / 2.0f - 60.0f, 
										1.0f, 
										DrawCallLabel, 
										F, 
										CompositeBatch, 
										EG::RGBA16_White()
									);

				EG::Fonts::PrintText(	
										(float)(-SCREENWIDTH) / 2.0f + EG::Fonts::GetStringAdvance(DrawCallLabel.c_str(), F), 
										(float)SCREENHEIGHT / 2.0f - 60.0f, 
										1.0f, 
										std::to_string(EG::QuadBatch::DrawCallCount), 
										EG::FontManager::GetFont("Reduction_14"), 
										CompositeBatch, 
										EG::RGBA16_ZombieGreen()
									);
			}
			CompositeBatch.End();
			CompositeBatch.RenderBatch();
		}
		UIProgram->Unuse();

		if (ShowConsole)
		{
			Enjon::Console::Draw();
		}
		glEnable(GL_DEPTH_TEST);

        // Swap the screen buffers
        Window.SwapBuffer();

        // FPS = Limiter.End();

        // Reset draw call count
        EG::QuadBatch::DrawCallCount = 0;

    }

    return 0;
}

bool ProcessInput(Enjon::Input::InputManager* Input, EG::Camera* Camera)
{
	static bool FirstMouse = true;
	bool MouseMovement = false;
	static float lastX = SCREENWIDTH / 2.0f;
	static float lastY = SCREENHEIGHT / 2.0f;
	static float xoffset = 0.0f;
	static float yoffset = 0.0f;

	float xPos = SCREENWIDTH / 2.0f;
	float yPos = SCREENHEIGHT / 2.0f;

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

    static float speed = 8.0f;
    static float dt = 0.01f;


	if (Input->IsKeyPressed(SDLK_ESCAPE))
	{
		return false;	
	}

	if (Camera->ProjType == EG::ProjectionType::Perspective)
	{
		Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::HIDE);

	    EM::Vec3 VelDir(0, 0, 0);
	
		if (Input->IsKeyDown(SDLK_w))
		{
			EM::Vec3 F = Camera->Forward();
			// F.y = 0.0f;
			// F = EM::Vec3::Normalize(F);
			VelDir += F;
		}
		if (Input->IsKeyDown(SDLK_s))
		{
			EM::Vec3 B = Camera->Backward();
			// B.y = 0.0f;
			// B = EM::Vec3::Normalize(B);
			VelDir += B;
		}
		if (Input->IsKeyDown(SDLK_a))
		{
			VelDir += Camera->Left();
		}
		if (Input->IsKeyDown(SDLK_d))
		{
			VelDir += Camera->Right();
		}

		if (VelDir.Length()) VelDir = EM::Vec3::Normalize(VelDir);

		Camera->Transform.Position += speed * dt * VelDir;

		auto MouseSensitivity = 7.5f;

		// Get mouse input and change orientation of camera
		auto MouseCoords = Input->GetMouseCoords();

		// Reset the mouse coords after having gotten the mouse coordinates
		SDL_WarpMouseInWindow(Window.GetWindowContext(), SCREENWIDTH / 2.0f, SCREENHEIGHT / 2.0f);

		Camera->OffsetOrientation(
									(EM::ToRadians((SCREENWIDTH / 2.0f - MouseCoords.x) * dt) * MouseSensitivity), 
									(EM::ToRadians((SCREENHEIGHT / 2.0f - MouseCoords.y) * dt) * MouseSensitivity)
								);
	}
	else if (Camera->ProjType == EG::ProjectionType::Orthographic)
	{
		const float PlayerSpeed = 3.0f;

		Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::SHOW);


		if (Input->IsKeyDown(SDLK_q))
		{
			Camera->OrthographicScale += dt * 0.5f;
			std::cout << Camera->OrthographicScale << std::endl;
		}	
		if (Input->IsKeyDown(SDLK_e))
		{
			Camera->OrthographicScale -= dt * 0.5f;
			std::cout << Camera->OrthographicScale << std::endl;
		}

		static EM::Vec3 Rotations(0.0f, 0.0f, 0.0f);
		static float rotational_speed = 1.0f;

		// Get player
		EG::ModelInstance& Player = Animations.at(0);

	    EM::Vec3 VelDir(0, 0, 0);
	
		if (Input->IsKeyDown(SDLK_w))
		{
			VelDir += EM::Vec3(-1, 0, -1);
		}
		if (Input->IsKeyDown(SDLK_s))
		{
			VelDir += EM::Vec3(1, 0, 1);
		}
		if (Input->IsKeyDown(SDLK_a))
		{
			VelDir += EM::Vec3(-1.0f, 0, 1);
		}
		if (Input->IsKeyDown(SDLK_d))
		{
			VelDir += EM::Vec3(1.0f, 0, -1); 
		}


		if (Input->IsKeyDown(SDLK_LSHIFT))
		{
			rotational_speed = 10.0f;
		}
		else rotational_speed = 1.0f;

		if (Input->IsKeyDown(SDLK_y))
		{ 
			if (Input->IsKeyDown(SDLK_UP))
			{
				Rotations.y += 0.01f * rotational_speed;
			}

			if (Input->IsKeyDown(SDLK_DOWN))
			{
				Rotations.y -= 0.01f * rotational_speed;;
			}

		}

		if (Input->IsKeyDown(SDLK_x))
		{ 
			if (Input->IsKeyDown(SDLK_UP))
			{
				Rotations.x += 0.01f * rotational_speed;;
			}

			if (Input->IsKeyDown(SDLK_DOWN))
			{
				Rotations.x -= 0.01f * rotational_speed;;
			}

		}

		if (Input->IsKeyDown(SDLK_z))
		{ 
			if (Input->IsKeyDown(SDLK_UP))
			{
				Rotations.z += 0.01f * rotational_speed;;
			}

			if (Input->IsKeyDown(SDLK_DOWN))
			{
				Rotations.z -= 0.01f * rotational_speed;;
			}

		}

		if (VelDir.Length()) VelDir = EM::Vec3::Normalize(VelDir);

		Camera->Transform.Position = Player.Transform.Position + EM::Vec3(2.5, 2, 3);
		Camera->Transform.Orientation = EM::Quaternion(-0.17f, 0.38f, 0.07f, 0.9f);

		// Camera->Transform.Orientation = EM::Quaternion::AngleAxis(EM::ToRadians(Rotations.x), EM::Vec3(1, 0, 0)) * 
		// 								EM::Quaternion::AngleAxis(EM::ToRadians(Rotations.y), EM::Vec3(0, 1, 0)) * 
		// 								EM::Quaternion::AngleAxis(EM::ToRadians(Rotations.z), EM::Vec3(0, 0, 1)); 

		Player.Transform.Position += PlayerSpeed * dt * VelDir;
		Camera->Transform.Position += PlayerSpeed * dt * VelDir;

		// std::cout << Rotations << '\n';
	}
	

	// Switch between perspective and orthographic camera projections
	if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyPressed(SDLK_o))
	{
		Camera->ProjType = EG::ProjectionType::Perspective;
	}
	else if (Input->IsKeyPressed(SDLK_o))
	{
		Camera->ProjType = EG::ProjectionType::Orthographic;
	}

	if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyPressed(SDLK_m))
	{
		glEnable(GL_MULTISAMPLE);
	}
	else if (Input->IsKeyPressed(SDLK_m))
	{
		glDisable(GL_MULTISAMPLE);
	}

	// Enable debug drawing
	if (Input->IsKeyPressed(SDLK_p))
	{
		DebugDrawingEnabled = !DebugDrawingEnabled;
	}

	if (Input->IsKeyPressed(SDLK_1))
	{
		DrawFrame = DrawFrameType::FINAL;	
	}

	if (Input->IsKeyPressed(SDLK_2))
	{
		DrawFrame = DrawFrameType::DIFFUSE;	
	}

	if (Input->IsKeyPressed(SDLK_3))
	{
		DrawFrame = DrawFrameType::NORMAL;	
	}

	if (Input->IsKeyPressed(SDLK_4))
	{
		DrawFrame = DrawFrameType::POSITION;	
	}

	if (Input->IsKeyPressed(SDLK_5))
	{
		DrawFrame = DrawFrameType::BLUR;	
	}

	if (Input->IsKeyPressed(SDLK_6))
	{
		DrawFrame = DrawFrameType::DEPTH;	
	}

	if (Input->IsKeyPressed(SDLK_7))
	{
		DrawFrame = DrawFrameType::LIGHTS;	
	}

	if (Input->IsKeyDown(SDLK_i))
	{
		if (Input->IsKeyDown(SDLK_DOWN))
		{
			if (Spot.Parameters.InnerCutoff > 0.0f) Spot.Parameters.InnerCutoff -= 0.01f;
		}
		else if (Input->IsKeyDown(SDLK_UP))
		{
			Spot.Parameters.InnerCutoff += 0.01f;
		}
	}

	if (Input->IsKeyDown(SDLK_u))
	{
		if (Input->IsKeyDown(SDLK_DOWN))
		{
			if (Spot.Parameters.OuterCutoff > 0.0f) Spot.Parameters.OuterCutoff -= 0.01f;
		}
		else if (Input->IsKeyDown(SDLK_UP))
		{
			Spot.Parameters.OuterCutoff += 0.01f;
		}
	}

	if (Input->IsKeyDown(SDLK_y))
	{
		if (Input->IsKeyDown(SDLK_DOWN))
		{
			if (Spot.Intensity > 0.0f) Spot.Intensity -= 0.01f;
		}
		else if (Input->IsKeyDown(SDLK_UP))
		{
			Spot.Intensity += 0.01f;
		}
	}

	static float movement_amount = 0.01f;
	if (Input->IsKeyDown(SDLK_n))
	{
		if (Input->IsKeyDown(SDLK_DOWN))
		{
			Spot.Position.y -= movement_amount;
		}
		if (Input->IsKeyDown(SDLK_UP))
		{
			Spot.Position.y += movement_amount;	
		}
		if (Input->IsKeyDown(SDLK_LEFT))
		{
			Spot.Position.x -= movement_amount;	
		}
		if (Input->IsKeyDown(SDLK_RIGHT))
		{
			Spot.Position.x += movement_amount;	
		}
	}
	if (Input->IsKeyDown(SDLK_m))
	{
		if (Input->IsKeyDown(SDLK_DOWN))
		{
			Spot.Position.z -= movement_amount;
		}
		if (Input->IsKeyDown(SDLK_UP))
		{
			Spot.Position.z += movement_amount;	
		}
	}

	if (Input->IsKeyPressed(SDLK_BACKQUOTE))
	{
		ShowConsole = !ShowConsole;
	}

	if (Input->IsKeyPressed(SDLK_j))
	{
		BlurArrayType = BlurType::SMALL;
	}
	if (Input->IsKeyPressed(SDLK_k))
	{
		BlurArrayType = BlurType::MEDIUM;
	}
	if (Input->IsKeyPressed(SDLK_l))
	{
		BlurArrayType = BlurType::LARGE;
	}


	return true;
}


// slut balls
#endif

#if 0

#include <stdio.h>
#include <iostream>
#include <unordered_map>

#include <System/Types.h>
#include <System/Internals.h>
#include <Scripting/ScriptingNode.h>
#include <Math/Maths.h>
#include <Defines.h>

using namespace Enjon;
using namespace Scripting;

#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{
	EM::Vec3 VecA(0.0f, 0.0f, 0.0f);
	EM::Vec3 VecB(3.0f, 3.0f, 0.0f);
	EM::Vec3 VecR(1.0f, 0.0f, 0.0f);

	// Scripted node functionality
	{
		EVec3Node A(VecA);
		EVec3Node B(VecB);
		EVec3Node R(VecR);
		EFloatNode Zero(0.0f);
		EFloatNode NegOne(-1.0f);
		CastToDegreesNode CTD;
		FloatMultiplicationNode FMN;
		Vec3SubtractionVec3Node VSN;
		Vec3NormalizeNode VN;
		Vec3DotProductNode VDP;
		Vec3GetYComponentNode VGY;
		InverseCosineNode ACOS;
		FloatIsLessThanCompareBranchNode FLT;

		// Float multiply
		FMN.SetInputs(&CTD, &NegOne);

		// Cast to Degrees
		CTD.SetInputs(&ACOS);

		// Acos
		ACOS.SetInputs(&VDP);

		// Dot product
		VDP.SetInputs(&VN, &R);

		// Norm
		VN.SetInputs(&VSN);

		// Subtraction
		VSN.SetInputs(&A, &B);

		// Get Y Component
		VGY.SetInputs(&VN);

		// Less than 
		FLT.SetInputs(&VGY, &Zero);
		FLT.SetOutputs(&FMN, &CTD);

		// Entry point
		FLT.Execute();

		if 		(FMN.HasExecuted) 	std::cout << "Mul: " << FMN.Data << std::endl;
		else if (CTD.HasExecuted)	std::cout << "Data: " << CTD.Data << std::endl;
	}


	// Calculated reference
	{
		auto Dir = EM::Vec3::Normalize(VecA - VecB);
		auto Angle = std::acos(Dir.Dot(VecR)) * 180.0f / EM::PI;
		if (Dir.y < 0.0f) Angle *= -1.0f;

		std::cout << "Actual: " << Angle << std::endl;
	}

	return 0;
}

#endif


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
	#define SCREENRES    EG::FULLSCREEN
#else
	#define SCREENWIDTH  1024
	#define SCREENHEIGHT 768
	#define SCREENRES EG::DEFAULT
#endif 

#define PROFILE(profiled) \
	ticks = SDL_GetTicks();

#define ENDPROFILE(profiled) \
	printf("%s: %d\n", profiled, SDL_GetTicks() - ticks);

#include <iostream>

#include <Enjon.h>
#include <System/Types.h>
#include <Graphics/Camera3D.h>
#include <Graphics/ModelAsset.h>
#include <Graphics/Camera.h>
#include <Generated.h>
#include <TestComponent.h>

EG::ModelAsset GlobalModel;
EG::ModelAsset Floor;
EG::ModelAsset Wall;
EG::ModelAsset Cube;
std::vector<EG::ModelInstance> Instances;
EG::Camera FPSCamera;

struct vert3
{
	float Position[3];
	float Normals[3];
	float Tangent[3];
	float Bitangent[3];
	float UV[2];	
	GLubyte Color[4];
};

std::vector<vert3> GetQuad(EM::Vec3 pos1, EM::Vec3 pos2, EM::Vec3 pos3, EM::Vec3 pos4, EM::Vec2 uv1, EM::Vec2 uv2, EM::Vec2 uv3, EM::Vec2 uv4, EM::Vec3 nm, EG::ColorRGBA8 color = EG::RGBA8_White())
{
    // calculate tangent/bitangent vectors of both triangles
    EM::Vec3 tangent1, bitangent1;
    EM::Vec3 tangent2, bitangent2;
    // - triangle 1
    EM::Vec3 edge1 = pos2 - pos1;
    EM::Vec3 edge2 = pos3 - pos1;
    EM::Vec2 deltaUV1 = uv2 - uv1;
    EM::Vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent1 = EM::Vec3::Normalize(tangent1);

    bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent1 = EM::Vec3::Normalize(bitangent1);

    // - triangle 2
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent2 = EM::Vec3::Normalize(tangent2);


    bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent2 = EM::Vec3::Normalize(bitangent2);

    std::vector<vert3> Quad;

    // Tri 1
    Quad.push_back({{pos1.x, pos1.y, pos1.z}, {nm.x, nm.y, nm.z}, {tangent1.x, tangent1.y, tangent1.z}, {bitangent1.x, bitangent1.y, bitangent1.z}, {uv1.x, uv1.y}, {color.r, color.g, color.b, color.a}});
    Quad.push_back({{pos2.x, pos2.y, pos2.z}, {nm.x, nm.y, nm.z}, {tangent1.x, tangent1.y, tangent1.z}, {bitangent1.x, bitangent1.y, bitangent1.z}, {uv2.x, uv2.y}, {color.r, color.g, color.b, color.a}});
    Quad.push_back({{pos3.x, pos3.y, pos3.z}, {nm.x, nm.y, nm.z}, {tangent1.x, tangent1.y, tangent1.z}, {bitangent1.x, bitangent1.y, bitangent1.z}, {uv3.x, uv3.y}, {color.r, color.g, color.b, color.a}});

    // Tri 2
    Quad.push_back({{pos1.x, pos1.y, pos1.z}, {nm.x, nm.y, nm.z}, {tangent2.x, tangent2.y, tangent2.z}, {bitangent2.x, bitangent2.y, bitangent2.z}, {uv1.x, uv1.y}, {color.r, color.g, color.b, color.a}});
    Quad.push_back({{pos3.x, pos3.y, pos3.z}, {nm.x, nm.y, nm.z}, {tangent2.x, tangent2.y, tangent2.z}, {bitangent2.x, bitangent2.y, bitangent2.z}, {uv3.x, uv3.y}, {color.r, color.g, color.b, color.a}});
    Quad.push_back({{pos4.x, pos4.y, pos4.z}, {nm.x, nm.y, nm.z}, {tangent2.x, tangent2.y, tangent2.z}, {bitangent2.x, bitangent2.y, bitangent2.z}, {uv4.x, uv4.y}, {color.r, color.g, color.b, color.a}});

    return Quad;
}

void LoadCubeAsset()
{
	// FRONT
	// positions
    EM::Vec3 front_1(-1.0, 1.0, -1.0);
    EM::Vec3 front_2(-1.0, -1.0, -1.0);
    EM::Vec3 front_3(1.0, -1.0, -1.0);
    EM::Vec3 front_4(1.0, 1.0, -1.0);
    // texture coordinates
    EM::Vec2 front_uv_1(0.0, 1.0);
    EM::Vec2 front_uv_2(0.0, 0.0);
    EM::Vec2 front_uv_3(1.0, 0.0);
    EM::Vec2 front_uv_4(1.0, 1.0);
    // normal vector
    EM::Vec3 front_nm(0.0, 0.0, -1.0);

    auto FrontFace = GetQuad(front_1, front_2, front_3, front_4, front_uv_1, front_uv_2, front_uv_3, front_uv_4, front_nm);

	// RIGHT
	// positions
    EM::Vec3 right_1(1.0, 1.0, -1.0);
    EM::Vec3 right_2(1.0, -1.0, -1.0);
    EM::Vec3 right_3(1.0, -1.0, 1.0);
    EM::Vec3 right_4(1.0, 1.0, 1.0);
    // texture coordinates
    EM::Vec2 right_uv_1(0.0, 1.0);
    EM::Vec2 right_uv_2(0.0, 0.0);
    EM::Vec2 right_uv_3(1.0, 0.0);
    EM::Vec2 right_uv_4(1.0, 1.0);
    // normal vector
    EM::Vec3 right_nm(1.0, 0.0, 0.0);

    auto RightFace = GetQuad(right_1, right_2, right_3, right_4, right_uv_1, right_uv_2, right_uv_3, right_uv_4, right_nm);

	// LEFT
	// positions
    EM::Vec3 left_1(-1.0, 1.0, 1.0);
    EM::Vec3 left_2(-1.0, -1.0, 1.0);
    EM::Vec3 left_3(-1.0, -1.0, -1.0);
    EM::Vec3 left_4(-1.0, 1.0, -1.0);
    // texture coordinates
    EM::Vec2 left_uv_1(0.0, 1.0);
    EM::Vec2 left_uv_2(0.0, 0.0);
    EM::Vec2 left_uv_3(1.0, 0.0);
    EM::Vec2 left_uv_4(1.0, 1.0);
    // normal vector
    EM::Vec3 left_nm(-1.0, 0.0, 0.0);

    auto LeftFace = GetQuad(left_1, left_2, left_3, left_4, left_uv_1, left_uv_2, left_uv_3, left_uv_4, left_nm);

	// BACK
	// positions
    EM::Vec3 back_1(-1.0,  1.0,  1.0);
    EM::Vec3 back_2(-1.0, -1.0,  1.0);
    EM::Vec3 back_3( 1.0, -1.0,  1.0);
    EM::Vec3 back_4( 1.0,  1.0,  1.0);
    // texture coordinates
    EM::Vec2 back_uv_1(0.0, 1.0);
    EM::Vec2 back_uv_2(0.0, 0.0);
    EM::Vec2 back_uv_3(1.0, 0.0);
    EM::Vec2 back_uv_4(1.0, 1.0);
    // normal vector
    EM::Vec3 back_nm(0.0, 0.0, 1.0);

    auto BackFace = GetQuad(back_1, back_2, back_3, back_4, back_uv_1, back_uv_2, back_uv_3, back_uv_4, back_nm);

	// TOP
	// positions
    EM::Vec3 top_1(-1.0,  1.0,  1.0);
    EM::Vec3 top_2(-1.0,  1.0, -1.0);
    EM::Vec3 top_3( 1.0,  1.0, -1.0);
    EM::Vec3 top_4( 1.0,  1.0,  1.0);
    // texture coordinates
    EM::Vec2 top_uv_1(0.0, 1.0);
    EM::Vec2 top_uv_2(0.0, 0.0);
    EM::Vec2 top_uv_3(1.0, 0.0);
    EM::Vec2 top_uv_4(1.0, 1.0);
    // normal vector
    EM::Vec3 top_nm(0.0, 1.0, 0.0);

    auto TopFace = GetQuad(top_1, top_2, top_3, top_4, top_uv_1, top_uv_2, top_uv_3, top_uv_4, top_nm);

	// BOTTOM
	// positions
    EM::Vec3 bottom_1(-1.0, -1.0,  1.0);
    EM::Vec3 bottom_2(-1.0, -1.0, -1.0);
    EM::Vec3 bottom_3( 1.0, -1.0, -1.0);
    EM::Vec3 bottom_4( 1.0, -1.0,  1.0);
    // texture coordinates
    EM::Vec2 bottom_uv_1(0.0, 1.0);
    EM::Vec2 bottom_uv_2(0.0, 0.0);
    EM::Vec2 bottom_uv_3(1.0, 0.0);
    EM::Vec2 bottom_uv_4(1.0, 1.0);
    // normal vector
    EM::Vec3 bottom_nm(0.0, -1.0, 0.0);

    auto BottomFace = GetQuad(bottom_1, bottom_2, bottom_3, bottom_4, bottom_uv_1, bottom_uv_2, bottom_uv_3, bottom_uv_4, bottom_nm);

    vert3 Verts[] = 
    {
    	// FrontFace
    	FrontFace.at(0), 
    	FrontFace.at(1), 
    	FrontFace.at(2), 
    	FrontFace.at(3),
    	FrontFace.at(4),
    	FrontFace.at(5),

    	// Right
    	RightFace.at(0), 
    	RightFace.at(1), 
    	RightFace.at(2), 
    	RightFace.at(3),
    	RightFace.at(4),
    	RightFace.at(5),

    	// Left
    	LeftFace.at(0), 
    	LeftFace.at(1), 
    	LeftFace.at(2), 
    	LeftFace.at(3),
    	LeftFace.at(4),
    	LeftFace.at(5),

    	// Back
    	BackFace.at(0), 
    	BackFace.at(1), 
    	BackFace.at(2), 
    	BackFace.at(3),
    	BackFace.at(4),
    	BackFace.at(5),

    	// Top
    	TopFace.at(0), 
    	TopFace.at(1), 
    	TopFace.at(2), 
    	TopFace.at(3),
    	TopFace.at(4),
    	TopFace.at(5),

    	// Bottom
    	BottomFace.at(0), 
    	BottomFace.at(1), 
    	BottomFace.at(2), 
    	BottomFace.at(3),
    	BottomFace.at(4),
    	BottomFace.at(5)
    };


    glGenBuffers(1, &Cube.Mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Cube.Mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Verts), Verts, GL_STATIC_DRAW);

    glGenVertexArrays(1, &Cube.Mesh.VAO);
    glBindVertexArray(Cube.Mesh.VAO);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert3), (void*)offsetof(vert3, Position));
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vert3), (void*)offsetof(vert3, Normals));
    // Tangent
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vert3), (void*)offsetof(vert3, Tangent));
    // Bitangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vert3), (void*)offsetof(vert3, Bitangent));
    // UV
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(vert3), (void*)offsetof(vert3, UV));
    // Color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vert3), (void*)offsetof(vert3, Color));

    // Unbind VAO
    glBindVertexArray(0);


    // Get shader and set texture
    auto Shader = EG::ShaderManager::GetShader("DefaultLighting");
    Shader->Use();
    	Shader->SetUniform("diffuseMap", 0);
    	Shader->SetUniform("normalMap", 1);
    Shader->Unuse();

    // Set shader
    Cube.Shader = Shader;
    // Set texture
    Cube.Texture = EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_pixel.png");
    // Set draw type
    Cube.DrawType = GL_TRIANGLES;
    // Set draw count
    Cube.DrawCount = 36;
}

const u32 ENTITY_AMOUNT = 5;

void LoadInstances()
{
	for (auto i = 0; i < ENTITY_AMOUNT; i++)
	{
		EG::ModelInstance C;
		C.Asset = &Cube;
		C.Transform.Position = EM::Vec3(ER::Roll(-10, 10), ER::Roll(-10, 10), ER::Roll(-10, 10));
		Instances.push_back(C);
	}
}

void RenderInstance(const EG::ModelInstance& Instance)
{
	// Get reference to asset pointer
	auto Asset = Instance.Asset;
	auto& Transform = Instance.Transform;

	static GLint CurrentTextureID = 0;

	switch(Asset->DrawType)
	{
		case GL_TRIANGLE_STRIP:
		{
			glBindVertexArray(Asset->VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Asset->IBO);
			{
				if (CurrentTextureID != Asset->Texture.id)
				{
					CurrentTextureID = Asset->Texture.id;

			        // Bind instance texture
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, Asset->Texture.id);
				}

				Asset->Shader->SetUniform("transform", Transform);
				glDrawElements(Asset->DrawType, Asset->DrawCount, GL_UNSIGNED_INT, nullptr);
			}
			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		} break;

		case GL_TRIANGLES:
		{
			glBindVertexArray(Asset->VAO);
			{
				if (CurrentTextureID != Asset->Texture.id)
				{
					CurrentTextureID = Asset->Texture.id;

			        // Bind instance texture
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, Asset->Texture.id);
				}

				static float t = 0.0f;
				t += 0.01f;

				// Bind normal
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_sheet_normal.png").id);

				EM::Mat4 Model;
				// Model *= EM::Mat4::Scale(Transform.Scale);
				Model *= EM::Mat4::Translate(Transform.Position);
				Model *= EM::QuaternionToMat4(Transform.Orientation);

				auto CamPos = FPSCamera.Transform.Position;

				Asset->Shader->SetUniform("model", Model);
				// Asset->Shader->SetUniform("transform", Transform);
				Asset->Shader->SetUniform("lightPosition", CamPos);
				glDrawArrays(Asset->DrawType, 0, Asset->DrawCount);
			}
			glBindVertexArray(0);
		} break;

	}

}

// bool ProcessInput(Enjon::Input::InputManager* Input, EG::Camera3D* Camera);
bool ProcessInput(Enjon::Input::InputManager* Input, EG::Camera* Camera);

typedef u32 entity;

// Main window
EG::Window Window;

// The MAIN function, from here we start the application and run the game loop
#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{
	Enjon::Init();

	float t = 0.0f;
	float FPS = 0.0f;
	float TimeIncrement = 0.0f;
	u32 ticks = 0;

	// Initialize window
	Window.Init("3D Test", SCREENWIDTH, SCREENHEIGHT, SCREENRES);
	Window.ShowMouseCursor(Enjon::Graphics::MouseCursorFlags::HIDE);


	// Init ShaderManager
	EG::ShaderManager::Init(); 

	// Init FontManager
	EG::FontManager::Init();

	FPSCamera = EG::Camera((Enjon::uint32)SCREENWIDTH, (Enjon::uint32)SCREENHEIGHT);

	EG::SpriteBatch Batch;
	Batch.Init();

	// Load model data
	LoadCubeAsset();
	LoadInstances();

	EU::FPSLimiter Limiter;
	Limiter.Init(60);

	// InputManager
	EI::InputManager Input;

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    glEnable( GL_MULTISAMPLE );
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Initialize FPSCamera
	FPSCamera.Transform.Position = EM::Vec3(7, 2, 7);
	FPSCamera.LookAt(EM::Vec3(0, 0, 0));
	FPSCamera.ProjType = EG::ProjectionType::Orthographic;
	FPSCamera.FieldOfView = 50.0f;
	FPSCamera.ViewPortAspectRatio = (Enjon::f32)SCREENWIDTH / (Enjon::f32)SCREENHEIGHT;
	FPSCamera.OrthographicScale = 4.5f;


    // Game loop
    bool running = true;
    while (running)
    {
    	static float t = 0.0f;
    	t += 0.001f;
    	if (t > 50000.0f) t = 0.0f;

    	Input.Update();

    	running = ProcessInput(&Input, &FPSCamera);

    	// Camera.Update();
    	auto MouseCoords = Input.GetMouseCoords();

    	// Update the FPS camera
    	EM::Vec3& CamPos = FPSCamera.Transform.Position;

    	// Rendering
		Window.Clear(1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, EG::RGBA16(0.05f, 0.05f, 0.05f, 1.0f));

        // Create transformations
        EM::Mat4 CameraMatrix;
    	CameraMatrix = FPSCamera.GetViewMatrix();

        Instances.at(0).Transform.Orientation = EM::Quaternion::AngleAxis(120 * EM::ToRadians(t), EM::Vec3(0, 1, 0)) * 
        										EM::Quaternion::AngleAxis(20  * EM::ToRadians(t), EM::Vec3(0, 0, 1));

        for (auto& c : Instances)
        {
        	auto Shader = c.Asset->Shader;
        	Shader->Use();
	        	Shader->SetUniform("camera", CameraMatrix);
		        RenderInstance(c);
	        Shader->Unuse();
        }

        // Swap the screen buffers
        Window.SwapBuffer();
    }
    // Properly de-allocate all resources once they've outlived their purpose
    return 0;
}

bool ProcessInput(Enjon::Input::InputManager* Input, EG::Camera* Camera)
{
	static bool FirstMouse = true;
	bool MouseMovement = false;
	static float lastX = SCREENWIDTH / 2.0f;
	static float lastY = SCREENHEIGHT / 2.0f;
	static float xoffset = 0.0f;
	static float yoffset = 0.0f;

	float xPos = SCREENWIDTH / 2.0f;
	float yPos = SCREENHEIGHT / 2.0f;

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

    static float speed = 8.0f;
    static float dt = 0.01f;


	if (Input->IsKeyPressed(SDLK_ESCAPE))
	{
		return false;	
	}

	if (Camera->ProjType == EG::ProjectionType::Perspective)
	{
	    EM::Vec3 VelDir(0, 0, 0);
	
		if (Input->IsKeyDown(SDLK_w))
		{
			EM::Vec3 F = Camera->Forward();
			// F.y = 0.0f;
			// F = EM::Vec3::Normalize(F);
			VelDir += F;
		}
		if (Input->IsKeyDown(SDLK_s))
		{
			EM::Vec3 B = Camera->Backward();
			// B.y = 0.0f;
			// B = EM::Vec3::Normalize(B);
			VelDir += B;
		}
		if (Input->IsKeyDown(SDLK_a))
		{
			VelDir += Camera->Left();
		}
		if (Input->IsKeyDown(SDLK_d))
		{
			VelDir += Camera->Right();
		}

		if (VelDir.Length()) VelDir = EM::Vec3::Normalize(VelDir);

		Camera->Transform.Position += speed * dt * VelDir;

		auto MouseSensitivity = 7.5f;

		// Get mouse input and change orientation of camera
		auto MouseCoords = Input->GetMouseCoords();

		// Reset the mouse coords after having gotten the mouse coordinates
		SDL_WarpMouseInWindow(Window.GetWindowContext(), SCREENWIDTH / 2.0f, SCREENHEIGHT / 2.0f);

		Camera->OffsetOrientation(
									(EM::ToRadians((SCREENWIDTH / 2.0f - MouseCoords.x) * dt) * MouseSensitivity), 
									(EM::ToRadians((SCREENHEIGHT / 2.0f - MouseCoords.y) * dt) * MouseSensitivity)
								);
	}
	

	// Switch between perspective and orthographic camera projections
	if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyPressed(SDLK_o))
	{
		Camera->ProjType = EG::ProjectionType::Perspective;
	}
	else if (Input->IsKeyPressed(SDLK_o))
	{
		Camera->ProjType = EG::ProjectionType::Orthographic;
	}

	if (Input->IsKeyDown(SDLK_LSHIFT) && Input->IsKeyPressed(SDLK_m))
	{
		glEnable(GL_MULTISAMPLE);
	}
	else if (Input->IsKeyPressed(SDLK_m))
	{
		glDisable(GL_MULTISAMPLE);
	}


	return true;
}


// slut balls
#endif






#if 0

#include <stdio.h>
#include <cmath>

struct vec3
{
	float x;
	float y;
	float z;
};

struct plane
{
	vec3 Normal;
	float Distance;	
};

enum planes_types
{
	LEFT, 
	RIGHT, 
	TOP, 
	BOTTOM, 
	NEAR, 
	FAR
};

struct frustum
{
	plane Planes[6];
};

vec3 CrossProduct(vec3 A, vec3 B)
{
	vec3 Cross;

	Cross.x = A.y * B.z - A.z * B.y; 	
	Cross.y = A.z * B.x - A.x * B.z;
	Cross.z = A.x * B.y - A.y * B.x;

	return Cross;
}

float VecLength(vec3 A)
{
	return sqrt(A.x * A.x + A.y * A.y + A.z * A.z);
}

vec3 Normalize(vec3 A)
{
	vec3 NormalizedVec;

	float Length = VecLength(A);

	NormalizedVec.x = A.x / Length;
	NormalizedVec.y = A.y / Length;
	NormalizedVec.z = A.z / Length;

	return NormalizedVec;
}

vec3 SubtractVec(vec3 A, vec3 B)
{
	vec3 ReturnVec;
	ReturnVec.x = A.x - B.x;
	ReturnVec.y = A.y - B.y;
	ReturnVec.z = A.z - B.z;

	return ReturnVec;
}

void PrintVec3(vec3 A)
{
	printf("(%f, %f, %f)", A.x, A.y, A.z);	
}

float Vec3Dot(vec3 A, vec3 B)
{
	return (A.x * B.x + A.y * B.y + A.z * B.z);
}

plane CalculatePlane(vec3 P0, vec3 P1, vec3 P2, vec3 P3)
{
	plane Plane;

	// Get vector from P1-P0
	vec3 V1 = SubtractVec(P1, P0);
	vec3 V2 = SubtractVec(P3, P2);

	// Cross these two
	vec3 Cross = CrossProduct(V1, V2);
	// Normalize
	vec3 Normalized = Normalize(Cross);

	// Find distance from origin
	// d = -ax - by - cz, where x = P.x, y = P.y, z = P.z
	float Distance = -Normalized.x * P0.x + -Normalized.y * P0.y + -Normalized.z * P0.z;

	Plane.Normal = Normalized;
	Plane.Distance = Distance;

	return Plane;
}

void PrintPlane(plane* Plane)
{
	PrintVec3(Plane->Normal);
	printf(", D: %f\n", Plane->Distance);
}

bool SphereFrustumIntersection(frustum* Frustum, vec3 Center, float Radius)
{
	for (int i = 0; i < 6; i++)
	{
		float Value = Vec3Dot(Frustum->Planes[i].Normal, Center) + Frustum->Planes[i].Distance + Radius;
		if (Value <= 0.0f) 
		{
			switch(i)
			{
				case LEFT: printf(" LEFT: %f\n", Value); break;
				case RIGHT: printf(" RIGHT: %f\n", Value); break;
				case TOP: printf(" TOP: %f\n", Value); break;
				case BOTTOM: printf(" BOTTOM: %f\n", Value); break;
				case NEAR: printf(" NEAR: %f\n", Value); break;
				case FAR: printf(" FAR: %f\n", Value); break;
			}
			return false;
		}
	}

	return true;
}

int main(int argc, char** argv)
{
	vec3 P0 = {-1.0f, 1.0f, 1.0f};
	vec3 P1 = { 1.0f, 1.0f, 1.0f};
	vec3 P2 = { 1.0f,-1.0f, 1.0f};
	vec3 P3 = {-1.0f,-1.0f, 1.0f};

	vec3 P4 = {-2.0f, 2.0f,-2.0f};
	vec3 P5 = { 2.0f, 2.0f,-2.0f};
	vec3 P6 = { 2.0f,-2.0f,-2.0f};
	vec3 P7 = {-2.0f,-2.0f,-2.0f};

	// Left plane
	plane Top 		= CalculatePlane(P0, P4, P0, P1);
	plane Bottom 	= CalculatePlane(P3, P7, P2, P3);
	plane Near 		= CalculatePlane(P3, P0, P0, P1);
	plane Far 		= CalculatePlane(P7, P4, P5, P4);
	plane Left 		= CalculatePlane(P0, P4, P3, P0);
	plane Right 	= CalculatePlane(P1, P5, P1, P2);

	printf("Top: ");
	PrintPlane(&Top);
	printf("Bottom: ");
	PrintPlane(&Bottom);
	printf("Near: ");
	PrintPlane(&Near);
	printf("Far: ");
	PrintPlane(&Far);
	printf("Right: ");
	PrintPlane(&Right);
	printf("Left: ");
	PrintPlane(&Left);

	frustum Frustum;
	Frustum.Planes[LEFT] 	= Left;
	Frustum.Planes[RIGHT] 	= Right;
	Frustum.Planes[TOP] 	= Top;
	Frustum.Planes[BOTTOM] 	= Bottom;
	Frustum.Planes[NEAR] 	= Near;
	Frustum.Planes[FAR] 	= Far;

	vec3 SphereCenter = {2.0f, 0.0f, 1.0f};
	float Radius = 1.0f;

	if (SphereFrustumIntersection(&Frustum, SphereCenter, Radius))
		printf("Intersection!\n");
	else
		printf("No Intersection!\n");


	return 0;
}


#endif







