
#define _MAIN_
#define _HAS_STD_BYTE 0

#ifdef _MAIN_

#include "main.h"

//////  zSpace Library
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>
#include <headers/zApp/include/zViewer.h>

//#include <headers/api/functionsets/zFnMesh.h>
//#include <headers/api/functionsets/zFnGraph.h>



using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS ----------------------------------------------------
//void drawTextAtVec(string s, zVector &pt)
//{
//	unsigned int i;
//	glRasterPos3f(pt.x, pt.y, pt.z);
//
//	for (i = 0; i < s.length(); i++)
//		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
//}

//bool ON_NurbsCurve_GetLength(
//	const ON_NurbsCurve& curve,
//	double* length,
//	double fractional_tolerance = 1.0e-8,
//	const ON_Interval* sub_domain = NULL
//)
//{
//	return curve.GetLength(length, fractional_tolerance, sub_domain);
//}

////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

/*!<model*/
zModel model;

zObjMesh tempObj;
zFnMesh fntempMesh;

zObjMesh operateObj;
zFnMesh fnOperateMesh;

zObjGraph dualObj;
zFnGraph fnDual;

zObjNurbsCurve o_curve;

string path = "C:/Users/vishu.b/desktop/cube.obj";

string file = "data/nurbsCurve.json";

//string path = "C:/Users/vishu.b/desktop/zSpace_graph_fromPentahedron_100x200.txt";

bool c_Dual = false;

bool d_dualgraph = true;
bool d_inputmesh = true;


int currentId = 0;
int rCnt = 0;
////// --- GUI OBJECTS ----------------------------------------------------


char s[200], text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------




////// ---------------------------------------------------- MODEL  ----------------------------------------------------





void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////

	// initialise model
	model = zModel(100000);
	
	
	zFnNurbsCurve fnNurbs(o_curve);
	fnNurbs.from(file, zJSON);

	double len = fnNurbs.getLength();
	printf("\n cvs %i | length %1.2f ", fnNurbs.numControlVertices(), fnNurbs.getLength());

	fnNurbs.setDisplayColor(zColor(1, 0, 0, 1));
	fnNurbs.setDisplayWeight(4);

	fnNurbs.to("data/out_nurbsCurve.json", zJSON);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_curve);

	// set display element booleans
	o_curve.setDisplayElements(true, true);
	

	//////////////////////////////////////////////////////////

	S = *new SliderGroup();
	S.addSlider(&background, "background");

	//S.addSlider(&formWeight, "formWeight");
	//S.sliders[1].attachToVariable(&formWeight, 0.00, 1.00);

	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));



	B.addButton(&d_inputmesh, "d_Input");

	B.addButton(&d_dualgraph, "d_Dual");

	B.addButton(&c_Dual, "c_Dual");

	


	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{



}


////// ---------------------------------------------------- VIEW  ----------------------------------------------------

void draw()
{

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	backGround(background);
	//drawGrid(20.0);

	glColor3f(1, 0, 0);


	S.draw();
	B.draw();
	// ------------------------ draw the path points / Tool orientations 
	
	model.draw();	

	//////////////////////////////////////////////////////////



	glColor3f(0, 0, 0);
	setup2d();

	AL_drawString(s, winW * 0.5, winH - 50);
	AL_drawString(text, winW * 0.5, winH - 75);
	AL_drawString(jts, winW * 0.5, winH - 100);
	AL_drawString(printfile.c_str(), winW * 0.5, winH - 125);



	int hts = 25;
	int wid = winW * 0.75;




	restore3d();
	//drawVector(camPt, vec(wid, hts + 25, 0), "cam");

}

////// ---------------------------------------------------- CONTROLLER  ----------------------------------------------------

void keyPress(unsigned char k, int xm, int ym)
{

	///// GRAPH GENERTOR PROGRAM 
	if (k == 'i')setCamera(15, -40, 60, -2, 4);



}

void mousePress(int b, int state, int x, int y)
{

	if (GLUT_LEFT_BUTTON == b && GLUT_DOWN == state)
	{

		B.performSelection(x, y);

		S.performSelection(x, y, HUDSelectOn);

	}

	if ((GLUT_LEFT_BUTTON == b && GLUT_UP == state) || (GLUT_RIGHT_BUTTON == b && GLUT_UP == state))
	{

	}
}

void mouseMotion(int x, int y)
{
	S.performSelection(x, y, HUDSelectOn);


	bool dragging = (glutGetModifiers() == GLUT_ACTIVE_ALT) ? true : false;
	int cur_msx = winW * 0.5;
	int cur_msy = winH * 0.5;
	camPt = screenToCamera(cur_msx, cur_msy, 0.2);

	//if( dragging)GS.LM.updateColorArray(lightscale, flipNormals, camPt);

}




#endif // _MAIN_
