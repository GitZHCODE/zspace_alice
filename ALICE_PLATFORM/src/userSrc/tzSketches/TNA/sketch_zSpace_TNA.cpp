#define _MAIN_
//#define _HAS_STD_BYTE 0

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
void drawTextAtVec(string s, Alice::vec &pt)
{
	unsigned int i;
	glRasterPos3f(pt.x, pt.y, pt.z);

	for (i = 0; i < s.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
}

////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

zModel model;

zObjMesh operateObj;
zFnMesh fnOperateMesh;

zObjGraph dualObj;
zFnGraph fnDual;

zObjMesh forceObj;
zObjMesh formObj;
zObjMesh resultObj;

zTsMeshVault myVault;

vector<zVector> formtargets;
zIntArray fixedVerts;
vector<int> tensionEdges;
bool gettargets = true;

vector<int> fixedVertices;

double formWeight = 0.9;
double angleTolerance = 5.0;
double forceTolerance = 0.01;

double form_minMax = 0.05;
double force_minMax = 0.1;
double forceDiagramScale = 1.5;

double vMass = 5.0;
double vThickness = 0.1;

double dT = 0.05;
zIntergrationType intType = zRK4;

string path = "data/TNA/tna.usda";
string exportPath = "data/TNA/tna_out.usda";

//string path = "data/TNA/tna_01.json";
//string exportPath = "data/TNA/tna_out_01.json";

bool splitDual = false;
bool c_form = false;
bool c_force = false;

bool horizontalE= false;
bool verticalE = false;

bool c_Result = false;

bool d_dualgraph = true;
bool d_inputmesh = true;
bool d_ForceDiagram = true;
bool d_FormDiagram = true;
bool d_Result = true;

bool rotate90 = true;
bool computeHE_targets = true;
bool computeFD = true;

bool FDM = false;

bool walkNext = false;

bool e_result = false;

int currentId = 0;
int rCnt = 0;

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
	   	 
	myVault = zTsMeshVault(resultObj, formObj, forceObj);

	// append to model for displaying the object	
	model.addObject(resultObj);
	model.addObject(formObj);
	model.addObject(forceObj);
	
	//////////////////////////////////////////////////////////

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");

	S.addSlider(&formWeight, "formWeight");
	S.sliders[1].attachToVariable(&formWeight, 0.00, 1.00);

	S.addSlider(&form_minMax, "form_minMax");
	S.sliders[2].attachToVariable(&form_minMax, 0.0001, 1.00);

	S.addSlider(&force_minMax, "force_minMax");
	S.sliders[3].attachToVariable(&force_minMax, 0.0001, 1.00);

	S.addSlider(&angleTolerance, "angleTolerance");
	S.sliders[4].attachToVariable(&angleTolerance, 0.001, 10.00);

	S.addSlider(&vMass, "vMass");
	S.sliders[5].attachToVariable(&vMass, 0.01, 10.00);

	S.addSlider(&vThickness, "vThickness");
	S.sliders[6].attachToVariable(&vThickness, 0.01, 1.0);

	S.addSlider(&forceDiagramScale, "forceScale");
	S.sliders[7].attachToVariable(&forceDiagramScale, 0.01, 20.00);
	
	S.addSlider(&forceTolerance, "forceTolerance");
	S.sliders[8].attachToVariable(&forceTolerance, 0.001, 1.00);


	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	int buttonCounter = 0;

	B.addButton(&c_form, "c_form");
	B.buttons[buttonCounter++].attachToVariable(&c_form);

	B.addButton(&c_force, "c_force");
	B.buttons[buttonCounter++].attachToVariable(&c_force);

	B.addButton(&horizontalE, "horizontalE");
	B.buttons[buttonCounter++].attachToVariable(&horizontalE);

	B.addButton(&c_Result, "c_Result");
	B.buttons[buttonCounter++].attachToVariable(&c_Result);

	B.addButton(&verticalE, "verticalE");
	B.buttons[buttonCounter++].attachToVariable(&verticalE);

	B.addButton(&e_result, "e_result");
	B.buttons[buttonCounter++].attachToVariable(&e_result);

	//////////////////////////////////////////////////////////////////////////

	operateObj.setDisplayElements(true, true, true);
	resultObj.setDisplayElements(false, true, true);
}

