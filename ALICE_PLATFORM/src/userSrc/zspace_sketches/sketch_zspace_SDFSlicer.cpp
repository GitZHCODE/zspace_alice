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


void patternCurves( zObjMesh &oMesh , int startVID)
{
	zFnMesh fnMesh(oMesh);

	zItMeshVertex v(oMesh, startVID);
	zItMeshHalfEdgeArray cHEdges;

	v.getConnectedHalfEdges(cHEdges);

	zItMeshHalfEdge startHe;;

	for (auto& cHE : cHEdges)
	{
		if (cHE.onBoundary()) startHe = cHE;
	}

	zItMeshHalfEdge he = startHe;

	do
	{
	
		zItMeshHalfEdge heU = he.getSym().getNext();

		bool exit = false;
		do
		{
			if (heU.getVertex().onBoundary()) exit = true;

			heU.getEdge().setColor(zColor(1, 0, 1, 1));

			heU = heU.getNext().getSym().getNext();

		} while (!exit);

		//he.getEdge().setColor(zColor(1, 1, 0, 1));

		he = he.getNext().getNext().getNext().getNext();;

	} while (he != startHe);

}

////////

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
zObjMesh oGradientMesh;
zObjGraph oGuideGraph;

zDomainFloat neopreneOffset(0, 0);
zDomainFloat oDomain(0.0001, 0.40);

zDomain<zPoint> bb(zPoint(-10, -10, 0), zPoint(10, 10, 0));
int resX = 256;
int resY = 256;

bool dSliceMesh = true;
bool dGradientMesh = true;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = false;
bool dTrimGraphs = false;
bool dField = false;
bool exportSDF = false;

bool displayAllGraphs = false;
int currentGraphId = 0;
int totalGraphs = 0;

float printPlaneSpace = 0.1;
float printLayerWidth = 0.01;

zDomainFloat printHeightDomain(0.1, 0.1);


int SDFFunc_Num = 4;
bool SDFFunc_NumSmooth = 0;

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
	fnSliceMesh.from("data/Slicer/printMesh_v3_2.obj", zOBJ);
	fnSliceMesh.setEdgeColor(zColor(0.8, 0.8, 0.8, 1));

	patternCurves(oSliceMesh, 11);

	zFnMesh fnPrintPlaneMesh(oPrintPlaneMesh);
	fnPrintPlaneMesh.from("data/Slicer/printPlanesMesh_2.obj", zOBJ);

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

	// graadient mesh
	zFnMesh fnGradientMesh(oGradientMesh);
	fnGradientMesh.from("data/Slicer/colorMesh_topo_v3_2.obj", zOBJ);

	// read color
	json j;
	bool chk = core.readJSON("data/Slicer/colorMesh_v3_2.json", j);

	vector<zDoubleArray> faceAttributes;
	core.readJSONAttribute(j, "FaceAttributes", faceAttributes);	

	zColorArray fColors;
	if (faceAttributes.size() > 0)
	{
		for (zItMeshFace f(oGradientMesh); !f.end(); f++)
		{
			int fID = f.getId();
			if (faceAttributes[fID].size() > 0)
			{
				fColors.push_back(zColor(faceAttributes[fID][3], faceAttributes[fID][4], faceAttributes[fID][5], 1));
			}
		}
	}
	
	fnGradientMesh.setFaceColors(fColors, true);

	//set slicer
	mySlicer.setSliceMesh(oSliceMesh, true);
	mySlicer.setStartEndPlanes(sPlane, ePlane, true);
	mySlicer.setMedialGraph(oGuideGraph);

	
	mySlicer.setOffsetDomain(oDomain);

	mySlicer.setGradientTriMesh(oGradientMesh);

	

	//--- FIELD
	mySlicer.createFieldMesh(bb, resX, resY);

	//--- closest point test 
	//zPointArray testPts;
	//testPts.push_back(zPoint(5.66, 4.94, 6.40));
	//zIntArray faceIDs ;
	//zPointArray cPts;
	//mySlicer.computeClosestPointToGradientMesh(testPts, faceIDs, cPts);

	//printf("\n %i | %1.2f %1.2f %1.2f ", faceIDs[0], cPts[0].x, cPts[0].y, cPts[0].z);
		
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oSliceMesh);
	model.addObject(oPrintPlaneMesh);
	model.addObject(oGradientMesh);

	// set display element booleans
	oSliceMesh.setDisplayElements(false, true, false);
	oPrintPlaneMesh.setDisplayElements(true, true, false);		
	oGradientMesh.setDisplayElements(true, false, true);

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

	B.addButton(&dSliceMesh, "dGradientMesh");
	B.buttons[3].attachToVariable(&dGradientMesh);		

	B.addButton(&dPrintPlane, "dPrintPlane");
	B.buttons[4].attachToVariable(&dPrintPlane);

	B.addButton(&dSectionGraphs, "dSectionGraphs");
	B.buttons[5].attachToVariable(&dSectionGraphs);

	B.addButton(&dContourGraphs, "dContourGraphs");
	B.buttons[6].attachToVariable(&dContourGraphs);

	B.addButton(&dTrimGraphs, "dTrimGraphs");
	B.buttons[7].attachToVariable(&dTrimGraphs);	

	B.addButton(&dField, "dField");
	B.buttons[8].attachToVariable(&dField);
	
}

void update(int value)
{
	oSliceMesh.setDisplayObject(dSliceMesh);
	oPrintPlaneMesh.setDisplayObject(dPrintPlane);
	oGradientMesh.setDisplayObject(dGradientMesh);
	
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
		
		string folderName = "data/Slicer/out/contours";
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

		folderName = "data/Slicer/out/trims";
		for (const auto& entry : std::filesystem::directory_iterator(folderName)) std::filesystem::remove_all(entry.path());

		graphID = 0;
		zObjGraphPointerArray tGraphs = mySlicer.getBlockTrimGraphs(numGraphs);
		for (auto& g : tGraphs)
		{
			zFnGraph fnIsoGraph(*g);
			if (fnIsoGraph.numVertices() == 0)
			{
				graphID++;
				continue;
			}

			string outName1 = folderName + "/outTrim_";
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

	/*if (dGradientMesh)
	{
		zObjMesh* o_gradientMesh = mySlicer.getRawGradientMesh();
		o_gradientMesh ->draw();
	}*/

	if (dSectionGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockSectionGraphs(numGraphs);

		totalGraphs = numGraphs;

		if (displayAllGraphs)
		{
			for (auto& g : graphs)
			{
				g->setDisplayElements(true, true);
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

	if (dTrimGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockTrimGraphs(numGraphs);

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
			int  end =  numGraphs;

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
