#ifndef PARTICLE_ENGINE_2D_H
#define PARTICLE_ENGINE_2D_H

#include <vector>

#include "Graphics/Color.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/GLTexture.h"
#include "System/Types.h"
#include "Math/Maths.h"

const int MAXPARTICLES = 10000;

namespace Enjon { namespace Graphics { namespace Particle2D { 

	/* 2D Particle struct */
	typedef struct 
	{
		Math::Vec3 Position;
		Math::Vec3 Velocity;
		float LifeTime;	
		float DecayRate;	
		ColorRGBA8 Color;	
		GLuint TexID; //< Not too sure about the cache coherency of this being here
	} Particle;

	/* 2D Particle Batch struct which will be registered with particle engine and is used to draw particles */
	typedef struct
	{
		/* Updates the particle batch */
		uint32 Update();

		Particle Particles[MAXPARTICLES];
		uint32 NextAvailableParticle;
		SpriteBatch* SB;
	} ParticleBatch2D;

	/* 2D Particle Engine struct which will hold all batches to be updated and drawn */
	typedef struct
	{
		/* Updates the particle engine */
		uint32 Update();

		std::vector<ParticleBatch2D*> ParticleBatches;		
	} ParticleEngine2D;


	/*-- Functions --*/

	/* Creates a ParticleBatch2D* and returns it */
	ParticleBatch2D* NewParticleBatch2D(SpriteBatch* SB);

	/* Adds a particle batch to a particle engine */
	uint32 AddParticleBatch(ParticleEngine2D* PE, ParticleBatch2D* PB);

	/* Adds a particle to a batch */
	uint32 AddParticle(Math::Vec3 P, Math::Vec3 V, ColorRGBA8 C);

	/* Frees memory of all particle batches */
	uint32 FreeBatches(ParticleEngine2D* PE);
}}}

#endif