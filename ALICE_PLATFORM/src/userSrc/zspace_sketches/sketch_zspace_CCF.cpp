#define _MAIN_

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

bool compute = false;
bool d_inMesh = true;
bool d_paramMesh = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
zUtilsCore core;

/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;
zObjMesh oMesh_in;
zFnMeshDynamics fnDyMesh;
zTsMeshParam myMeshParam;

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

	zFnMesh fnMesh_in(oMesh_in);
	fnMesh_in.from("data/pSolver/ccf_corner.obj", zOBJ);
	fnMesh_in.setEdgeColor(zColor(0.5, 0.5, 0.5, 1));

	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/pSolver/ccf_corner.obj", zOBJ);
	

	fnDyMesh.create(oMesh, false);

	fnDyMesh.setVertexWeight(5);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_in);

	// set display element booleans
	//zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	//o_paramMesh->setDisplayElements(false, true, false);

	oMesh.setDisplayElements(true, true, false);
	oMesh_in.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[2].attachToVariable(&d_paramMesh);

}

void update(int value)
{
	if (compute)
	{
		//// Harmonic Parameterization
		//myMeshParam.computeParam_Harmonic();

		//// ARAP Parameterization
		/*myMeshParam.computeParam_ARAP();

		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();

		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();

		for (zItMeshEdge e(*o_paramMesh); !e.end(); e++)
		{
			int eID = e.getId();

			zItMeshEdge e_inMesh(*o_inMesh, eID);

			if (abs(e.getLength() - e_inMesh.getLength()) > 0.01)
			{
				e.setColor(zColor(1, 0, 0, 1));
			}
			else e.setColor(zColor(0, 1, 0, 1));
		}*/


		

		double tol = 0.001;
		zDoubleArray devs_planarity;
		bool exit_planar;
			
		/*fnDyMesh.addPlanarityForce(zVolumePlanar, tol, devs_planarity, exit_planar);

		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol) f.setColor(zColor(0, 1, 0, 1));
			else f.setColor(zColor(1, 0, 1, 1));
		}*/

		bool exit_gaussian;
		zDoubleArray devs_gaussian;
		fnDyMesh.addGaussianForce(tol, devs_gaussian, exit_gaussian);

		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			if (devs_gaussian[v.getId()] < tol) v.setColor(zColor(1, 1, 1, 1));
			else v.setColor(zColor(0, 0, 0, 1));
		}

		float vGauss_max, vGauss_min;
		vGauss_max = core.zMax(devs_gaussian);
		vGauss_min = core.zMin(devs_gaussian);

		printf("\n gauss devs : %1.4f %1.4f ", vGauss_max, vGauss_min);

		fnDyMesh.update(0.1,zRK4,true,true, true);
		//fnMesh.ad


		//zFnMesh fnParam(*o_paramMesh);
		//fnParam.to("data/Parameterization/paramMesh.obj", zOBJ);

		// LSCM Parameterization
		//myMeshParam.computeParam_LSCM();

		// N ROSY
		//myMeshParam.compute_NRosy();

		// Geodesics heat method
		/*zIntArray ids = { 0,5 };
		zFloatArray geodesics;
		myMeshParam.computeGeodesics_Heat(ids, geodesics);*/

		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	/*if (d_inMesh)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		o_inMesh->draw();
		
	}

	if (d_paramMesh)
	{
		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
		o_paramMesh->draw();

	}*/
	
	
	model.draw();


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
