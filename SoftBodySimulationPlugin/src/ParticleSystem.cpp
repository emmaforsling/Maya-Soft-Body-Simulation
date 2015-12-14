#include "../include/ParticleSystem.h"

ParticleSystem::ParticleSystem( MPointArray _points,
								std::vector<float> _springLengths,
							    std::vector<std::array<int, 2> > _edgeVerticesVector, 
							    std::vector<std::array<int, 4> > _faces,
							    float _k,
							    float _b,
							    float _mass,
							    float _elasticity,
							    float _gasVariable,
							    MFloatVectorArray _faceNormals,
							    MMatrix _world_to_local_matrix )
{
	// Initializing variables for the Mass-spring system 
	p = _points;														// array with the positions of the points
	springLengths = _springLengths;										// array with the lengths of the springs
	edgeVerticesVector = _edgeVerticesVector;							// array with the edges, and the two vertices the edge is connected to
	
	// Initializing private variables
	F = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );	// initializing the Force vector array to have the same length as array p	
	v = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );	// initializing the velocity vector array to have the same length as array p

	// Initializing pressureValue as 0.0	
	pressureValue = 0.0f;
	
	// Initializing spring constants, mass for the points and elasticity
	k = _k;
	mass = _mass;
	elasticity = _elasticity;
	b = _b; 					// dämparkonstant
	
	// Initializing variables for the gas model 
	// TODO: Initialize and fill the variable faceNormals!!!!
	faces = _faces;
	faceNormals = _faceNormals;
	pressureVector = MFloatVectorArray( faces.size(), MFloatVector(0.0, 0.0, 0.0) );		// TODO: ändra längd
	gasVariable = _gasVariable;
	world_to_local_matrix = _world_to_local_matrix;
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

		// make the distVec to a MVector so that the function .normalize() can be called
		MVector delta_p_hat = distVec;
		delta_p_hat = delta_p_hat.normal();
		MVector delta_v = v[v0_idx] - v[v1_idx];			//tror detta blir fel??????

		//MGlobal::displayInfo( ("Elongation: " + std::to_string(elongation)).c_str() );

		// Calculate spring force (Hooke's law)
		float springForce = ( -k * elongation) - (b * (delta_v * delta_p_hat) ) ;

		// Apply force to both vertices
		F[v0_idx] += (double)springForce * delta_p_hat;
		F[v1_idx] -= (double)springForce * delta_p_hat;
	}

	MFloatVectorArray pressureForce = calculatePressure();
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

	// Loop over all points
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

	// Loop over all points
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
	MFloatVectorArray pressureForce = MFloatVectorArray( faces.size(), MFloatVector(0.0, 0.0, 0.0) );

	calculateIdealGasApprox();
	
	// Loop over all faces
	for(int i = 0; i < faces.size(); ++i)
	{
		float faceArea1;
		float faceArea2;
		
		// Get the vertices for the triangle (face)
		MVector vertex1 = getPosition(faces[i][0]);
		MVector vertex2 = getPosition(faces[i][1]);
		MVector vertex3 = getPosition(faces[i][2]);
		MVector vertex4 = getPosition(faces[i][3]);

		// Deteremine edges
		MVector edge1 = vertex2 - vertex1;
		MVector edge2 = vertex3 - vertex2;
		MVector edge3 = vertex4 - vertex3;
		MVector edge4 = vertex1 - vertex4;

		// Calculate the area of the face by calculating the crossproduct of e1 and e2: eq:  |e1 x e2| / 2
		faceArea1 = ( (edge1 ^ edge2).length() )/2.0;
		faceArea2 = ( (edge3 ^ edge4).length() )/2.0;
		
		// Calculate the pressure.
		pressureVector[i] = pressureValue * faceNormals[i];

		// Calculate the pressure force
		pressureForce[i] = pressureVector[i] * (faceArea1+faceArea2);
	}

	return pressureForce;
}

/*
 *  P = (nRT) / V 	Ideal gas approximation equation
 **/
void ParticleSystem::calculateIdealGasApprox()
{
	/*float T = 1.0; // Temperature of the substance, in Kelvin
	float R = 1.0; // Gas constant, 8.314462 J mol^-1 K^-1
	float n = 1.0; // Gas mol number, 0.02504 10^21 cm^−3*/

	float V = calculateVolume(); // Volume of the object
	
	pressureValue = gasVariable / V; // P is the pressure value
}

/**
*	Function calculateVolulme()
*	Calculates the volume of a quadrilateral mesh by constructing tetrahedra out of the two
*	triangles that constitute one quad and the local object origin.
**/
float ParticleSystem::calculateVolume()
{
	float meshVolume = 0.f;

	// Loop through all faces (quadrilaterals)
	for(int i = 0; i < faces.size(); ++i)
	{
		// Extract all points of the face (local coordinates)
		MVector v0 = world_to_local_matrix * p[faces[i][0]];
		MVector v1 = world_to_local_matrix * p[faces[i][1]];
		MVector v2 = world_to_local_matrix * p[faces[i][2]];
		MVector v3 = world_to_local_matrix * p[faces[i][3]];

		// Construct two tetrahedra with their respective fourth point at the local object origin,
		// and calculate their volume and increment the total volume of the mesh.
		meshVolume += ( v0 * (v1 ^ v2) ) / 6.0f;
		meshVolume += ( v0 * (v2 ^ v3) ) / 6.0f;
	}

	meshVolume = sqrt(meshVolume * meshVolume);

	return meshVolume;
}

