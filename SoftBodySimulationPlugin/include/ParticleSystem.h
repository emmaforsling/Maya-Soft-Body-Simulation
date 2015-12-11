#ifndef PARTICLE_SYSTEM
#define PARTICLE_SYSTEM

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>

#include <vector>
#include <array>

class ParticleSystem
{
public:
	ParticleSystem(MFloatPointArray _points, std::vector<float> _springLenghts, std::vector<std::array<int, 2> > _vertexPairs);
	~ParticleSystem();
	
	void simulateSystem(int currentVertexIdx, MIntArray neighborVertexIndices, float dt);
	void updateForces(float dt);		//
	void updateVelocities(float dt);	//Euler, call first
	void updatePositions(float dt);		//Euler, call after updateVelocities()

private:
	MFloatPointArray points;
	int numberOfPoints;

	MFloatVectorArray F;	// Force
	MFloatVectorArray v; 	// Velocity
	MFloatVectorArray p;	// Position
};

#endif
