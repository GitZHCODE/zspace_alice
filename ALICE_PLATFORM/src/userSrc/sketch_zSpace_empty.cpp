
#define _MAIN_


#ifdef _MAIN_

#include "main.h"

//////  zSpace Library
#include <zSpace/zSpaceImplement.h>
#include <zSpace/zDisplay.h>



using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

bool run = false;
bool updateColor = false;
int rCnt = 0;
////// --- GUI OBJECTS ----------------------------------------------------


char s[200],text[200], text1[200], jts[400];

double background = 0.9;

vec camPt;
////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------

//largeMesh LM;
int fileNum = 0;
string printfile;
////// ---------------------------------------------------- MODEL  ----------------------------------------------------

void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////


	


	
	//////////////////////////////////////////////////////////


	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	/////////////////////////////

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&updateColor, "updateColor");

	
	

	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{
	
	
	
		
	

}

////// ---------------------------------------------------- VIEW  ----------------------------------------------------

void draw()
{

	backGround(background);
	//drawGrid(20.0);

	glColor3f(1, 0, 0);
	

	S.draw();
	B.draw();
	
	
		


	
	

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

	if((GLUT_LEFT_BUTTON == b && GLUT_UP == state) || (GLUT_RIGHT_BUTTON == b && GLUT_UP == state))
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
