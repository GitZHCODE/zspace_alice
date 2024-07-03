//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;

void computeBoundaryVertices(zObjMesh* o_mesh , zIntArray& vIDs, zIntArray &bIDs)
{
	bIDs.clear();

	for (int j = 0; j < vIDs.size(); j++)
	{
		zItMeshVertex v(*o_mesh, vIDs[j]);
		zItMeshHalfEdgeArray cHEdges;
		v.getConnectedHalfEdges(cHEdges);

		zItMeshHalfEdge startHe, He;
		for (auto& cHE : cHEdges)
		{
			if (cHE.onBoundary())startHe = cHE;
		}

		He = startHe;

		do
		{
			bIDs.push_back(He.getStartVertex().getId());
			He = He.getNext();

		} while (He != startHe);
	}
	

}

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars_start, zFloatArray& scalars_end, int currentContourID, int totalContours, zObjGraphArray &o_contourGraphs)
{

	if (currentContourID >= o_contourGraphs.size())
	{
		cout << "Error: currentContourID greater than or eual to size of o_contourGraphs." << endl;
		return;
	}

	zFloatArray scalars;
	scalars.assign(scalars_start.size(), -1);

	// weighted scalars
	float weight = ((float)(currentContourID + 1) / (float)totalContours);
	
	for (int j = 0; j < scalars.size(); j++)
	{
		scalars[j] = weight * scalars_start[j] - (1 - weight) * scalars_end[j];
	}

	// Generate the isocontour using the threshold value
	zPointArray positions;
	zIntArray edgeConnects;
	zColorArray vColors;
	int pres = 3;
	zFnMesh fnMesh(*o_mesh);
	fnMesh.getIsoContour(scalars, 0.0, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

	// Create graph from the isocontour
	zFnGraph tempFn(o_contourGraphs[currentContourID]);
	tempFn.create(positions, edgeConnects);
	tempFn.setEdgeColor(zColor(255, 255, 255, 1));
	tempFn.setEdgeWeight(2);
	tempFn.setVertexColors(vColors, false);

	// color mesh
	zColor *mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}

void colorMesh(zObjMesh* o_mesh, zFloatArray& scalars)
{
	zFnMesh fnMesh(*o_mesh);

	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}
////////////////////////////////////////////////////////////////////////// General

bool compute_heat = false;
bool compute_contour = false;
bool compute_color = false;
bool compute_color_start = true;

bool d_inMesh = true;
bool d_paramMesh = true;
bool d_sliceMesh = true;
bool d_ContourGraphs = true;
bool d_AllGraphs = false;

double background = 0.35;


int currentGraphId = 0;
int totalGraphs = 60;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjMesh o_sliceMesh;
zObjGraphArray o_contours;
zFloatArray geodesics_start;
zFloatArray geodesics_end;

////// --- GUI OBJECTS ----------------------------------------------------


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	zFnMesh fnSliceMesh(o_sliceMesh);
	fnSliceMesh.from("data/Parameterization/heatTest_ring_triangulated.obj", zOBJ);

	// read mesh
	myMeshParam.setFromFile("data/Parameterization/heatTest_ring_triangulated.obj", zOBJ);

	// set contours
	o_contours.clear();
	o_contours.assign(totalGraphs, zObjGraph());
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans

	o_sliceMesh.setDisplayElements(false, true, false);

	zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
	o_inMesh->setDisplayElements(false, false, true);

	zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	o_paramMesh->setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_heat, "compute_heat");
	B.buttons[0].attachToVariable(&compute_heat);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[2].attachToVariable(&d_paramMesh);

	B.addButton(&d_sliceMesh, "d_sliceMesh");
	B.buttons[3].attachToVariable(&d_sliceMesh);

	B.addButton(&d_ContourGraphs, "d_ContourGraphs");
	B.buttons[4].attachToVariable(&d_ContourGraphs);
	
	

}

void update(int value)
{
	if (compute_heat)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();

		// Geodesics heat method
		zIntArray start_ids = { 8 };
		zIntArray start_boundary_ids;
		computeBoundaryVertices(o_inMesh, start_ids, start_boundary_ids);
		myMeshParam.computeGeodesics_Heat(start_boundary_ids, geodesics_start);

		zIntArray end_ids = { 4, 11, 14 };
		zIntArray end_boundary_ids;
		computeBoundaryVertices(o_inMesh, end_ids, end_boundary_ids);
		myMeshParam.computeGeodesics_Heat(end_boundary_ids, geodesics_end);

		printf("\n boundary start %i | end %i ", start_boundary_ids.size(), end_boundary_ids.size());

		compute_heat = !compute_heat;
	}

	if (compute_contour)
	{
		//zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		for (int i = 0; i < totalGraphs; i++)
		{
			computeContours(&o_sliceMesh, geodesics_end, geodesics_start, i, totalGraphs, o_contours);
		}
		
		compute_contour = !compute_contour;
	}

	if (compute_color)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		colorMesh(o_inMesh, (compute_color_start) ? geodesics_start: geodesics_end);

		compute_color_start = !compute_color_start;
		compute_color = !compute_color;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (d_inMesh)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		//o_inMesh->setDisplayElements(true, false, false);
		o_inMesh->draw();
		
	}

	if (d_sliceMesh)
	{
		o_sliceMesh.draw();
	}

	if (d_paramMesh)
	{
		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
		//o_paramMesh->setDisplayElements(true, false, false);
		o_paramMesh->draw();

	}
	
	if (d_ContourGraphs)
	{
		if (d_AllGraphs)
		{
			for (auto& g : o_contours)
			{
				g.draw();
			}
		}
		else
		{
			int i = currentGraphId;			
			if (totalGraphs > 0 && i >= 0 && i < totalGraphs)
			{
				o_contours[i].draw();
			}

		}
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Layers #:" + to_string(totalGraphs), vec(winW - 350, winH - 800, 0));
	drawString("Current Layer #:" + to_string(currentGraphId), vec(winW - 350, winH - 775, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("p - compute geodesics", vec(winW - 350, winH - 625, 0));
	drawString("o - compute contours", vec(winW - 350, winH - 600, 0));
	drawString("c - compute colors of flip fields", vec(winW - 350, winH - 575, 0));
	

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute_heat = true;;
	if (k == 'o') compute_contour = true;;
	if (k == 'c') compute_color = true;

	if (k == 'w')
	{
		if (currentGraphId < totalGraphs - 1)currentGraphId++;;
	}
	if (k == 's')
	{
		if (currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') d_AllGraphs = !d_AllGraphs;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
