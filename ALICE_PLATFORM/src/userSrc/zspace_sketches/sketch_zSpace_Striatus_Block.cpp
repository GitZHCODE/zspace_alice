//#define _MAIN_


#ifdef _MAIN_

#include "main.h"

//////  zSpace Library
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS ----------------------------------------------------
void drawTextAtVec(string s, zVector &pt)
{
	unsigned int i;
	glRasterPos3f(pt.x, pt.y, pt.z);

	for (i = 0; i < s.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
}


////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh o_inputMesh;
zObjMesh o_guideMesh;
zObjMesh o_guideThickMesh;
zObjMesh o_guideSmoothThickMesh;



bool exportMesh = false;
bool importMesh = false;

bool d_inputMesh = false;
bool d_guideMesh = true;
bool d_guideThickMesh = false;

/*!<Function sets*/

/*!<Tool sets*/
zTsSDFBridge mySDF;

int voxelId = 1;

int presFac = 1;
int nF = 0;

string filePath = "data/striatus/testMesh.json";

string exportBRGPath = "data/striatus/out_Block_0.json";
string importBRGPath = "data/striatus/tozha.json";


////// --- GUI OBJECTS ----------------------------------------------------


char s[200],text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------

void exportBlockJSON(string path, zObjMesh &oMesh)
{
	zFnMesh fnMesh(oMesh);	
	fnMesh.to(path, zJSON);
	
	// read existing data in the json 
	json j;

	ifstream in_myfile;
	in_myfile.open(path.c_str());

	int lineCnt = 0;

	if (in_myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
	}

	in_myfile >> j;
	in_myfile.close();

	// CREATE JSON FILE
	
	// blockId attributes
	vector<int> leftPlanes = { 520, 551 };
	vector<int> rightPlanes = { 519, 552 };
	vector<int> medialStartEnd = { 20, 24 };

	// Json file 
	j["LeftPlanes"] = leftPlanes;
	j["RightPlanes"] = rightPlanes;

	j["MedialStartEnd"] = medialStartEnd;

	// EXPORT	
	ofstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return;
	}

	//myfile.precision(16);
	myfile << j.dump();
	myfile.close();

}



////// ---------------------------------------------------- MODEL  ----------------------------------------------------



void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////
	model = zModel(100000);

	zFnMesh fnGuideMesh(o_guideMesh);
	fnGuideMesh.from(filePath, zJSON);

	exportBlockJSON(exportBRGPath, o_guideMesh);
	

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_guideMesh);

	// set display element booleans
	o_guideMesh.setDisplayElements(false, true, true);
	
	//////////////////////////////////////////////////////////


	S = *new SliderGroup();
	
	S.addSlider(&background, "background");

	//S.addSlider(&offset, "offset");
	//S.sliders[1].attachToVariable(&offset, 0.01, 1.00);


	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));	

	B.addButton(&d_guideMesh, "d_guideMesh");
	

	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{


	if (exportMesh)
	{		

		exportMesh = !exportMesh;
	}

	if (importMesh)
	{
		
	}
	



}


////// ---------------------------------------------------- VIEW  ----------------------------------------------------

void draw()
{
	
	

	backGround(background);
	//drawGrid(20.0);

	//glDisable(GL_CULL_FACE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);


	glColor3f(1, 0, 0);
	

	S.draw();
	B.draw();

	o_guideMesh.setDisplayObject(d_guideMesh);
	o_guideThickMesh.setDisplayObject(d_guideThickMesh);	
	o_inputMesh.setDisplayObject(d_inputMesh);

	model.draw();	
	


	//////////////////////////////////////////////////////////



	glColor3f(0, 0, 0);
	setup2d();

	AL_drawString(s, winW * 0.5, winH - 50);
	AL_drawString(text, winW * 0.5, winH - 75);
	AL_drawString(jts, winW * 0.5, winH - 100);
	AL_drawString(printfile.c_str(), winW * 0.5, winH - 125);
	
	

	//int hts = 100;
	//int wid = winW * 0.01;

	
	//----
	
	//hts += 50;
	restore3d();
	
	



}

////// ---------------------------------------------------- CONTROLLER  ----------------------------------------------------

void keyPress(unsigned char k, int xm, int ym)
{

	///// GRAPH GENERTOR PROGRAM 
	if (k == 'i')setCamera(15, -40, 60, -2, 4);
	
	if (k == 'c')  importMesh = true;

	if (k == 'v') exportMesh = true;
	
	
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}




#endif // _MAIN_
