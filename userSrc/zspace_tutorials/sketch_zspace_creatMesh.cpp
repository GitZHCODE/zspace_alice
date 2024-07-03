//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM Method

// returntype  functionName(var varname, var varname_1){}
void createMyMesh(zObjMesh &oMesh)
{
	zFnMesh fnMesh(oMesh);

	zPointArray vPositions;

	int v0 = vPositions.size();
	vPositions.push_back(zPoint(-1, -1, 0));	// v0

	int v1 = vPositions.size();;
	vPositions.push_back(zPoint(1, -1, 0));		// v1

	int v2 = vPositions.size();;
	vPositions.push_back(zPoint(1, 1, 0));		// v2

	int v3 = vPositions.size();;
	vPositions.push_back(zPoint(-1, 1, 0));		// v3

	int v4 = vPositions.size();;
	vPositions.push_back(zPoint(-2, -1, 0));	// v4

	int v5 = vPositions.size();;
	vPositions.push_back(zPoint(-2, 1, 0));		// v5

	zIntArray pCounts, pConnects;

	pCounts.push_back(4); // f0
	pCounts.push_back(4); // f1

	pConnects.push_back(v0);
	pConnects.push_back(v1);
	pConnects.push_back(v2);
	pConnects.push_back(v3);


	pConnects.push_back(v0);
	pConnects.push_back(v3);
	pConnects.push_back(v5);
	pConnects.push_back(v4);

	fnMesh.create(vPositions, pCounts, pConnects);
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zUtilsCore core;
zObjMesh oMesh;


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
	zFnMesh fnMesh_in(oMesh);

	createMyMesh(oMesh);

	// creating duplicate
	zObjMesh oDuplicateMesh;
	fnMesh_in.getDuplicate(oDuplicateMesh);	

	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(false, true, true);

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
		

		compute = !compute;	
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
		//model.draw();
		
	}

	 // do for loop through all vertex positions
	// draw line between  pi & pv1

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

	
}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;		
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
