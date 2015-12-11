#include "../include/ParticleSystem.h"

ParticleSystem::ParticleSystem(MPointArray _points, std::vector<float> _springLengths, std::vector<std::array<int, 2> > _edgeVerticesVector)
{
	p = _points;
	springLengths = _springLengths;
	edgeVerticesVector = _edgeVerticesVector;
	
	// Initializing private variables
	F = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );
	v = MFloatVectorArray( p.length(), MFloatVector(0.0, 0.0, 0.0) );

	// Initializing spring constants, mass for the points and elasticity
	k = 0.75;
	mass = 1.0f;
	elasticity = 0.8f;
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
		// If a vertex is below the ground and also moven downward
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
		new_v = v[i] + (F[i] / mass) * dt;		// a = F / m 

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

// void MCS::checkCollisions(glm::vec3& p, glm::vec3& v) const{
//     glm::vec3 n;
//     float pos;
//     for (int i = 0; i < collisionPlanes.size(); ++i){
//         n = collisionPlanes[i].normal_;
//         pos = collisionPlanes[i].position_;
//         float p_dot_n = glm::dot(p,n);

//         if (p_dot_n < pos){
//             glm::vec3 p_offset = (p_dot_n - pos)*n;
//             glm::vec3 v_parallel_n = glm::dot(v,n)*n;
//             glm::vec3 v_orthogonal_n = v - v_parallel_n;
//             //std::cout << "--" << std::endl;
//             //std::cout << "           v: " << v[0] << " " << v[1] << " " << v[2] << std::endl;
//             //std::cout << "v_parallel_n: "<< v_parallel_n[0] << " " << v_parallel_n[1] << " " << v_parallel_n[2] << std::endl;

//             p -= p_offset;
//             v -= v_parallel_n*(1.0f+collisionPlanes[i].elasticity_);
//             v -= v_orthogonal_n*collisionPlanes[i].friction_;
//         }
//     }
// }
