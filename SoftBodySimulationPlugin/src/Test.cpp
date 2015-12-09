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

#include "../include/Test.h"


/*

This example show a custom solver at work.  Two nCloth objects are created,
one is disconnected from the default nucleus solver, and hooked to
this custom solver node, which will just compute a sine wave motion on
the cloth in time.

A custom solver needs to have a minimum of 3 attributes:

-startState     To be connected from the cloth object to the solver
-currentState   To be connected from the cloth object to the solver
-nextState      To be connected from the solver object to the cloth

and usually a 4th attribute that is the current time.

The idea is that when a solve is needed, the cloth object will pull on the
nextState attribute.  At this point the solver should pull on either the
currentState attribute or the startState, depending on the current time.
Once it has the state information, the solver can extract that information,
solve one step, and stuff that information back into the MnCloth to 
complete the solve.

Below is some example code to test this plugin:

//**************************************************************************
//Note: Before running this code, make sure the plugin testNucleusNode is loaded!
//**************************************************************************
global proc setupCustomSolverScene()
{

    file -f -new;

    string $pPlane1[] = `polyPlane -w 5 -h 5 -sx 10 -sy 10 -ax 0 1 0 -cuv 2 -ch 1`;
    move -r -10 0 0;
    createNCloth 0;

    string $pPlane2[] = `polyPlane -w 5 -h 5 -sx 10 -sy 10 -ax 0 1 0 -cuv 2 -ch 1`;
    createNCloth 0;

    //Hookup plane2 (the cloth object created for plane2 is named nClothShape2) to our custom solver instead.

    //First, disconnect it from the default nucleus solver:
    disconnectAttr nClothShape2.currentState nucleus1.inputActive[1];
    disconnectAttr nClothShape2.startState nucleus1.inputActiveStart[1];
    disconnectAttr nucleus1.outputObjects[1] nClothShape2.nextState;

    //create our custom solver:
    createNode testNucleusNode;

    //Hookup plane2 to our custom solver:
    connectAttr testNucleusNode1.nextState[0] nClothShape2.nextState;
    connectAttr nClothShape2.currentState testNucleusNode1.currentState[0];
    connectAttr nClothShape2.startState testNucleusNode1.startState[0];
    connectAttr time1.outTime testNucleusNode1.currentTime;

}

*/

