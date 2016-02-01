#include <assert.h>

#include "Graphics/ParticleEngine2D.h"


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

			// Move position of particle by its velocity
			p.Position += p.Velocity;	
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

	/* Creates a ParticleBatch2D* and returns it */
	ParticleBatch2D* NewParticleBatch2D(SpriteBatch* SB)
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
	uint32 AddParticle(Math::Vec3 P, Math::Vec3 V, ColorRGBA8 C, GLuint ID, float DR, ParticleBatch2D* PB)
	{
		// Get next available index in particles
		uint32 i = FindNextAvailableParticle(PB);

		// Place back in Particles
		PB->Particles[i] = {P, V, 1.0f, DR, C, ID};

		return 1;
	}

	/* Finds next available particle in particle batch */
	uint32 FindNextAvailableParticle(ParticleBatch2D* PB)
	{
		uint32* NAP = &PB->NextAvailableParticle;

		// Loop from next available to max
		for (uint32 i = *NAP; i < MAXPARTICLES; i++)
		{
			if (PB->Particles[i].LifeTime == 0.0f)
			{
				*NAP = i;
				return i;
			} 
		}

		// Loop from beginning to next available - 1
		for (uint32 i = 0; i < *NAP - 1; i++)	
		{
			if (PB->Particles[i].LifeTime == 0.0f)
			{
				*NAP = i;
				return i;
			}
		}

		// Otherwise, loop back around
		return 0;
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

	///< Now need to focus on drawing particles...



}}}
