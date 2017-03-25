#include <assert.h>

#include "Graphics/ParticleEngine2D.h"
#include "Graphics/FontManager.h"
#include "IO/ResourceManager.h"
#include "Math/Maths.h"
#include "Defines.h"
#include "Utils/Errors.h"


namespace Enjon { namespace Particle2D { 

	/*-- Function Declarations --*/

	/* Finds next available particle in particle batch */
	uint32 FindNextAvailableParticle(ParticleBatch2D* PB);

	/* Updates the particle batch */
	uint32 ParticleBatch2D::Update()
	{
		// Update each individual particle
		// TODO(John): Optimize this by only looping through alive particles
		for (auto& p : Particles)
		{
			float* LT = &p.LifeTime;
			float* DR = &p.DecayRate; 
			
			// Decay particle lifetime by rate
			*LT -= *DR;

			Vec3* Position = &p.Position;
			Vec3* Velocity = &p.Velocity;

			// Move position of particle by its velocity
			Position->x += Velocity->x;
			Position->z += Velocity->z;

			// Add z to y component of position
			Position->y += Velocity->y + Velocity->z;

			// Decay particle alpha over time
			// p.Color.a -= p.DecayRate * 0.5f;
			p.Color.a *= *LT;
		}

		return 1;
	}

	/* Updates the particle engine */
	uint32 ParticleEngine2D::Update()
	{
		// Update each individual particle batch in engine
		for (auto& b : ParticleBatches)
		{
			b->Update();
		}

		return 1;
	}

	/* Creates a ParticleEngine2D* and returns it */
	ParticleEngine2D* NewParticleEngine()
	{
		ParticleEngine2D* PE = new ParticleEngine2D();
		assert(PE);
		return PE;	
	}

	/* Creates a ParticleBatch2D* and returns it */
	ParticleBatch2D* NewParticleBatch(Enjon::SpriteBatch* SB)
	{
		ParticleBatch2D* PB = new ParticleBatch2D();
		// PB->Particles.reserve(MAXPARTICLES);
		for (auto i = 0; i < MAXPARTICLES; i++)
		{
			PB->Particles.push_back(Particle{});
		}
		assert(PB);
		PB->SB = SB;
		return PB;		
	}


	/* Adds a particle batch to a particle engine */
	uint32 AddParticleBatch(ParticleEngine2D* PE, ParticleBatch2D* PB)
	{
		// Assert that PE and PB exist
		assert(PB);
		assert(PE);

		// Push back PB into batches
		PE->ParticleBatches.push_back(PB);

		return 1;
	}

	/* Adds a particle to a batch */
	uint32 AddParticle(Vec3 P, Vec3 V, Enjon::Vec2 D, ColorRGBA16 C, GLuint ID, float DR, ParticleBatch2D* PB, Vec4 UV)
	{
		// Get next available index in particles
		uint32 i = FindNextAvailableParticle(PB);

		if (i >= MAXPARTICLES) return MAXPARTICLES;

		// Place back in Particles
		auto p = &PB->Particles.at(i);
		p->Position = P;
		p->Velocity = V;
		p->Dimensions = D;
		p->LifeTime = 1.0f;
		p->DecayRate = DR; 
		p->Color = C;
		p->TexID = ID;
		p->UV = UV;

		return 1;
	}

	/* Finds next available particle in particle batch */
	uint32 FindNextAvailableParticle(ParticleBatch2D* PB)
	{
		uint32 NAP = PB->NextAvailableParticle;

		// Loop from next available to max
		for (uint32 i = NAP; i < MAXPARTICLES; ++i)
		{
			if (PB->Particles.at(i).LifeTime <= 0.0f)
			{
				PB->NextAvailableParticle = i;
				return i;
			} 
		}

		if (NAP - 1 >= MAXPARTICLES) NAP = MAXPARTICLES - 1;
		
		// Loop from beginning to next available - 1
		for (uint32 i = 0; i < NAP - 1; i++)	
		{
			if (PB->Particles.at(i).LifeTime <= 0.0f)
			{
				PB->NextAvailableParticle = i;
				return i;
			}
		}

		// Otherwise, loop back around
		PB->NextAvailableParticle = 0; 
		return MAXPARTICLES;
	}	

	/* Frees memory of given particle engine */
	uint32 FreeEngine(ParticleEngine2D* PE)
	{
		assert(PE);
		FreeBatches(PE);
		delete(PE);
		return 1;
	}

	/* Frees memory of all particle batches */
	uint32 FreeBatches(ParticleEngine2D* PE)
	{
		for (auto& PB : PE->ParticleBatches)
		{
			delete(PB);	
		}

		return 1;
	}

	/* Adds all particles in each particle batch to be drawn */
	void Draw(ParticleEngine2D* PE, Camera2D* Camera) 
	{
		if (Camera == nullptr) Utils::FatalError("PARTICLE_ENGINE::DRAW::Camera is null.");

		// Loop through all particle batches in engine
		for (auto& PB : PE->ParticleBatches)
		{
			// Loop through all particles in PB
			for (auto& P : PB->Particles)
			{
				// Get particle at i
				// auto* P = &PB->Particles.at(i);

				if (Camera->IsBoundBoxInCamView(P.Position.XY(), P.Dimensions))
				{
					// Only draw if alive
					if (P.LifeTime > 0.0f && P.Color.a > 0.0f)
					{
						// Add particle to sprite batch to be rendered
						PB->SB->Add(Vec4(P.Position.XY(), P.Dimensions), P.UV, P.TexID, P.Color, P.Position.y - P.Position.z);
					}
				}
			}
		}
	}

