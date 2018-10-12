#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif


#include <zSpace/zMesh.h>


namespace zSpace
{

	zSPACE_API zGraph createCentreLineGraph(vector<zMesh> & inputVolumeMeshes,vector<double> &volColor,  zAttributeUnorderedMap <string, int> &volumeFace_Vertex, vector<int> &graphPosition_volumeMesh, vector<int> &graphPosition_volumeFace, vector<double> &graphVertex_Offset);

	
	zSPACE_API zMesh createPolytopalMesh(zMesh &inputVolumeMesh,int &volMeshId, zGraph &centerLinegraph, zAttributeUnorderedMap <string, int> &volumeFace_Vertex, vector<double> &graphVertex_Offset);

	zSPACE_API zMesh remeshSmoothPolytopalMesh(zMesh &lowPolytopalMesh, zMesh &smoothPolytopalMesh, int SUBDIVS = 1);

	zSPACE_API void closePolytopalMesh(zMesh &inputVolumeMesh, zMesh &smoothPolytopalMesh, int SUBDIVS = 1);

	zSPACE_API bool computeRulingIntersection(zMesh &smoothPolytopalMesh, int v0, int v1, zVector &closestPt);

	zSPACE_API zMesh createSurfaceMesh(zMesh &inputVolumeMesh, double boundaryOffset = 0.1, double centerOffset = 0.1);


	zSPACE_API void explodePolytopalMeshes(vector<zMesh> & inputVolumeMeshes, zGraph &centerlineGraph, vector<int> &graphPosition_volumeMesh, double scaleFactor =1);
}

