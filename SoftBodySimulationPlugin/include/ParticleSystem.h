#ifndef PARTICLE_SYSTEM
#define PARTICLE_SYSTEM

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

#include <vector>
#include <array>

class ParticleSystem
{
public:
	ParticleSystem(MPointArray _points, std::vector<float> _springLengths,
				   std::vector<std::array<int, 2> > _edgeVerticesVector);
	~ParticleSystem();
	
	void simulateSystem(float dt);
	void updateForces(float dt);		// 1
	void updateVelocities(float dt);	// 2
	void updatePositions(float dt);		// 3

	// Get Functions
	MFloatVector getForce(int i){return F[i];};
	MFloatVector getVelocity(int i){return v[i];};
	MFloatPoint getPosition(int i){return p[i];};

	MPointArray getPositions(){return p;};

private:
	MFloatVectorArray F;	// Force
	MFloatVectorArray v; 	// Velocity
	MPointArray p;			// Position of the points
	
	float k; 				// Spring constant
	float mass;				// kg
	float elasticity;		

	std::vector<float> springLengths;
	std::vector<std::array<int, 2> > edgeVerticesVector;
};

#endif
