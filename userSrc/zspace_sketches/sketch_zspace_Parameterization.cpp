//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool d_inMesh = true;
bool d_paramMesh = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

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
	myMeshParam.setFromFile("data/Parameterization/Folded_Corner.obj", zOBJ);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans
	zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	o_paramMesh->setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[2].attachToVariable(&d_paramMesh);

}

void update(int value)
{
	if (compute)
	{
		//// Harmonic Parameterization
		//myMeshParam.computeParam_Harmonic();

		//// ARAP Parameterization
		myMeshParam.computeParam_ARAP();

		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();

		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();

		for (zItMeshEdge e(*o_paramMesh); !e.end(); e++)
		{
			int eID = e.getId();

			zItMeshEdge e_inMesh(*o_inMesh, eID);

			if (abs(e.getLength() - e_inMesh.getLength()) > 0.01)
			{
				e.setColor(zColor(1, 0, 0, 1));
			}
			else e.setColor(zColor(0, 1, 0, 1));
		}

		for (zItMeshVertex v(*o_inMesh); !v.end(); v++)
		{
			printf("\n %i | %1.4f ", v.getId(), v.getGaussianCurvature());
		}

		//zFnMesh fnParam(*o_paramMesh);
		//fnParam.to("data/Parameterization/paramMesh.obj", zOBJ);

		// LSCM Parameterization
		//myMeshParam.computeParam_LSCM();

		// N ROSY
		//myMeshParam.compute_NRosy();

		// Geodesics heat method
		/*zIntArray ids = { 0,5 };
		zFloatArray geodesics;
		myMeshParam.computeGeodesics_Heat(ids, geodesics);*/

		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (d_inMesh)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		o_inMesh->draw();
		
	}

	if (d_paramMesh)
	{
		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
		o_paramMesh->draw();

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
