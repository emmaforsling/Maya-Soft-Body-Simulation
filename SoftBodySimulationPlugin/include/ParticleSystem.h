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
				   std::vector<std::array<int, 2> > _edgeVerticesVector,
				   std::vector<std::array<int, 3> > _faces,
				   float _k, float _mass, float _elasticity, float _gasVariable);
	~ParticleSystem();
	
	void simulateSystem(float dt);		// Calls the 3 update-functions below
	void updateForces(float dt);		// 1
	void updateVelocities(float dt);	// 2
	void updatePositions(float dt);		// 3

	// Get Functions
	MFloatVector getForce(int i){return F[i];};
	MFloatVector getVelocity(int i){return v[i];};
	MFloatPoint getPosition(int i){return p[i];};

	MPointArray getPositions(){return p;};

	MFloatVectorArray calculatePressure();
	void calculateIdealGasApprox();
	float calculateVolume();
	

private:
	// Used for the mass-spring system
	MFloatVectorArray F;				// Force
	MFloatVectorArray v; 				// Velocity
	MPointArray p;						// Position of the points

	std::vector<float> springLengths;
	std::vector<std::array<int, 2> > edgeVerticesVector;

	float k; 				// Spring constant
	float mass;				// kg
	float elasticity;

	// Used for the gas model
	MFloatVectorArray pressureVector; 	// Pressure vector
	MFloatVectorArray faceNormals;		// Face normals
	std::vector<std::array<int, 3> > faces;

	float pressureValue;
	float gasVariable; 					// Ideal gas approximation value


	
};

#endif
