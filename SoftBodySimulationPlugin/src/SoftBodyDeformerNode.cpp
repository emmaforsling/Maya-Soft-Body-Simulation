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
    MItMeshEdge itInputMeshEdge = MItMeshEdge(o_input_geom, &status);

    // Allocate memory for storing all adges of the mesh
    springLengths = new double[itInputMeshEdge.count(&status)];

    // Initialize all spring resting lengths
    if(currentFrame == 1)
    {
        while(!itInputMeshEdge.isDone())
        {
            int idx = itInputMeshEdge.index();

            // Get the length of the edge
            double edgeLength;
            itInputMeshEdge.getLength(edgeLength);
            // Store the lenght in the allocated array
            springLengths[idx] = float(edgeLength);

            //MGlobal::displayInfo(std::to_string(springLengths[idx]).c_str());

            // Increment iterator
            itInputMeshEdge.next();
        }
    }

    // Create neighbor vertices array
    MIntArray neighborVertices;

    // Get the normal array from the input mesh
    MFloatVectorArray normals = MFloatVectorArray();
    fn_input_mesh.getVertexNormals(true, normals, MSpace::kTransform);

    /*
    std::string output = "Current position = ";
    output += std::to_string((it_geo.position() * local_to_world_matrix).x) + " " +
        std::to_string((it_geo.position() * local_to_world_matrix).y) + " " +
        std::to_string((it_geo.position() * local_to_world_matrix).z);
    MGlobal::displayInfo(output.c_str());
    */

    // Loop through the geometry and set vertex positions
    while(!itInputMeshVertex.isDone())
    {
        int idx = itInputMeshVertex.index();
        MVector nrm = MVector(normals[idx]);
        MPoint pos = itInputMeshVertex.position();

        // Get all neighboring vertices for current vertex
        itInputMeshVertex.getConnectedVertices(neighborVertices);

        // Loop through neighbor vertices and calculate [TEMP] average distance vectors
        MFloatVector totDist, avgDist;
        MFloatVector totForce, curForce;

        std::string output = "Frame " + std::to_string(currentFrame);
        output += ". Neighbor vertices for vertex " + std::to_string(itInputMeshVertex.index()) + ":";
        int count_neighbors = 0;
        for(int i = 0; i < neighborVertices.length(); ++i)
        {
            // Create temporary point variable
            MPoint neighborPos;
            // Store current neighbor position in this variable
            status = fn_input_mesh.getPoint(neighborVertices[i], neighborPos);
            // Calculate distance to current neighbor
            MFloatVector distToNeighbor = neighborPos - pos;
            // Increment the total distance variable
            totDist += distToNeighbor;

            // TODO: Calculate spring force
            // Hooke's law: F = kX, where X is elongation from equilibrium. This requires that the springs
            // are initialized with "resting lengths" when initilizing the deformer node. Perhaps...? =)

            output += " " + std::to_string(neighborVertices[i]);
            count_neighbors++;
        }

        // Calculate average distance
        avgDist.x = totDist.x / count_neighbors;
        avgDist.y = totDist.y / count_neighbors;
        avgDist.z = totDist.z / count_neighbors;

        // Append average distance to output string
        output += ", average distance: " + std::to_string(avgDist.x) + " " + std::to_string(avgDist.y) + " " + std::to_string(avgDist.z);

        //MGlobal::displayInfo(output.c_str());

        MPoint gravityDisplacement = timeDiff.value() * gravityVec;             // Fel, fel fel...
        MPoint new_pos = pos;
        itInputMeshVertex.setPosition(new_pos);

        MGlobal::displayInfo(std::to_string((pos * local_to_world_matrix).z).c_str());

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