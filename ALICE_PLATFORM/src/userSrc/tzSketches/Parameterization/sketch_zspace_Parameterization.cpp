//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/local_basis.h>
//#include <igl/readOFF.h>
#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/PI.h>

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
zFloatArray dScalars;

////// --- GUI OBJECTS ----------------------------------------------------

#pragma region IGL_NROSY
////// --- igl Objects --------------------------------------------------
// Mesh
Eigen::MatrixXd V;
Eigen::MatrixXi F;

// Constrained faces id
Eigen::VectorXi b;

// Cosntrained faces representative vector
Eigen::MatrixXd bc;

// Degree of the N-RoSy field
int N = 4;

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
	myMeshParam.setFromFile("data/Parameterization/heatTest.obj", zOBJ);
		
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
		//myMeshParam.computeParam_ARAP();

		// LSCM Parameterization
		//myMeshParam.computeParam_LSCM();

		// Geodesics heat method
		zIntArray vertices = { 18,28};
		myMeshParam.computeGeodesics_Heat(vertices, dScalars);

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
