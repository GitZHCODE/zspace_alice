#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#define distanceTolerance 0.00001

#include <zSpace/zUtilities.h>
#include<zSpace/zMesh.h>
#include<zSpace/zScalarField.h>

namespace zSpace
{

	//---- FIELD UTILITIES

	/*! \brief This method creates a mesh from the input sclar field.
	*
	*	\param	[in]	field				- zScalarfield2D
	*	\return			zMesh				- mesh of the scalar field.
	*/
	zSPACE_API zMesh createFieldMesh(zScalarField2D field);

	/*! \brief This method creates a vertex distance Field from the input vector of zVector positions.
	*
	*	\param	[in]	fieldMesh			- zMesh of the field.
	*	\param	[in]	points				- vector of positions.
	*	\param	[out]	scalars				- vector of scalar values.
	*/
	zSPACE_API void assignScalarsAsVertexDistance(zMesh &fieldMesh, vector<zVector> &points, vector<double> &scalars);
	
	/*! \brief This method creates a vertex distance Field from the input HE data-structure vertex positions.
	*
	*	\tparam			T					- Type to work with zGraph & zMesh
	*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
	*/
	template<class T>
	void assignScalarsAsVertexDistance(zMesh &fieldMesh, T &HE_DataStructure, bool boundaryVert, double a, double b, vector<double> &scalars);

	/*! \brief This method creates a edge distance Field from the input HE data-structure.
	*
	*	\tparam			T					- Type to work with zGraph & zMesh
	*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
	*/
	template<class T>
	void assignScalarsAsEdgeDistance(zMesh &fieldMesh, T &HE_DataStructure, double a, double b, vector<double> &scalars);

	/*! \brief This method creates a edge distance Field from the input HE data-structure.
	*
	*	\tparam			T					- Type to work with zGraph & zMesh
	*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
	*/
	template<class T>
	void assignScalarsAsHalfEdgeDistance(zMesh &fieldMesh, T &HE_DataStructure, bool flip, double a, double b,  vector<double> &scalars);


	/*! \brief This method creates a union of the fields at the input buffers and stores them in the result buffer.
	*
	*	\param	[in]	scalars0				- value of buffer.
	*	\param	[in]	scalars1				- value of buffer.
	*	\param	[in]	scalarsResult			- value of buffer to store the results.
	*/
	zSPACE_API void union_fields(vector<double>& scalars0, vector<double>& scalars1, vector<double>& scalarsResult);

	/*! \brief This method creates a difference of the fields at the input buffers and stores them in the result buffer.
	*
	*	\param	[in]	scalars0				- value of buffer.
	*	\param	[in]	scalars1				- value of buffer.
	*	\param	[in]	scalarsResult			- value of buffer to store the results.
	*/
	zSPACE_API void difference_fields(vector<double>& scalars0, vector<double>& scalars1, vector<double>& scalarsResult);

	/*! \brief This method creates a intersect of the fields at the input buffers and stores them in the result buffer.
	*
	*	\param	[in]	buffer0				- value of buffer.
	*	\param	[in]	buffer1				- value of buffer.
	*	\param	[in]	res_buffer			- value of buffer to store the results.
	*/
	zSPACE_API void intersect_fields(vector<double>& scalars0, vector<double>& scalars1, vector<double>& scalarsResult);

	/*! \brief This method uses an input plane to clip an existing scalar field.
	*
	*	\param	[in]	mesh				- scalar field mesh.
	*	\param	[in]	scalars				- vector of scalar values. Need to be equivalent to number of mesh vertices.
	*	\param	[in]	clipPlane			- input zPlane used for clipping.
	*/
	zSPACE_API void clipwithPlane(zMesh &mesh, vector<double>& scalars, zPlane& clipPlane );

	/*! \brief This method updates the color values at the given field buffer.
	*
	*	\param	[int]	dMin	- minimum scalar value
	*	\param	[in]	dMax	- maximum scalar value
	*	\param	[in]	buffer	- buffer of scalars.
	*/
	zSPACE_API void updateColors(zMesh &fieldMesh, vector<double>& scalars);

	zSPACE_API zGraph isoContour(zMesh &inMesh, double threshold = 0.5);

	zSPACE_API zMesh isolineMesh(zMesh &inMesh, double threshold = 0.5, bool invertMesh = false);

	zSPACE_API zMesh isobandMesh(zMesh &inMesh,  double thresholdLow = 0.2, double thresholdHigh = 0.5, bool invertMesh = false);

	
	zSPACE_API int marchingSquares_Isoline_getCase(bool vertexBinary[4]);

	zSPACE_API int marchingSquares_Isoband_getCase(int vertexTernary[4]);

	zSPACE_API void marchingSquares_Isoline_computePoly(int& faceId, zMesh &inMesh,  vector<zVector> &positions, vector<int> &polyConnects, vector<int> &polyCounts, zAttributeUnorderedMap <string, int> &positionVertex, double &threshold, bool invertMesh);

	zSPACE_API void marchingSquares_Isoband_computePoly(int& faceId, zMesh &inMesh,  vector<zVector> &positions, vector<int> &polyConnects, vector<int> &polyCounts, zAttributeUnorderedMap <string, int> &positionVertex, double &thresholdLow, double &thresholdHigh);

	zSPACE_API zVector getContourPosition(double &threshold, zVector& vertex_lower, zVector& vertex_higher, double& scalar_lower, double& scalar_higher);

	
	

	//---- UTILITIES

	zSPACE_API zMesh combineDisjointMesh(zMesh &m1, zMesh &m2);

	zSPACE_API zMesh extrudeMeshUp(zMesh &m,  float extrudeThickness , bool thicknessTris = false);

