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

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zUtilsCore core;
zObjMesh oMesh;
zFnMeshDynamics fnDy;

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
	string path = "data/nansha/0.6Max-v1.json";
	zFnMesh fnMesh_in(oMesh);
	fnMesh_in.from(path, zJSON);
	
	
	fnDy.create(oMesh, false);

	

	// planarity
	//zDoubleArray planarDevs;
	//fnMesh_in.getPlanarityDeviationPerFace(planarDevs, zPlanarSolverType::zQuadPlanar, true, 0.01);

	//float min_pl = core.zMin(planarDevs);
	//float max_pl = core.zMax(planarDevs);
	//zDomainFloat pl_domain(min_pl, max_pl);

	//zDomainColor col_domain(zRED, zBLUE);

	//for (zItMeshFace f(oMesh); !f.end(); f++)
	//{
	//	zColor v_blendColor = core.blendColor(planarDevs[f.getId()], pl_domain, col_domain, zHSV);
	//	f.setColor(v_blendColor);
	//}


	// gaussian curavature
	/*zDoubleArray vertexCurvature;
	fnMesh_in.getGaussianCurvature(vertexCurvature);

	float min_gc = core.zMin(vertexCurvature);
	float max_gc = core.zMax(vertexCurvature);

	zDomainFloat gc_domain(min_gc, max_gc);
	zDomainFloat out_domain(0.0, 1.0);
	


	for (zItMeshVertex v(oMesh); !v.end(); v++)
	{
		zColor v_blendColor = core.blendColor(vertexCurvature[v.getId()], gc_domain, col_domain, zHSV);
		v.setColor(v_blendColor);

		float remapValue = core.ofMap((float) vertexCurvature[v.getId()], gc_domain, out_domain);
		vertexCurvature[v.getId()] = remapValue;

	}

	fnMesh_in.computeFaceColorfromVertexColor();*/


	// principal Curvature

	/*zCurvatureArray pCurvuatures;
	zVectorArray pV1, pV2;

	fnMesh_in.getPrincipalCurvatures(pCurvuatures, pV1, pV2); */
	// visualise the PC direction per vertex, color PV1 as cyan, PV2 as magenta
		
	// get centers
	/*zPointArray eCens, fCens;
	fnMesh_in.getCenters(zHEData::zEdgeData, eCens);
	fnMesh_in.getCenters(zHEData::zFaceData, fCens);*/

	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(false, true, true);
	
	//oMesh.setFaceCenters(fCens);
	//oMesh.setEdgeCenters(eCens);
	//oMesh.setDisplayElementIds(true, false, true);
	//oMesh.setDisplayFaceNormals(true, 0.1);

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
		zDoubleArray planarDevs;
		zVectorArray planar_forceDirs;
		bool planar_exit;
		
		for (int i = 0; i < 1; i++)
		{
			fnDy.addPlanarityForce(1.0, 0.0001, zVolumePlanar, planarDevs, planar_forceDirs, planar_exit, zSolverForceConstraints::zConstraintFree);
			fnDy.update(0.1, zIntergrationType::zRK4, true, true, true);
		}
		

		// check deviations
		float min_pl = core.zMin(planarDevs);
		float max_pl = core.zMax(planarDevs);
		fnDy.getPlanarityDeviationPerFace(planarDevs, zQuadPlanar, true, 0.01);
		
		printf("\n planar %1.4f %1.4f ", min_pl, max_pl);

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

	 // do for loop through all vertex positions
	// draw line between  pi & pv1

	


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
