#define _MAIN_


#ifdef _MAIN_

#include "main.h"

//////  zSpace Library

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsStatics.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS ----------------------------------------------------

////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

zModel model;


zObjMesh o_input;
zObjMesh o_result;

zTsMeshVault myVault;

double vLoad = 1.0;
double forceDensity = 0.25;
double factor = 5;

//string path = "data/FDM-TNA/FDM-testMesh.obj";
//string path = "data/fdm_01.json";
//string exportPath = "data/fdm_out.obj";

string path = "data/FDM/fdm.usda";
string exportPath = "data/FDM/fdm_out.usda";

//string pathJSON = "C:/Users/vishu.b/Desktop/Dnipro_C01export_001.json";

bool formFind = false;
bool setQ = false;
bool setLoad = false;
bool d_Input = false;
bool d_Result = true;
bool readGraph = true;


bool exportResult = false;

vector<zVector> forces;



////// --- GUI OBJECTS ----------------------------------------------------


char s[200],text[200], text1[200], jts[400];
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

	model = zModel(100000);



	S = *new SliderGroup();

	S.addSlider(&background, "background");

	S.addSlider(&vLoad, "vLoad");
	S.sliders[1].attachToVariable(&vLoad, 0.0, 50.0);

	S.addSlider(&forceDensity, "forceDensity");
	S.sliders[2].attachToVariable(&forceDensity, 0.0, 2.0);

	

	S.addSlider(&factor, "factor");
	S.sliders[3].attachToVariable(&factor, 1.0, 10.0);

	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));




	int buttonCounter = 0;

	B.addButton(&readGraph, "readFile");
	B.buttons[buttonCounter++].attachToVariable(&readGraph);

	B.addButton(&setQ, "setForceDensity");
	B.buttons[buttonCounter++].attachToVariable(&setQ);

	B.addButton(&setLoad, "setLoad");
	B.buttons[buttonCounter++].attachToVariable(&setLoad);

	B.addButton(&formFind, "formFind");
	B.buttons[buttonCounter++].attachToVariable(&formFind);

	B.addButton(&d_Input, "d_Input");
	B.buttons[buttonCounter++].attachToVariable(&d_Input);

	B.addButton(&d_Result, "d_Result");
	B.buttons[buttonCounter++].attachToVariable(&d_Result);
	

	B.addButton(&exportResult, "e_result");
	B.buttons[buttonCounter++].attachToVariable(&exportResult);


	//readGraph = true;



	


	


	// append to model for displaying the object
	model.addObject(o_input);
	model.addObject(o_result);

	o_result.setDisplayElements(false, true, false);
	o_input.setDisplayElements(false, true, false);
	//resultObj.setDisplayTransform(true);

	//////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{
	o_result.setDisplayObject(d_Result);
	o_input.setDisplayObject(d_Input);
	
	if (setQ)
	{
		// set forcedensities
		myVault.setForceDensity(forceDensity);

		zFloatArray fDensities;
		
		for (zItMeshEdge e(o_input); !e.end(); e++)
		{
			zItMeshVertexArray eVerts;
			e.getVertices(eVerts);

			if (eVerts[0].getColor() == zWHITE || eVerts[1].getColor() == zWHITE) fDensities.push_back(forceDensity * factor);
			else fDensities.push_back(forceDensity);
		
		}

		myVault.setForceDensities(fDensities);
	}


	if (setLoad)
	{
		// set loads
		myVault.setVertexMass(vLoad);
	}

	if (formFind)
	{
		myVault.forceDensityMethod();	
		
	}
	if (readGraph)
	{
		zFnMesh fnInput = zFnMesh(o_input);
		//fnInput.from(path, zJSON);
		fnInput.from(path, zUSD);
		fnInput.setEdgeColor(zGREEN);

		zFnMesh fnresult = zFnMesh(o_result);
		//fnresult.from(path, zJSON);
		fnresult.from(path, zUSD);


		myVault = zTsMeshVault(o_result);


		zIntArray vConstraints;
		for (zItMeshVertex v(o_result); !v.end(); v++)
		{
			/*if (v.onBoundary())
			{
				vConstraints.push_back(v.getId());
			}*/

			if (v.getColor() == zBLACK) vConstraints.push_back(v.getId());
		
		}
		printf("\n num constraints %i", vConstraints.size());
		
		myVault.setConstraints(zResultDiagram, vConstraints);
		myVault.setVertexWeightsfromConstraints(zResultDiagram);

		// default set loads
		myVault.setVertexMass(vLoad);

		myVault.setVertexThickness(0.1);

		// set forcedensities
		myVault.setForceDensity(forceDensity);

		readGraph = !readGraph;
	}

	if (exportResult)
	{	
		// export to obj
		//myVault.fnResult.to(exportPath, zOBJ);
		myVault.fnResult.to(exportPath, zUSD);
		exportResult = !exportResult;
	}


}


////// ---------------------------------------------------- VIEW  ----------------------------------------------------

void draw()
{

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	backGround(background);
	drawGrid(20.0);

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
	if (k == 'e') exportResult = true;
	if (k == 'r') readGraph = true;
}

void mousePress(int b, int state, int x, int y)
{

}

void mouseMotion(int x, int y)
{
	

}




#endif // _MAIN_
