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

private:
	MFloatVector* points;
	int numberOfPoints;

};

#endif
