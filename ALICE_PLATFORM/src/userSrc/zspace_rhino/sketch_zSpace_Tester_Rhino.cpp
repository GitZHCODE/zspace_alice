
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



////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

//zModel model;

zObjMesh tempObj;
zFnMesh fntempMesh;

zObjMesh operateObj;
zFnMesh fnOperateMesh;

zObjGraph dualObj;
zFnGraph fnDual;

string path = "C:/Users/vishu.b/desktop/cube.obj";

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

	//model = zModel(100000);


	zRhinoFnMesh result_RhinoMesh(operateObj);
	result_RhinoMesh.from(path, zOBJ);

	ON_Mesh rMesh;
	result_RhinoMesh.toRhinoMesh(rMesh);

	ON_BoundingBox bb = rMesh.BoundingBox();

	//printf("\n rhino p: %1.2f % 1.2f %1.2f ", rMesh.m_V[0].x, rMesh.m_V[0].y, rMesh.m_V[0].z);
	printf("\n rhino bb_min: %1.2f % 1.2f %1.2f ", bb.Min().x, bb.Min().y, bb.Min().z);
	printf("\n rhino bb_max: %1.2f % 1.2f %1.2f ", bb.Max().x, bb.Max().y, bb.Max().z);

	
	zRhinoFnMesh temp_RhinoMesh(tempObj);

	temp_RhinoMesh.fromRhinoMesh(rMesh);
	temp_RhinoMesh.to("C:/Users/vishu.b/Desktop/rhinoTest.Obj", zOBJ);


	ON_ClassArray<ON_NurbsCurve> nc;

	ON_3dPointArray points0;
	points0.Append(ON_3dPoint(0, 0, 0));
	// points0.Append(ON_3dPoint(0.25, 0, 0));
	 //points0.Append(ON_3dPoint(0.5, 0, 0));
	// points0.Append(ON_3dPoint(0.75, 0, 0));
	points0.Append(ON_3dPoint(1, 0, 0));
	nc.AppendNew();
	nc[0].CreateClampedUniformNurbs(3, 2, points0.Count(), points0);

	ON_3dPointArray points1;
	points1.Append(ON_3dPoint(1, 0, 0));
	// points1.Append(ON_3dPoint(1, 0.25, 0));
	 //points1.Append(ON_3dPoint(1, 0.5, 0));
	// points1.Append(ON_3dPoint(1, 0.75, 0));
	points1.Append(ON_3dPoint(1, 1, 0));
	nc.AppendNew();
	nc[1].CreateClampedUniformNurbs(3, 2, points1.Count(), points1);

	ON_3dPointArray points2;
	points2.Append(ON_3dPoint(1, 1, 0));
	// points2.Append(ON_3dPoint(0.75, 1, 0));
	 //points2.Append(ON_3dPoint(0.5, 1, 0));
	 //points2.Append(ON_3dPoint(0.25, 1, 0));
	points2.Append(ON_3dPoint(0, 1, 0));
	nc.AppendNew();
	nc[2].CreateClampedUniformNurbs(3, 2, points2.Count(), points2);

	ON_3dPointArray points3;
	points3.Append(ON_3dPoint(0, 1, 0));
	//points3.Append(ON_3dPoint(0, 0.75, 0));
	//points3.Append(ON_3dPoint(0, 0.5, 0));
	//points3.Append(ON_3dPoint(0, 0.25, 0));
	points3.Append(ON_3dPoint(0, 0, 0));
	nc.AppendNew();
	nc[3].CreateClampedUniformNurbs(3, 2, points3.Count(), points3);

	const ON_Curve* c[4];
	for (int i = 0; i < 4; i++)
	{
		c[i] = &nc[i];
		double length = ON_UNSET_VALUE;
		if (c[i]->GetLength(&length))
		{
			// RhinoApp().Print(L"Curve with %f length created\n", length);
			std::cout << "curve with " << length << " length created" << std::endl;
		}
	}

	ON_Brep* brep = RhinoCreateEdgeSrf(4, c);
	if (nullptr != brep)
	{
		std::cout << "Brep with " << brep->m_F.Count() << " faces created" << std::endl;
		std::cout << "Surface with " << brep->BoundingBox().Area() << " area created" << std::endl;
		//RhinoApp().Print(L"Brep with %f area created\n", brep->BoundingBox().Area());

		//  ON_Surface* sur = brep->m_F[0].Offset(0.1, 0.001);


		 // std::cout << "Surface with " << sur->BoundingBox().Area() << " area created" << std::endl;
		  //RhinoApp().Print(L"Surface with %f area created\n", sur->BoundingBox().Area());

		  //delete sur;
		delete brep; // Don't leak...
	}



	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(tempObj);

	// set display element booleans
	//operateObj.setShowElements(true, true, true);
	

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

	
	//model.draw();

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
