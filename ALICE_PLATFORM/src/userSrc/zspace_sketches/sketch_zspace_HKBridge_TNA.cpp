//#define _MAIN_

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
void drawTextAtVec(string s, Alice::vec& pt)
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
bool gettargets = true;

vector<int> fixedVertices;

double formWeight = 1.0;
double angleTolerance = 0.001;
double forceTolerance = 0.001;

double form_minMax = 0.3;
double force_minMax = 0.01;
double forceDiagramScale = 0.1;


double dT = 0.5;
zIntergrationType intType = zRK4;

string path = "data/suspensionBridge_06.json";


bool c_form = false;
bool c_force = false;

bool horizontalE = false;

bool d_inputmesh = true;
bool d_ForceDiagram = true;
bool d_FormDiagram = true;

bool rotate90 = true;
bool computeHE_targets = true;

bool exportFiles = false;

bool alignFORCE_TOP = false;

int form_v0 = 2;
int form_v1 = 4;


zColor red(1, 0, 0, 1);
zColor blue(0, 0, 1, 1);
zColor green(0, 1, 0, 1);

int currentId = 0;
int rCnt = 0;
////// --- GUI OBJECTS ----------------------------------------------------


char s[200], text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------


////// --- CUSTOM METHODS -------------------------------------------------
void alignForceDiagram_bottom(zTsMeshVault &_myVault, zObjMesh& _o_form, zObjMesh& _o_force, int vID0, int vID1)
{
	zItMeshVertex v0(_o_form, vID0);
	zItMeshVertex v1(_o_form, vID1);

	zVector dir = v1.getPosition() - v0.getPosition();
	dir.normalize();

	zItMeshHalfEdgeArray cHEdges;
	v0.getConnectedHalfEdges(cHEdges);

	float angle = 180;
	zItMeshHalfEdge he;
	for (auto& cHE : cHEdges)
	{
		float tmp_angle = cHE.getVector().angle(dir);
		if (tmp_angle < angle)
		{
			he = cHE;
			angle = tmp_angle;
		}
	}

	bool flip = (he.onBoundary()) ? false : true;
	if (flip) he = he.getSym();

	// get all connected faces between v0 & v1
	bool exit = false;
	zIntArray formFaces_bottom;

	do
	{
		formFaces_bottom.push_back(he.getSym().getFace().getId());


		if (flip)
		{
			if (he.getStartVertex() == v1) exit = true;
			he = he.getPrev();
		}
		else
		{
			if (he.getVertex() == v1) exit = true;
			he = he.getNext();
		}

	} while (!exit);

	// align all force vertices corresponding form faces
		
	zPoint* vPositions = _myVault.fnForce.getRawVertexPositions();

	zPoint avgPt;
	for (auto& fID : formFaces_bottom)
	{
		avgPt += vPositions[fID];		
	}

	avgPt /= formFaces_bottom.size();

	zPoint startPt;
	startPt.x = avgPt.x;
	startPt.y = vPositions[formFaces_bottom[0]].y;
	

	zVector eDir(0, 1, 0);
	
	float dist = abs(vPositions[formFaces_bottom[0]].y - vPositions[formFaces_bottom[formFaces_bottom.size() - 1]].y);
	float eLen = dist/ formFaces_bottom.size();	

	zPoint currentPt = startPt;
	
	zFloatArray vWeights;
	vWeights.assign(_myVault.fnForce.numVertices(), 1.0);

	for (auto& fID : formFaces_bottom)
	{
		vWeights[fID] = 0.0;
		vPositions[fID] = currentPt;
		currentPt += (eDir * -eLen);
	}
	

	_myVault.setVertexWeights(zForceDiagram, vWeights);


}

