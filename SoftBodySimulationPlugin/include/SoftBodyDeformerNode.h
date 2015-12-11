#ifndef SOFT_BODY_DEFORMER_NODE
#define SOFT_BODY_DEFORMER_NODE

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>

#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnUnitAttribute.h>

#include <maya/MPxDeformerNode.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>

#include "../include/ParticleSystem.h"

// Simple deformer
// It will deform the vertices along the normals of the object surface.
class softBodyDeformerNode : public MPxDeformerNode
{
public:
	softBodyDeformerNode(){};
	virtual MStatus deform(MDataBlock& data, MItGeometry& it_geo,
	const MMatrix &local_to_world_matrix, unsigned int m_index);
	static void* creator();
	static MStatus initialize();

	static MTypeId id;
	// Inflation tells how much to displace
	//static MObject inflation_attr;
	static MObject current_time;

	double* springLengths;

	// Global attributes
	static MObject aGravityMagnitude;
	static MObject aGravityDirection;

	// Object attributes from rigid body
	static MObject aCurrentTime;

private:
	static MTime tPrevious;
	ParticleSystem* particleSystem;
};

#endif
