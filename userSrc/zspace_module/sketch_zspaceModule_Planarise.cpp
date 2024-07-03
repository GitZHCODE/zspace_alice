//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <headers/zModules/projectionsolver/zSpace_PlanariseSolver.h>

using namespace zSpace;
using namespace std;
/*!<Objects*/

zUtilsCore core;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

float planarityTol = 0.000001;

double dT = 1.0;
zIntergrationType intType = zRK4;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

/*!< input mesh object  */
zObjMesh o_Mesh;
zObjMesh o_inMesh;

zDoubleArray out_vPositions, out_deviations;
zIntArray pCounts, pConnects;
zDoubleArray vPositions;
zIntArray triCounts, triConnects;

MatrixXd V;

/*!< container of  particle objects  */
zObjParticleArray o_Particles;

/*!< container of particle function set  */
vector<zFnParticle> fnParticles;

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
	zFnMesh fnInMesh(o_inMesh);
	fnInMesh.from("data/distort_cube.obj", zOBJ);
	fnInMesh.setEdgeColor(zColor(1, 0, 0, 1));

	zFnMesh fnMesh(o_Mesh);
	fnMesh.from("data/distort_cube.obj", zOBJ);

	fnMesh.getPolygonData(pConnects, pCounts);

	zPoint* positions = fnMesh.getRawVertexPositions();
	vPositions.assign(fnMesh.numVertices() * 3, double());

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		vPositions[i * 3 + 0] = positions[i].x;
		vPositions[i * 3 + 1] = positions[i].y;
		vPositions[i * 3 + 2] = positions[i].z;
	}

	zInt2DArray fTriangles;
	fnMesh.getMeshTriangles(fTriangles);
	
	for (int i = 0; i < fTriangles.size(); i++)
	{
		int nTris = floor (fTriangles[i].size() / 3);
		triCounts.push_back(nTris);

		for (int j = 0; j < fTriangles[i].size(); j++)
		{
			triConnects.push_back(fTriangles[i][j]);
			//printf("\n %i ", fTriangles[i][j]);
		}
	}

	out_vPositions.assign(fnMesh.numVertices() * 3, double());
	out_deviations.assign(fnMesh.numPolygons(), double());

	planariseSolver_initialise(&vPositions[0], &pCounts[0], &pConnects[0], &triCounts[0], &triConnects[0],fnMesh.numVertices(), fnMesh.numPolygons(), true, &out_deviations[0]);
		

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_Mesh);
	model.addObject(o_inMesh);

	// set display element booleans
	o_Mesh.setDisplayElements(false, true, false);
	o_inMesh.setDisplayElements(false, true, false);


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
		zFnMesh fnMesh(o_Mesh);

		planariseSolver_compute(1, planarityTol, &out_vPositions[0], &out_deviations[0]);

		//planarise(&vPositions[0], &pCounts[0], &pConnects[0], &triCounts[0], &triConnects[0], fnMesh.numVertices(), fnMesh.numPolygons(), false, 1, 0.001, &out_vPositions[0], &out_deviations[0]);

		//quadPlanarise(&vPositions[0], &pCounts[0], &pConnects[0], fnMesh.numVertices(), fnMesh.numPolygons(), false, 1, planarityTol, &out_vPositions[0], &out_deviations[0]);


		zPoint* positions = fnMesh.getRawVertexPositions();
		
		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			positions[i].x = out_vPositions[i * 3 + 0];
			positions[i].y = out_vPositions[i * 3 + 1];
			positions[i].z = out_vPositions[i * 3 + 2];
		}

		for (int i = 0; i < fnMesh.numPolygons(); i++)
		{
			printf("\n %1.7f ", out_deviations[i]);
		}
		
		compute = !compute;
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
		
	}
	

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p')compute = true;;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
