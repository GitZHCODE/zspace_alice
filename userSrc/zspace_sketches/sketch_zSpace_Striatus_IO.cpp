
//#define _MAIN_


#ifdef _MAIN_

#include "main.h"

//////  zSpace Library
#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <zApp/include/zTsGeometry.h>

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

string filePath_transform = "data/striatus/Transform.txt";
zTransform bridgeTransform;

string exportBRGPath = "data/striatus/out_BRG.json";
string importBRGPath = "data/striatus/tozha_220622.json";


////// --- GUI OBJECTS ----------------------------------------------------


char s[200],text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------

bool readTransform(string path, zTransformationMatrix& transMat)
{
	ifstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	transMat = zTransformationMatrix();

	zFloat4 translation = {0,0,0,0};
	zFloat4 rotation = { 0,0,0,0 };
	zFloat4 scale = { 1,1,1,1 };

	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		vector<string> perlineData = core.splitString(str, " ");

		if (perlineData.size() > 0)
		{
			// Translation
			if (perlineData[0] == "TranslateX")
			{
				translation[0] = atof(perlineData[1].c_str());				
			}

			if (perlineData[0] == "TranslateY")
			{
				translation[1] = atof(perlineData[1].c_str());
			}

			if (perlineData[0] == "TranslateZ")
			{
				translation[2] = atof(perlineData[1].c_str());
			}

			// Rotation
			if (perlineData[0] == "RotateX")
			{
				rotation[0] = atof(perlineData[1].c_str());
			}

			if (perlineData[0] == "RotateY")
			{
				rotation[1] = atof(perlineData[1].c_str());
			}

			if (perlineData[0] == "RotateZ")
			{
				rotation[2] = atof(perlineData[1].c_str());
			}

			// Scale
			if (perlineData[0] == "ScaleX")
			{
				scale[0] = atof(perlineData[1].c_str());
			}

			if (perlineData[0] == "ScaleY")
			{
				scale[1] = atof(perlineData[1].c_str());
			}

			if (perlineData[0] == "ScaleZ")
			{
				scale[2] = atof(perlineData[1].c_str());
			}

		}


	}

	myfile.close();

	transMat.setTranslation(translation);
	transMat.setRotation(rotation);
	transMat.setScale(scale);

}



////// ---------------------------------------------------- MODEL  ----------------------------------------------------


zObjMesh brg_formForce;


void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////
	model = zModel(100000);

	zTransformationMatrix transMAT;
	readTransform(filePath_transform, transMAT);
	bridgeTransform = transMAT.asMatrix();

	
	zFnMesh fnGuideMesh(o_guideMesh);
	fnGuideMesh.from(filePath, zJSON);	
	
	mySDF.setGuideMesh(o_guideMesh);

	zFnMesh fnGuidetThickMesh(o_guideThickMesh);
	fnGuidetThickMesh.from(filePath_thick, zJSON);	
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


		mySDF.toBRGJSON(exportBRGPath, points, norms, thickPoints, bridgeTransform);

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
	drawGrid(20.0);

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
	
	glColor3f(0, 0, 0);


	drawString("KEY Press", vec(winW - 350, winH - 600, 0));
	drawString("v - ExportJSON", vec(winW - 350, winH - 575, 0));
	drawString("c - ImportJSON", vec(winW - 350, winH - 550, 0));


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
