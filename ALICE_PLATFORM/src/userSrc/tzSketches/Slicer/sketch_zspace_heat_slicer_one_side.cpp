//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;

void computeBoundaryVertices(zObjMesh* o_mesh , zIntArray& vIDs, zIntArray &bIDs)
{
	bIDs.clear();

	if(vIDs.size()>0)
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

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars, int currentContourID, int totalContours, zObjGraphArray &o_contourGraphs)
{

	if (currentContourID >= o_contourGraphs.size())
	{
		cout << "Error: currentContourID greater than or eual to size of o_contourGraphs." << endl;
		return;
	}

	// weighted scalars
	float weight = ((float)(currentContourID + 1) / (float)totalContours);

	// Generate the isocontour using the threshold value
	zPointArray positions;
	zIntArray edgeConnects;
	zColorArray vColors;
	int pres = 3;
	zFnMesh fnMesh(*o_mesh);

	fnMesh.getIsoContour(scalars, weight, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

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

bool reload = false;
bool exportTo = false;

double background = 0.35;


int currentGraphId = 0;
double totalGraphs = 50;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjMesh o_sliceMesh;
zObjGraphArray o_contours;
zFloatArray geodesics_start;
zIntArray start_boundary_ids;

string inPath_obj = "data/Slicer/sliceMesh3.obj";
string inPath = "data/Slicer/sliceMesh3.json";
string outPath = "data/Slicer/outFolder";

////// --- GUI OBJECTS ----------------------------------------------------

void fromFile()
{
	//clear current set of things
	zFnMesh fnSliceMesh(o_sliceMesh);
	geodesics_start.clear();
	start_boundary_ids.clear();

	//read mesh
	fnSliceMesh.from(inPath_obj, zOBJ);
	myMeshParam = zTsMeshParam();
	myMeshParam.setFromFile(inPath_obj, zOBJ);

	//read properties
	json j;
	bool chk = core.json_read(inPath, j);

	if (chk)
	{
		const auto& j_startVIds = j["StartVIds"].get<vector<int>>();
		const auto& j_numGraphs = j["NumGraphs"].get<int>();

		//assign start vids
		for (auto& id : j_startVIds)
			start_boundary_ids.push_back(id);

		//assign total num graphs
		totalGraphs = j_numGraphs;
	}

	o_contours.clear();
	o_contours.assign(totalGraphs, zObjGraph());
}

void toFile()
{
	// Cleanup existing files in the output directory
	fs::remove_all(outPath);
	// Recreate the output directory
	fs::create_directories(outPath);

	int counter_folder = 0;
	int counter_file = 0;

	//export mesh
	zFnMesh fnMesh(*myMeshParam.getRawInMesh());
	fnMesh.to(outPath + "/outMesh.json", zJSON);

	//export graphs
	for (auto& contour : o_contours)
	{
		//make file
		string file = outPath + "/graph_" + to_string(counter_file) + ".json";
		zFnGraph fnGraph(contour);
		fnGraph.to(file, zJSON);
		counter_file++;
	}

	cout << endl;
	cout << "All files have been exported to: " + outPath << endl;
}


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	//fnSliceMesh.from("data/Slicer/sliceMesh3.obj", zOBJ);
	//myMeshParam.setFromFile("data/Slicer/sliceMesh3.obj", zOBJ);

	fromFile();
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

	S.addSlider(&totalGraphs, "totalGraphs");
	S.sliders[1].attachToVariable(&totalGraphs, 0, 100);

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
	
	B.addButton(&reload, "reload");
	B.buttons[5].attachToVariable(&reload);

	B.addButton(&exportTo, "exportTo");
	B.buttons[6].attachToVariable(&exportTo);
	

}

void update(int value)
{
	if (compute_heat)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();

		//zIntArray start_boundary_ids = { 24,42,72 };
		myMeshParam.computeGeodesics_Exact(start_boundary_ids, geodesics_start);

		cout << endl;
		cout << "boundary start:" << start_boundary_ids.size() << endl;

		zDomainFloat startMinMax(core.zMin(geodesics_start), core.zMax(geodesics_start));
		zDomainFloat outMinMax(0, 1);

		for (auto& v : geodesics_start)
		{
			v = core.ofMap(v, startMinMax, outMinMax);
		}

		cout << "start min:" << core.zMin(geodesics_start) << endl;
		cout << "start max:" << core.zMax(geodesics_start) << endl;
		compute_heat = !compute_heat;
	}

	if (compute_contour)
	{
		o_contours.clear();
		o_contours.assign(totalGraphs, zObjGraph());

		for (int i = 0; i < totalGraphs; i++)
		{
			computeContours(&o_sliceMesh, geodesics_start, i, totalGraphs, o_contours);
		}
		
		compute_contour = !compute_contour;
	}

	if (compute_color)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		colorMesh(o_inMesh, geodesics_start);

		compute_color_start = !compute_color_start;
		compute_color = !compute_color;
	}

	if (reload)
	{
		fromFile();
		reload = !reload;
	}

	if (exportTo)
	{
		toFile();
		exportTo = !exportTo;
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