void alignForceDiagram_top(zTsMeshVault& _myVault, zObjMesh& _o_form, zObjMesh& _o_force, int vID0, int vID1)
{
	zItMeshVertex v0(_o_form, vID0);
	zItMeshVertex v1(_o_form, vID1);

	zVector dir = v1.getPosition() - v0.getPosition();
	dir.normalize();

	zItMeshHalfEdgeArray cHEdges;
	v0.getConnectedHalfEdges(cHEdges);

	zVector he0_vec = cHEdges[0].getVector();
	zVector he1_vec = cHEdges[1].getVector();

	float angle = 0;
	zItMeshHalfEdge he;
	for (auto& cHE : cHEdges)
	{
		float tmp_angle= cHE.getVector().angle(dir);
		if (tmp_angle > angle)
		{
			he = cHE;
			angle = tmp_angle;
		}
	}
	
	bool flip = (he.onBoundary()) ? false : true;
	if (flip) he = he.getSym();

	// get all connected faces between v0 & v1
	bool exit = false;
	zIntArray formFaces_bottom;

	do
	{
		formFaces_bottom.push_back(he.getSym().getFace().getId());


		if (flip)
		{
			if (he.getStartVertex() == v1) exit = true;
			he = he.getPrev();
		}
		else
		{
			if (he.getVertex() == v1) exit = true;
			he = he.getNext();
		}

	} while (!exit);

	// align all force vertices corresponding form faces

	zPoint* vPositions = _myVault.fnForce.getRawVertexPositions();

	zPoint minBB, maxBB;
	_myVault.fnForce.getBounds(minBB, maxBB);
	

	zPoint avgPt;
	for (auto& fID : formFaces_bottom)
	{
		avgPt += vPositions[fID];		

		//printf("\n %i ", fID);
	}

	avgPt /= formFaces_bottom.size();

	for (auto& fID : formFaces_bottom)
	{		
		vPositions[fID].x = avgPt.x;		
	}



}

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


	S.addSlider(&forceDiagramScale, "forceScale");
	S.sliders[7].attachToVariable(&forceDiagramScale, 0.01, 10.00);

	S.addSlider(&forceTolerance, "forceTolerance");
	S.sliders[8].attachToVariable(&forceTolerance, 0.001, 1.00);


	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));


	B.addButton(&c_form, "c_form");

	B.addButton(&c_force, "c_force");

	B.addButton(&horizontalE, "horizontalE");

	B.addButton(&d_inputmesh, "d_inputmesh");

	B.addButton(&d_ForceDiagram, "d_ForceDiagram");

	B.addButton(&d_FormDiagram, "d_FormDiagram");




	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{
	operateObj.setDisplayObject(d_inputmesh);
	formObj.setDisplayObject(d_FormDiagram);
	forceObj.setDisplayObject(d_ForceDiagram);

	if (c_form)
	{

		myVault.fnForm.from(path, zJSON);

		zIntArray fixedVerts;
		zFloatArray vWeights;


		zColor* vColors = myVault.fnForm.getRawVertexColors();

		for (int i = 0; i < myVault.fnForm.numVertices(); i++)
		{
			//bool fixed = (vColors[i] == red || vColors[i] == blue) ? true : false;

			if (/*vColors[i] == red ||*/ vColors[i] == blue) fixedVerts.push_back(i);

			(vColors[i] == red || vColors[i] == blue) ? vWeights.push_back(0.0) : vWeights.push_back(1.0);
		}

		// set constraints		

		// set default tension
		vector<bool> fixVerts_bool;
		for (int i = 0; i < myVault.fnForm.numVertices(); i++) fixVerts_bool.push_back(false);
		for (int i = 0; i < fixedVerts.size(); i++)fixVerts_bool[fixedVerts[i]] = true;

		vector<int> tensionEdges;

		//for (zItMeshEdge e(formObj); !e.end(); e++)
		//{
		//	if (e.getHalfEdge(0).onBoundary() && e.getHalfEdge(1).onBoundary()) continue;
		//	
		//		int v0 = e.getHalfEdge(0).getVertex().getId();
		//		int v1 = e.getHalfEdge(0).getStartVertex().getId();

		//		//if (fixVerts_bool[v0] && fixVerts_bool[v1]) continue;

		//		if (vColors[v0] == red && vColors[v1] == blue) continue;
		//		if (vColors[v0] == blue && vColors[v1] == red) continue;
		//		
		//		tensionEdges.push_back(e.getHalfEdge(0).getId());
		//		tensionEdges.push_back(e.getHalfEdge(1).getId());

		//}

		myVault.setConstraints(zFormDiagram, fixedVerts);
		myVault.setVertexWeights(zFormDiagram, vWeights);

		myVault.setTensionEdges(zFormDiagram, tensionEdges);
		formObj.setDisplayElements(true, true, false);

		computeHE_targets = true;

		c_form = !c_form;

	}

	if (c_force)
	{
		myVault.createForceFromForm(rotate90);

		myVault.translateForceDiagram(50);
		forceObj.setDisplayElements(true, true, false);
		
		alignForceDiagram_bottom(myVault, formObj, forceObj, form_v0, form_v1);

		c_force = !c_force;
	}

	if (horizontalE)
	{

		if (gettargets)
		{
			myVault.getHorizontalTargetEdges(zForceDiagram, formtargets);
			gettargets = !gettargets;
		}

		bool out = myVault.equilibriumHorizontal(computeHE_targets, formWeight, dT, intType, 1, angleTolerance, form_minMax, force_minMax, true, true);
		
		if (out) printf("\n equilibriumREACHED");
		horizontalE = !horizontalE;

	}	

	if (alignFORCE_TOP)
	{
		alignForceDiagram_top(myVault, formObj, forceObj, form_v0, form_v1);
		alignFORCE_TOP = !alignFORCE_TOP;
	}
	
	if (exportFiles)
	{
		myVault.fnForm.to("data/formMesh.obj", zOBJ);
		myVault.fnForce.to("data/forceMesh.obj", zOBJ);

		exportFiles = !exportFiles;
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

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'r')
	{
		c_form = true;
		c_force = true;
	}

	if (k == 'p')
	{		
		horizontalE = true;;
	}	

	if (k == 'c')
	{
		computeHE_targets = true;
	}

	if (k == 'o')
	{
		alignFORCE_TOP = true;
	}

	if (k == 'e')
	{
		exportFiles = true;
	}

	
	

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
