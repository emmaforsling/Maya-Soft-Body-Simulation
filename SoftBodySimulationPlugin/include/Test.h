/*#ifndef _TESTNUCLEUSNODE_
#define _TESTNUCLEUSNODE_

//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnPlugin.h>
#include <math.h>
#include "stdio.h"

class testNucleusNode : public MPxNode
{
public:
    virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);

    static void *creator() 
    { 
        return new testNucleusNode; 
    }

    static MStatus initialize();
    static const MTypeId id;

// member variables:
    static MObject startState;
    static MObject currentState;
    static MObject nextState;
    static MObject currentTime;

};
 
#endif*/

#ifndef _TESTNPASSIVENODE_
#define _TESTNPASSIVENODE_
//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
#include <maya/MPxNode.h>
#include <maya/MnRigid.h>
class testNpassiveNode : public MPxNode
{
public:
    virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);
    testNpassiveNode();
    static void *creator() 
    { 
        return new testNpassiveNode; 
    }
    static MStatus initialize();
    static const MTypeId id;
    // attributes
    static MObject currentState;
    static MObject startState;
    static MObject currentTime;
    static MObject inputGeom;
    MnRigid  fNObject;
};
 
#endif