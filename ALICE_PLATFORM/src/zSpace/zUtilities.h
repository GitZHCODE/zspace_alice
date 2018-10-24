
// DESCRIPTION:
//      Text file reader
//

#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <vector>
#include <algorithm>    // std::sort
//#include <windows.h>	// Header File For Windows
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <vector>
#include <stdio.h>
#include <direct.h>
#include <time.h> 
#include <ctype.h>
#include <numeric>
using namespace std;

#include <zSpace/zVector.h>
#include <zSpace/zDatatypes.h>
#include <zSpace/zAttributetypes.h>
#include <zSpace/zEnumerators.h>
//#include <zSpace/zExchange.h>



#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Eigen>
#include <Eigen/Sparse>
using namespace Eigen;

// define
#ifndef PI
#define PI       3.14159265358979323846
#endif

#ifndef TWO_PI
#define TWO_PI   6.28318530717958647693
#endif

#ifndef M_TWO_PI
#define M_TWO_PI   6.28318530717958647693
#endif

#ifndef FOUR_PI
#define FOUR_PI 12.56637061435917295385
#endif

#ifndef HALF_PI
#define HALF_PI  1.57079632679489661923
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI/180.0)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0/PI)
#endif

namespace zSpace
{

	// split String 
	zSPACE_API vector<string> splitString(const string& str, const string& delimiter);
	
	// domain mapping
	zSPACE_API double ofMap(double value, double inputMin, double inputMax, double outputMin, double outputMax);

	/*! \brief This method computes the best fit plane for the given points using PCA
	*
	*	\param		[in]	points			- input points.
	*	\return 			Best fit zPlane.
	*/
	zSPACE_API zPlane getBestFitPlane(vector<zVector>& points);

	/*! \brief This method returns the minimum of the two input values.
	*
	*	\param		[in]	val0			- input value 1.
	*	\param		[in]	val1			- input value 2.
	*	\return 			double			- minimum value
	*/
	zSPACE_API double zMin(double val0, double val1);

	/*! \brief This method returns the maximum of the two input values.
	*
	*	\param		[in]	val0			- input value 1.
	*	\param		[in]	val1			- input value 2.
	*	\return 			double			- maximum value
	*/
	zSPACE_API double zMax(double val0, double val1);

	/*! \brief This method computes the bounding box for the given points using PCA
	*
	*	\param		[in]	points			- input points.
	*	\param		[out]	minBB			- lower bounds as zVector
	*	\param		[out]	maxBB			- upper bounds as zVector
	*/
	zSPACE_API void boundingboxPCA(vector<zVector> points, zVector &minBB , zVector &maxBB, zVector &minBB_local, zVector &maxBB_local);

	/*! \brief This method computes the distances in X,Y,Z for the input bounds.
	*
	*	\param		[in]	minBB			- lower bounds as zVector.
	*	\param		[in]	maxBB			- upper bounds as zVector
	*	\param		[out]	Dims			- distances in X,Y,Z axis in local frame
	*/
	zSPACE_API zVector getDimsFromBounds(zVector &minBB, zVector &maxBB);

	/*! \brief This method computes the tranformation to the world matrix.
	*
	*	\param [in]		infilename			- input file name including the directory path.
	*/
	zSPACE_API Matrix4f toWorldMatrix(Matrix4f &inMatrix);

	/*! \brief This method computes the tranformation to the local matrix.
	*
	*	\param [in]		infilename			- input file name including the directory path.
	*/
	zSPACE_API Matrix4f toLocalMatrix(Matrix4f &inMatrix);

	/*! \brief This method computes the tranformation from one plane to another.
	*
	*	\param [in]		infilename			- input file name including the directory path.
	*/
	zSPACE_API Matrix4f PlanetoPlane(Matrix4f &from, Matrix4f &to);

	/*! \brief This method computes the tranformation to change the basis.
	*
	*	\param [in]		infilename			- input file name including the directory path.
	*/
	zSPACE_API Matrix4f ChangeBasis(Matrix4f &from, Matrix4f &to);

	/*! \brief This method computes the input target as per the input new basis.
	*
	*	\param [in]		infilename			- input file name including the directory path.
	*/
	zSPACE_API Matrix4f target_newBasis(Matrix4f &target, Matrix4f &newBasis);

	/*! \brief This method  converts a Vector4f to zVector.
	*
	*	\param [in]		pt			- input point as zVector.
	*	\return			Vector4f	- output point.
	*/
	zSPACE_API Vector4f toEigenVector(zVector &pt);

	/*! \brief This method  converts a Vector4f to zVector.
	*
	*	\param [in]		pt			- input point as Vector4f.
	*	\return			zVector	- output point.
	*/
	zSPACE_API zVector toZVector(Vector4f &pt);

