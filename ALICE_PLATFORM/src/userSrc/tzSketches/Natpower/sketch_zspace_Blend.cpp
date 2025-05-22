#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <igl/point_mesh_squared_distance.h>
#include <igl/lscm.h>
#include <igl/barycentric_coordinates.h>
//#include <igl/point_in_triangle.h>
#include <igl/boundary_loop.h>

#include <userSrc/tzSketches/Natpower/include/zTsSlicer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM
zUtilsCore core;

zTsSlicer slicer;

////////////////////////////////////////////////////////////////////////// General

bool b_slice = false;
bool b_section = false;
bool display = false;
bool exportTo = false;

double background = 0.75;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/


zObjMesh oMesh;
zObjGraph sectionGraph;
zObjGraph cableGraph;

int currentID = 0;
int totalGraphs = 0;
bool displayAllContours = false;

double sliceSpacing = 0.1;

string folder = "data/Natpower/out";

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

	// read mesh
	string path = "data/Natpower/sectionMesh.usda";
	zObjMesh temp;
	zFnMesh graphMesh(temp);
	graphMesh.from(path, zUSD);

	cout << "\n numVertices: " << graphMesh.numVertices();
	cout << "\n numEdges: " << graphMesh.numEdges();
	zPointArray pos;
	zIntArray pConnects;
	int eCount = 0;
	for (zItMeshEdge e(temp); !e.end(); e++)
	{
		zPointArray vertices;
		e.getVertexPositions(vertices);
		pos.insert(pos.end(), vertices.begin(), vertices.end());

		pConnects.push_back(eCount);
		pConnects.push_back(eCount + 1);
		
		eCount += 2;
	}

	zFnGraph fnSection(sectionGraph);
	fnSection.create(pos, pConnects);
	cout << "\n numVertices: " << fnSection.numVertices();
	cout << "\n numEdges: " << fnSection.numEdges();

	zFnGraph fnCable(cableGraph);
	zPointArray cablePos = { zPoint(-1.4,-2.6,0),zPoint(-1.39,-2.6,1.26) };
	zIntArray cableConnects = { 0,1 };
	fnCable.create(cablePos,cableConnects);

	//model.addObject(sectionGraph);
	int blockId = 0;
	cout << "\n block id?" << endl;
	cin >> blockId;
	json j;
	core.json_read("data/Natpower/blockMesh_" + to_string(blockId) + ".json", j);

	zFnMesh fnMesh(oMesh);
	fnMesh.from(j);

	zIntArray medialIDS;
	core.json_readAttribute(j, "MedialStartEnd", medialIDS);

	zIntArray featuredNumStrides;
	core.json_readAttribute(j, "FeaturedNumStrides", featuredNumStrides);

	//for (auto id : medialIDS) printf("\n %i ", id);

	int numBlendFrames;
	zVector norm(0, 0, 1);

	slicer.clear();

	zTsSlicerParams params;
	params.featuredNumStrides = featuredNumStrides;
	params.medialIds = medialIDS;
	params.spacing = sliceSpacing;
	params.norm = zVector(0, 0, 1);
	slicer.initialise(oMesh, params, &cableGraph);
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&sliceSpacing, "sliceSpacing");
	S.sliders[1].attachToVariable(&sliceSpacing, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&b_slice, "b_slice");
	B.buttons[0].attachToVariable(&b_slice);
	B.addButton(&b_section, "b_section");
	B.buttons[1].attachToVariable(&b_section);
	B.addButton(&display, "display");
	B.buttons[2].attachToVariable(&display);
	B.addButton(&exportTo, "exportTo");
	B.buttons[3].attachToVariable(&exportTo);
}

void update(int value)
{
	if (b_slice)
	{
		slicer.compute_slice();
		totalGraphs = slicer.numLayers;

		b_slice = !b_slice;
	}

	if (b_section)
	{
		slicer.compute_section(sectionGraph);

		b_section = !b_section;
	}

	if (exportTo)
	{
		slicer.exportTo(folder);
		exportTo = !exportTo;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();

		if (displayAllContours)
		{
			slicer.draw();

		}
		else
		{
			slicer.layers[currentID].drawSectionMesh();
			slicer.layers[currentID].drawSectionGraph();
			slicer.layers[currentID].drawUnrolledMesh();
			slicer.layers[currentID].drawUnrolledGraph();
		}
	}


	

	/*int counter = 0;
	for (auto& loop : vLoops)
	{
		string s = to_string(counter);
		model.displayUtils.drawTextAtPoint(s, loop[0].getVertex().getPosition());
		counter++;
	}*/

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	//if (k == 'p') compute = true;;	


	if (k == 'w')
	{
		if (currentID < totalGraphs - 1)currentID++;;
	}
	if (k == 's')
	{
		if (currentID > 0)currentID--;;
	}

	if (k == 'd') displayAllContours = !displayAllContours;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
