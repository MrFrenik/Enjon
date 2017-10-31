#ifndef ENJON_PARTICLE_ENGINE_2D_H
#define ENJON_PARTICLE_ENGINE_2D_H

#include <vector>
#include <iostream>

#include "Graphics/Color.h"
#include "Graphics/Camera2D.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/GLTexture.h"
#include "Graphics/Font.h"
#include "System/Types.h"
#include "Math/Maths.h"
#include "Defines.h"

const int MAXPARTICLES = 8000;

namespace Enjon { namespace Particle2D { 

	/* 2D Particle struct */
	typedef struct 
	{
		Vec3 Position;
		Vec3 Velocity;
		Vec3 VelocityGoal;
		Enjon::Vec2 Dimensions;
		float LifeTime;	
		float DecayRate;	
		ColorRGBA32 Color;	
		GLuint TexID; //< Not too sure about the cache coherency of this being here
		Vec4 UV;
	} Particle;

	/* 2D Particle Batch struct which will be registered with particle engine and is used to draw particles */
	typedef struct
	{
		/* Updates the particle batch */
		uint32 Update();

		std::vector<Particle> Particles;
		// Particle Particles[MAXPARTICLES];
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

	/* Creates a ParticleEngine2D* and returns it */
	ParticleEngine2D* NewParticleEngine();

	/* Creates a ParticleBatch2D* and returns it */
	ParticleBatch2D* NewParticleBatch(Enjon::SpriteBatch* SB);

	/* Adds a particle batch to a particle engine */
	uint32 AddParticleBatch(ParticleEngine2D* PE, ParticleBatch2D* PB);

	/* Adds a particle to a batch */
	uint32 AddParticle(Vec3 P, Vec3 V, Enjon::Vec2 D, ColorRGBA32 C, GLuint ID, float DR, ParticleBatch2D* PB, Vec4 UV = Vec4(0, 0, 1, 1));

	/* Frees memory of given particle engine */
	uint32 FreeEngine(ParticleEngine2D* PE);

	/* Frees memory of all particle batches */
	uint32 FreeBatches(ParticleEngine2D* PE);

	/* Adds all particles in each particle batch to be drawn */
	void Draw(ParticleEngine2D* PE, Enjon::Camera2D* Camera);

	void DrawFire(Enjon::Particle2D::ParticleBatch2D* Batch, Vec3 Position);
}}

#endif