	void DrawFire(Particle2D::ParticleBatch2D* Batch, Vec3 Position)
	{
		// Totally testing for shiggles
		static float PCounter = 0.0f;
		// PCounter += 0.25f;
		static GLuint PTex = Enjon::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_1.png").id;
		static GLuint PTex2 = Enjon::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_2.png").id;
		static GLuint PTex3 = Enjon::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_3.png").id;
		static GLuint PTex4 = Enjon::ResourceManager::GetTexture("../IsoARPG/assets/textures/bg-light.png").id;

		static ColorRGBA16 Gray = RGBA16(0.3f, 0.3f, 0.3f, 1.0f);

		static float SmokeCounter = 0.0f;
		SmokeCounter += 0.25f;
		if (SmokeCounter >= 1.0f)
		{
			for (int i = 0; i < 10; i++)
			{
				float XPos = Random::Roll(-50, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(2, 5), XVel = Random::Roll(-2, 2), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(100, 200), XSize = Random::Roll(150, 200);
				int Roll = Random::Roll(1, 3);
				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				int RedAmount = Random::Roll(0, 50);
				int Alpha = Random::Roll(995, 1000) / 1000.0f;


				Particle2D::AddParticle(Vec3(Position.x - 20.0f, Position.y + 20.0f, Position.z), Vec3(XVel, YVel, ZVel), 
					Enjon::Vec2(XSize, YSize), RGBA16(Gray.r, Gray.g, Gray.b + 0.1f, 0.185f), tex, 0.0025f, Batch);
			}
			SmokeCounter = 0.0f;
		}

		static float FlameCounter = 0.0f;
		FlameCounter += 0.25f;
		if (FlameCounter >= 1.0f)
		{
			ColorRGBA16 Fire = RGBA16(3.0f, 0.3f, 0.1f, 0.5f);
			for (int i = 0; i < 1; i++)
			{
				float XPos = Random::Roll(-50, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(2, 4), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(75, 125), XSize = Random::Roll(75, 125);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				Particle2D::AddParticle(Vec3(Position.x, Position.y, Position.z), Vec3(XVel, YVel, ZVel), 
					Enjon::Vec2(XSize, YSize), Fire, tex, 0.025f, Batch);
			}
			FlameCounter = 0.0f;
		}

		
		static float InnerFlameCounter = 0.0f;
		InnerFlameCounter += 0.05f;
		if (InnerFlameCounter >= 1.0f)
		{
			ColorRGBA16 Fire = RGBA16(5.0f, 0.8f, 0.1f, 2.0f);
			for (int i = 0; i < 1; i++)
			{
				float XPos = Random::Roll(-50, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(2, 4), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(50, 75), XSize = Random::Roll(50, 75);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				Particle2D::AddParticle(Vec3(Position.x, Position.y, Position.z), Vec3(XVel, YVel, ZVel), 
					Enjon::Vec2(XSize, YSize), Fire, tex, 0.05f, Batch);
			}
			InnerFlameCounter = 0.0f;
		}

		static float LightFlameCounter = 0.0f;
		LightFlameCounter += 0.025f;
		if (LightFlameCounter >= 1.0f)
		{
			ColorRGBA16 Fire = RGBA16(8.0f, 1.6f, 0.0f, 0.005f);
			for (int i = 0; i < 4; i++)
			{
				float XPos = Random::Roll(-100, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(1, 2), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(200, 300), XSize = Random::Roll(200, 300);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				Particle2D::AddParticle(Vec3(Position.x - 90.0f, Position.y - 50.0f, Position.z), Vec3(XVel, YVel, ZVel), 
					Enjon::Vec2(XSize, YSize), Fire, tex, 0.025f, Batch);
			}
			LightFlameCounter = 0.0f;
		}

		static float LightCounter = 0.0f;
		LightFlameCounter += 0.025f;
		if (LightFlameCounter >= 1.0f)
		{
			ColorRGBA16 Fire = RGBA16(8.0f, 1.6f, 0.0f, 0.005f);
			for (int i = 0; i < 1; i++)
			{
				float XPos = Random::Roll(-100, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(1, 2), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(500, 500), XSize = Random::Roll(500, 500);
				int Roll = Random::Roll(1, 3);

				Particle2D::AddParticle(Vec3(Position.x - 90.0f, Position.y - 50.0f, Position.z), Vec3(XVel, YVel, ZVel), 
					Enjon::Vec2(XSize, YSize), Fire, PTex4, 0.7f, Batch);
			}
			LightFlameCounter = 0.0f;
		}

		static float Ember = 0.0f;
		Ember += 0.05f;
		if (Ember >= 1.0f)
		{
			ColorRGBA16 Fire = RGBA16(5.0f, 0.8f, 0.0f, 5.0f);
			for (int i = 0; i < 15; i++)
			{
				float XPos = Random::Roll(-100, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(5, 10), XVel = Random::Roll(-5, 5), YVel = Random::Roll(-5, 5),
								YSize = Random::Roll(1, 5), XSize = Random::Roll(1, 3);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				Particle2D::AddParticle(Vec3(Position.x + 20.0f, Position.y + 20.0f, Position.z), Vec3(XVel, YVel, ZVel), 
					Enjon::Vec2(XSize, YSize), Fire, PTex, 0.05f, Batch);
			}
			Ember = 0.0f;
		}
	}

}}



















