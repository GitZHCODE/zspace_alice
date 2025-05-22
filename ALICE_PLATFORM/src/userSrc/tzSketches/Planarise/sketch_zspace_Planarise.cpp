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
bool display = true;
bool toFile = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMeshArray oMeshes;

vector<zFnMeshDynamics> fnDyMeshes;
vector<zDoubleArray> devs_planarity;
vector<zIntArray> fixedVertexIds;

double tol_d = 0.01;
double dT = 0.1;
bool b_goNext = false;

int numMeshes;
int myCounter = 0;
string dir_in = "data/pSolver/in/";
string dir_out = "data/pSolver/out/";

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

	zStringArray files;
	numMeshes = core.getNumfiles_Type(dir_in, zJSON);

	if (numMeshes != 0)
	{
		core.getFilesFromDirectory(files, dir_in, zJSON);
		oMeshes.assign(numMeshes, zObjMesh());
		devs_planarity.assign(numMeshes, zDoubleArray());
		fixedVertexIds.assign(numMeshes, zIntArray());
		fnDyMeshes.assign(numMeshes, zFnMeshDynamics());
	}

	for (int i = 0; i < numMeshes; i++)
	{
		string file = files[i];
		zFnMesh fnMesh(oMeshes[i]);
		fnMesh.from(file, zJSON);

		zObjMesh* oMesh(&oMeshes[i]);
		//fnDyMeshes[i].create(*oMesh, false);
		fnDyMeshes[i].create(*oMesh, true);

		model.addObject(*oMesh);
		oMesh->setDisplayElements(true, true, true);

		//make fixed vertices
		//zColorArray vertexColors;
		//fnMesh.getVertexColors(vertexColors);

		//for(auto&cols:vertexColors)
		//cout << "cols:" << cols.r << endl;

		//for (int j = 0; j < vertexColors.size(); j++)
		//{
		//	devs_planarity[i].push_back(-1);

		//	cout << "col:" << vertexColors[j].r << endl;
		//	if (vertexColors[j].r == 1)
		//		fixedVertexIds[i].push_back(j);
		//}

		fnDyMeshes[i].setFixed(fixedVertexIds[i]);
	}


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object


	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&tol_d, "tol_d");
	S.sliders[0].attachToVariable(&tol_d, 0, 0.1);
	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&b_goNext, "b_goNext");
	B.buttons[1].attachToVariable(&b_goNext);

}

void update(int value)
{
	if (compute)
	{
		bool exit_planar = false;

		zObjMesh* oMesh(&oMeshes[myCounter]);
		zFnMeshDynamics* fnDyMesh(&fnDyMeshes[myCounter]);

		double mxDev = 0;
		zVectorArray forceDir;

		fnDyMesh->addPlanarityForce(1.0, tol_d, zQuadPlanar, devs_planarity[myCounter],forceDir, exit_planar);
		//fnDyMesh->addPlanarityForce(1.0, tol_d, zVolumePlanar, devs_planarity[myCounter], forceDir, exit_planar);
		fnDyMesh->update(dT, zRK4, true, true, true);

		for (zItMeshFace f(*oMesh); !f.end(); f++)
		{
			if (devs_planarity[myCounter][f.getId()] < tol_d)
				f.setColor(zColor(0, 1, 0, 1));
			else f.setColor(zColor(1, 0, 1, 1));
		}

		double fPlanar_max = core.zMax(devs_planarity[myCounter]);
		double fPlanar_min = core.zMin(devs_planarity[myCounter]);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);


		if (exit_planar || b_goNext)
		{
			
			if (myCounter < numMeshes - 1)
			{
				myCounter++;
				exit_planar = false;
			}
			else
			{
				compute = !compute;
				toFile = true;
			}
			b_goNext = false;
		}
	}

	if (toFile)
	{

		for (int i = 0; i < numMeshes; i++)
		{
			zFnMesh fn(oMeshes[i]);
			fn.to(dir_out + "out_" + to_string(i) + ".json", zJSON);
		}

		toFile = !toFile;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		
		zPoint* vPositions = fnDyMeshes[myCounter].getRawVertexPositions();

		for (int i = 0; i < fnDyMeshes[myCounter].numVertices(); i++) {
			bool fixedV = std::find(std::begin(fixedVertexIds[myCounter]), std::end(fixedVertexIds[myCounter]), i) != std::end(fixedVertexIds[myCounter]);

			if (fixedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(1, 0, 0, 1), 5);
		}
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
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
