
#ifndef ENJON_PHYSICS_DEFINES_H
#define ENJON_PHYSICS_DEFINES_H

// TODO(): Abstract away Bullet Implementations
#include <Bullet/btBulletCollisionCommon.h> 
#include <Bullet/btBulletDynamicsCommon.h>

namespace Enjon 
{
	using BulletRigidBody							= btRigidBody;
	using BulletCollisionShape						= btCollisionShape;
	using BulletRigidBodyMotionState				= btMotionState;
	using BulletDefaultMotionState					= btDefaultMotionState;
	using BulletDynamicPhysicsWorld					= btDynamicsWorld;
	using BulletDefaultCollisionConfiguration		= btDefaultCollisionConfiguration;
	using BulletCollisionDispatcher					= btCollisionDispatcher;
	using BulletBroadphaseInterface					= btBroadphaseInterface;
	using BulletSequentialImpulseConstraintSolver	= btSequentialImpulseConstraintSolver;
	using BulletDiscreteDynamicWorld				= btDiscreteDynamicsWorld;
	using BulletDBVTBroadPhase						= btDbvtBroadphase; 
	using BulletEmptyShape							= btEmptyShape;
	using BulletBoxShape							= btBoxShape;
	using BulletSphereShape							= btSphereShape;
	using BulletCylinderShape						= btCylinderShape;
	using BulletConeShape							= btConeShape;
	using BulletCapsuleShape						= btCapsuleShape;
	using BV3										= btVector3;
	using BQuat										= btQuaternion;
	using BTransform								= btTransform;
	using BulletRigidBodyConstructionInfo			= btRigidBody::btRigidBodyConstructionInfo;
	using BulletClosestRayResultCallback			= btCollisionWorld::ClosestRayResultCallback;
	using BulletClosestConvexResultCallback			= btCollisionWorld::ClosestConvexResultCallback;
	using BulletAllHitsRayResultCallback			= btCollisionWorld::AllHitsRayResultCallback;
}

#endif
