//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////// --- CUSTOM METHODS ----------------------------------------------------

zUtilsCore core;

void checkPlanarity(zObjMesh& oMesh, double tolerance = EPS)
{
	zFnMesh fnMesh(oMesh);

	vector<zIntArray> fTris;
	zPointArray fCenters;
	zDoubleArray fVolumes;
	zVectorArray fNormalTargets;
	zVectorArray fNormals;

	fnMesh.getMeshTriangles(fTris);

	fnMesh.getMeshFaceVolumes(fTris, fCenters, fVolumes, true);

	for (zItMeshFace f(oMesh); !f.end(); f++)
	{
		int i = f.getId();

		//if (abs(fVolumes[i]) < tolerance) f.setColor(zColor(0, 0.95, 0.270, 1));
		//else f.setColor(zColor(0.95, 0, 0.27, 1));

		double uA, uB;
		zPoint pA, pB;

		zPointArray fVerts;
		f.getVertexPositions(fVerts);

		core.line_lineClosestPoints(fVerts[0], fVerts[2], fVerts[1], fVerts[3], uA, uB, pA, pB);

		float dist = pA.distanceTo(pB);
		if (dist < tolerance) f.setColor(zColor(0, 0.95, 0.270, 1));
		else f.setColor(zColor(0.95, 0, 0.27, 1));
	}
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool exportMesh = false;

double background = 0.35;
double tolerance = 0.001;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/


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
	fnMesh.from("data/PlanarityCheck/inMesh.obj", zOBJ);

	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(false, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	
	S.addSlider(&tolerance, "tolerance");
	S.sliders[1].attachToVariable(&tolerance, 0.0001, 0.05);

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
		checkPlanarity(oMesh, tolerance);
		compute = !compute;	
	}

	if (exportMesh)
	{

		zFnMesh fnMesh(oMesh);
		fnMesh.to("data/PlanarityCheck/outMesh.json", zJSON);

		exportMesh = !exportMesh;
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

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;	

	if (k == 'e') exportMesh = true;;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
