#ifndef PARTICLE_SYSTEM
#define PARTICLE_SYSTEM

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatPointArray.h>

class ParticleSystem
{
public:
	ParticleSystem(MFloatPointArray _points);
	~ParticleSystem();

	void updateForces(float dt);		// 1
	void updateVelocities(float dt);	// 2
	void updatePositions(float dt);		// 3

private:
	MFloatVectorArray F;	// Force
	MFloatVectorArray v; 	// Velocity
	MFloatPointArray p;		// Position of the points
};

#endif
