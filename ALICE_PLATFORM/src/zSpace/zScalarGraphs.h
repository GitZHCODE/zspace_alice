#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif


#include <zSpace\zMesh.h>
#include <zSpace\zScalarField.h>
#include <zSpace\zUtilities.h>
#include <vector>

namespace zSpace
{

	class zSPACE_API zScalarGraphs
	{

	public:

		//----  ATTRIBUTES
		zMesh mesh;
		vector<vector<double>> scalars;
		vector<vector<zColor>> colors;

		Matrix4f Transform;

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zScalarGraphs();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_gridMesh		- grid mesh with equal vertices in X and  Y axis.
		*/
		zScalarGraphs(zMesh &_Mesh);

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zScalarGraphs();

		//---- METHODS

		

		/*! \brief This method creates a vertex distance Field from the input HE data-structure.
		*
		*	\tparam			T					- Type to work with zGraph & zMesh
		*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
		*/
		template<class T>
		void assignScalarsAsVertexDistance(T &HE_DataStructure, bool boundaryVert = false, double a = 0.01, double b = 0.003, int buffer = 0);

		/*! \brief This method creates a edge distance Field from the input HE data-structure.
		*
		*	\tparam			T					- Type to work with zGraph & zMesh
		*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
		*/
		template<class T>
		void assignScalarsAsEdgeDistance(T &HE_DataStructure, double a = 0.01, double b = 0.003, int buffer = 0);

		/*! \brief This method creates a union of the fields at the input buffers and stores them in the result buffer.
		*
		*	\param	[in]	buffer0				- value of buffer.
		*	\param	[in]	buffer1				- value of buffer.
		*	\param	[in]	res_buffer			- value of buffer to store the results.
		*/
		void union_fields(int buffer0, int buffer1, int res_buffer);

		/*! \brief This method creates a difference of the fields at the input buffers and stores them in the result buffer.
		*
		*	\param	[in]	buffer0				- value of buffer.
		*	\param	[in]	buffer1				- value of buffer.
		*	\param	[in]	res_buffer			- value of buffer to store the results.
		*/
		void difference_fields(int buffer0, int buffer1, int res_buffer);

		/*! \brief This method creates a intersect of the fields at the input buffers and stores them in the result buffer.
		*
		*	\param	[in]	buffer0				- value of buffer.
		*	\param	[in]	buffer1				- value of buffer.
		*	\param	[in]	res_buffer			- value of buffer to store the results.
		*/
		void intersect_fields(int buffer0, int buffer1, int res_buffer);

		void clipwithPlane(zPlane plane, int buffer = 0);

		/*! \brief This method contours the scalar field and converts it into a zGraph.
		*
		*	\param	[in]	threshold			- iso value for contouring.
		*	\return			zGraph				- countour lines coverted to a graph.
		*/
		zGraph isoContour(double threshold = 0.5, int buffer = 0);

		/*! \brief This method returns the blend scalar values between current field and input field.
		*
		*	\param	[in]	field_other			- other field for blending.
		*	\param	[in]	buffer				- buffer value for current field.
		*	\param	[in]	buffer_other		- buffer value for other field.
		*	\param	[in]	frame				- current frame value  of blend.
		*	\param	[in]	startFrame			- start frame value  of blend.
		*	\param	[in]	endFrame			- end frame value  of blend.
		*	\return			scalar values		- blend scalar values as vector<double>.
		*/
		vector<double> blendScalarFields(zScalarGraphs &field_other, int buffer, int buffer_other, int frame, int startFrame = 1, int endframe = 100);

		//---- UTILITY METHODS

		/*! \brief This method computes the distance function.
		*
		*	\param	[in]	r	- distance value.
		*	\param	[in]	a	- value of a.
		*	\param	[in]	b	- value of b.
		*/
		double F_of_r(double &r, double &a, double &b);

		/*! \brief This method computes the minimum distance between a point and edge and the closest Point on the edge. (http://paulbourke.net/geometry/pointlineplane/)
		*
		*	\param	[in]	pt			- point
		*	\param	[in]	e0			- start point of edge.
		*	\param	[in]	e1			- end point of edge.
		*	\param	[out]	closest_Pt	- closest point on edge to the input point.
		*	\return			minDist		- distance to closest point.
		*/
		double minDist_Edge_Point_2D(zVector& pt, zVector& e0, zVector& e1, zVector& closest_Pt);

		/*! \brief This method computes the min and max scalar values at the given Scalars buffer.
		*
		*	\param	[out]	dMin	- stores the minimum scalar value
		*	\param	[out]	dMax	- stores the maximum scalar value
		*	\param	[in]	buffer	- buffer of scalars.
		*/
		void getMinMaxOfScalars(double &dMin, double &dMax, int buffer = 0);

		/*! \brief This method normalises the scalar values at the given field buffer.
		*
		*	\param	[in]	buffer	- buffer of scalars.
		*/
		void normaliseScalars(int buffer = 0);

		/*! \brief This method laplace smoothens the scalar values at the given field buffer.
		*
		*	\param	[in]	buffer	- buffer of scalars.
		*/
		void smoothScalars_Colors(int buffer = 0, int numSteps = 1);

