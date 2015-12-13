#include "../include/softBodyDeformerNode.h"

MTypeId softBodyDeformerNode::id(0x00000002);

MObject softBodyDeformerNode::aGravityMagnitude;
MObject softBodyDeformerNode::aGravityDirection;
MObject softBodyDeformerNode::aCurrentTime;
MObject softBodyDeformerNode::aSpringConstant;
MObject softBodyDeformerNode::aMass;
MObject softBodyDeformerNode::aElasticity;
//MObject softBodyDeformerNode::aGasApprox;

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
    float env             = data.inputValue(envelope).asFloat();
    MVector gravityVec    = data.inputValue(aGravityMagnitude).asDouble() * data.inputValue(aGravityDirection).asVector();
    MTime tNow            = data.inputValue(aCurrentTime).asTime();
    
    // Get the spring constant, mass, elasticity input values
    float springConstant   = data.inputValue(aSpringConstant).asFloat();
    float mass             = data.inputValue(aMass).asFloat();
    float elasticity       = data.inputValue(aElasticity).asFloat();
    //float gass             = data.inputValue(aGasApprox).asFloat();

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
    MItMeshVertex  itInputMeshVertex  = MItMeshVertex(o_input_geom, &status);
    MItMeshPolygon itInputMeshPolygon = MItMeshPolygon(o_input_geom, &status);
    MItMeshEdge    itInputMeshEdge    = MItMeshEdge(o_input_geom, &status);

    // Temporary arrays for storing edge properties
    std::vector<float> springLengths;
    std::vector<std::array<int, 2> > edgeVerticesVector;
    std::vector<std::array<int, 3> > faceVerticesVector;

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

        // Loop through polygons and create face list
        while(!itInputMeshPolygon.isDone())
        {
            int idx = itInputMeshPolygon.index();

            // Get vertices connected to the face
            MIntArray connectedVertices;
            itInputMeshPolygon.getConnectedVertices(connectedVertices);

            // Create temporary face vertices
            std::array<int, 3> tempVerts;
            tempVerts[0] = connectedVertices[0];
            tempVerts[1] = connectedVertices[1];
            tempVerts[2] = connectedVertices[2];

            // Append vertices to face list
            faceVerticesVector.push_back(tempVerts);

            // Increment iterator
            itInputMeshPolygon.next();
        }

        // Get initial mesh positions (world coordinates), spring lengths and vertex pair indices
        MPointArray initialPositions = MPointArray();
        fn_input_mesh.getPoints(initialPositions, MSpace::kWorld);

        // Create particle system from initial data
        particleSystem = new ParticleSystem(initialPositions, springLengths, edgeVerticesVector, faceVerticesVector, springConstant, mass, elasticity);
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

/*
 * Initialize the attributes that will be displayed in maya
**/
MStatus softBodyDeformerNode::initialize()
{
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;

    // Gravity magnitude
    aGravityMagnitude = nAttr.create("aGravityMagnitude", "gm", MFnNumericData::kDouble, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(0.0);
    nAttr.setMax(10.0);
    nAttr.setChannelBox(true);

    // Gravity directon
    aGravityDirection = nAttr.create("aGravityDirection", "gd", MFnNumericData::k3Double, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    // Spring constant
    aSpringConstant = nAttr.create("aSpringConstant", "sc", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(0.75);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    // Vertex Mass
    aMass = nAttr.create("aMass", "am", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(1.0);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    // Elasticity
    aElasticity = nAttr.create("aElasticity", "ae", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(0.8);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    /*// Gas approximation
    aGasApprox = nAttr.create("aGasApprox", "ga", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(0.8);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);*/

    // Time
    aCurrentTime = uAttr.create("aCurrentTime", "ct", MFnUnitAttribute::kTime, 0.0);
    uAttr.setDefault(MAnimControl::currentTime().as(MTime::kFilm));
    uAttr.setChannelBox(true);

    // Add the attributes
    addAttribute(aCurrentTime);
    addAttribute(aGravityMagnitude);
    addAttribute(aGravityDirection);
    addAttribute(aSpringConstant);
    addAttribute(aMass);
    addAttribute(aElasticity);
    //addAttribute(aGasApprox);


    // Affect
    attributeAffects(aCurrentTime, outputGeom);
    attributeAffects(aGravityMagnitude, outputGeom);
    attributeAffects(aGravityDirection, outputGeom);
    attributeAffects(aSpringConstant, outputGeom);
    attributeAffects(aMass, outputGeom);
    attributeAffects(aElasticity, outputGeom);

    return MS::kSuccess;
}
