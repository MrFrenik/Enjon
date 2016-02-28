#include <assert.h>

#include "Graphics/ParticleEngine2D.h"
#include "Graphics/FontManager.h"
#include "IO/ResourceManager.h"
#include "Math/Maths.h"
#include "Defines.h"
#include "Utils/Errors.h"


namespace Enjon { namespace Graphics { namespace Particle2D { 

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

			Math::Vec3* Position = &p.Position;
			Math::Vec3* Velocity = &p.Velocity;

			// Move position of particle by its velocity
			Position->x += Velocity->x;
			Position->z += Velocity->z;

			// Add z to y component of position
			Position->y += Velocity->y + Velocity->z;

			// Decay particle alpha over time
			p.Color.a -= p.DecayRate * 0.5f;
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
	ParticleBatch2D* NewParticleBatch(SpriteBatch* SB)
	{
		ParticleBatch2D* PB = new ParticleBatch2D();
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
	uint32 AddParticle(Math::Vec3 P, Math::Vec3 V, Math::Vec2 D, ColorRGBA16 C, GLuint ID, float DR, ParticleBatch2D* PB)
	{
		// Get next available index in particles
		uint32 i = FindNextAvailableParticle(PB);

		// Place back in Particles
		auto* p = &PB->Particles[i];
		p->Position = P;
		p->Velocity = V;
		p->Dimensions = D;
		p->LifeTime = 1.0f;
		p->DecayRate = DR; 
		p->Color = C;
		p->TexID = ID;

		return 1;
	}

	/* Finds next available particle in particle batch */
	uint32 FindNextAvailableParticle(ParticleBatch2D* PB)
	{
		uint32* NAP = &PB->NextAvailableParticle;

		// Loop from next available to max
		for (uint32 i = *NAP; i < MAXPARTICLES; i++)
		{
			if (PB->Particles[i].LifeTime <= 0.0f)
			{
				*NAP = i;
				return i;
			} 
		}

		// Loop from beginning to next available - 1
		for (uint32 i = 0; i < *NAP - 1; i++)	
		{
			if (PB->Particles[i].LifeTime <= 0.0f)
			{
				*NAP = i;
				return i;
			}
		}

		// Otherwise, loop back arounda
		*NAP = 0; 
		return *NAP;
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
	void Draw(ParticleEngine2D* PE, Enjon::Graphics::Camera2D* Camera) 
	{
		if (Camera == nullptr) Utils::FatalError("PARTICLE_ENGINE::DRAW::Camera is null.");

		// Loop through all particle batches in engine
		for (auto PB : PE->ParticleBatches)
		{
			// Loop through all particles in PB
			for (uint32 i = 0; i < MAXPARTICLES; i++)
			{
				// Get particle at i
				auto* P = &PB->Particles[i];

				if (Camera->IsBoundBoxInCamView(P->Position.XY(), P->Dimensions))
				{
					// Only draw if alive
					if (P->LifeTime > 0.0f && P->Color.a > 0.0f)
					{
						// Add particle to sprite batch to be rendered
						PB->SB->Add(Math::Vec4(P->Position.XY(), P->Dimensions), Math::Vec4(0, 0, 1, 1), P->TexID, P->Color, P->Position.y - P->Position.z);
					}
				}
			}
		}
	}

	void DrawFire(Enjon::Graphics::Particle2D::ParticleBatch2D* Batch, EM::Vec3 Position)
	{
		// Totally testing for shiggles
		static float PCounter = 0.0f;
		// PCounter += 0.25f;
		static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_1.png").id;
		static GLuint PTex2 = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_2.png").id;
		static GLuint PTex3 = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/smoke_3.png").id;
		static GLuint PTex4 = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/bg-light.png").id;

		static EG::ColorRGBA16 Gray = EG::RGBA16(0.3f, 0.3f, 0.3f, 1.0f);

		// std::string S("23.5");
	 //    std::string::const_iterator c;
	 //    float x = 100.0f;
	 //    float y = 100.0f;
	 //    float advance = 0.0f;
	 //    float scale = 0.5f;
	 //    for (c = S.begin(); c != S.end(); c++) 
	 //    {
		// 	EG::Fonts::CharacterStats CS = 
		// 				EG::Fonts::GetCharacterAttributes(Math::Vec2(x, y), scale, EG::FontManager::GetFont("Bold"), c, &advance);

		// 	// Create particle
		// 	EG::Particle2D::AddParticle(EM::Vec3(CS.DestRect.x, CS.DestRect.y, 0.0f), EM::Vec3(0.0f, 0.0f, 1.0f), EM::Vec2(50.0f, 50.0f), 
		// 									EG::RGBA16_Orange(), CS.TextureID, 0.025f, Batch);

		// 	x += advance * scale;
	 //    }



		static float SmokeCounter = 0.0f;
		SmokeCounter += 0.25f;
		if (SmokeCounter >= 1.0f)
		{
			for (int i = 0; i < 10; i++)
			{
				float XPos = Random::Roll(-50, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(2, 5), XVel = Random::Roll(-2, 2), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(100, 150), XSize = Random::Roll(100, 150);
				int Roll = Random::Roll(1, 3);
				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				int RedAmount = Random::Roll(0, 50);
				int Alpha = Random::Roll(0.8f, 1.0f);


				EG::Particle2D::AddParticle(Math::Vec3(Position.x - 20.0f, Position.y + 20.0f, Position.z), Math::Vec3(XVel, YVel, ZVel), 
					Math::Vec2(XSize, YSize), EG::RGBA16(Gray.r, Gray.g, Gray.b + 0.1f, Gray.a - Alpha), tex, 0.025f, Batch);
			}
			SmokeCounter = 0.0f;
		}

		static float FlameCounter = 0.0f;
		FlameCounter += 0.25f;
		if (FlameCounter >= 1.0f)
		{
			EG::ColorRGBA16 Fire = EG::RGBA16(3.0f, 0.3f, 0.1f, 1.0f);
			for (int i = 0; i < 1; i++)
			{
				float XPos = Random::Roll(-50, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(2, 4), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(75, 125), XSize = Random::Roll(50, 100);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				EG::Particle2D::AddParticle(Math::Vec3(Position.x, Position.y, Position.z), Math::Vec3(XVel, YVel, ZVel), 
					Math::Vec2(XSize, YSize), Fire, tex, 0.025f, Batch);
			}
			FlameCounter = 0.0f;
		}

		
		static float InnerFlameCounter = 0.0f;
		InnerFlameCounter += 0.05f;
		if (InnerFlameCounter >= 1.0f)
		{
			EG::ColorRGBA16 Fire = EG::RGBA16(5.0f, 0.8f, 0.1f, 2.0f);
			for (int i = 0; i < 1; i++)
			{
				float XPos = Random::Roll(-50, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(2, 4), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(50, 75), XSize = Random::Roll(50, 75);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				EG::Particle2D::AddParticle(Math::Vec3(Position.x, Position.y, Position.z), Math::Vec3(XVel, YVel, ZVel), 
					Math::Vec2(XSize, YSize), Fire, tex, 0.05f, Batch);
			}
			InnerFlameCounter = 0.0f;
		}

		static float LightFlameCounter = 0.0f;
		LightFlameCounter += 0.025f;
		if (LightFlameCounter >= 1.0f)
		{
			EG::ColorRGBA16 Fire = EG::RGBA16(8.0f, 1.6f, 0.0f, 0.005f);
			for (int i = 0; i < 4; i++)
			{
				float XPos = Random::Roll(-100, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(1, 2), XVel = Random::Roll(-1, 1), YVel = Random::Roll(-1, 1),
								YSize = Random::Roll(200, 300), XSize = Random::Roll(200, 300);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				EG::Particle2D::AddParticle(Math::Vec3(Position.x - 90.0f, Position.y - 50.0f, Position.z), Math::Vec3(XVel, YVel, ZVel), 
					Math::Vec2(XSize, YSize), Fire, PTex4, 0.025f, Batch);
			}
			LightFlameCounter = 0.0f;
		}

		static float Ember = 0.0f;
		Ember += 0.05f;
		if (Ember >= 1.0f)
		{
			EG::ColorRGBA16 Fire = EG::RGBA16(5.0f, 0.8f, 0.0f, 5.0f);
			for (int i = 0; i < 15; i++)
			{
				float XPos = Random::Roll(-100, 100), YPos = Random::Roll(-50, 100), ZVel = Random::Roll(5, 10), XVel = Random::Roll(-5, 5), YVel = Random::Roll(-5, 5),
								YSize = Random::Roll(1, 5), XSize = Random::Roll(1, 3);
				int Roll = Random::Roll(1, 3);

				GLuint tex;
				if (Roll == 1) tex = PTex;
				else if (Roll == 2) tex = PTex2;
				else tex = PTex3; 

				EG::Particle2D::AddParticle(Math::Vec3(Position.x + 20.0f, Position.y + 20.0f, Position.z), Math::Vec3(XVel, YVel, ZVel), 
					Math::Vec2(XSize, YSize), Fire, PTex, 0.05f, Batch);
			}
			Ember = 0.0f;
		}
	}

}}}



