		/*! \brief This method updates the color values at the given field buffer.
		*
		*	\param	[int]	dMin	- minimum scalar value
		*	\param	[in]	dMax	- maximum scalar value
		*	\param	[in]	buffer	- buffer of scalars.
		*/
		void updateColors(double &dMax, double &dMin, int buffer = 0);

	};

}



// Temmplate method Definition

//This method creates a Metaball Field from the input HE data-structure.
template<class T>
void zSpace::zScalarGraphs::assignScalarsAsVertexDistance(T & HE_DataStructure, bool boundaryVert, double a, double b, int buffer)
{
	// create scalars if  the buffer doesn't exist
	if (buffer >= scalars.size())
	{
		vector<double> scalarVals;
		vector<zColor> colorVals;

		for (int i = 0; i < mesh.vertexPositions.getValuesSize(); i++)
		{
			scalarVals.push_back(1);
			colorVals.push_back(zColor(1, 0, 0, 1));
		}

		scalars.push_back(scalarVals);
		colors.push_back(colorVals);
	}


	// update values from meta balls

	//printf("\n HE_DataStructure.numVertices() : %i ", HE_DataStructure.numVertices());

	for (int i = 0; i < mesh.vertexPositions.getValuesSize(); i++)
	{
		double d = 0.0;
		double tempDist = 10000;

		for (int j = 0; j < HE_DataStructure.numVertices(); j++)
		{
			double r = mesh.vertexPositions.getValue(i).distanceTo(HE_DataStructure.vertexPositions.getValue(j));


			if (boundaryVert)
			{
				if (HE_DataStructure.checkVertexValency(j, 1))
				{
					if (r < tempDist)
					{
						d = F_of_r(r, a, b);
						//printf("\n F_of_r:  %1.4f ", F_of_r(r, a, b));
						tempDist = r;
					}
				}
			}
			else
			{
				if (r < tempDist)
				{
					d = F_of_r(r, a, b);
					//printf("\n F_of_r:  %1.4f ", F_of_r(r, a, b));
					tempDist = r;
				}
			}


			//d += F_of_r(r, a, b);
			//printf("\n F_of_r:  %1.2f ", F_of_r(r, a, b));
		}

		scalars[buffer][i] = d;
	}

	double dMin, dMax;
	getMinMaxOfScalars(dMin, dMax, buffer);

	for (int i = 0; i < scalars[buffer].size(); i++)scalars[buffer][i] = dMax - scalars[buffer][i];

	normaliseScalars(buffer);

}

//This method creates a edge distance Field from the input HE data - structure.
template<class T>
void zSpace::zScalarGraphs::assignScalarsAsEdgeDistance(T& HE_DataStructure, double a, double b, int buffer)
{
	// create scalars if  the buffer doesn't exist
	while (buffer >= scalars.size())
	{
		vector<double> scalarVals;
		vector<zColor> colorVals;

		for (int i = 0; i < mesh.vertexPositions.getValuesSize(); i++)
		{
			scalarVals.push_back(1);
			colorVals.push_back(zColor(1, 0, 0, 1));
		}

		scalars.push_back(scalarVals);
		colors.push_back(colorVals);
	}

	// update values from edge distance

	//printf("\n HE_DataStructure.numEdges() : %i ", HE_DataStructure.numEdges());

	for (int i = 0; i < mesh.vertexPositions.getValuesSize(); i++)
	{
		double d = 0.0;
		double tempDist = 10000;

		for (int j = 0; j < HE_DataStructure.numEdges(); j += 2)
		{

			int e0 = HE_DataStructure.edges[j].getVertex()->getVertexId();
			int e1 = HE_DataStructure.edges[j + 1].getVertex()->getVertexId();

			zVector closestPt;

			double r = minDist_Edge_Point_2D(mesh.vertexPositions.getValue(i), HE_DataStructure.vertexPositions.getValue(e0), HE_DataStructure.vertexPositions.getValue(e1), closestPt);

			//printf("\n v: %i e: %i r:  %1.2f ", i,j, r);

			//printf("\n pt: %1.2f %1.2f %1.2f closestEdgePt: %1.2f %1.2f %1.2f r: %1.2f", mesh.vertexPositions.getValue(i).x, mesh.vertexPositions.getValue(i).y, mesh.vertexPositions.getValue(i).z, closestPt.x, closestPt.y, closestPt.z,r);

			if (r < tempDist)
			{
				d = F_of_r(r, a, b);
				//printf("\n F_of_r:  %1.4f ", F_of_r(r, a, b));

				tempDist = r;
			}

			//d += F_of_r(r, a, b);
			//printf("\n F_of_r:  %1.4f ", F_of_r(r, a, b));
		}

		scalars[buffer][i] = d;
		//printf("\n scalars[buffer][i]:  %1.4f ", scalars[buffer][i]);
	}

	double dMin, dMax;
	getMinMaxOfScalars(dMin, dMax, buffer);

	for (int i = 0; i < scalars[buffer].size(); i++)scalars[buffer][i] = dMax - scalars[buffer][i];

	normaliseScalars(buffer);

}
