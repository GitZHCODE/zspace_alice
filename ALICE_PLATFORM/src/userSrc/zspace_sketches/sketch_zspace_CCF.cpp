//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include <igl/avg_edge_length.h>
#include <igl/cotmatrix.h>
#include <igl/invert_diag.h>
#include <igl/massmatrix.h>
#include <igl/parula.h>
#include <igl/per_corner_normals.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>
#include <igl/principal_curvature.h>
#include <igl/gaussian_curvature.h>
#include <igl/read_triangle_mesh.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool computePlanar = false;
bool computeDev = false;
bool computeCCF = false;
bool exportMESH = false;

bool d_inMesh = true;
bool d_paramMesh = true;

double background = 0.35;

int numIterations = 1;

////// --- zSpace Objects --------------------------------------------------
zUtilsCore core;

/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;
zObjMesh oMesh_in;
zFnMeshDynamics fnDyMesh;
zTsMeshParam myMeshParam;

double tol_dev = 0.000001;
double tol_planar = 0.001;
float dT = 0.1;

zVectorArray forceDir_dev;
zVectorArray forceDir_planar;

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

	string path = "data/pSolver/ccf_corner.obj";
	//string path = "data/pSolver/DF_Y_shape.obj";

	zFnMesh fnMesh_in(oMesh_in);
	fnMesh_in.from(path, zOBJ);
	fnMesh_in.setEdgeColor(zColor(0.5, 0.5, 0.5, 1));

	zFnMesh fnMesh(oMesh);
	fnMesh.from(path, zOBJ);
	

	fnDyMesh.create(oMesh, false);

	zIntArray fixedVerts = { 0,1,5,6,7,8,9,12,14,15,16,17 };
	fnDyMesh.setFixed(fixedVerts);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_in);

	// set display element booleans
	//zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	//o_paramMesh->setDisplayElements(false, true, false);

	oMesh.setDisplayElements(true, true, true);
	oMesh_in.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	
	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[0].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[1].attachToVariable(&d_paramMesh);

	//B.addButton(&compute, "compute");
	//B.buttons[2].attachToVariable(&compute);

}

void update(int value)
{
	

	if (computePlanar)
	{
				
		zDoubleArray devs_planarity;
		bool exit_planar;
			
		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addPlanarityForce(1.0, tol_planar, zVolumePlanar, devs_planarity, forceDir_planar, exit_planar);

			fnDyMesh.update(dT, zRK4, true, true, true);
			
		}
		
		// Planar deviations
		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_planar) f.setColor(zColor(0, 1, 0, 1));
			else f.setColor(zColor(1, 0, 1, 1));
		}

		float fPlanar_max, fPlanar_min;
		fPlanar_max = core.zMax(devs_planarity);
		fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);
		

		computePlanar = !computePlanar;
	}

	if (computeDev)
	{

		bool exit_gaussian;
		zDoubleArray devs_gaussian;		

		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addDevelopabilityForce(0.1, tol_dev, devs_gaussian, forceDir_dev, exit_gaussian);			
			fnDyMesh.update(dT, zRK4, true, true, true);
			
		}
		
		// Gaussian deviations
		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			if (devs_gaussian[v.getId()] < tol_dev) v.setColor(zColor(0, 0, 1, 1));
			else v.setColor(zColor(1, 0, 0, 1));

			//if (!v.onBoundary())printf("\n %i %1.6f ", v.getId(), devs_gaussian[v.getId()]);
		}

		float vGauss_max, vGauss_min;
		vGauss_max = core.zMax(devs_gaussian);
		vGauss_min = core.zMin(devs_gaussian);
		printf("\n gauss devs : %1.6f %1.6f \n", vGauss_max, vGauss_min);
	

		computeDev = !computeDev;
	}

	if (computeCCF)
	{
		bool exit_planar;
		zDoubleArray devs_planarity;

		bool exit_gaussian;
		zDoubleArray devs_gaussian;


		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addPlanarityForce(1.0, tol_planar, zVolumePlanar, devs_planarity, forceDir_planar, exit_planar);
			fnDyMesh.addDevelopabilityForce(0.1, tol_dev, devs_gaussian, forceDir_dev, exit_gaussian);

			fnDyMesh.update(dT, zRK4, true, true, true);
		}

		// Planar deviations
		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_planar) f.setColor(zColor(0, 1, 0, 1));
			else f.setColor(zColor(1, 0, 1, 1));
		}

		float fPlanar_max, fPlanar_min;
		fPlanar_max = core.zMax(devs_planarity);
		fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);

		// Gaussian deviations
		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			if (devs_gaussian[v.getId()] < tol_dev) v.setColor(zColor(0, 0, 1, 1));
			else v.setColor(zColor(1, 0, 0, 1));
		}

		float vGauss_max, vGauss_min;
		vGauss_max = core.zMax(devs_gaussian);
		vGauss_min = core.zMin(devs_gaussian);
		printf("\n gauss devs : %1.6f %1.6f \n", vGauss_max, vGauss_min);

		computeCCF = !computeCCF;
	}

	if (exportMESH)
	{	
		fnDyMesh.to("data/pSolver/ccf_corner_out.json", zJSON);		

		exportMESH = !exportMESH;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	
	zPoint* vPositions = fnDyMesh.getRawVertexPositions();
	
	for (int i = 0; i < forceDir_planar.size(); i++)
	{
		zPoint v2 = vPositions[i] + (forceDir_planar[i] * 0.1);
		model.displayUtils.drawLine(vPositions[i], v2, BLUE);
	}

	for (int i = 0; i < forceDir_dev.size(); i++)
	{
		zPoint v2 = vPositions[i] + (forceDir_dev[i] * 0.1);
		model.displayUtils.drawLine(vPositions[i], v2, RED);
	}
	
	model.draw();


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computePlanar = true;;	

	if (k == 'd') computeDev = true;;

	if (k == 'c') computeCCF = true;

	if (k == 'e') exportMESH = true;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
