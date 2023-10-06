//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include<headers/zToolsets/geometry/zTsGraphPolyhedra.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zObjMeshArray convexhulls;

zUtilsCore core;

/*!<Tool sets*/
zObjGraph graph;
zTsGraphPolyhedra polyhedra;

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
	string path = "data/staticGraph.json";
	//string path = "data/spatialGraph.json";
	polyhedra = zTsGraphPolyhedra(graph);

	polyhedra.setDisplayModel(model);
	polyhedra.createGraphFromFile(path, zJSON, false);
	polyhedra.create();

	polyhedra.setDisplayGraphElements(true, false, false);
	polyhedra.setDisplayHullElements(false);
	polyhedra.setDisplayPolyhedraElements(true,false,false,false,false);



	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//zObjGraph* spatialGraph = polyhedra.getRawSpatialGraph();
	//model.addObject(graph);

	// set display element booleans
	//graph.setDisplayElements(true, true);

	////////////////////////////////////////////////////////////////////////// Sliders
	
	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{

		//bool c = true;
		//polyhedra.equilibrium(c, 1, zEuler);

		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		polyhedra.draw();
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
