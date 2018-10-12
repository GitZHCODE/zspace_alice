#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif


#include <zSpace/zMesh.h>

namespace zSpace
{

	struct zSPACE_API zDerivative
	{
		zVector dp; // derivative of position : velocity
		zVector dv; // derivative of velocity: acceleration
	};

	struct zSPACE_API zState
	{
		zVector p; //  position 
		zVector v; // velocity
	};

	///
	template <class T>
	zSPACE_API T getEdgeNodeMatrix(zMesh &operateMesh, vector<bool>& fixedVerticesBoolean,int &numEdges, zHEData type = zGraphVertex);

	zSPACE_API SpMat subMatrix(zMesh &operateMesh, SpMat &C, vector<int> &nodes);

	zSPACE_API MatrixXd subMatrix(zMesh &operateMesh, MatrixXd &X, vector<int> &nodes);

	zSPACE_API bool FDM(zMesh &operateMesh, vector<double>& forceDensities, vector<int>& fixedVertices, vector<double> &load);

	//-- TNA


	zSPACE_API zMesh createFormMesh(zMesh &operateMesh, vector<int> &fixedVertices);

	zSPACE_API zMesh createForceMesh(zMesh &formMesh, vector<bool> &fixedVertices, vector<int> &formEdge_forceEdge,  bool rotate90 = true);


	//Horizontal Equilibrium

	zSPACE_API void  TNA_HE(zMesh &formMesh, zMesh &forceMesh, vector<bool>& form_tensionEdges, vector<bool>& force_tensionEdges, vector<int> &formEdge_ForceEdge, vector<zVector> &targetEdges_form, vector<zVector> &targetEdges_force, vector<double> &form_vWeights, vector<double> &force_vWeights, double formWeight = 1, double minMax_formEdge = 0.01, double minMax_forceEdge = 0.1, double angleTolerance = 0.01);
	
	zSPACE_API void TNA_HE_targets(zMesh &formMesh, zMesh &forceMesh, vector<bool>& form_tensionEdges, vector<int> &formEdge_ForceEdge, double formWeight, vector<zVector> &targetEdges_form, vector<zVector> &targetEdges_force);

	zSPACE_API void TNA_updateDiagramMesh(zMesh &diagramMesh, vector<bool>& tensionEdges, vector<zVector> &targetEdges, vector<double> &vWeights, double minMax_diagramEdge);
	
	zSPACE_API bool TNA_checkParallelity(zMesh &formMesh, zMesh &forceMesh, vector<bool>& form_tensionEdges, vector<int> &formEdge_ForceEdge, double &minDeviation, double &maxDeviation, double angleTolerance = 1.0, bool printInfo = false);

	zSPACE_API void TNA_scaleForceMesh(zMesh &formMesh, zMesh &forceMesh);

	//Vertical Equilibrium
	zSPACE_API bool  TNA_VE(zMesh &resultMesh, zMesh &formMesh, zMesh &forceMesh, vector<bool> &fixedVertices_boolean, vector<bool>& form_tensionEdges, vector<int> &formEdge_ForceEdge, vector<double> &localthickness, double forceMesh_scale =1);

	zSPACE_API void  TNA_VE_Iterative(zMesh &resultMesh, vector<zState> &vState, vector<zVector> &vForces, vector<double> vMass, vector<bool> &fixedVertices_boolean, double timeStep = 1.0, zIntergrationType integrateType = zEuler );

	
	zSPACE_API void TNA_updateResultMesh(zMesh &resultMesh, vector<zState> &vState, vector<bool> &fixedVertices_boolean, vector<zDerivative> &vDerivative);

	zSPACE_API bool TNA_checkForceTolerance(vector<zVector> &forces, vector<bool> &fixedVertices_boolean, double forceThreshold, bool printInfo = false);

	
	//Dynamic Relaxation
	zSPACE_API vector<zState> getInitialState(zMesh &resultMesh, zMesh &formMesh);

	zSPACE_API void TNA_VE_Forces(zMesh &resultMesh, zMesh &formMesh, zMesh &forceMesh, vector<bool> &fixedVertices_boolean,  vector<bool> & form_tensionEdges, vector<int> &formEdge_ForceEdge, vector<double> &localthickness, vector<zVector> &vForces , vector<double> &vMass, double forceMesh_scale = 1,double gravity = 9.81, bool addDUMMYVERTICAL = false);

	zSPACE_API void TNA_resetForces(vector<zVector> &forces);
	
	zSPACE_API vector<zVector> TNA_selfWeight(zMesh &resultMesh, vector<bool> &fixedVertices_boolean, vector<bool>& form_tensionEdges, vector<double> &vertexMass,double minHeight, double maxHeight, double gravity = 9.81);
	
	zSPACE_API vector<zVector> TNA_axialForce(zMesh &resultMesh, zMesh &formMesh, zMesh &forceMesh, vector<bool> &fixedVertices_boolean, vector<bool> & form_tensionEdges, vector<int> &formEdge_ForceEdge, double minHeight, double maxHeight,  double forceMesh_scale);
	
	zSPACE_API vector<zVector> TNA_initialVerticalForce(zMesh &resultMesh, vector<bool> &fixedVertices_boolean, vector<bool>& form_tensionEdges);

	zSPACE_API void addForces(vector<zVector> &totalForces, vector<zVector> &forceToAdd);

	zSPACE_API void integrateForces(vector<zState> &vState, vector<zVector> &forces, vector<double> & vertexMass,vector<zDerivative> &vDerivative, double dt, zIntergrationType integrateType = zEuler);


	// FORCE POLYHEDRAs
	
	zSPACE_API vector<zMesh> createForcePolyhedras(zMesh &formMesh, vector<zVector> &fCenters, vector<int> &formVertex_forcePolyhedra_Map);

	zSPACE_API void alignForcePolyhedras(vector<zMesh> &forcePolyhedras , zMesh &formMesh, vector<zVector> &fCenters, vector<int> &formVertex_forcePolyhedra_Map);
	

	//-- UTILITIES
	

	zSPACE_API vector<double> vertexMass(zMesh &resultMesh, vector<double> & localThickness);


	/*! \brief This methods gives the checker board pattern for the  specified type starting with the start index.
	*
	*	\param		[in]	startId		- start index for breadth first search.
	*	\param		[in]	type		- zGraphVertex/ zMeshVertex or zGraphEdge/ zMeshEdge.
	*	\return				vector<bool>- vector of booleans with checkered pattern.
	*/
	zSPACE_API vector<bool> getCheckerBoardPattern(zMesh &HE_Datastructure, int startId, zHEData type = zVertexData);

	zSPACE_API vector<zVector> getCenters(zMesh &HE_Datastructure, zHEData type);

	zSPACE_API bool getCommonFace(zMesh &HE_Datastructure, int edge1, int edge2, int &outFaceId);

	

	zSPACE_API zMesh stereoBlockQuad(zMesh &operateMesh, vector<zVector> &vertexNormals, vector<double> &offsetWidth, vector<double> & offsetLength, vector<double> & offsetThick_top, vector<double> & offsetThick_bottom);

	

	zSPACE_API void exportTNA_Attributes(string outDirectory, double &scale, vector<int> &fixedVerts, vector<int> &formEdge_forceEdge, vector<int> &formVertex_forcePolygon);

	zSPACE_API void importTNA_Attributes(string outDirectory, double &scale, vector<int> &fixedVerts, vector<int> &formEdge_forceEdge, vector<int> &formVertex_forcePolygon);
}

