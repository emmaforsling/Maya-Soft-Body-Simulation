#include "../include/ParticleSystem.h"

ParticleSystem::ParticleSystem(MFloatPointArray _points)
{
	p = _points;
	MGlobal::displayInfo( ( "ParticleSystem::numberOfPoints = " + std::to_string(p.length()) ).c_str() );
	
	k = 0.25;
	mass = 1.0f;
	elasticity = 1.0f;
}


ParticleSystem::~ParticleSystem()
{

}


		
// 	void MCS::checkCollisions(glm::vec3& p, glm::vec3& v) const{
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


/*
 * Hooks law
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
	// Gravity

	for(int i = 0; i < F.length(); ++i)
	{

		/* Collision with floor - Fullösning */
		if(p[i].y <= 0){
			
			// Calculate the change in velocity, delta_v
			MFloatVector delta_v;							// final velocity - initial velocity
			delta_v = v[i] - MFloatVector(0,0,0);
	
			// Calculate the impulse J
			MFloatVector J = -(elasticity + 1) * mass * delta_v;	
			F[i] += J / dt;

			// Move the point upward a little bit in y direction
			p[i].y += 0.01;
		}
	}

}

/* 
 * Euler Explicit
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
	MFloatVector new_p;

	for(int i = 0; i < p.length(); ++i)
	{
		// Calculate the new position
		new_p = p[i] + v[i] * dt;

		// Update the position
		p[i] = new_p;
	}

}
