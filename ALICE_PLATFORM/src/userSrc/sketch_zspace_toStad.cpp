//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zColor BLACK(0, 0, 0, 1);
zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);

bool toSTAD(string path, zObjMesh& o_Mesh, zIntArray &supports, float &totalArea)
{
	ofstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	//INPUT WIDTH 79
	int width = 79;
	myfile << "\n INPUT WIDTH " << width;
	
	//UNIT METER KN
	myfile << "\n UNIT METER KN ";
	
	//JOINT COORDINATES
	myfile << "\n JOINT COORDINATES ";
	
	// vertex positions
	// 1 -0.771834 0 1.26659;
	for (zItMeshVertex v(o_Mesh); !v.end(); v++ )
	{
		zPoint vPos = v.getPosition();		
		myfile << "\n " << (v.getId() + 1) << " " << vPos.x << " " << vPos.z << " " << vPos.y << ";";
	}

	//MEMBER INCIDENCES
	myfile << "\n MEMBER INCIDENCES ";
	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		zIntArray eVerts;
		e.getVertices(eVerts);

		myfile << "\n " << (e.getId() + 1) << " " << (eVerts[0] + 1) << " " << (eVerts[1] + 1) << ";";
	}

	//DEFINE MATERIAL START
	myfile << "\n DEFINE MATERIAL START ";

	//ISOTROPIC STEEL
	myfile << "\n ISOTROPIC STEEL ";
	
	//	E 2.05e+008
	myfile << "\n E " << 2.05e+008;

	//	POISSON 0.3
	myfile << "\n POISSON " << 0.3;

	//DENSITY 76.8195
	myfile << "\n DENSITY " << 76.8195;

	//ALPHA 1.2e-005
	myfile << "\n ALPHA " << 1.2e-005;
	
	//DAMP 0.03
	myfile << "\n DAMP " << 0.03;

	//	TYPE STEEL
	myfile << "\n TYPE STEEL ";

	//STRENGTH FY 253200 FU 407800 RY 1.5 RT 1.2
	myfile << "\n STRENGTH FY " << 253200 << " FU " << 407800 << " RY " << 1.5 << " RT " << 1.2;
	
	//ISOTROPIC CONCRETE
	myfile << "\n ISOTROPIC CONCRETE ";
	
	//E 2.17185e+007
	myfile << "\n E " << 2.17185e+007;

	//POISSON 0.17
	myfile << "\n POISSON " << 0.17;

	//DENSITY 23.5616
	myfile << "\n DENSITY " << 23.5616;

	//ALPHA 1e-005
	myfile << "\n ALPHA " << 1e-005;

	//DAMP 0.05
	myfile << "\n DAMP " << 0.05;

	//TYPE CONCRETE
	myfile << "\n TYPE CONCRETE ";

	//STRENGTH FCU 27579
	myfile << "\n STRENGTH FCU "<< 27579;
	
	//END DEFINE MATERIAL
	myfile << "\n END DEFINE MATERIAL ";

	//CONSTANTS
	myfile << "\n CONSTANTS ";

	//MATERIAL STEEL ALL
	myfile << "\n MATERIAL STEEL ALL ";

	//SUPPORTS
	myfile << "\n SUPPORTS ";
	myfile << "\n ";

	for (int sID : supports)
	{
		myfile << (sID + 1) << " ";
	}
	myfile << "PINNED ";

	//LOAD 1 LOADTYPE None  TITLE LOAD CASE 1
	myfile << "\n LOAD " << 1 << " LOADTYPE None  TITLE LOAD CASE  " << 1;

	//SELFWEIGHT Y -1.5 
	myfile << "\n SELFWEIGHT Y " << -1.5;

	//JOINT LOAD
	zFnMesh fnMesh(o_Mesh);
	
	zPointArray fCenters;
	fnMesh.getCenters(zFaceData, fCenters);

	zPointArray heCenters;
	fnMesh.getCenters(zHalfEdgeData, heCenters);

	zFloatArray vAreas;
	totalArea = fnMesh.getVertexAreas(fCenters, heCenters, vAreas);
	
	
	myfile << "\n JOINT LOAD ";

	//	1 2 3 4 5 6 7 8 9 10 FY - 2.5
	for (zItMeshVertex v(o_Mesh); !v.end(); v++)
	{
		int id = v.getId();
		if (v.getColor() == RED) myfile << "\n " << (id + 1) << " FY " << -3.5 * vAreas[id];
		if (v.getColor() == BLUE)myfile << "\n " << (id + 1) << " FY " << -2.5 * vAreas[id];
	}
	
	//PERFORM ANALYSIS
	myfile << "\n PERFORM ANALYSIS ";
	
	//PARAMETER 1
	myfile << "\n PARAMETER " << 1;
	
	//CODE IS800 LSD
	myfile << "\n CODE IS800 LSD " ;
	
	//BEAM 1 ALL
	myfile << "\n BEAM 1 ALL ";

	//FYLD 250000 ALL
	myfile << "\n FYLD 250000 ALL ";
	
	//CHECK CODE ALL
	myfile << "\n CHECK CODE ALL ";

	//FINISH
	myfile << "\n FINISH ";

	myfile.close();
	cout << endl << " STAD exported. File:   " << path.c_str() << endl;
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;
zIntArray supports;


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
	fnMesh.from("data/toSTAD/toStad_2804.json", zJSON);

	//  supports
	//supports = zIntArray{ 2,3,5,9,13,15,17,21,23,25,29,31,33,37,39,41,45,47,49,53,55,896,899,900,902,903,905,906,907,909,911,912,914,915,916 };
	int redCounter = 0;
	int blueCounter = 0;
	for (zItMeshVertex v(oMesh); !v.end(); v++)
	{
		if (v.getColor() == BLACK)
		{
			supports.push_back(v.getId());
		}

		if (v.getColor() == RED)redCounter++;
		if (v.getColor() == BLUE)blueCounter++;
	}
	
	printf("\n num supports %i ", supports.size());
	printf("\n red %i blue %i ", redCounter, blueCounter);

	fnMesh.setEdgeColor(zColor(0.75, 0.75, 0.75, 1), false);
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	
	// set display element booleans
	oMesh.setDisplayElements(true, true, false);
	
	
	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

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
		float totalArea;
		toSTAD("data/toSTAD/outTest.std", oMesh, supports, totalArea);

		printf("\n totalArea: %1.3f ", totalArea);

		compute = !compute;	
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
		model.draw();
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
