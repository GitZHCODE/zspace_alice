//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oSliceMesh;
zObjMesh oPrintPlaneMesh;
zObjGraph oGuideGraph;

zDomainFloat neopreneOffset(0, 0);

zDomain<zPoint> bb(zPoint(-7, -7, 0), zPoint(7, 7, 0));
int resX = 512;
int resY = 512;

bool dSliceMesh = true;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = false;
bool dField = false;

bool displayAllGraphs = false;
int currentGraphId = 0;

float printPlaneSpace = 2;
float printLayerWidth = 0.02;
float raftLayerWidth = 0.03;

/*!<Tool sets*/
zTsSDFSlicer mySlicer;

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
	zFnMesh fnSliceMesh(oSliceMesh);
	fnSliceMesh.from("data/Slicer/printMesh.obj", zOBJ);
	fnSliceMesh.setEdgeColor(zColor(0.8, 0.8, 0.8, 1));

	zFnMesh fnPrintPlaneMesh(oPrintPlaneMesh);
	fnPrintPlaneMesh.from("data/Slicer/printPlanesMesh.obj", zOBJ);

	// compute planes
	zVector tempY(0, 1, 0);

	zPointArray fCens;
	fnPrintPlaneMesh.getCenters(zFaceData, fCens);

	zPointArray fNorms;
	fnPrintPlaneMesh.getFaceNormals(fNorms);
		
	zVector Z = fNorms[0];
	zPoint O = fCens[0];
	zVector X = tempY ^ Z;
	X.normalize();
	zVector Y = Z ^ X;
	Y.normalize();	
	zTransform sPlane = mySlicer.setTransformFromVectors(O, X, Y, Z);

	Z = fNorms[1];
	O = fCens[1];
	X = tempY ^ Z;
	X.normalize();
	Y = Z ^ X;
	Y.normalize();
	zTransform ePlane = mySlicer.setTransformFromVectors(O, X, Y, Z);

	// make graph
	zIntArray eConnects = { 0,1 };
	zFnGraph fnGraph(oGuideGraph);
	fnGraph.create(fCens, eConnects);

	//set slicer
	mySlicer.setSliceMesh(oSliceMesh);
	mySlicer.setStartEndPlanes(sPlane, ePlane);
	mySlicer.setGuideGraph(oGuideGraph);

	//--- FIELD
	mySlicer.createFieldMesh(bb, resX, resY);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oSliceMesh);
	model.addObject(oPrintPlaneMesh);

	// set display element booleans
	oSliceMesh.setDisplayElements(false, true, false);
	oPrintPlaneMesh.setDisplayElements(true, true, false);

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

	B.addButton(&dSliceMesh, "dSliceMesh");
	B.buttons[2].attachToVariable(&dSliceMesh);

	B.addButton(&dPrintPlane, "dPrintPlane");
	B.buttons[3].attachToVariable(&dPrintPlane);

	B.addButton(&dSectionGraphs, "dSectionGraphs");
	B.buttons[4].attachToVariable(&dSectionGraphs);

	B.addButton(&dContourGraphs, "dContourGraphs");
	B.buttons[5].attachToVariable(&dContourGraphs);

	B.addButton(&dField, "dField");
	B.buttons[6].attachToVariable(&dField);
	
}

void update(int value)
{
	oSliceMesh.setDisplayObject(dSliceMesh);
	oPrintPlaneMesh.setDisplayObject(dPrintPlane);

	if (compute)
	{
		mySlicer.computePrintBlocks(printPlaneSpace, printLayerWidth, raftLayerWidth, neopreneOffset,true,true);

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
		model.draw();
		
	}

	if (dSectionGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockSectionGraphs(numGraphs);

		if (displayAllGraphs)
		{
			for (auto& g : graphs)
			{
				g->draw();
			}
		}
		else
		{
			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{				
				graphs[i]->draw();
			}

		}
		

		vector<zTransform> transforms = mySlicer.getBlockFrames();
		for (int k = 0; k < transforms.size(); k++) model.displayUtils.drawTransform(transforms[k],0.05);
		
	}

	if (dContourGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockContourGraphs(numGraphs);

		if (displayAllGraphs)
		{
			for (auto& g : graphs)
			{
				g->draw();
			}
		}
		else
		{
			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i]->draw();
			}

		}
	}

	if (dField)
	{
		zObjMeshScalarField* o_field = mySlicer.getRawFieldMesh();

		o_field->setDisplayElements(false, false, true);
		o_field->draw();
				
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') currentGraphId++;;
	if (k == 's')
	{
		if(currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') displayAllGraphs = !displayAllGraphs;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