void update(int value)
{
	
	if (c_form)
	{
		myVault.fnForm.clear();
		myVault.fnForm.from(path, zUSD);

		//Get constraints (based on vertex color in this case
		for (zItMeshVertex v(formObj); !v.end(); v++)
		{
			if (v.getColor() == zColor(0, 0, 0, 1))
			{
				fixedVerts.push_back(v.getId());
			}
		}

		//Set constraints
		myVault.setConstraints(zFormDiagram, fixedVerts);	

		//Set vertex weights based on provided constraints (ensures those vertices do not move)
		myVault.setVertexWeightsfromConstraints(zFormDiagram);

		vector<bool> fixVerts_bool;
		for (int i = 0; i < myVault.fnForm.numVertices(); i++) fixVerts_bool.push_back(false);
		for (int i = 0; i < fixedVerts.size(); i++) fixVerts_bool[fixedVerts[i]] = true;


		
		for (zItMeshHalfEdge he(*myVault.formObj); !he.end(); he++) 
			if(he.getStartVertex().getColor() == zColor(1,1,1,1) &&
				he.getVertex().getColor() == zColor(1,1,1,1)) 
				tensionEdges.push_back(he.getId());
		myVault.setTensionEdges(zFormDiagram, tensionEdges);

		formObj.setDisplayElements(true, true, false);

		c_form = !c_form;
	}

	if (c_force)
	{
		myVault.fnForce.clear();
		myVault.createForceFromForm(rotate90);

		myVault.translateForceDiagram(2.0);

		forceObj.setDisplayElements(true, true, false);
		
		c_force = !c_force;
	}

	if (horizontalE)
	{

		if (gettargets)
		{
			myVault.getHorizontalTargetEdges(zForceDiagram, formtargets);
			gettargets = !gettargets;
		}

		bool out = myVault.equilibriumHorizontal(computeHE_targets, formWeight, dT, intType, 100, angleTolerance, form_minMax, force_minMax, true, true);

		horizontalE = !out;
	}

	if (c_Result)
	{		
		zFnMesh fn(resultObj);
		fn.clear();

		myVault.createResultFromForm();

		// set vertexWeights 
		myVault.setVertexWeightsfromConstraints(zDiagramType::zFormDiagram);

		myVault.setVertexMassfromVertexArea();

		c_Result = !c_Result;
	}


	if (verticalE)
	{
		// set vertex thickness
		myVault.setVertexThickness(vThickness);

		bool out = myVault.equilibriumVertical(computeFD, forceDiagramScale);

		verticalE = !out;

	}

	if (e_result)
	{
		zFnMesh fnMesh(resultObj);
		fnMesh.to(exportPath, zUSD);

		e_result = !e_result;
	}
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


	if (d_inputmesh) 
	{ 
		operateObj.draw(); 
	}
	if (d_FormDiagram)
	{
		formObj.draw();
	}
	if (d_ForceDiagram)
	{
		forceObj.draw();
	}
	if (d_Result)
	{
		resultObj.draw();
	}


	model.draw();
	
	/*if (formtargets.size() > 0)
	{
		for (int i = 0; i < myVault.fnForce.numEdges(); i += 1)
		{
			int sId = myVault.fnForce.getStartVertexIndex(i);
			zVector pos = myVault.fnForce.getVertexPosition(sId);

			zVector t = formtargets[i];

			model.displayUtils.drawLine(pos, pos + t);


			vec p(pos.x, pos.y,pos.z);
			drawTextAtVec(to_string(sId), p);
		}


	}*/
	

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

	


}

void mousePress(int b, int state, int x, int y)
{

	
}

void mouseMotion(int x, int y)
{
	

}




#endif // _MAIN_