	/*! \brief This method  returns the intersection of two planes which is  line.
	*
	*	\param [in]		nA	- input normal of plane A.
	*	\param [in]		nB	- input normal of plane B.
	*	\param [in]		pA		- input point on plane A.
	*	\param [in]		pB		- input point on plane B.
	*	\param [out]	outPt1	- intersection point 1.
	*	\param [out]	outPt2	- intersection point 2.
	*	\return			bool	- true if the planes intersect.
	*/
	zSPACE_API bool plane_planeIntersection(zVector &nA, zVector &nB, zVector &pA, zVector &pB, zVector &outP1, zVector &outP2);

	/*! \brief This method  returns the closest points of two lines. 
	*
	*	\param [in]		a0		- first input point on line A.
	*	\param [in]		a1		- second input point on line A.
	*	\param [in]		b0		- first input point on line B.
	*	\param [in]		b1		- second input point on line B.
	*	\param [out]	uA		- line parameter for closest point on A .
	*	\param [out]	uB		- line parameter for closest point on B .
	*	\return			bool	- true if the planes intersect.
	*/
	zSPACE_API bool line_lineClosestPoints(zVector &a0, zVector &a1, zVector &b0, zVector &b1, double &uA, double &uB );

	/*! \brief This method  returns the intersection of two lines which is  point.
	*
	*	\param [in]		p0		- first input point on line A.
	*	\param [in]		p1		- second input point on line A.
	*	\param [in]		p2		- first input point on line B.
	*	\param [in]		p3		- second input point on line B.
	*	\param [out]	outPt	- intersection point .
	*	\return			bool	- true if the planes intersect.
	*/
	zSPACE_API bool line_lineIntersection(zVector &p0, zVector &p1, zVector &p2, zVector &p3, zVector &outPt);

	/*! \brief This method  returns the intersection of two lines which is  point.
	*
	*	\param [in]		p0		- first input point on line A.
	*	\param [in]		p1		- second input point on line A.
	*	\param [in]		p2		- first input point on line B.
	*	\param [in]		p3		- second input point on line B.
	*	\param [out]	outPt	- intersection point .
	*	\return			bool	- true if the planes intersect.
	*/
	zSPACE_API bool line_PlaneIntersection(zVector &p1, zVector &p2, zVector &planeNorm, zVector &p3, zVector &intersectionPt);


	// MAP Utlities 

	zSPACE_API bool existsInMap(string hashKey, unordered_map<string, int> map, int &outVal);


	// VECTOR UTILITIES

	/*! \brief This method returns the area of triagle defined by the two input zVectors.
	*
	*	\return				double		- area of triangle defirned by the vectors.
	*/
	zSPACE_API double triangleArea(zVector &v1, zVector &v2, zVector &v3);


	zSPACE_API zVector rotateAboutAxis(zVector inVec, zVector axisVec, double angle = 0);


	// NUMBER UTILITIES

	/*! \brief This method returns a random number in the input domain.
	*
	*	\param [in]		min		- domain minimum value.
	*	\param [in]		max		- domain maximum value.
	*	\return			int		- random number between the 2 input values.
	*/
	zSPACE_API int randomNumber(int min, int max);

	zSPACE_API double randomNumber_double(double min, double max);


	// FIELD UTILITIES
	
	/*! \brief This method computes the minimum distance between a point and edge and the closest Point on the edge. (http://paulbourke.net/geometry/pointlineplane/)
	*
	*	\param	[in]	pt			- point
	*	\param	[in]	e0			- start point of edge.
	*	\param	[in]	e1			- end point of edge.
	*	\param	[out]	closest_Pt	- closest point on edge to the input point.
	*	\return			minDist		- distance to closest point.
	*/
	zSPACE_API double minDist_Edge_Point_2D(zVector & pt, zVector & e0, zVector & e1, zVector & closest_Pt);

	/*! \brief This method computes the distance function.
	*
	*	\param	[in]	r	- distance value.
	*	\param	[in]	a	- value of a.
	*	\param	[in]	b	- value of b.
	*/
	zSPACE_API double F_of_r(double &r, double &a, double &b);

	/*! \brief This method computes the min and max scalar values at the given Scalars buffer.
	*
	*	\param	[out]	dMin	- stores the minimum scalar value
	*	\param	[out]	dMax	- stores the maximum scalar value
	*	\param	[in]	buffer	- buffer of scalars.
	*/
	zSPACE_API void getMinMaxOfScalars( vector<double>& scalars, double &dMin, double &dMax);

	/*! \brief This method normalises the scalar values at the given field buffer.
	*
	*	\param	[in]	buffer	- buffer of scalars.
	*/
	zSPACE_API void normaliseScalars(vector<double>& scalars);



	// LIGHT UTILITIES

	zSPACE_API zColor computeOcculusionColor(zVector& norm, zVector &lightVec);


	// MAP UTILITIES
	zSPACE_API bool vertexExists(zAttributeUnorderedMap <string, int> & positionVertex, zVector & pos, int & outVertexId);
}
