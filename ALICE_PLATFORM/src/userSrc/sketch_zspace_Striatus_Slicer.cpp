#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool computeFRAMES = false;
bool computeSDF = false;
bool display = true;

bool computeTRANSFORM = false;
bool toLOCAL = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

int blockStride = 4;
int braceStride = 1;

int blockID = 32;

int SDFFunc_Num = 4;


zDomainFloat neopreneOffset(0.000, 0.001);

bool deckBlock = true;
zDomain<zPoint> bb;

int resX = 512;
int resY = 512;

string fileDir = "data/striatus/100_Draft/";

string filePath = "data/striatus/100_Draft/deck_3.json";
string exportBRGPath = "data/striatus/out_PrintBlock_3.json";

string exportINC3DPath = "C:/Users/vishu.b/Desktop/Striatus_IO/test/blocks";

bool dGuideMesh = true;
bool dSliceMesh_Left = true;
bool dSliceMesh_Right = true;
bool dMedialGraph = true;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = false;
bool dField = false;

bool exportContours = false;

bool displayAllGraphs = false;
int currentGraphId = 0;
int totalGraphs = 0;
bool frameCHECKS = false;

float printPlaneSpace = 0.0115;
float printLayerWidth = 0.04;
float raftLayerWidth = 0.04;

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
	//mySlicer.setFromJSON(filePath, blockStride, braceStride);
	mySlicer.setFromJSON(fileDir, blockID);

	deckBlock = mySlicer.onDeckBlock();
	bb = (!deckBlock) ? zDomain<zPoint>(zPoint(-0.6, -0.35, 0), zPoint(0.6, 1.5, 0)) : zDomain<zPoint>(zPoint(-1.5, -0.5, 0), zPoint(1.5, 0.5, 0));


	//--- FIELD
	mySlicer.createFieldMesh(bb, resX, resY);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object


	// set display element booleans
	zObjMesh* guideMesh = mySlicer.getRawGuideMesh();
	guideMesh->setDisplayElements(true, true, false);

	zObjGraph* medialGraph = mySlicer.getRawMedialGraph();
	medialGraph->setDisplayElements(true, true);

	zObjMesh* leftMesh = mySlicer.getRawLeftMesh();
	leftMesh->setDisplayElements(false, true, false);

	zObjMesh* rightMesh = mySlicer.getRawRightMesh();
	rightMesh->setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&computeFRAMES, "computeFRAMES");
	B.buttons[0].attachToVariable(&computeFRAMES);

	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	B.addButton(&dGuideMesh, "dGuideMesh");
	B.buttons[2].attachToVariable(&dGuideMesh);

	B.addButton(&dSliceMesh_Left, "dSliceMesh_Left");
	B.buttons[3].attachToVariable(&dSliceMesh_Left);

	B.addButton(&dSliceMesh_Right, "dSliceMesh_Right");
	B.buttons[4].attachToVariable(&dSliceMesh_Right);

	B.addButton(&dMedialGraph, "dMedialGraph");
	B.buttons[5].attachToVariable(&dMedialGraph);

	B.addButton(&dPrintPlane, "dPrintPlane");
	B.buttons[6].attachToVariable(&dPrintPlane);

	B.addButton(&dSectionGraphs, "dSectionGraphs");
	B.buttons[7].attachToVariable(&dSectionGraphs);

	B.addButton(&dContourGraphs, "dContourGraphs");
	B.buttons[8].attachToVariable(&dContourGraphs);

	B.addButton(&dField, "dField");
	B.buttons[9].attachToVariable(&dField);
	
}

void update(int value)
{
	

	if (computeFRAMES)
	{
		mySlicer.computePrintBlocks(printPlaneSpace, printLayerWidth, raftLayerWidth, SDFFunc_Num, neopreneOffset,true,false);

		frameCHECKS = mySlicer.checkPrintLayerHeights();

		computeFRAMES = !computeFRAMES;
	}

	if (computeSDF)
	{
		mySlicer.computePrintBlocks(printPlaneSpace, printLayerWidth, raftLayerWidth, SDFFunc_Num, neopreneOffset, false, true);

		computeSDF = !computeSDF;
	}

	if (computeTRANSFORM)
	{
		mySlicer.setTransforms(toLOCAL);
		toLOCAL = !toLOCAL;

		computeTRANSFORM = !computeTRANSFORM;
	}

	if (exportContours)
	{

		string filename = exportINC3DPath;
		filename += (!deckBlock) ? "/Balustrade" : "/Deck";

		mySlicer.exportJSON(filePath, filename,"3dp_block", printLayerWidth, raftLayerWidth);

		exportContours = !exportContours;
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

	int numMinPts =0;
	zObjPointCloud* o_minPts = mySlicer.getRawCriticalPoints(true);
	o_minPts->draw();

	if (dSliceMesh_Left)
	{
		zObjMesh* leftMesh = mySlicer.getRawLeftMesh();
		leftMesh->draw();
	}
	
	if (dSliceMesh_Right)
	{
		zObjMesh* rightMesh = mySlicer.getRawRightMesh();
		rightMesh->draw();
	}

	if (dGuideMesh)
	{
		zObjMesh* guideMesh = mySlicer.getRawGuideMesh();		
		guideMesh->draw();
	}

	if (dMedialGraph)
	{
		zObjGraph* medialGraph = mySlicer.getRawMedialGraph();
		medialGraph->draw();
	}

	if (dSectionGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockSectionGraphs(numGraphs);

		totalGraphs = (deckBlock) ? floor(numGraphs * 0.5) : numGraphs;

		if (displayAllGraphs)
		{
			for (auto& g : graphs)
			{
				g->setDisplayVertices(true);
				g->draw();
			}
		}
		else
		{
			int i = currentGraphId;
			int  end = (deckBlock) ? floor(numGraphs * 0.5) : numGraphs;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{		
				graphs[i]->setDisplayVertices(true);
				graphs[i]->draw();

				if (deckBlock)
				{
					graphs[i + end]->setDisplayVertices(true);
					graphs[i + end]->draw();
				}
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
			int  end = (deckBlock) ? floor(numGraphs * 0.5) : numGraphs;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i]->draw();

				if (deckBlock)graphs[i + end]->draw();
				
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
	
	drawString("Total Layers #:" + to_string(totalGraphs), vec(winW - 350, winH - 800, 0));
	drawString("Current Layer #:" + to_string(currentGraphId), vec(winW - 350, winH - 775, 0));

	string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));
	


	drawString("KEY Press", vec(winW - 350, winH - 600, 0));
	drawString("p - Compute Frames", vec(winW - 350, winH - 575, 0));
	drawString("o - Compute SDF", vec(winW - 350, winH - 550, 0));
	drawString("t - Transform", vec(winW - 350, winH - 525, 0));

	drawString("w - increment Current Graph ID", vec(winW - 350, winH - 500, 0));
	drawString("s - decrement Current Graph ID", vec(winW - 350, winH - 475, 0));
	drawString("d - display all graphs", vec(winW - 350, winH - 450, 0));

	drawString("e - export graphs", vec(winW - 350, winH - 425, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computeFRAMES = true;;
	if (k == 'o') computeSDF = true;;
	if (k == 't') computeTRANSFORM = true;;

	if (k == 'w')
	{
		if(currentGraphId < totalGraphs - 1)currentGraphId++;;
	}
	if (k == 's')
	{
		if(currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') displayAllGraphs = !displayAllGraphs;

	if (k == 'e') exportContours = !exportContours;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
