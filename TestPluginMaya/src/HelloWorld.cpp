
#include <maya/MArgList.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
 
class HelloWorld : public MPxCommand {
 public:
  HelloWorld() {};
  virtual MStatus doIt(const MArgList& argList);
  static void* creator();
};

#include <maya/MFnPlugin.h>
 
void* HelloWorld::creator() { return new HelloWorld; }
 
MStatus HelloWorld::doIt(const MArgList& argList) {
  MGlobal::displayInfo("Hello World!");
  return MS::kSuccess;
}
 
MStatus initializePlugin(MObject obj) {
  MFnPlugin plugin(obj, "Chad Vernon", "1.0", "Any");
  MStatus status = plugin.registerCommand("helloWorld", HelloWorld::creator);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}
 
MStatus uninitializePlugin(MObject obj) {
  MFnPlugin plugin(obj);
  MStatus status = plugin.deregisterCommand("helloWorld");
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}