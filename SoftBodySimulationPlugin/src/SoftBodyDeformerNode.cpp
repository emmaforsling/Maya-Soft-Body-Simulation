#include "../include/softBodyDeformerNode.h"

MTypeId softBodyDeformerNode::id(0x00000002);
//MObject softBodyDeformerNode::inflation_attr;
//MObject softBodyDeformerNode::current_time;

MObject softBodyDeformerNode::aGravityMagnitude;
MObject softBodyDeformerNode::aGravityDirection;
MObject softBodyDeformerNode::aCurrentTime;
MTime softBodyDeformerNode::tPrevious;

void* softBodyDeformerNode::creator()
{
    return new softBodyDeformerNode;
}

MStatus softBodyDeformerNode::deform(MDataBlock& data, MItGeometry& it_geo,
                                     const MMatrix &local_to_world_matrix, unsigned int m_index)
{
    MStatus status;

    // Get the current frame
    MTime currentTime = MAnimControl::currentTime();
    int currentFrame = (int)currentTime.value();

    // Get the envelope and the inflation input value
    float env = data.inputValue(envelope).asFloat();
    //double inflation = data.inputValue(inflation_attr).asDouble();
    MVector gravityVec = data.inputValue(aGravityMagnitude).asDouble() *
    data.inputValue(aGravityDirection).asVector();

    MTime tNow = data.inputValue(aCurrentTime).asTime();
    MTime timeDiff = tNow - tPrevious;
    tPrevious = tNow;
    gravityVec = data.inputValue(aGravityMagnitude).asDouble() *
    data.inputValue(aGravityDirection).asVector();

    // Get the input mesh (fn_input_mesh)
    MArrayDataHandle h_input = data.outputArrayValue( input, &status );
    CHECK_MSTATUS_AND_RETURN_IT( status )
    status = h_input.jumpToElement( m_index );
    CHECK_MSTATUS_AND_RETURN_IT( status )
    MObject o_input_geom = h_input.outputValue().child( inputGeom ).asMesh();
    MFnMesh fn_input_mesh( o_input_geom );

    // Create vertex and mesh iterators from input mesh
    MItMeshVertex itInputMeshVertex = MItMeshVertex(o_input_geom, &status);
    MItMeshVertex itInputMeshVertex2 = MItMeshVertex(o_input_geom, &status);
    MItMeshEdge itInputMeshEdge = MItMeshEdge(o_input_geom, &status);

    // Temporary arrays for storing edge properties
    std::vector<float> springLengths;
    std::vector<std::array<int, 2> > vertexPairs;

    // Initialize everything on the first frame. TODO: Use constructor...
    if(currentFrame == 1)
    {
        // Loop through edges and extract edge length and indices of connected vertices
        while(!itInputMeshEdge.isDone())
        {
            int idx = itInputMeshEdge.index();
            // Get the length of the edge
            double edgeLength;
            itInputMeshEdge.getLength(edgeLength);
            // Append the current edge length to spring length list
            springLengths.push_back( float(edgeLength) );
            
            // Get indices of connected vertices
            int vert1_idx = itInputMeshEdge.index(0);
            int vert2_idx = itInputMeshEdge.index(1);

            // Create temporary vertex pair
            std::array<int, 2> tempPair;
            tempPair[0] = vert1_idx;
            tempPair[1] = vert2_idx;

            // Append temporary vertex pair to list
            vertexPairs.push_back(tempPair);

            // Increment iterator
            itInputMeshEdge.next();
        }

        // Create particle system from initial mesh positions, spring lengths and vertex pairs
        MFloatPointArray initialPositions = MFloatPointArray();
        fn_input_mesh.getPoints(initialPositions, MSpace::kTransform);
        particleSystem = new ParticleSystem(initialPositions, springLengths, vertexPairs);
    }

    // Get the normal array from the input mesh
    MFloatVectorArray normals = MFloatVectorArray();
    fn_input_mesh.getVertexNormals(true, normals, MSpace::kTransform);

    // Loop through the geometry and set vertex positions
    while(!itInputMeshVertex.isDone())
    {
        int idx = itInputMeshVertex.index();
        MVector nrm = MVector(normals[idx]);
        MPoint pos = itInputMeshVertex.position();

        MPoint gravityDisplacement = timeDiff.value() * gravityVec;             // Fel, fel fel...
        MPoint new_pos = pos;
        itInputMeshVertex.setPosition(new_pos);

        // MGlobal::displayInfo(std::to_string((pos * local_to_world_matrix).z).c_str());

        // Increment iterator
        itInputMeshVertex.next();
    }

    return MS::kSuccess;
}

MStatus softBodyDeformerNode::initialize()
{
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;

    // Create a numeric attribute "inflation"
    //inflation_attr = nAttr.create("inflation", "in", MFnNumericData::kDouble, 0.0);
    nAttr.setMin(0.0);
    nAttr.setMax(10.0);
    nAttr.setChannelBox(true);

    aGravityMagnitude = nAttr.create("aGravityMagnitude", "gm", MFnNumericData::kDouble, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(0.0);
    nAttr.setMax(10.0);
    nAttr.setChannelBox(true);

    aGravityDirection = nAttr.create("aGravityDirection", "gd", MFnNumericData::k3Double, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    aCurrentTime = uAttr.create("aCurrentTime", "ct", MFnUnitAttribute::kTime, 0.0);
    uAttr.setDefault(MAnimControl::currentTime().as(MTime::kFilm));
    uAttr.setChannelBox(true);

    /*
    // Time attribute
    current_time = uAttr.create("current_time", "ct", MFnUnitAttribute::kTime, 0.0);
    uAttr.setDefault(MAnimControl::currentTime().as(MTime::kFilm));
    uAttr.setChannelBox(true);
    */

    // Add the attributes
    //addAttribute(inflation_attr);
    //addAttribute(current_time);

    addAttribute(aCurrentTime);
    addAttribute(aGravityMagnitude);
    addAttribute(aGravityDirection);

    // Affects
    //attributeAffects(inflation_attr, outputGeom);
    attributeAffects(aCurrentTime, outputGeom);
    attributeAffects(aGravityMagnitude, outputGeom);
    attributeAffects(aGravityDirection, outputGeom);

    //attributeAffects(current_time, outputGeom);
    // Make the deformer weights paintable (maybe wait with this)
    // MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer softBodyDeformerNode weights;");

    return MS::kSuccess;
}
