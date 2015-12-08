#ifndef PUSH_DEFORMER_NODE
#define PUSH_DEFORMER_NODE

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>

#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MPxDeformerNode.h>

// Simple deformer node
// It will deform the vertices along the normals of the object surface.
class softBodyDeformerNode : public MPxDeformerNode {
 public:
  softBodyDeformerNode() {};
  virtual MStatus deform(MDataBlock& data, MItGeometry& it_geo,
                         const MMatrix &local_to_world_matrix, unsigned int m_index);
  static void* creator();
  static MStatus initialize();
 
  static MTypeId id;
  // Inflation tells how much to displace
  static MObject inflation_attr;
};

#endif