#include "BlendNode.h"
#include <maya/MFnPlugin.h>
 
MTypeId BlendNode::id(0x00000002);
MObject BlendNode::aBlendMesh;
MObject BlendNode::aBlendWeight;
 
void* BlendNode::creator() { return new BlendNode; }
 
MStatus BlendNode::deform(MDataBlock& data, MItGeometry& itGeo,
                          const MMatrix &localToWorldMatrix, unsigned int mIndex) {
  MStatus status;
 
  // Get the envelope and blend weight
  float env = data.inputValue(envelope).asFloat();
  float blendWeight = data.inputValue(aBlendWeight).asFloat();
  blendWeight *= env;
 
  // Get the blend mesh
  MObject oBlendMesh = data.inputValue(aBlendMesh).asMesh();
  if (oBlendMesh.isNull()) {
    // No blend mesh attached so exit node.
    return MS::kSuccess;
  }
 
  // Get the blend points
  MFnMesh fnBlendMesh(oBlendMesh, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPointArray blendPoints;
  fnBlendMesh.getPoints(blendPoints);
 
  MPoint pt;
  float w = 0.0f;
  for (; !itGeo.isDone(); itGeo.next()) {
    // Get the input point
    pt = itGeo.position();
    // Get the painted weight value
    w = weightValue(data, mIndex, itGeo.index());
    // Perform the deformation
    pt = pt + (blendPoints[itGeo.index()] - pt) * blendWeight * w;
    // Set the new output point
    itGeo.setPosition(pt);
  }
 
  return MS::kSuccess;
}
 
MStatus BlendNode::initialize() {
  MFnTypedAttribute tAttr;
  MFnNumericAttribute nAttr;
   
  aBlendMesh = tAttr.create("blendMesh", "blendMesh", MFnData::kMesh);
  addAttribute(aBlendMesh);
  attributeAffects(aBlendMesh, outputGeom);
  
  aBlendWeight = nAttr.create("blendWeight", "bw", MFnNumericData::kFloat);
  nAttr.setKeyable(true);
  addAttribute(aBlendWeight);
  attributeAffects(aBlendWeight, outputGeom);
 
  // Make the deformer weights paintable
  MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer blendNode weights;");
 
  return MS::kSuccess;
}
 
MStatus initializePlugin(MObject obj) {
  MStatus status;
  MFnPlugin plugin(obj, "Chad Vernon", "1.0", "Any");
 
  // Specify we are making a deformer node
  status = plugin.registerNode("blendNode", BlendNode::id, BlendNode::creator,
                               BlendNode::initialize, MPxNode::kDeformerNode);
  CHECK_MSTATUS_AND_RETURN_IT(status);
 
  return status;
}
 
MStatus uninitializePlugin(MObject obj) {
  MStatus     status;
  MFnPlugin plugin(obj);
 
  status = plugin.deregisterNode(BlendNode::id);
  CHECK_MSTATUS_AND_RETURN_IT(status);
 
  return status;
}

// MStatus SomeDeformer::deform( MDataBlock& data, MItGeometry& itGeo, const MMatrix &localToWorldMatrix, unsigned int geomIndex )
// {
//     MStatus status;
//     MArrayDataHandle hInput = data.outputArrayValue( input, &status );
//     CHECK_MSTATUS_AND_RETURN_IT( status )
//     status = hInput.jumpToElement( geomIndex );
//     CHECK_MSTATUS_AND_RETURN_IT( status )
//     MObject oInputGeom = hInput.outputValue().child( inputGeom ).asMesh();
//     MFnMesh fnInputMesh( oInputGeom );
// }