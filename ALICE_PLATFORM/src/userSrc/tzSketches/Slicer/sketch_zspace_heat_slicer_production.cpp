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


struct SliceBlock
{
	int id;
	float minScalar;
	float maxScalar;
	float controlHeight;
	int totalGraphs;

	zTsMeshParam sliceMesh;
	zObjGraphArray o_contours;
	zIntArray start_boundary_ids;
	zIntArray end_boundary_ids;

	//default constructor
	SliceBlock()
		:id(0), minScalar(FLT_MAX), maxScalar(FLT_MIN), totalGraphs(0), controlHeight(0),
		sliceMesh(zTsMeshParam()),o_contours(zObjGraphArray()),
		start_boundary_ids(zIntArray()), end_boundary_ids(zIntArray()){}

};


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
bool d_ContourGraphs = false;
bool d_AllGraphs = false;
bool toFile = false;

bool compute_block = false;
bool compute_block_all = false;

double background = 0.35;

int block_id = 0;
double block_id_slider = 0;
int graph_id = 0;
int numBlocks = 0;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

//vector<SliceBlock> blocks;
SliceBlock block;

zFloatArray geodesics_start;
zFloatArray geodesics_end;

//string inPath = "data/Slicer/sliceMesh.json";
string inPath = "data/Slicer/in/";
string outPath = "data/Slicer/out/";

zStringArray files_obj, files_json;


////// --- GUI OBJECTS ----------------------------------------------------
void computeBlock()
{
	block = SliceBlock();
	//block = blocks[block_id];
	cout << endl << "blockId:" << block_id << endl;

	json j;
	bool chk;
	block.sliceMesh.setFromFile(files_obj[block_id], zOBJ);
	chk = core.readJSON(files_json[block_id], j);

	if (chk)
	{
		block.id = j["BlockId"].get<int>();
		block.totalGraphs = j["NumLayers"].get<int>();
		block.controlHeight = j["MaxPrintHeight"].get<float>();
		block.start_boundary_ids = j["BoundariesStart"].get<zIntArray>();
		block.end_boundary_ids = j["BoundariesEnd"].get<zIntArray>();
	}
}

void computeHeat()
{
	geodesics_start.clear();
	geodesics_end.clear();

	// Geodesics heat method
	block.sliceMesh.computeGeodesics_Heat(block.start_boundary_ids, geodesics_start);
	block.sliceMesh.computeGeodesics_Heat(block.end_boundary_ids, geodesics_end);

	float min = FLT_MAX;
	float max = FLT_MIN;
	for (auto& val : geodesics_start)
	{
		min = (val < min) ? val : min;
		max = (val > max) ? val : max;
	}

	for (auto& val : geodesics_end)
	{
		min = (val < min) ? val : min;
		max = (val > max) ? val : max;
	}

	block.minScalar = min;
	block.maxScalar = max;
	block.totalGraphs = (int)ceil(max / block.controlHeight);

	printf("\n boundary start %i | end %i ", block.start_boundary_ids.size(), block.end_boundary_ids.size());

}

void computeContour()
{
	zObjMesh* o_sliceMesh = block.sliceMesh.getRawInMesh();

	// set contours
	block.o_contours.clear();
	block.o_contours.assign(block.totalGraphs, zObjGraph());

	for (int i = 0; i < block.totalGraphs; i++)
	{
		computeContours(o_sliceMesh, geodesics_end, geodesics_start, i, block.totalGraphs, block.o_contours);
	}

	d_ContourGraphs = true;
}

void computeColor()
{
	zObjMesh* o_inMesh = block.sliceMesh.getRawInMesh();
	colorMesh(o_inMesh, (compute_color_start) ? geodesics_start : geodesics_end);

	compute_color_start = !compute_color_start;
}

