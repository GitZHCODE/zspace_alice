//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool exportMesh = false;
bool smoothColors = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjGraph oGraph;

zObjComputeMesh oCompMesh;

zPointArray positions;

double minThershold = 0.0;
double maxThershold = 5.0;

int startVID = 0;
int numSKIP = 0;

////// --- CUSTOM METHOD --------------------------------------------------

void colorMesh(zObjMesh& _oMesh, int sVID, int numSkip, float minThreshold , float maxThreshold)
{
	zItMeshVertex vStart(_oMesh, sVID);
	zItMeshHalfEdgeArray cHdges;

	vStart.getConnectedHalfEdges(cHdges);

	zItMeshHalfEdge heStart;
	for (auto& he : cHdges)
	{
		if (!he.getEdge().onBoundary()) heStart = he;
	}


	zItMeshHalfEdge he = heStart;
	zItMeshHalfEdgeArray heLoop;

	bool exit = false;

	do
	{
		if (he.getVertex().onBoundary()) exit = true;
		heLoop.push_back(he);

		if(!exit) he = he.getNext().getSym().getNext();

	} while (!exit);


	for (int i = numSkip; i < heLoop.size() - numSkip; i++)
	{
		heLoop[i].getEdge().setColor(zColor(0, 1, 0, 1));
		heLoop[i].getEdge().setWeight(2);
	}

	// color mesh
	zFnMesh fnMesh(_oMesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zColorArray vColors;
	fnMesh.getVertexColors(vColors);

	zFloatArray vDistance;
	vDistance.assign(fnMesh.numVertices(), 0.0);

	for (int i = 0; i< fnMesh.numVertices(); i++)
	{
		vDistance[i] = 100000;

		for (int j = numSkip; j < heLoop.size() - numSkip; j++)
		{
			int v0 = heLoop[j].getStartVertex().getId();
			int v1 = heLoop[j].getVertex().getId();
			zVector closestPt;
			float dist_temp = core.minDist_Edge_Point(vPositions[i], vPositions[v0], vPositions[v1], closestPt);

			if (dist_temp < vDistance[i]) vDistance[i] = dist_temp;
		}
	}


	float minD, maxD;
	minD = core.zMin(vDistance);
	maxD = core.zMax(vDistance);

	zDomainColor colDomain(zColor(1, 0, 0, 0), zColor(0, 0, 0, 1));
	zDomainFloat distDomain(minThreshold, maxThreshold);

	for (int i =0; i< fnMesh.numVertices(); i++)
	{
		if (vDistance[i] <= minThreshold) vColors[i] = colDomain.min;
		else if (vDistance[i] >= maxThreshold)vColors[i] = colDomain.max;
		else
		{
			vColors[i] = core.blendColor(vDistance[i], distDomain, colDomain, zRGB);

			if (vColors[i].r > 1) vColors[i].r = 1;
			if (vColors[i].g > 1) vColors[i].g = 1;
			if (vColors[i].b > 1) vColors[i].b = 1;
			
		}
	}

	fnMesh.setVertexColors(vColors, true);

	
	
}

////// --- GUI OBJECTS ----------------------------------------------------


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
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/guangdu/guangdu_inMesh.obj", zOBJ);
	
	ifstream myfile;
	string infilename = "data/guangdu/inData.txt";
	myfile.open(infilename.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << infilename.c_str() << endl;
		
	}
	else
	{
		while (!myfile.eof())
		{
			string str;
			getline(myfile, str);

			vector<string> perlineData = core.splitString(str, " ");

			if (perlineData.size() > 0)
			{
				// vertex
				if (perlineData[0] == "sV")
				{
					startVID = atoi(perlineData[1].c_str());
				}

				if (perlineData[0] == "nSkip")
				{
					numSKIP = atoi(perlineData[1].c_str());
				}
			}
		}

		myfile.close();

	}
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(true, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&minThershold, "minThershold");
	S.sliders[1].attachToVariable(&minThershold, 0, 20);

	S.addSlider(&maxThershold, "maxThershold");
	S.sliders[2].attachToVariable(&maxThershold, 0, 20);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{
		colorMesh(oMesh, startVID, numSKIP, minThershold, maxThershold);

		compute = !compute;	
	}

	if (smoothColors)
	{
		zFnMesh fnMesh(oMesh);
		fnMesh.smoothColors(1, zVertexData);

		fnMesh.computeFaceColorfromVertexColor();

		smoothColors = !smoothColors;
	}

	if (exportMesh)
	{
		zFnMesh fnMesh(oMesh);
		fnMesh.to("data/guangdu/guangdu_outMesh.json", zJSON);

		exportMesh = !exportMesh;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'e') exportMesh = true;;

	if (k == 'n') smoothColors = true;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
