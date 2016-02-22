#include <assert.h>

#include "Graphics/ParticleEngine2D.h"
#include "Graphics/FontManager.h"
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

		// Otherwise, loop back around
		return 0;
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
	void Draw(ParticleEngine2D* PE) 
	{
		// Loop through all particle batches in engine
		for (auto PB : PE->ParticleBatches)
		{
			// Loop through all particles in PB
			for (uint32 i = 0; i < MAXPARTICLES; i++)
			{
				// Get particle at i
				auto* P = &PB->Particles[i];

				// Only draw if alive
				if (P->LifeTime > 0.0f && P->Color.a > 0.0f)
				{
					// Add particle to sprite batch to be rendered
					PB->SB->Add(Math::Vec4(P->Position.XY(), P->Dimensions), Math::Vec4(0, 0, 1, 1), P->TexID, P->Color, P->Position.y - P->Position.z);
				}
			}
		}
	}

}}}



















