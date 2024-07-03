//#define _MAIN_

#ifdef _MAIN_

#include "main.h"
#include <chrono>

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <headers/zModules/projectionsolver/zSpace_MinimalSurfaceSolver.h>

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

double tol = 0.001;

//double dT = 1.0;
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
	fnInMesh.from("data/test_minSrf.obj", zOBJ);
	fnInMesh.setEdgeColor(zColor(1, 0, 0, 1));

	zFnMesh fnMesh(o_Mesh);
	fnMesh.from("data/test_minSrf.obj", zOBJ);

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
	out_deviations.assign(fnMesh.numVertices(), double());

	computeMesh_initialise(&vPositions[0], &pCounts[0], &pConnects[0], &triCounts[0], &triConnects[0],fnMesh.numVertices(), fnMesh.numPolygons());
	
	// fixed vertices
	zIntArray fixedVertices;
	zIntArray cVertexIDs;
	zIntArray cVertsCount;

	for (zItMeshVertex v(o_Mesh); !v.end(); v++)
	{
		if (v.onBoundary())
		{
			//printf("\n %i ", v.getId());
			fixedVertices.push_back(v.getId());
		}

		zIntArray cVs;
		v.getConnectedVertices(cVs);
		
		cVertsCount.push_back(cVs.size());
		for (auto vId : cVs) cVertexIDs.push_back(vId);

	}

	computeMesh_setFixed(&fixedVertices[0], fixedVertices.size());

	// connected vertices
	computeMesh_setCVertices(&cVertexIDs[0], &cVertsCount[0], fnMesh.numVertices());


	cout << endl << "dT : " <<dT ;
	computeMesh_setDT(0.1);
	cout << endl << "dT : " << dT;


	// Start measuring time
	vector<double> sortVals = vPositions;
	auto begin = std::chrono::high_resolution_clock::now();

	
	std::sort(sortVals.begin(), sortVals.end());
	
	// Stop measuring time and calculate the elapsed time
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

	printf("\n STD Sort Time measured: %.7f seconds.", elapsed.count() * 1e-9);
	printf("\n min %1.2f max %1.2f.\n", sortVals[0], sortVals[sortVals.size() -1]);
	
	vector<double> sortVals1 = vPositions;
	auto begin1 = std::chrono::high_resolution_clock::now();
	double temp = 0;
	for (int i = 0; i < sortVals1.size(); i++)
	{
		for (int j = 0; j < sortVals1.size(); j++)
		{
			if (sortVals1[i] < sortVals1[j])
			{
				swap(sortVals1[i], sortVals1[j]);
			}
		}
	}

	// Stop measuring time and calculate the elapsed time
	auto end1 = std::chrono::high_resolution_clock::now();
	auto elapsed1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - begin1);

	printf("\n Sort Time measured: %.7f seconds.\n", elapsed1.count() * 1e-9);
	printf(" min %1.2f max %1.2f.\n", sortVals1[0], sortVals1[sortVals.size() - 1]);

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
		bool out;
		out = computeMesh_minSrf(100, true, tol, &out_vPositions[0], &out_deviations[0]);

		zPoint* positions = fnMesh.getRawVertexPositions();

		zDomainFloat mCurvature(100000, -100000);
		
		for (zItMeshVertex v(o_Mesh); !v.end(); v++)
		{
			int i = v.getId();
			positions[i].x = out_vPositions[i * 3 + 0];
			positions[i].y = out_vPositions[i * 3 + 1];
			positions[i].z = out_vPositions[i * 3 + 2];

			if (!v.onBoundary())
			{
				if (out_deviations[i] < mCurvature.min) mCurvature.min = out_deviations[i];
				if (out_deviations[i] > mCurvature.max) mCurvature.max = out_deviations[i];

			}
			
		}

		/*zColorArray vCols;
		zDomainColor colDomain(zColor(0, 1, 0, 1), zColor(1, 0, 0, 1));
		for (zItMeshVertex v(o_Mesh); !v.end(); v++)
		{
			int i = v.getId();
			if (!v.onBoundary())
			{
				zColor col = core.blendColor(out_deviations[i], mCurvature, colDomain, zHSV);
				vCols.push_back(col);
			}
			else
			{
				vCols.push_back(zColor());
			}
			
		}

		fnMesh.setVertexColors(vCols, true);*/

		printf("\n mean curv %1.6f  %1.6f \n", mCurvature.min, mCurvature.max);

		
		
		compute = !out;
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

	if (k == 'e')
	{
		//export
		zFnMesh fnMesh(o_Mesh);
		fnMesh.to("data/outMinSrf.obj", zOBJ);
	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
