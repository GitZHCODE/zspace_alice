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

bool computeFRAMES = false;
bool computeSDF = false;
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
int resX = 256;
int resY = 256;

bool dSliceMesh = true;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = false;
bool dField = false;
bool exportSDF = false;

bool displayAllGraphs = false;
int currentGraphId = 0;
int totalGraphs = 0;

float printPlaneSpace = 0.1;
float printLayerWidth = 0.08;

zDomainFloat printHeightDomain(0.15, 0.15);


int SDFFunc_Num = 1;
bool SDFFunc_NumSmooth = 2;

int numSDFLayers = 3;
bool allSDFLayers = true;

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
	fnSliceMesh.from("data/Slicer/printMesh_1.json", zJSON);
	fnSliceMesh.setEdgeColor(zColor(0.8, 0.8, 0.8, 1));

	json j;
	bool chk = core.readJSON("data/Slicer/printMesh_1.json", j);

	zDoubleArray edgeCreases;
	core.readJSONAttribute(j, "EdgeCreaseData", edgeCreases);

	for (zItMeshEdge e(oSliceMesh); !e.end(); e++)
	{
		int eID = e.getId();
		if (edgeCreases[eID] > 0) e.setColor(zColor(1, 0, 1, 1));
	}

	zFnMesh fnPrintPlaneMesh(oPrintPlaneMesh);
	fnPrintPlaneMesh.from("data/Slicer/printPlanesMesh_1.obj", zOBJ);

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
	zTransform sPlane = core.getTransformFromVectors(O, X, Y, Z);

	cout << endl << sPlane;

	Z = fNorms[1];
	O = fCens[1];
	X = tempY ^ Z;
	X.normalize();
	Y = Z ^ X;
	Y.normalize();
	zTransform ePlane = core.getTransformFromVectors(O, X, Y, Z);

	cout << endl << ePlane;

	// make graph
	zIntArray eConnects = { 0,1 };
	zFnGraph fnGraph(oGuideGraph);
	fnGraph.create(fCens, eConnects);

	//set slicer
	mySlicer.setSliceMesh(oSliceMesh, true);
	mySlicer.setStartEndPlanes(sPlane, ePlane, true);
	mySlicer.setMedialGraph(oGuideGraph);

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

	B.addButton(&computeFRAMES, "computeFRAMES");
	B.buttons[0].attachToVariable(&computeFRAMES);

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

	if (computeFRAMES)
	{
		printf("\n ----------- \n Block \n");
		mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, true, false);


		computeFRAMES = !computeFRAMES;
	}

	if (computeSDF)
	{
		mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, false, true);

		computeSDF = !computeSDF;
	}

	if (exportSDF)
	{

		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockContourGraphs(numGraphs);
		
		string folderName = "data/Slicer/Contours";
		for (const auto& entry : std::filesystem::directory_iterator(folderName)) std::filesystem::remove_all(entry.path());

		int graphID = 0;
		for (auto& g : graphs)
		{
			zFnGraph fnIsoGraph(*g);
			if (fnIsoGraph.numVertices() == 0)
			{
				graphID++;
				continue;
			}

			string outName1 = folderName + "/outContour_";
			outName1 += to_string(graphID) + ".json";

			fnIsoGraph.to(outName1, zJSON);
			graphID++;
			
		}
		
		printf("\n %i graphs exported. ", graphID);
		exportSDF = !exportSDF;
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

		totalGraphs = numGraphs;

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

	drawString("Total Layers #:" + to_string(totalGraphs), vec(winW - 350, winH - 800, 0));
	drawString("Current Layer #:" + to_string(currentGraphId), vec(winW - 350, winH - 775, 0));

	
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computeFRAMES = true;;
	if (k == 'o') computeSDF = true;;

	if (k == 'w')
	{
		if (currentGraphId < totalGraphs - 1)currentGraphId++;;
	}
	if (k == 's')
	{
		if(currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') displayAllGraphs = !displayAllGraphs;

	if (k == 'e') exportSDF = true;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
