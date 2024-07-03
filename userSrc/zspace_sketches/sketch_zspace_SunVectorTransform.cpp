//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>



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

zUtilsCore core;

zObjMesh oMesh;
zObjMesh oMesh_transform;
zObjGraph oGraph;

zPointArray positions;

zTransform t;

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
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/solar_Sun.obj", zOBJ);
	
	
	zFnMesh fnMesh_t(oMesh_transform);
	fnMesh_t.from("data/solar_Sun.obj", zOBJ);

	zVectorArray fNorms;
	fnMesh_t.getFaceNormals(fNorms);
	
	zPointArray fCens;
	fnMesh_t.getCenters(zHEData::zFaceData, fCens);

	zVector O = fCens[0];
	zVector Z = fNorms[0];

	Z.normalize();
	zVector basis(0, 1, 0);
	zVector X = basis ^ Z;

	zVector Y = Z ^ X;
	Y.normalize();

	X = Y ^ Z;
	X.normalize();

	t.setIdentity();
	t(0, 0) = X.x; t(0, 1) = X.y; t(0, 2) = X.z;
	t(1, 0) = Y.x; t(1, 1) = Y.y; t(1, 2) = Y.z;
	t(2, 0) = Z.x; t(2, 1) = Z.y; t(2, 2) = Z.z;
	t(3, 0) = O.x; t(3, 1) = O.y; t(3, 2) = O.z;


	fnMesh_t.setTransform(t, false, false);

	zTransform local;
	local.setIdentity();	

	fnMesh_t.setTransform(local, false, true);
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_transform);
	
	// set display element booleans
	oMesh.setDisplayElements(true, true, true);	
	
	oMesh_transform.setDisplayElements(true, true, true);
	oMesh_transform.setDisplayTransform(true);
	
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
		zFnMesh fnMesh(oMesh);
		fnMesh.smoothMesh(1);
		fnMesh.to("data/outMesh.json", zJSON);;

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
		model.draw();

		model.displayUtils.drawTransform(t, 1);
		
	}

	for(auto &p : positions) model.displayUtils.drawPoint(p);

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

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
