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
    MMatrix local_to_world_matrix_inv = local_to_world_matrix.inverse();

    // Get the current frame
    MTime currentTime = MAnimControl::currentTime();
    int currentFrame = (int)currentTime.value();

    // Get the envelope, gravity and current time input values
    float env = data.inputValue(envelope).asFloat();
    MVector gravityVec = data.inputValue(aGravityMagnitude).asDouble() *
    data.inputValue(aGravityDirection).asVector();
    MTime tNow = data.inputValue(aCurrentTime).asTime();
    
    // Calculate time difference and update previous time
    MTime timeDiff = tNow - tPrevious;
    tPrevious = tNow;

    // Get the input mesh (fn_input_mesh)
    MArrayDataHandle h_input = data.outputArrayValue( input, &status );
    CHECK_MSTATUS_AND_RETURN_IT( status )
    status = h_input.jumpToElement( m_index );
    CHECK_MSTATUS_AND_RETURN_IT( status )
    MObject o_input_geom = h_input.outputValue().child( inputGeom ).asMesh();
    MFnMesh fn_input_mesh( o_input_geom );

    // Create vertex, face and edge iterators from input mesh
    MItMeshVertex itInputMeshVertex = MItMeshVertex(o_input_geom, &status);
    MItMeshVertex itInputMeshVertex2 = MItMeshVertex(o_input_geom, &status);
    MItMeshEdge itInputMeshEdge = MItMeshEdge(o_input_geom, &status);

    // Temporary arrays for storing edge properties
    std::vector<float> springLengths;
    std::vector<std::array<int, 2> > edgeVerticesVector;
    std::vector<std::array<int, 3> > faces;

    // TODO!!!! FIXA faces!

    // Initialize everything on the first frame. TODO: Use constructor instead...?
    if(currentFrame == 1)
    {
        // Loop through edges and extract edge length and indices of connected vertices
        while(!itInputMeshEdge.isDone())
        {
            int idx = itInputMeshEdge.index();
            // Get the length of the edge
            double edgeLength;
            itInputMeshEdge.getLength(edgeLength);

            // MGlobal::displayInfo( ("Edge length: " + std::to_string(edgeLength)).c_str() );

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
            edgeVerticesVector.push_back(tempPair);

            // Increment iterator
            itInputMeshEdge.next();
        }

        // Get initial mesh positions (world coordinates), spring lengths and vertex pair indices
        MPointArray initialPositions = MPointArray();
        fn_input_mesh.getPoints(initialPositions, MSpace::kWorld);

        /*
        // Display stuff
        for(int i = 0; i < initialPositions.length(); ++i)
        {
            MGlobal::displayInfo( ("Vertex position: " + std::to_string(initialPositions[i].x) + " "
                                                       + std::to_string(initialPositions[i].y) + " "
                                                       + std::to_string(initialPositions[i].z) ).c_str() );
        }
        */

        // Create particle system from initial data
        particleSystem = new ParticleSystem(initialPositions, springLengths, edgeVerticesVector, faces);
    }
    else
    {
        // Get the normal array (world coordinates) from the input mesh
        MFloatVectorArray normals = MFloatVectorArray();
        fn_input_mesh.getVertexNormals(true, normals, MSpace::kWorld);

        // Update particle system
        int simSteps = 10;
        for(int i = 0; i < simSteps; ++i)
        {
            particleSystem->simulateSystem((float)( timeDiff.value() / (float)simSteps) );
        }

        // Get new positions (world coordinates) from particle system
        MPointArray newPositions = particleSystem->getPositions();

        // Loop through the geometry and set vertex positions
        while(!itInputMeshVertex.isDone())
        {
            int idx = itInputMeshVertex.index();

            // Get normal
            //MVector nrm = MVector(normals[idx]);

            // Transform new position to local coordinates
            MPoint new_pos = newPositions[idx] * local_to_world_matrix_inv;
            itInputMeshVertex.setPosition(new_pos);

            // Increment iterator
            itInputMeshVertex.next();
        }
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
