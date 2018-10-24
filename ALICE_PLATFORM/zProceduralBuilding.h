#pragma once

#include <zSpace/zUtilities.h>
#include<zSpace/zMeshUtilities.h>


using namespace std;

namespace zSpace
{
	struct zProceduralFloor
	{
		zMesh floor_slab;
		zMesh floor_soffit;
		zMesh floor_balcony;

		zMesh glass;
		zMesh balustrade;

		double floorWidth;
		double floorHeight;
		double slabDepth;
		double terraceWidth;
		double balconyWidth;

		double GFA;

		Matrix4f floorTransform;

		void translatePositions(zMesh &m, zVector& trans);

		void transformFloor(Matrix4f& trans);
	};

	class zProceduralBuilding
	{
	public:

		//----  ATTRIBUTES
		zMesh plot;
		zMesh buildingOutline;

		zGraph slabGraph;
		zGraph terraceGraph;
		zGraph balconyGraph;
		zGraph bridgeGraph;

		zScalarField2D field;
		zMesh fieldMesh;

		double maxHeight;
		double floorWidth;
		double floorHeight;
		double slabDepth;
		double terraceWidth;
		double balconyWidth;
		double bridgeWidth;


		vector<zMesh> slabs2D;
		vector<zProceduralFloor> floors;


		double building_Height;

		double building_GFA;

		double building_PlotOffset;

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zProceduralBuilding();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zProceduralBuilding();

		//--- COMPUTE METHODS 

		/*! \brief This method sets the plot mesh to the input mesh.
		*
		*	\param		[in]	zMesh	- plotMesh ( needs to convex polygon).
		*/
		void setPlotOutline(zMesh& _plot);

		/*! \brief This method sets the building outline based on the offset from the plot.
		*
		*	\param		[in]	plotOffset	- equal plot offset on all edges of the plot.
		*/
		void setBuildingOutline(double plotOffset);

		void generateEdgeDistanceField(zGraph& inGraph, double& width, vector<vector<double>>& scalars);

		void generateFloors_GFA(bool floor_fieldBooleans[4], vector<vector<double>>& scalarValues, double& requiredGFA, double& maxHeight, bool dBuffer);

		void addFloor(bool floor_fieldBooleans[4], vector<vector<double>>& scalars, bool triangulate = false);

		void editFloor(int floorNum, double& maxHeight, double& floorWidth, double& floorHeight, double& slabDepth, double& terraceWidth, double& balconyWidth, bool triangulate = false);

		//--- UTILITY METHODS 

		double getTotalHeight();

		double getGFA(zMesh &slab);


		zMesh getOffsetMesh(zMesh& inMesh, double offset);

		zGraph getSlabGraph(zMesh& inMesh, double& offsetweight, int splitSteps);

		zGraph getBalconyGraph(zMesh& inMesh, double& offsetweight, int splitSteps, int numVerts_Balcony, int numVerts_Gap, int startId = 0);

		zGraph getTerraceGraph(zMesh& inMesh, double& offsetweight, int splitSteps, int numVerts_Terrace, int numVerts_Gap, int startId = 0);

		zGraph getBridgeGraph(zMesh& inMesh, double& offsetweight, int splitSteps, int numBridges, int numVerts_Gap, int startId = 0);

		zGraph getMeshFaceEdgesasGraph(zMesh& inMesh, int faceIndex);

		zGraph splitGraph(zMesh& inMesh, double& offsetWeight);


	};
}



