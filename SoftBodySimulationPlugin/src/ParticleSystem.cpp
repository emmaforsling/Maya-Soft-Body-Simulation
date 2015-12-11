#include "../include/ParticleSystem.h"

ParticleSystem::ParticleSystem(MFloatVector* _points, int _numberOfPoints)
{
	points = _points;
	numberOfPoints = _numberOfPoints;
	MGlobal::displayInfo( ("ParticleSystem::numberOfPoints = " + std::to_string(numberOfPoints)).c_str() );
}

ParticleSystem::~ParticleSystem()
{

}


/*
 *
**/
void ParticleSystem::updateForces(float dt)
{
	
	// Gravity

	// Collision with floor
	for(int i = 0; i < F.length(); ++i)
	{
			
	}
}

/* 
 * Euler Explicit
 * new_v = v + a * dt
 * in which:
 * v = current velocity, a = current acceleration, dt = step length
**/
void ParticleSystem::updateVelocities(float dt)
{	
	MFloatVector new_v;
	float mass = 1.0;							// kg

	for(int i = 0; i < v.length(); ++i)
	{
		// Calculate new velocity
		new_v = v[i] + (F[i] / mass) * dt;		// a = F / m 

		// Check collision

		// Update new velocity
		v[i] = new_v;
	}
}

/* 
 * Euler Explicit
 * new_pos = p + new_v * dt
 * in which:
 * p = current position, new_v = the new velocity calculated in updateVelocities(), dt = step length 
**/
void ParticleSystem::updatePositions(float dt)
{
	MFloatVector new_p;

	for(int i = 0; i < p.length(); ++i)
	{
		new_p = p[i] + v[i] * dt;

		p[i] = new_p;
	}

}
