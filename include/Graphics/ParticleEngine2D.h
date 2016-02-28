#ifndef PARTICLE_ENGINE_2D_H
#define PARTICLE_ENGINE_2D_H

#include <vector>

#include "Graphics/Color.h"
#include "Graphics/Camera2D.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/GLTexture.h"
#include "Graphics/Font.h"
#include "System/Types.h"
#include "Math/Maths.h"

const int MAXPARTICLES = 10000;

namespace Enjon { namespace Graphics { namespace Particle2D { 

	/* 2D Particle struct */
	typedef struct 
	{
		Math::Vec3 Position;
		Math::Vec3 Velocity;
		Math::Vec3 VelocityGoal;
		Math::Vec2 Dimensions;
		float LifeTime;	
		float DecayRate;	
		ColorRGBA16 Color;	
		GLuint TexID; //< Not too sure about the cache coherency of this being here
	} Particle;

	/* 2D Particle Batch struct which will be registered with particle engine and is used to draw particles */
	typedef struct
	{
		/* Updates the particle batch */
		uint32 Update();

		Particle Particles[MAXPARTICLES];
		uint32 NextAvailableParticle;
		Graphics::SpriteBatch* SB;
	} ParticleBatch2D;

	/* 2D Particle Engine struct which will hold all batches to be updated and drawn */
	typedef struct
	{
		/* Updates the particle engine */
		uint32 Update();

		std::vector<ParticleBatch2D*> ParticleBatches;		
	} ParticleEngine2D;


	/*-- Functions --*/

	/* Creates a ParticleEngine2D* and returns it */
	ParticleEngine2D* NewParticleEngine();

	/* Creates a ParticleBatch2D* and returns it */
	ParticleBatch2D* NewParticleBatch(SpriteBatch* SB);

	/* Adds a particle batch to a particle engine */
	uint32 AddParticleBatch(ParticleEngine2D* PE, ParticleBatch2D* PB);

	/* Adds a particle to a batch */
	uint32 AddParticle(Math::Vec3 P, Math::Vec3 V, Math::Vec2 D, ColorRGBA16 C, GLuint ID, float DR, ParticleBatch2D* PB);

	/* Frees memory of given particle engine */
	uint32 FreeEngine(ParticleEngine2D* PE);

	/* Frees memory of all particle batches */
	uint32 FreeBatches(ParticleEngine2D* PE);

	/* Adds all particles in each particle batch to be drawn */
	void Draw(ParticleEngine2D* PE, Enjon::Graphics::Camera2D* Camera);
}}}

#endif