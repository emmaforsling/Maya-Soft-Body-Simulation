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
	void updateForces(float dt);		// 1
	void updateVelocities(float dt);	// 2
	void updatePositions(float dt);		// 3

	// Get Functions
	MFloatVector getForce(int i){return F[i];};
	MFloatVector getVelocity(int i){return v[i];};
	MFloatVector getPosition(int i){return p[i];};

private:
	MFloatVectorArray F;	// Force
	MFloatVectorArray v; 	// Velocity
	MFloatPointArray p;		// Position of the points
	
	float k; 				// Spring constant
	float mass;				// kg
	float elasticity;		

	double* springLengths;	// 
};

#endif
