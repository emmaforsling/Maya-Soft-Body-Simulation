#include "../include/softBodyDeformerNode.h"

MTypeId softBodyDeformerNode::id(0x00000002);

MObject softBodyDeformerNode::gravityMagnitudeAttribute;
MObject softBodyDeformerNode::gravityDirectionAttribute;
MObject softBodyDeformerNode::currentTimeAttribute;
MObject softBodyDeformerNode::springConstantAttribute;
MObject softBodyDeformerNode::damperConstantAttribute;
MObject softBodyDeformerNode::massAttribute;
MObject softBodyDeformerNode::elasticityAttribute;
MObject softBodyDeformerNode::gasPropertiesValueAttribute;

MTime softBodyDeformerNode::tPrevious;

void* softBodyDeformerNode::creator()
{
    return new softBodyDeformerNode;
}

MStatus softBodyDeformerNode::deform(MDataBlock& data, MItGeometry& it_geo,
                                     const MMatrix &local_to_world_matrix, unsigned int m_index)
{
    MStatus status;
    MMatrix world_to_local_matrix = local_to_world_matrix.inverse();

    // Get the current frame
    MTime currentTime = MAnimControl::currentTime();
    int currentFrame = (int)currentTime.value();

    // Get the envelope, gravity and current time input values
    float env             = data.inputValue(envelope).asFloat();
    MVector gravityVec    = data.inputValue(gravityMagnitudeAttribute).asDouble() * data.inputValue(gravityDirectionAttribute).asVector();
    MTime tNow            = data.inputValue(currentTimeAttribute).asTime();
    
    // Get the spring constant, mass, elasticity input values
    float springConstant   = data.inputValue(springConstantAttribute).asFloat();
    float mass             = data.inputValue(massAttribute).asFloat();
    float elasticity       = data.inputValue(elasticityAttribute).asFloat();
    float gas              = data.inputValue(gasPropertiesValueAttribute).asFloat();
    float damperConstant   = data.inputValue(damperConstantAttribute).asFloat();

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
    MFloatVectorArray faceNormals;

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

        // Loop through polygons and create face list and face normal list
        while(!itInputMeshPolygon.isDone())
        {
            int idx = itInputMeshPolygon.index();

            // Get vertices connected to the face
            MIntArray connectedVertices;
            status = itInputMeshPolygon.getConnectedVertices(connectedVertices);

            MIntArray faceVertexIndices;
            MPointArray faceVertexPositions;
            itInputMeshPolygon.getTriangle(0, faceVertexPositions, faceVertexIndices, MSpace::kWorld);

            // Create temporary face vertices
            std::array<int, 3> tempVerts;
            tempVerts[0] = faceVertexIndices[0];
            tempVerts[1] = faceVertexIndices[1];
            tempVerts[2] = faceVertexIndices[2];

            // Append vertices to face list
            faceVerticesVector.push_back(tempVerts);

            // Get face normal
            MVector curFaceNormal;
            itInputMeshPolygon.getNormal(curFaceNormal, MSpace::kWorld);
  
            // Append normal to the normal list
            faceNormals.append(curFaceNormal);

            // Increment iterator
            itInputMeshPolygon.next();
        }

        // Get initial mesh positions (world coordinates), spring lengths and vertex pair indices
        MPointArray initialPositions = MPointArray();
        fn_input_mesh.getPoints(initialPositions, MSpace::kWorld);

        // Create particle system from initial data
        particleSystem = new ParticleSystem(initialPositions,
                                            springLengths,
                                            edgeVerticesVector,
                                            faceVerticesVector,
                                            springConstant,
                                            damperConstant,
                                            mass,
                                            elasticity,
                                            gas,
                                            faceNormals,
                                            world_to_local_matrix);
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

            // Transform new position to local coordinates
            MPoint new_pos = newPositions[idx] * world_to_local_matrix;
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
    gravityMagnitudeAttribute = nAttr.create("gravityMagnitudeAttribute", "gm", MFnNumericData::kDouble, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(0.0);
    nAttr.setMax(10.0);
    nAttr.setChannelBox(true);

    // Gravity directon
    gravityDirectionAttribute = nAttr.create("gravityDirectionAttribute", "gd", MFnNumericData::k3Double, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(-1.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    // Spring constant
    springConstantAttribute = nAttr.create("springConstantAttribute", "sc", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(10.0);
    nAttr.setMin(0.0);
    nAttr.setMax(10.0);
    nAttr.setChannelBox(true);

    // Damper constant
    damperConstantAttribute = nAttr.create("damperConstantAttribute", "dc", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(0.9);
    nAttr.setMin(0.0);
    nAttr.setMax(10.0);
    nAttr.setChannelBox(true);

    // Vertex Mass
    massAttribute = nAttr.create("massAttribute", "am", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(1.0);
    nAttr.setMin(0.0);
    nAttr.setMax(20.0);
    nAttr.setChannelBox(true);

    // Elasticity
    elasticityAttribute = nAttr.create("elasticityAttribute", "ae", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(0.0);
    nAttr.setMin(0.0);
    nAttr.setMax(1.0);
    nAttr.setChannelBox(true);

    // Gas approximation
    gasPropertiesValueAttribute = nAttr.create("gasPropertiesValueAttribute", "ga", MFnNumericData::kFloat, 0.0);
    nAttr.setDefault(4.0);
    nAttr.setMin(0.0);
    nAttr.setMax(100.0);
    nAttr.setChannelBox(true);

    // Time
    currentTimeAttribute = uAttr.create("currentTimeAttribute", "ct", MFnUnitAttribute::kTime, 0.0);
    uAttr.setDefault(MAnimControl::currentTime().as(MTime::kFilm));
    uAttr.setChannelBox(true);

    // Add the attributes
    addAttribute(currentTimeAttribute);
    addAttribute(gravityMagnitudeAttribute);
    addAttribute(gravityDirectionAttribute);
    addAttribute(springConstantAttribute);
    addAttribute(damperConstantAttribute);
    addAttribute(massAttribute);
    addAttribute(elasticityAttribute);
    addAttribute(gasPropertiesValueAttribute);

    // Affect
    attributeAffects(currentTimeAttribute, outputGeom);
    attributeAffects(gravityMagnitudeAttribute, outputGeom);
    attributeAffects(gravityDirectionAttribute, outputGeom);
    attributeAffects(springConstantAttribute, outputGeom);
    attributeAffects(damperConstantAttribute, outputGeom);
    attributeAffects(massAttribute, outputGeom);
    attributeAffects(elasticityAttribute, outputGeom);
    attributeAffects(gasPropertiesValueAttribute, outputGeom);

    return MS::kSuccess;
}
