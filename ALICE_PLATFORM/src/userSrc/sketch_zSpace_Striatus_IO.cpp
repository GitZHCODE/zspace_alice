
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

zPointArray points;
zVectorArray norms;
zPointArray thickPoints;

zPointArray points_brg;
zVectorArray norms_brg;

zFloatArray normDeviations;

zPointArray noInfoPts;

/*!<Function sets*/

/*!<Tool sets*/
zTsSDFBridge mySDF;

int voxelId = 1;

int presFac = 1;
int nF = 0;

string filePath = "data/striatus/center_s2.json";
string filePath_thick = "data/striatus/thickness_s2.json";

string exportBRGPath = "data/striatus/out_BRG.json";
string importBRGPath = "data/striatus/tozha_220622.json";


////// --- GUI OBJECTS ----------------------------------------------------


char s[200],text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------




////// ---------------------------------------------------- MODEL  ----------------------------------------------------


zObjMesh brg_formForce;


void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////
	model = zModel(100000);

	zFnMesh fnGuideMesh(o_guideMesh);
	//fnGuideMesh.from(filePath, zJSON);
	mySDF.setGuideMesh(o_guideMesh);

	zFnMesh fnGuidetThickMesh(o_guideThickMesh);
	//fnGuidetThickMesh.from(filePath_thick, zJSON);
	mySDF.setThickGuideMesh(o_guideThickMesh);
	

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_guideMesh);

	model.addObject(o_guideThickMesh);


	// set display element booleans
	o_guideMesh.setDisplayElements(false, true, true);

	o_guideThickMesh.setDisplayElements(false, true, false);
	
	//////////////////////////////////////////////////////////


	S = *new SliderGroup();
	
	S.addSlider(&background, "background");

	//S.addSlider(&offset, "offset");
	//S.sliders[1].attachToVariable(&offset, 0.01, 1.00);


	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));	

	B.addButton(&d_guideMesh, "d_guideMesh");

	B.addButton(&d_guideThickMesh, "d_guideThickMesh");

	B.addButton(&exportMesh, "e_PlaneMesh");
	

	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{


	if (exportMesh)
	{
		mySDF.toBRGJSON(exportBRGPath, points, norms, thickPoints);

		exportMesh = !exportMesh;
	}

	if (importMesh)
	{
		zFnMesh fnGuideMesh(o_guideMesh);
		fnGuideMesh.clear();
		fnGuideMesh.from(importBRGPath, zJSON);

		zPointArray vThickness;
		mySDF.fromBRGJSON(importBRGPath, points_brg, norms_brg, vThickness);

		zFnMesh fnGuideThickMesh(o_guideThickMesh);
		fnGuideThickMesh.clear();

		int nVerts = fnGuideMesh.numVertices();
		zIntArray pCounts, pConnects;

		for (zItMeshFace f(o_guideMesh); !f.end(); f++)
		{
			zIntArray fVerts;
			f.getVertices(fVerts);

			pCounts.push_back(fVerts.size());
			for (auto& v : fVerts) pConnects.push_back(v * 2);

			pCounts.push_back(fVerts.size());
			for (auto& v : fVerts) pConnects.push_back(v * 2 + 1);
		}
		

		fnGuideThickMesh.create(vThickness, pCounts, pConnects);
		
		
		fnGuideMesh.to("data/striatus/fromBRG_center.json", zJSON);
		fnGuideThickMesh.to("data/striatus/fromBRG_thickness.json", zJSON);

		
		importMesh = !importMesh;
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