/*const MTypeId testNucleusNode::id( 0x85002 );


#include <maya/MFnNObjectData.h>
#include <maya/MnCloth.h>

MObject testNucleusNode::startState;
MObject testNucleusNode::currentState;
MObject testNucleusNode::nextState;
MObject testNucleusNode::currentTime;


inline void statCheck( MStatus stat, MString msg )
{
    if ( !stat )
    {
        cout<<msg<<"\n";
    }
}

MStatus testNucleusNode::compute(const MPlug &plug, MDataBlock &data)
{
    MStatus stat;

    if ( plug == nextState )
    {
        
        //get the value of the currentTime 
        MTime currTime = data.inputValue(currentTime).asTime();        
        MObject inputData;
        //pull on start state or current state depending on the current time.
        if(currTime.value() <= 0.0) 
        {
            MArrayDataHandle multiDataHandle = data.inputArrayValue(startState);
            multiDataHandle.jumpToElement(0);
            inputData =multiDataHandle.inputValue().data();
        }
        else 
        {
            MArrayDataHandle multiDataHandle = data.inputArrayValue(currentState);
            multiDataHandle.jumpToElement(0);
            inputData = multiDataHandle.inputValue().data();
        }                
                
        MFnNObjectData inputNData(inputData);                
        MnCloth * nObj = NULL;
        inputNData.getObjectPtr(nObj);        
        
        MFloatPointArray points;
        nObj->getPositions(points);
        unsigned int ii;

        for(ii = 0; ii < points.length(); ii++) 
        {            
            //points[ii].y = (float) sin(points[ii].x + currTime.value() * 4.0f * (3.1415f / 180.0f));
            points[ii].x = (float)(points[ii].x + (currTime.value() * 0.1));
        }

        nObj->setPositions(points);

        delete nObj;        
        data.setClean(plug);
    }
    else if ( plug == currentState )
    {        
        data.setClean(plug);

    }
    else if (plug == startState) 
    {        
        data.setClean(plug);
    }
    else {
        stat = MS::kUnknownParameter;
    }
    return stat;
}



MStatus testNucleusNode::initialize()
{
    MStatus stat;

    MFnTypedAttribute tAttr;

    startState = tAttr.create("startState", "sst", MFnData::kNObject, MObject::kNullObj, &stat );

    //statCheck(stat, "failed to create startState");
    tAttr.setWritable(true);
    tAttr.setStorable(true);
    tAttr.setHidden(true);
    tAttr.setArray(true);


    currentState = tAttr.create("currentState", "cst", MFnData::kNObject, MObject::kNullObj, &stat );

    //statCheck(stat, "failed to create currentState");
    tAttr.setWritable(true);
    tAttr.setStorable(true);
    tAttr.setHidden(true);
    tAttr.setArray(true);
    

    nextState = tAttr.create("nextState", "nst", MFnData::kNObject, MObject::kNullObj, &stat );

    //statCheck(stat, "failed to create nextState");
    tAttr.setWritable(true);
    tAttr.setStorable(true);
    tAttr.setHidden(true);
    tAttr.setArray(true);

    MFnUnitAttribute uniAttr;
    currentTime = uniAttr.create( "currentTime", "ctm" , MFnUnitAttribute::kTime,  0.0, &stat  );       

    addAttribute(startState);
    addAttribute(currentState);
    addAttribute(nextState);
    addAttribute(currentTime);
    
    attributeAffects(startState, nextState);
    attributeAffects(currentState, nextState);  
    attributeAffects(currentTime, nextState);   

    return MStatus::kSuccess;
}

MStatus initializePlugin ( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin(obj, "Autodesk - nCloth Prototype 4", "8.5", "Any");   

    status = plugin.registerNode ( "testNucleusNode", testNucleusNode::id,testNucleusNode ::creator, testNucleusNode::initialize );

    if ( !status )
    {
        status.perror("registerNode");
        return status;
    }

    return status;
}

MStatus uninitializePlugin ( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(testNucleusNode::id);
    if ( !status )
    {
        status.perror("deregisterNode");
        return status;
    }

    return status;
}*/

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
//#include "testNpassiveNode.h"
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
#include <maya/MFnNObjectData.h>
#include <maya/MAnimControl.h>
#include "stdio.h"
/*
Introduction to interacting with the N Solver
=============================================
In order to create an N cloth object that can interact with the Nucleus solver, 
your object will need to own a MnCloth, which represents the underlying
N cloth and its data.
your node will also need the following 6 attributes:
ATTR                Type        Description
startState          kNObject    initial state of your N object
currentState        kNObject    current state of your N object
currentTime         Time        connection to the current time
inputGeom           kMesh       input mesh       
inputGeom and currentTime are self explanatory.
A connection is to be made from the nucleus solver's outputObjects attribute to 
the nextState attribute of your node.  Also, you need to connect the currentState
and startState attributes of your node to the inputActive and inputActiveStart attributes
on the solver node respectively.
Once these connections are made, the normal sequence of events is the following:
The refresh will trigger a pull on the output mesh attribute.  At this your node will
pull on the nextState attribute, triggering a solve from the solver.  Depending on the current
time, the solver will trigger pulls on either the currentState or startState attributes of your
node.  If the startState is pulled on, you need to initialize the MnCloth which you node
owns from the input geometry.  Once this is done and the data passed back to the solver,
a solve will occur, and the solver will automatically update your MnCloth behind the scenes.
At this point you may extract the results of the solve via methods on the MnCloth and apply it
to the output mesh.
Below is a script that show how to test this node:
//This example show how 2 cloth objects falling and colliding with a sphere
//side by side.  One is a default nCloth object, the other is a cloth
//object created by our plugin.
//**************************************************************************
//Note: Before running this code, make sure the plugin testNpassiveNode is loaded!
//**************************************************************************
global proc setupCustomPassiveScene()
{
    file -f -new;
    //plane1 and 2 will be driven by regular nCloth
    string $pPlane1[] = `polyPlane -w 5 -h 5 -sx 10 -sy 10 -ax 0 1 0 -cuv 2 -ch 1`;
    move -r -10 0 0;
    createNCloth 0;
    //plane2 will act as input to our testNpassiveNode
    string $pPlane2[] = `polyPlane -w 5 -h 5 -sx 10 -sy 10 -ax 0 1 0 -cuv 2 -ch 1`;
    createNCloth 0;
    //sphere 1  will be a regular a passive object.
    string $pSphere1[] = `polySphere -r 1 -sx 20 -sy 20 -ax 0 1 0 -cuv 2 -ch 1`;
    move -r -10 -3 0;
    makeCollideNCloth;
    //sphere 2  will be a plugin  a passive object.
    string $pSphere2[] = `polySphere -r 1 -sx 20 -sy 20 -ax 0 1 0 -cuv 2 -ch 1`;
    move -r 0 -3 0;
    createNode testNpassiveNode;
    connectAttr pSphereShape2.worldMesh[0] testNpassiveNode1.inputGeom;
    connectAttr testNpassiveNode1.currentState nucleus1.inputPassive[1];
    connectAttr testNpassiveNode1.startState nucleus1.inputPassiveStart[1];
    connectAttr time1.outTime testNpassiveNode1.currentTime;
}
*/
const MTypeId testNpassiveNode::id( 0x85004 );
MObject testNpassiveNode::startState;
MObject testNpassiveNode::currentState;
MObject testNpassiveNode::currentTime;
MObject testNpassiveNode::inputGeom;
inline void statCheck( MStatus stat, MString msg )
{
    if ( !stat )
    {
        cout<<msg<<"\n";
    }
}
testNpassiveNode::testNpassiveNode()
{
    //Create my N cloth.
    fNObject.createNRigid();
}
MStatus testNpassiveNode::compute(const MPlug &plug, MDataBlock &data)
{
    MStatus stat;
    if ( plug == currentState )
    { 
      // get old positions and numVerts
      // if num verts is different, reset topo and zero velocity
      // if num verts is the same, compute new velocity
        int ii,jj;
        // initialize MnCloth
        MObject inMeshObj = data.inputValue(inputGeom).asMesh();
                
        MFnMesh inputMesh(inMeshObj);       
        unsigned int numVerts = 0;
        numVerts = inputMesh.numVertices();
        unsigned int prevNumVerts;
        fNObject.getNumVertices(prevNumVerts);
        if(numVerts != prevNumVerts) {
                int numPolygons = inputMesh.numPolygons();
                int * faceVertCounts = new int[numPolygons];
                
        
                int facesArrayLength = 0;
                for(ii=0;ii<numPolygons;ii++) {
                MIntArray verts;
                inputMesh.getPolygonVertices(ii,verts);
                faceVertCounts[ii] = verts.length();
                facesArrayLength += verts.length();
                }
                int * faces = new int[facesArrayLength];
                int currIndex = 0;
                for(ii=0;ii<numPolygons;ii++) {
                MIntArray verts;
                inputMesh.getPolygonVertices(ii,verts);
                for(jj=0;jj<(int)verts.length();jj++) {
                    faces[currIndex++] = verts[jj];
                }
            }
                int numEdges = inputMesh.numEdges();
                int * edges = new int[2*numEdges];
                currIndex = 0;
                for(ii=0;ii<numEdges;ii++) {
                int2 edge;
                inputMesh.getEdgeVertices(ii,edge);
                edges[currIndex++] = edge[0];
                edges[currIndex++] = edge[1];
                }
                // When you are doing the initialization, the first call must to be setTopology().  All other
                // calls must come after this.
            fNObject.setTopology(numPolygons, faceVertCounts, faces,numEdges, edges );
                delete[] faceVertCounts;
                delete[] faces;
                delete[] edges;        
                MFloatPointArray vertexArray;
                inputMesh.getPoints(vertexArray, MSpace::kWorld);
                fNObject.setPositions(vertexArray,true);
                MFloatPointArray velocitiesArray;
                velocitiesArray.setLength(numVerts);
                for(ii=0;ii<(int)numVerts;ii++) {
                velocitiesArray[ii].x = 0.0f;
                velocitiesArray[ii].y = 0.0f;
                velocitiesArray[ii].z = 0.0f;
                velocitiesArray[ii].w = 0.0f;
            }
            fNObject.setVelocities(velocitiesArray);
        } else {
                MFloatPointArray vertexArray;
                MFloatPointArray prevVertexArray;
                inputMesh.getPoints(vertexArray, MSpace::kWorld);
                fNObject.getPositions(prevVertexArray);
            // you may want to get the playback rate for the dt
            // double dt = MAnimControl::playbackBy() \ 24.0;
            // or get the real dt by caching the last eval time
            double dt = 1.0/24.0;
                MFloatPointArray velocitiesArray;
                velocitiesArray.setLength(numVerts);
                for(ii=0;ii<(int)numVerts;ii++) {
                velocitiesArray[ii].x = (float)( (vertexArray[ii].x - prevVertexArray[ii].x)/dt);
                velocitiesArray[ii].y = (float)( (vertexArray[ii].y - prevVertexArray[ii].y)/dt);
                velocitiesArray[ii].z = (float)( (vertexArray[ii].x - prevVertexArray[ii].z)/dt);
                velocitiesArray[ii].w = 0.0f;
            }
            fNObject.setVelocities(velocitiesArray);
            fNObject.setPositions(vertexArray,true);
        }
        // in real life, you'd get these attribute values each frame and set them
        fNObject.setThickness(0.1f);
        fNObject.setBounce(0.0f);
        fNObject.setFriction(0.1f);
        fNObject.setCollisionFlags(true, true, true);               
        MFnNObjectData outputData;
        MObject mayaNObjectData = outputData.create();
        outputData.setObject(mayaNObjectData);
        
        outputData.setObjectPtr(&fNObject);        
        outputData.setCached(false);
        MDataHandle currStateOutputHandle = data.outputValue(currentState);
        currStateOutputHandle.set(outputData.object());
      
    }
    if ( plug == startState )
    {
        int ii,jj;
        // initialize MnCloth
        MObject inMeshObj = data.inputValue(inputGeom).asMesh();
                
        MFnMesh inputMesh(inMeshObj);       
        int numPolygons = inputMesh.numPolygons();
            int * faceVertCounts = new int[numPolygons];
                
        
            int facesArrayLength = 0;
            for(ii=0;ii<numPolygons;ii++) {
                MIntArray verts;
                inputMesh.getPolygonVertices(ii,verts);
                faceVertCounts[ii] = verts.length();
                facesArrayLength += verts.length();
            }
            int * faces = new int[facesArrayLength];
            int currIndex = 0;
            for(ii=0;ii<numPolygons;ii++) {
                MIntArray verts;
                inputMesh.getPolygonVertices(ii,verts);
                for(jj=0;jj<(int)verts.length();jj++) {
                    faces[currIndex++] = verts[jj];
               }
           }
            int numEdges = inputMesh.numEdges();
            int * edges = new int[2*numEdges];
            currIndex = 0;
            for(ii=0;ii<numEdges;ii++) {
                int2 edge;
                inputMesh.getEdgeVertices(ii,edge);
                edges[currIndex++] = edge[0];
                edges[currIndex++] = edge[1];
            }
            // When you are doing the initialization, the first call must to be setTopology().  All other
            // calls must come after this.
            fNObject.setTopology(numPolygons, faceVertCounts, faces,numEdges, edges );
            delete[] faceVertCounts;
            delete[] faces;
            delete[] edges;        
            unsigned int numVerts = 0;
            numVerts = inputMesh.numVertices();        
            MFloatPointArray vertexArray;
            inputMesh.getPoints(vertexArray, MSpace::kWorld);
            fNObject.setPositions(vertexArray,true);
            MFloatPointArray velocitiesArray;
            velocitiesArray.setLength(numVerts);
            for(ii=0;ii<(int)numVerts;ii++) {
                velocitiesArray[ii].x = 0.0f;
                velocitiesArray[ii].y = 0.0f;
                velocitiesArray[ii].z = 0.0f;
                velocitiesArray[ii].w = 0.0f;
           }
           fNObject.setVelocities(velocitiesArray);
           fNObject.setThickness(0.1f);
            fNObject.setBounce(0.0f);
            fNObject.setFriction(0.1f);
            fNObject.setCollisionFlags(true, true, true);               
        
            MFnNObjectData outputData;
            MObject mayaNObjectData = outputData.create();
            outputData.setObject(mayaNObjectData);
        
            outputData.setObjectPtr(&fNObject);        
            outputData.setCached(false);
            MDataHandle startStateOutputHandle = data.outputValue(startState);
            startStateOutputHandle.set(outputData.object());
    }
    else {
        stat = MS::kUnknownParameter;
    }
    return stat;
}
MStatus testNpassiveNode::initialize()
{
    MStatus stat;
    MFnTypedAttribute tAttr;
    inputGeom = tAttr.create("inputGeom", "ing", MFnData::kMesh, MObject::kNullObj, &stat );
    //statCheck(stat, "failed to create inputGeom");
    tAttr.setWritable(true);
    tAttr.setStorable(true);
    tAttr.setHidden(true);
    currentState = tAttr.create("currentState", "cus", MFnData::kNObject, MObject::kNullObj, &stat );
   // statCheck(stat, "failed to create currentState");
    tAttr.setWritable(true);
    tAttr.setStorable(false);
    tAttr.setHidden(true);
    startState = tAttr.create( "startState", "sts", MFnData::kNObject, MObject::kNullObj, &stat );
    //statCheck(stat, "failed to create startState");
    tAttr.setWritable(true);
    tAttr.setStorable(false);
    tAttr.setHidden(true);
    MFnUnitAttribute uniAttr;
    currentTime = uniAttr.create( "currentTime", "ctm" , MFnUnitAttribute::kTime,  0.0, &stat  );    
    addAttribute(inputGeom);
    addAttribute(currentTime);
    addAttribute(startState);
    addAttribute(currentState);
    
    attributeAffects(inputGeom, startState);
    attributeAffects(inputGeom, currentState);
    attributeAffects(currentTime, currentState);    
    attributeAffects(currentTime, startState);    
    return MStatus::kSuccess;
}
MStatus initializePlugin ( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin(obj, "Autodesk - nCloth Prototype 5", "9.0", "Any");
    status = plugin.registerNode ( "testNpassiveNode", testNpassiveNode::id,testNpassiveNode ::creator, testNpassiveNode::initialize );
    if ( !status )
    {
        status.perror("registerNode");
        return status;
    }
    return status;
}
MStatus uninitializePlugin ( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterNode(testNpassiveNode::id);
    if ( !status )
    {
        status.perror("deregisterNode");
        return status;
    }
    return status;
}