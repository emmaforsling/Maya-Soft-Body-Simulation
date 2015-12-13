#include "../include/ParticleSystem.h"

ParticleSystem::ParticleSystem(MPointArray _points, std::vector<float> _springLengths, std::vector<std::array<int, 2> > _edgeVerticesVector, std::vector<std::array<int, 3> > _faces)
{
	/* 
	 * Initializing variables for the Mass-spring system 
	**/
	p = _points;														// array with the positions of the points
	springLengths = _springLengths;										// array with the lengths of the springs
	edgeVerticesVector = _edgeVerticesVector;							// array with the edges, and the two vertices the edge is connected to
	
	// Initializing private variables
	F = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );	// initializing the Force vector array to have the same length as array p	
	v = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );	// initializing the velocity vector array to have the same length as array p

	// Initializing spring constants, mass for the points and elasticity
	k = 0.75;
	mass = 1.0f;
	elasticity = 0.8f;
	pressureValue = 0.0f;

	/* 
	 * Initializing varibales for the gas model 
	**/
	// TODO: Initialize and fill the variable faceNormals!!!!
	faces = _faces;
	pressureVector = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );		// TODO: ändra längd
}

ParticleSystem::~ParticleSystem()
{

}

/*
 *	Function that calls the following functions: updateForces, updateVelocities and updatePositions.
**/
void ParticleSystem::simulateSystem(float dt)
{	
	updateForces(dt);
	updateVelocities(dt);
	updatePositions(dt);
}

/*
 * Hooke's law
 * F = -k * x
 * k : spring constant, x : elongation, F : Force
 *
 * J = F * dt & J = mass *(v2 - v1)
 * F = J / dt
 * J : impulse, dt : step length, F: force, v2: final velocity, v1: initial velocity
 * 
**/
void ParticleSystem::updateForces(float dt)
{
	// Loop through force vector and add external forces
	for(int i = 0; i < F.length(); ++i)
	{
		// Gravity
		F[i] = MFloatVector(0.0f, -0.02f, 0.0f);

		// Handle collision with floor (fullösning)
		// If a vertex is below the ground and also moving downward
		if(p[i].y < 0.0 && v[i].y < 0.0)
		{
			// Calculate the change in velocity, delta_v
			MFloatVector delta_v;							// final velocity - initial velocity
			delta_v = v[i] - MFloatVector(0,0,0);
	
			// Calculate the impulse J
			MFloatVector J = -(elasticity + 1) * mass * delta_v;	
			F[i] += J / dt;

			// Display some stuff
			// MGlobal::displayInfo( ("Vertex position: " + std::to_string( (J / dt).x) + " "
   			//                                            + std::to_string( (J / dt).y) + " "
   			//                                            + std::to_string( (J / dt).z) ).c_str() );

			// Move the point upward a little bit in y direction
			//p[i].y = 0.01;
		}
	}

	// Loop through edges and add internal forces to all particles
	for(int i = 0; i < springLengths.size(); ++i)
	{
		// Get vertices connected to current edge
		std::array<int, 2> curEdgeVertices = edgeVerticesVector[i];
		int v0_idx = curEdgeVertices[0];
		int v1_idx = curEdgeVertices[1];

		// Get resting length of current edge
		float curEdgeRestLength = springLengths[i];

		// Calculate distance vector between vertices, length of the edge and its elongation
		MPoint distVec = p[v0_idx] - p[v1_idx];
		float edgeLength = (float)p[v0_idx].distanceTo(p[v1_idx]);
		float elongation = edgeLength - springLengths[i];

		//MGlobal::displayInfo( ("Elongation: " + std::to_string(elongation)).c_str() );

		// Calculate spring force (Hooke's law)
		float springForce = -k * elongation;

		// Apply force to both vertices
		F[v0_idx] += (double)springForce * distVec;
		F[v1_idx] -= (double)springForce * distVec;
	}

	// TODO:: Call the function calculatePressureForce, which returns the pressureForce for the gas.
	// This pressureForce is then applied to the force F. 

}

/* 
 * Euler explicit
 * new_v = v + a * dt
 * in which:
 * v = current velocity, a = current acceleration, dt = step length
**/
void ParticleSystem::updateVelocities(float dt)
{	
	MFloatVector new_v;

	for(int i = 0; i < v.length(); ++i)
	{
		// Calculate new velocity
		new_v = v[i] + (F[i] / mass) * dt;		// a = F / mass 

		// Update the velocity
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
	MFloatPoint new_p;

	for(int i = 0; i < p.length(); ++i)
	{
		//MGlobal::displayInfo( ("Particle velocity: " + std::to_string(v[i].x)).c_str() );
		// Calculate the new position
		new_p = p[i] + v[i] * dt;

		// Update the position
		p[i] = new_p;
	}

}


/*
 * In this function the pressure and the pressure force is calculated.	
 * To calculate the pressure value the equation P_vec = P_val * n is implemented.
 * To calculate the pressure force, first the area of a triangle (face) has to be determined: |e1 x e2| / 2
 * the the equation P_force = P_vec * faceArea can be calculated.
**/
MFloatVectorArray ParticleSystem::calculatePressure()
{
	MFloatVectorArray pressureForce;

	for(int i = 0; i< pressureVector.length(); ++i)
	{
		float faceArea;
		
		// Get the verticies for the triangle (face)
		MVector vertex1 = getPosition(faces[i][0]);
		MVector vertex2 = getPosition(faces[i][1]);
		MVector vertex3 = getPosition(faces[i][2]);

		// Deteremine edges
		MVector edge1 = vertex2 - vertex1;
		MVector edge2 = vertex3 - vertex2;

		// Calculate the area of the face by calculating the crossproduct of e1 and e2: eq:  |e1 x e2| / 2
		faceArea = ( (edge1 ^ edge2).length() )/2.0;

		// Calculate the pressure.
		pressureVector[i] = pressureValue * faceNormals[i];

		// Calculate the pressure force
		pressureForce[i] = pressureVector[i] * faceArea;
	}

	return pressureForce;
}
// P = (nRT) / V
float ParticleSystem::calculateIdealGasApprox()
{
	float T = 1.0; // Temperature
	float R = 1.0; // Gas constant, not correct or anything
	float n = 1.0; // Gas mol number, same as above lol

	float V = calculateVolume(); // Volume of the object

	float P = (T * R * n) / V;

	return P;

}
float ParticleSystem::calculateVolume()
{
	return 1.0;
}
