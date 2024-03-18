//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>
#include <unordered_set>
#include <unordered_map>

using namespace zSpace;
using namespace std;

unordered_map<int, vector<int>> boundaries;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;

void computeBoundary(zObjMesh* o_mesh, unordered_map<int, vector<int>>& boundaries)
{

	/* find all boundary vertices */
	zIntArray boundaryVertices;

	// Loop through all vertices and add boundary vertices to the boundaryVertices array
	zItMeshVertex v(*o_mesh);
	for (v.begin(); !v.end(); v++) {
		if (v.onBoundary()) boundaryVertices.push_back(v.getId());
	}


	unordered_set<int> visited;
	int numBoundaries = 0;

	// Loop until all boundary vertices have been visited
	while (visited.size() < boundaryVertices.size())
	{
		// Find an unvisited boundary vertex to start the boundary loop
		auto startVertex = find_if(boundaryVertices.begin(), boundaryVertices.end(), [&](int id) {
			return visited.count(id) == 0;
			});

		// If there are no unvisited boundary vertices, exit the loop
		if (startVertex == boundaryVertices.end())
			break;

		// Create a vector to store the current boundary
		vector<int> currentBoundary;

		// Find a half-edge on the boundary starting at the current vertex
		zItMeshVertex v(*o_mesh, *startVertex);
		zItMeshHalfEdge start(*o_mesh), next(*o_mesh);

		zIntArray connectedHalfEdges;
		v.getConnectedHalfEdges(connectedHalfEdges);

		auto he = find_if(connectedHalfEdges.begin(), connectedHalfEdges.end(), [&](int id) {
			return zItMeshHalfEdge(*o_mesh, id).onBoundary();
			});
		start = zItMeshHalfEdge(*o_mesh, *he);

		// Mark the starting vertex as visited and add it to the current boundary
		visited.insert(start.getVertex().getId());
		currentBoundary.push_back(start.getVertex().getId());
		next = start.getNext();

		// Loop through all the half-edges on the boundary and add their vertices to the current boundary
		while (next != start) {
			visited.insert(next.getVertex().getId());
			currentBoundary.push_back(next.getVertex().getId());
			next = next.getNext();
		}

		// Add the current boundary to the boundaries map and increment the number of boundaries
		boundaries[numBoundaries++] = currentBoundary;
	}
}

void computeBoundaryVertices(zObjMesh* o_mesh, zIntArray& vIDs, zIntArray& bIDs)
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

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars_start, zFloatArray& scalars_end, int currentContourID, int totalContours, zObjGraphArray& o_contourGraphs)
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
bool toFile = false;

double background = 0.35;


int currentGraphId = 0;
int totalGraphs = 0;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjMesh o_sliceMesh;
zObjGraphArray o_contours;
zFloatArray geodesics_start;
zFloatArray geodesics_end;

//string inPath = "data/Slicer/sliceMesh.json";
string path = "data/Slicer/";
zIntArray start_boundary_ids;
zIntArray end_boundary_ids;

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
	//fnSliceMesh.from("data/Parameterization/heatTest_ring.obj", zOBJ);
	//fnSliceMesh.from("data/Slicer/minSrf.json", zJSON);
	//fnSliceMesh.from("data/Slicer/sliceMesh_tri.obj", zOBJ);
	fnSliceMesh.from(path + "sliceMesh.obj", zOBJ);

	// read mesh
	//myMeshParam.setFromFile("data/Parameterization/heatTest_ring_triangulated.obj", zOBJ);
	//myMeshParam.setFromFile("data/Slicer/minSrf.json", zJSON);
	//myMeshParam.setFromFile("data/Slicer/sliceMesh.obj", zOBJ);
	//myMeshParam.setFromFile("data/Slicer/sliceMesh_tri.obj", zOBJ);
	myMeshParam.setFromFile(path + "sliceMesh.obj", zOBJ);

	json j;
	bool chk = core.readJSON(path + "sliceMesh.json", j);
	totalGraphs = j["NumLayers"].get<int>();
	totalGraphs = 100;
	start_boundary_ids = j["BoundariesStart"].get<zIntArray>();
	end_boundary_ids = j["BoundariesEnd"].get<zIntArray>();


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

	B.addButton(&toFile, "export");
	B.buttons[5].attachToVariable(&toFile);



}

void update(int value)
{
	if (compute_heat)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		//computeBoundary(o_inMesh, boundaries);

		// Geodesics heat method
		//zIntArray start_ids = { 150 };

		//start_boundary_ids = boundaries[0];
		//computeBoundaryVertices(o_inMesh, start_ids, start_boundary_ids);

		//json j;
		//bool chk = core.readJSON(inPath, j);
		//totalGraphs = j["NumLayers"].get<int>();

		//zIntArray start_boundary_ids;
		//start_boundary_ids = j["BoundariesStart"].get<zIntArray>();
		myMeshParam.computeGeodesics_Heat(start_boundary_ids, geodesics_start);

		//zIntArray end_ids = { 28702 };
		//zIntArray end_boundary_ids;

		//end_boundary_ids = boundaries[1];

		//computeBoundaryVertices(o_inMesh, end_ids, end_boundary_ids);
		//zIntArray end_boundary_ids;
		//end_boundary_ids = j["BoundariesEnd"].get<zIntArray>();
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
		colorMesh(o_inMesh, (compute_color_start) ? geodesics_start : geodesics_end);

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

	if (toFile)
	{
		for (int i = 0; i < o_contours.size(); i++)
		{
			zFnGraph fnGraph(o_contours[i]);
			string path = "data/Slicer/out/";
			path += "graph_";
			path += to_string(i);
			path += ".json";

			fnGraph.to(path, zJSON);
		}
		toFile = !toFile;
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
	drawString("d - display all contour graphs", vec(winW - 350, winH - 550, 0));
	drawString("e - export graphs", vec(winW - 350, winH - 525, 0));


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
