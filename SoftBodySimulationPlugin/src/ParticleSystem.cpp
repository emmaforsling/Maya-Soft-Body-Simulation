#include "../include/ParticleSystem.h"

ParticleSystem::ParticleSystem(MFloatVector* _points, int _numberOfPoints)
{
	points = _points;
	numberOfPoints = _numberOfPoints;
	MGlobal::displayInfo( ("ParticleSystem::numberOfPoints = " + std::to_string(numberOfPoints)).c_str() );
}

ParticleSystem::~ParticleSystem()
{

}
