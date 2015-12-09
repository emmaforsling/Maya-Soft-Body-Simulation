#include "../include/SoftBodySimulationPlugin.h"

MStatus initializePlugin(MObject obj) {
  MStatus status;
  MFnPlugin plugin(obj, "Emma Forsling Parborg, Martin Gråd, Ellen Häger", "1.0", "Any");
 
  // Make a deformer node (cmdr.deformer(type = 'softBodyDeformer'))
  status = plugin.registerNode("softBodyDeformer", softBodyDeformerNode::id, softBodyDeformerNode::creator,
                               softBodyDeformerNode::initialize, MPxNode::kDeformerNode);
  CHECK_MSTATUS_AND_RETURN_IT(status);
    
  // Make a node (cmdr.createNode('softBodyDeformerNode')
  status = plugin.registerNode ( "softBodyDeformerNode", softBodyDeformerNode::id,softBodyDeformerNode ::creator, softBodyDeformerNode::initialize );
  //CHECK_MSTATUS_AND_RETURN_IT(status);

  // Make a hello world command (cmds.helloWorld)
  status = plugin.registerCommand("helloWorld", HelloWorld::creator);
  CHECK_MSTATUS_AND_RETURN_IT(status);
 
  return status;
}
 
MStatus uninitializePlugin(MObject obj) {
  MStatus     status;
  MFnPlugin plugin(obj);
 
  status = plugin.deregisterNode(softBodyDeformerNode::id);
  CHECK_MSTATUS_AND_RETURN_IT(status);
    
  status = plugin.deregisterNode(softBodyDeformerNode::id);
  //CHECK_MSTATUS_AND_RETURN_IT(status);
    
  status = plugin.deregisterCommand("helloWorld");
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}