	zSPACE_API void offsetMeshFace(zMesh &m, int faceIndex, double offset, vector<zVector>& offsetPositions);

	zSPACE_API void offsetMeshFace_Variable(zMesh &m, int faceIndex, vector<double>& offsets, zVector& faceCenter, zVector& faceNormal,  vector<zVector>& intersectionPositions);

	zSPACE_API void transformMesh(zMesh &m, Matrix4f& Transform);

	zSPACE_API void setVertexColor(zMesh &m, zColor col , bool setFaceColor = false);

	zSPACE_API void setVertexColors(zMesh &m, vector<zColor>& col, bool setFaceColor = false);

	zSPACE_API void setFaceColor(zMesh &m, zColor col, bool setVertexColor = false);

	zSPACE_API void setFaceColors(zMesh &m, vector<zColor>& col, bool setVertexColor = false);

	
}



// Temmplate method Definition

//This method creates a Metaball Field from the input HE data-structure.
template<class T>
void zSpace::assignScalarsAsVertexDistance(zMesh &fieldMesh, T & HE_DataStructure, bool boundaryVert, double a, double b, vector<double> &scalars)
{
	vector<double> out;

	
	// update values from meta balls

	printf("\n HE_DataStructure.numVertices() : %i  fieldMesh: %i ", HE_DataStructure.numVertices(), fieldMesh.numVertices());

	for (int i = 0; i < fieldMesh.vertexPositions.size(); i++)
	{
		double d = 0.0;
		double tempDist = 10000;

		for (int j = 0; j < HE_DataStructure.numVertices(); j++)
		{
			double r = fieldMesh.vertexPositions[i].distanceTo(HE_DataStructure.vertexPositions[j]);


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

		out.push_back(d);
	}

	double dMin, dMax;
	getMinMaxOfScalars(out, dMin, dMax);

	for (int i = 0; i < out.size(); i++)out[i] = dMax - out[i];

	normaliseScalars(out);

	scalars = out;

}


template<class T>
void zSpace::assignScalarsAsEdgeDistance(zMesh & fieldMesh, T & HE_DataStructure, double a, double b, vector<double>& scalars)
{
	vector<double> out;

	// update values from edge distance
	for (int i = 0; i < fieldMesh.vertexPositions.size(); i++)
	{
		double d = 0.0;
		double tempDist = 10000;

		for (int j = 0; j < HE_DataStructure.numEdges(); j ++)
		{

			int e0 = HE_DataStructure.edges[j].getVertex()->getVertexId();
			int e1 = HE_DataStructure.edges[j].getSym()->getVertex()->getVertexId();

			zVector closestPt;

			double r = minDist_Edge_Point_2D(fieldMesh.vertexPositions[i], HE_DataStructure.vertexPositions[e0], HE_DataStructure.vertexPositions[e1], closestPt);

			
			
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

		out.push_back(d);
		//printf("\n scalars[buffer][i]:  %1.4f ", scalars[buffer][i]);
	}

	double dMin, dMax;
	getMinMaxOfScalars(out,dMin, dMax);

	for (int i = 0; i < out.size(); i++)out[i] = dMax - out[i];

	normaliseScalars(out);

	scalars = out;
}



template<class T>
void zSpace::assignScalarsAsHalfEdgeDistance(zMesh & fieldMesh, T & HE_DataStructure, bool flip, double a, double b, vector<double>& scalars)
{
	vector<double> out;

	// update values from edge distance

	//printf("\n HE_DataStructure.numEdges() : %i ", HE_DataStructure.numEdges());

	for (int i = 0; i < fieldMesh.vertexPositions.size(); i++)
	{
		double d = 0.0;
		double tempDist = 10000;

		for (int j = 0; j < HE_DataStructure.numEdges(); j++ )
		{

			
				int e0 = HE_DataStructure.edges[j].getVertex()->getVertexId();
				int e1 = HE_DataStructure.edges[j].getSym()->getVertex()->getVertexId();



				zVector closestPt;

				double r = minDist_Edge_Point_2D(fieldMesh.vertexPositions[i], HE_DataStructure.vertexPositions[e0], HE_DataStructure.vertexPositions[e1], closestPt);

				if (r < tempDist)
				{

					//double weightB = (edgeWeights.size() == HE_DataStructure.numEdges()) ? edgeWeights[j] * b : 1.0 * b;

					d= F_of_r(r, a, b);


					tempDist = r;
				}
			

		}
		
		out.push_back(d);
		//printf("\n scalars[buffer][i]:  %1.4f ", scalars[buffer][i]);
	}



	double dMin, dMax;
	getMinMaxOfScalars(out, dMin, dMax);

	for (int i = 0; i < out.size(); i++)out[i] = dMax - out[i];

	normaliseScalars(out);


	/*if (flip)
	{
		for (int j = 0; j < HE_DataStructure.numEdges(); j += 2)
		{
			int e0 = HE_DataStructure.edges[j].getVertex()->getVertexId();
			int e1 = HE_DataStructure.edges[j].getSym()->getVertex()->getVertexId();


			zPlane edgePlane;

			edgePlane.X = (HE_DataStructure.vertexPositions[e0] - HE_DataStructure.vertexPositions[e1]);
			edgePlane.X.normalize();

			edgePlane.Y = zVector(0, 0, 1);

			edgePlane.Z = edgePlane.X ^ edgePlane.Y;
			edgePlane.Z.normalize();

			edgePlane.O = HE_DataStructure.vertexPositions[e1];

			clipwithPlane(fieldMesh, out, edgePlane);
		}
	}*/

	scalars = out;
}

