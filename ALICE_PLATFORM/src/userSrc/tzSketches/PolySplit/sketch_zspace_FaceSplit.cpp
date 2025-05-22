//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

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
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/facesplit_1.obj", zOBJ);

	
	zPointArray fCens;
	fnMesh.getCenters(zHEData::zFaceData, fCens);
	oMesh.setFaceCenters(fCens);

	zPointArray eCens;
	fnMesh.getCenters(zHEData::zEdgeData, eCens);
	oMesh.setEdgeCenters(eCens);
	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(false, true, true);
	oMesh.setDisplayElementIds(true, true, true);

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
		int faceID;
		cout << "\n Please enter face ID to split: ";
		cin >> faceID;

		int edgeID_0;
		cout << "Please enter split edge ID 0: ";
		cin >> edgeID_0;

		float edgeFactor_0;
		cout << "Please enter split edge factor 0: ";
		cin >> edgeFactor_0;

		int edgeID_1;
		cout << "Please enter split edge ID 1: ";
		cin >> edgeID_1;

		float edgeFactor_1;
		cout << "Please enter split edge factor 1: ";
		cin >> edgeFactor_1;

		zFnMesh fnMesh(oMesh);
		fnMesh.splitFace(faceID, edgeID_0, edgeID_1, edgeFactor_0, edgeFactor_1);
		
		printf("\n working 1");
		// for diaplay
		zPointArray fCens;
		fnMesh.getCenters(zHEData::zFaceData, fCens);
		oMesh.setFaceCenters(fCens);

		printf("\n working 2");

		zPointArray eCens;
		fnMesh.getCenters(zHEData::zEdgeData, eCens);
		oMesh.setEdgeCenters(eCens);

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
		
	}

	if (display)
	{
		// zspace model draw
		model.draw();

		

	}


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
