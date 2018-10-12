#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <zSpace/zUtilities.h>
#include<zSpace/zMeshUtilities.h>


using namespace std;

namespace zSpace
{
	struct zSPACE_API zProceduralFloor
	{
		zMesh floor_slab;		
		zMesh floor_soffit;
		
		zMesh glass;
		zMesh balustrade;

		double floorWidth;
		double floorHeight;
		double slabDepth;
		double terraceWidth;
		double balconyWidth;

		double GFA;

		Matrix4f floorTransform;

		void translatePositions(zMesh &m , zVector& trans);
		
		void transformFloor(Matrix4f& trans);



	};
	
	class zSPACE_API zProceduralBuilding
	{
	public:
		//----  ATTRIBUTES
		zMesh plot;
		zMesh buildingOutline;
		
		zScalarField2D field;
		zMesh fieldMesh;

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

		void generateDistanceFieldfromBuilding( double& floorWidth, vector<double>& scalars);

		void generateFloors_GFA(double& requiredGFA, double& maxHeight, double& floorWidth, double& floorHeight, double& slabDepth, double& terraceWidth, double& balconyWidth, bool triangulate = false);

		void addFloor(double& maxHeight, double& floorWidth, double& floorHeight, double& slabDepth, double& terraceWidth, double& balconyWidth,  bool triangulate = false);

		void editFloor(int floorNum, double& maxHeight, double& floorWidth, double& floorHeight, double& slabDepth, double& terraceWidth, double& balconyWidth, bool triangulate = false);

		//--- UTILITY METHODS 

		double getTotalHeight();

		double getGFA(zMesh &slab);

	};
}



