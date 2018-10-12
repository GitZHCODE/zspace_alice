
#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif




#include <zSpace/zBufferRender.h>

#include <zSpace/zRobot.h>
#include <zSpace/zScalarGraphs.h>
#include <zSpace/zSlime.h>


namespace zSpace
{
	/*! \brief This method draws a point.
	\param		 [in]		pos			- position of the point to be drawn.
	\param		 [in]		col			- color of the point.
	\param		 [in]		wt			- weight of the point.
	*/
	zSPACE_API void drawPoint(zVector &pos, zColor col, double wt);

	/*! \brief This method draws a line between the given two points.
	*	\param		[in]		p0			- start Point of the line.
	*	\param		[in]		p1			- end Point of the line.
	*	\param		[in]		col			- color of the line.
	*	\param		[in]		wt			- weight of the line.
	*/
	zSPACE_API void drawLine(zVector &p0, zVector &p1, zColor col, double wt);

	/*! \brief This method displays the a face of zMesh.
	*
	*	\param		[in]	pos				- vector of type zVector storing the polygon points.
	*	\param		[in]	col				- color of the polygon.
	*
	*/
	zSPACE_API void drawPolygon(vector<zVector> &pos, zColor col);

	/*! \brief This method displays the zGraph.
	*
	*	\param		[in]	graph		- zGraph to be displayed.
	*	\param		[in]	showVerts	- boolean true if the vertices of zGraph are to be displayed.
	*	\param		[in]	showEdges	- boolean true if the edges of zGraph are to be displayed.
	*/
	zSPACE_API void drawGraph( zGraph &graph, bool showVerts = false, bool showEdges = true);


	/*! \brief This method displays the zMesh.
	*
	*	\param		[in]	mesh			- zMesh to be displayed.
	*	\param		[in]	showVerts		- boolean true if the vertices of zMesh are to be displayed.
	*	\param		[in]	showEdges		- boolean true if the edges of zMesh are to be displayed.
	*	\param		[in]	showFaces		- boolean true if the faces of zMesh are to be displayed.
	*
	*/
	zSPACE_API void drawMesh(zMesh &mesh, bool showVerts, bool showEdges, bool showFaces);

	zSPACE_API void drawMesh_DihedralEdges(zMesh &mesh, vector<double> & edge_dihedralAngles, double angleThreshold = 45);

	/*! \brief This method displays the scalar feild at the input buffer of the zScalarGraphs.
	*
	*	\param		[in]	scalargraph		- zScalarGraphs to be displayed.
	*	\param		[in]	buffer			- buffer of scalars in  zScalarGraphs to be displayed.
	*/
	zSPACE_API void drawScalarField(zScalarGraphs &scalargraph,int buffer =0);

	// display scalar field
	zSPACE_API void drawScalarField2D(zScalarField2D &scalarField2D, bool showPoints = true,  bool showDirections = false);


	/*! \brief This method displays the zRobot.
	*/
	zSPACE_API void drawRobot(zRobot &inRobot, double factor = 0.1, bool drawJointFrames = true, bool  drawJointMesh = false, bool drawWireFrame = false, bool drawGCodePoints = false, bool drawTargetFrame = false);


	/*! \brief This method displays zSlime.
	*/
	zSPACE_API void drawSlime(zSlime &slime, bool drawAgentPos = true, bool drawAgentDir = true, bool drawAgentTrail = true, bool foodSource = true, bool drawEnvironment = false, bool drawEnvironmentBoundary = true);

	// ----  BUFFER DISPLAYS

	zSPACE_API void drawPointsFromBuffer(zBufferObject &bufObj, bool vColors = true);

	zSPACE_API void drawLinesFromBuffer(zBufferObject &bufObj, bool vColors = true);

	zSPACE_API void drawTrianglesFromBuffer(zBufferObject &bufObj, bool vColors = true);

	zSPACE_API void drawQuadsFromBuffer(zBufferObject &bufObj, bool vColors = true);

	
}


