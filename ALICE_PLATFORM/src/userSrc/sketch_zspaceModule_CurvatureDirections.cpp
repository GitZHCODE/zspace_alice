//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include<headers/zModules/geometryprocessing/zSpace_Curvatures.h>

using namespace zSpace;
using namespace std;

/*!<Objects*/
zUtilsCore core;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;

int nV, nF;
zDoubleArray out_pD1, out_pD2 , out_pV1, out_pV2;

zIntArray pCounts, pConnects;
zDoubleArray vPositions;
zIntArray triCounts, triConnects;

zVectorArray pDir1, pDir2;

zPoint* positions;

zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);

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
	fnMesh.from("data/test_minSrf _kangaroo.obj", zOBJ);
	
	//fnMesh.triangulate();

	fnMesh.getPolygonData(pConnects, pCounts);

	positions = fnMesh.getRawVertexPositions();
	vPositions.assign(fnMesh.numVertices() * 3, double());

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		vPositions[i * 3 + 0] = positions[i].x;
		vPositions[i * 3 + 1] = positions[i].y;
		vPositions[i * 3 + 2] = positions[i].z;
	}

	zInt2DArray fTriangles;
	fnMesh.getMeshTriangles(fTriangles);

	
	int nTrisTotal = 0;
	for (int i = 0; i < fTriangles.size(); i++)
	{
		int nTris = floor(fTriangles[i].size() / 3);
		triCounts.push_back(nTris);

		for (int j = 0; j < fTriangles[i].size(); j++)
		{
			triConnects.push_back(fTriangles[i][j]);
			//printf("\n %i ", fTriangles[i][j]);
		}

		nTrisTotal += nTris;
	}
	
	printf("\n nTris %i ", nTrisTotal);
	computeMesh_initialise(&vPositions[0], &pCounts[0], &pConnects[0], &triCounts[0], &triConnects[0], fnMesh.numVertices(), fnMesh.numPolygons());


	out_pD1.assign(fnMesh.numVertices() * 3, double());
	out_pD2.assign(fnMesh.numVertices() * 3, double());
	out_pV1.assign(fnMesh.numVertices(), double());
	out_pV2.assign(fnMesh.numVertices(), double());
	
	nV = fnMesh.numVertices();
	nF = fnMesh.numPolygons();

	pDir1.assign(2 * nV, zVector());
	pDir2.assign(2 * nV, zVector());

	
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(false, true, false);

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
		curvatureDirections(&out_pV1[0], &out_pV2[0], &out_pD1[0], &out_pD2[0]);

		for (int i = 0; i < nV; i++)
		{
			pDir1[i * 2].x = out_pD1[i * 3 + 0];
			pDir1[i * 2].y = out_pD1[i * 3 + 1];
			pDir1[i * 2].z = out_pD1[i * 3 + 2];

			pDir1[i * 2 + 1] = pDir1[i * 2] * -1;
		
			pDir2[i * 2].x = out_pD2[i * 3 + 0];
			pDir2[i * 2].y = out_pD2[i * 3 + 1];
			pDir2[i * 2].z = out_pD2[i * 3 + 2];

			pDir2[i * 2 + 1] = pDir2[i * 2] * -1;
			
		}

		zDoubleArray out_HN, out_HV;
		zFnMesh fnMesh(oMesh);
		out_HN.assign(fnMesh.numVertices() * 3, double());
		out_HV.assign(fnMesh.numVertices(), double());

		meanCurvature(&out_HV[0], &out_HN[0]);



		zDomainFloat mCurvature(100000, -100000);
		zDomainFloat mCurvature1(100000, -100000);

		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			int i = v.getId();

			if (!v.onBoundary())
			{
				float H = (out_pV1[i] + out_pV2[i]) * 0.5;
				if (H < mCurvature.min) mCurvature.min = H;
				if (H > mCurvature.max) mCurvature.max = H;

				if (out_HV[i] < mCurvature1.min) mCurvature1.min = out_HV[i];
				if (out_HV[i] > mCurvature1.max) mCurvature1.max = out_HV[i];

			}

		}

		printf("\n mean curv %1.6f  %1.6f \n", mCurvature.min, mCurvature.max);
		printf("\n mean curv1 %1.6f  %1.6f \n", mCurvature1.min, mCurvature1.max);

		display = true;

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
		// zspace model draw
		for (int i = 0; i < nV; i++)
		{
			zPoint p0 = positions[i];
			zPoint p1 = p0 + pDir1[i * 2 + 0];
			zPoint p2 = p0 + pDir1[i * 2 + 1];

			zPoint p3 = p0 + pDir2[i * 2 + 0];
			zPoint p4 = p0 + pDir2[i * 2 + 1];

			
			model.displayUtils.drawLine(p0, p1, RED);
			model.displayUtils.drawLine(p0, p2, RED);
			model.displayUtils.drawLine(p0, p3, BLUE);
			model.displayUtils.drawLine(p0, p4, BLUE);
		}
		
	}

	model.draw();

	


	//////////////////////////////////////////////////////////

	setup2d();

	//glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