void exportToFile()
{
	// Create the output directory if it doesn't exist
	string folder = outPath + "block" + "_" + to_string(block.id);
	if (!fs::exists(folder))
		fs::create_directory(folder);

	folder += "/";

	for (int i = 0; i < block.o_contours.size(); i++)
	{
		zFnGraph fnGraph(block.o_contours[i]);

		// Generate the output path
		string filePath = folder + "graph_" + to_string(i) + ".json";

		// Export the fnGraph to the JSON file
		fnGraph.to(filePath, zJSON);
	}

	cout << endl;
	cout << "minScalar:" << block.minScalar << endl;
	cout << "maxScalar:" << block.maxScalar << endl;
	cout << "targetLayerHeight:" << block.controlHeight << endl;
	cout << "totalGraphs:" << block.totalGraphs << endl;

	cout << "Exported to: " << folder << endl;
	cout << "-------------------" << endl;
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

	core.getFilesFromDirectory(files_obj, inPath, zOBJ);
	core.getFilesFromDirectory(files_json, inPath, zJSON);
	numBlocks = core.getNumfiles_Type(inPath,zJSON);

	cout << endl << "NumBlocks:" << numBlocks << endl;
	cout << "---------------------" << endl;

	// read mesh
	//blocks.assign(numBlocks, SliceBlock());


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans

	zObjMesh* o_inMesh = block.sliceMesh.getRawInMesh();
	o_inMesh->setDisplayElements(false, false, true);

	zObjMesh* o_paramMesh = block.sliceMesh.getRawParamMesh();
	o_paramMesh->setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&block_id_slider, "block_id");
	S.sliders[1].attachToVariable(&block_id_slider, 0, numBlocks);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_block, "compute_block");
	B.buttons[0].attachToVariable(&compute_block);

	B.addButton(&compute_block_all, "compute_block_all");
	B.buttons[1].attachToVariable(&compute_block_all);

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
	if (compute_block)
	{
		block_id = (int)floor(block_id_slider);
		computeBlock();
		compute_block = !compute_block;
	}

	if (compute_block_all)
	{
		block = SliceBlock();
		for (int i = 0; i < numBlocks; i++)
		{
			block_id = i;
			computeBlock();
			computeHeat();
			computeContour();
			computeColor();
			exportToFile();
		}
		compute_block_all = !compute_block_all;
	}

	if (compute_heat)
	{
		computeHeat();
		compute_heat = !compute_heat;
	}

	if (compute_contour)
	{
		computeContour();
		compute_contour = !compute_contour;
	}

	if (compute_color)
	{
		computeColor();
		compute_color = !compute_color;
	}

	if (toFile)
	{
		exportToFile();
		toFile = !toFile;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (d_sliceMesh)
	{
		zObjMesh* sliceMesh = block.sliceMesh.getRawInMesh();
		sliceMesh->draw();
	}

	if (d_paramMesh)
	{
		zObjMesh* o_paramMesh = block.sliceMesh.getRawParamMesh();
		o_paramMesh->setDisplayElements(false, true, false);
		o_paramMesh->draw();

	}

	if (d_ContourGraphs)
	{
		if (d_AllGraphs)
		{
			for (auto& g : block.o_contours)
			{
				g.draw();
			}
		}
		else
		{
			int i = graph_id;
			if (block.totalGraphs > 0 && i >= 0 && i < block.totalGraphs)
			{
				block.o_contours[i].draw();
			}

		}
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Current Block #:" + to_string(block.id), vec(winW - 350, winH - 850, 0));
	drawString("Current Layer #:" + to_string(graph_id), vec(winW - 350, winH - 825, 0));
	drawString("Total Layers #:" + to_string(block.totalGraphs), vec(winW - 350, winH - 800, 0));

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
		if (graph_id < block.totalGraphs - 1)graph_id++;;
	}
	if (k == 's')
	{
		if (graph_id > 0)graph_id--;;
	}

	if (k == 'd') d_AllGraphs = !d_AllGraphs;

	if (k == 'e') toFile = !toFile;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
