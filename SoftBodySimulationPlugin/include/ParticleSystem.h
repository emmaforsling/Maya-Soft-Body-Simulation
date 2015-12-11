#ifndef PARTICLE_SYSTEM
#define PARTICLE_SYSTEM

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MFloatVectorArray.h>

class ParticleSystem
{
public:
	ParticleSystem(MFloatVector* _points, int _numberOfPoints);
	~ParticleSystem();

	void updateForces(float dt);		//
	void updateVelocities(float dt);	//Euler, call first
	void updatePositions(float dt);		//Euler, call after updateVelocities()

private:
	MFloatVector* points;
	int numberOfPoints;

	MFloatVectorArray F;	// Force
	MFloatVectorArray v; 	// Velocity
	MFloatVectorArray p;	// Position
};

#endif
