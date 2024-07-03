//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <zToolsets/geometry/zTsGraphPolyhedra.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool readGraph = false;
bool equilibrium = false;
bool equilibriumREACHED = false;

bool display = true;
bool dFormGraph = true;
bool dForceMeshes = true;
bool displayAllMeshes = true;

double background = 0.35;

int currentMeshID = 0;
int totalMeshes = 0;

bool compTargets = true;
double formWeight = 1.0;
double areaScale = 1.0;
double dT = 0.1;
float minMax_formEdge = 0.1;
bool areaForce = false;
int numIterations = 100;
double angleTolerance = 5;
double areaTolerance = 0.1;
bool printInfo = true;

bool exportFiles = false;


////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zUtilsCore core;
zIntergrationType type = zRK4;

/*!<Tool sets*/
zTsGraphPolyhedra myPolyhedra;

////// --- CUSTOM METHODS -------------------------------------------------


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

	

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	zObjGraph* formGraph = myPolyhedra.getRawFormGraph();
	formGraph->setDisplayElements(true, true);

	int numMeshes = 0;
	zObjMeshPointerArray meshes = myPolyhedra.getRawForceMeshes(numMeshes);
	for (auto& m : meshes) m->setDisplayElements(false, true, true);
	

	////////////////////////////////////////////////////////////////////////// Sliders
	
	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&formWeight, "formWeight");
	S.sliders[1].attachToVariable(&formWeight, 0, 1);

	S.addSlider(&angleTolerance, "angleTolerance");
	S.sliders[2].attachToVariable(&angleTolerance, 0, 45);

	S.addSlider(&areaTolerance, "areaTolerance");
	S.sliders[3].attachToVariable(&areaTolerance, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&equilibrium, "equilibrium");
	B.buttons[0].attachToVariable(&equilibrium);

	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	B.addButton(&dFormGraph, "dFormGraph");
	B.buttons[2].attachToVariable(&dFormGraph);

	B.addButton(&dForceMeshes, "dForceMeshes");
	B.buttons[3].attachToVariable(&dForceMeshes);

	B.addButton(&displayAllMeshes, "displayAllMeshes");
	B.buttons[4].attachToVariable(&displayAllMeshes);

}

void update(int value)
{
	if (readGraph)
	{
		//string path = "data/graphPolyhedras/inGraph.json";
		//string path = "data/graphPolyhedras/nodes_four.json";
		//myPolyhedra.setFormGraphFromFile(path, zJSON, false);

		myPolyhedra.setFormGraphFromOffsetMeshes("data/graphPolyhedras/topMesh.obj", "data/graphPolyhedras/bottomMesh.obj", zOBJ);

		//myPolyhedra.createForceMeshes();

		readGraph = !readGraph;
	}

	if (equilibrium)
	{
		equilibriumREACHED = myPolyhedra.equilibrium(compTargets, formWeight, areaScale, dT, type, 0.1, areaForce, numIterations, angleTolerance, areaTolerance, true);

		zDomainDouble* angleDeviations = myPolyhedra.getRawAngleDeviation();
		if (angleDeviations->max < 30) areaForce = true;
		//else if (angleDeviations->max > angleTolerance + 8) areaForce = false;
		if (equilibriumREACHED) printf("\n Equilibrium Reached !");

		equilibrium = !equilibrium;	
	}

	if (exportFiles)
	{
		myPolyhedra.exportFiles("data/graphPolyhedras");	

		exportFiles = !exportFiles;
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
	}

	if (dFormGraph)
	{
		zObjGraph* formGraph = myPolyhedra.getRawFormGraph();
		formGraph->draw();
	}
	

	if (dForceMeshes)
	{
		int numMeshes = 0;
		zObjMeshPointerArray meshes = myPolyhedra.getRawForceMeshes(numMeshes);

		totalMeshes = numMeshes;

		if (displayAllMeshes)
		{
			for (auto& m : meshes)
			{
				m->draw();
			}
		}
		else
		{
			int i = currentMeshID;
			
			if (numMeshes > 0 && i >= 0 & i < numMeshes)
			{
				meshes[i]->draw();			

			}

		}
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Nodes #:" + to_string(totalMeshes), vec(winW - 350, winH - 800, 0));
	drawString("Current Node #:" + to_string(currentMeshID), vec(winW - 350, winH - 775, 0));

	string equilibriumText = (equilibriumREACHED) ? "true" : "False";
	drawString("equilibrium REACHED:" + equilibriumText, vec(winW - 350, winH - 750, 0));

	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("r - read graph", vec(winW - 350, winH - 600, 0));
	drawString("p - compute equilibrium", vec(winW - 350, winH - 575, 0));
	drawString("c - compute targets", vec(winW - 350, winH - 550, 0));
	drawString("e - export files", vec(winW - 350, winH - 525, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'r')
	{
		readGraph = true;
	}

	if (k == 'p')
	{
		compTargets = true;
		equilibrium = true;;
	}

	if (k == 'c')
	{
		compTargets = true;
	}

	if (k == 'e')
	{
		exportFiles = true;
	}


	if (k == 'w')
	{
		if (currentMeshID < totalMeshes - 1)currentMeshID++;;
	}
	if (k == 's')
	{
		if (currentMeshID > 0)currentMeshID--;;
	}

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
