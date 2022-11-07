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

zUtilsCore core;


bool computeHEIGHTS_Folder = false;
bool computeFRAMES = false;
bool computeALL = false;
bool display = true;
bool readBLOCK = false;

bool computeTRANSFORM = false;
bool toLOCAL = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/


zObjMesh o_centerMesh;

int blockStride = 4;
int braceStride = 1;

double blockID = 13;

int SDFFunc_Num = 5;
bool SDFFunc_NumSmooth = 2;

int numSDFLayers = 159;
bool allSDFLayers = true;




zDomainFloat neopreneOffset(0.012, 0.012);

bool deckBlock = true;
zDomain<zPoint> bb;
zDomain<zPoint> bb_current;

int resX = 512;
int resY = 512;

string fileDir = "data/Block_Slicer/";
string exportPath = "data/Block_Slicer/OutCurves/";



bool dGuideMesh = true;
bool dSliceMesh_Left = true;
bool dSliceMesh_Right = true;
bool dMedialGraph = true;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = false;
bool dTrimGraphs = false;
bool dField = false;

bool exportContours = false;

bool displayAllGraphs = false;
int currentGraphId = 0;
int totalGraphs = 0;
bool frameCHECKS = false;

float printPlaneSpace = 0.008;
float printLayerWidth = 0.048;
float raftLayerWidth = 0.048;

zDomainFloat printHeightDomain(0.012, 0.012);

/*!<Tool sets*/
zTsSDFSlicer mySlicer;

////// --- CUSTOM METHODS ----------------------------------------------------

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

void readBlockAttributes(string dir, int _blockID, int* medialVertices, int* leftPlaneFaces, int* rightPlaneFaces, int& blockStride, int& braceStride)
{
	bool deckBlock;

	string fileDeck = dir + "deck_" + to_string(_blockID) + ".txt";
	bool checkDeck = core.fileExists(fileDeck);
	if (checkDeck) deckBlock = true;
	if (checkDeck) printf("\n %s exists", fileDeck.c_str());
	else printf("\n %s doesnt exists", fileDeck.c_str());

	string fileBalustrade = dir + "balustrade_" + to_string(_blockID) + ".txt";
	bool checkBalustrade = core.fileExists(fileBalustrade);
	if (checkBalustrade) deckBlock = false;
	if (checkBalustrade) printf("\n %s exists", fileBalustrade.c_str());
	else printf("\n %s doesnt exists", fileBalustrade.c_str());


	string infilename = (checkDeck) ? fileDeck : fileBalustrade;
	bool fileChk = core.fileExists(infilename);

	if (!fileChk) return;

	ifstream myfile;
	myfile.open(infilename.c_str());

	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		vector<string> perlineData = core.splitString(str, ",");

		if (perlineData.size() > 0)
		{
			leftPlaneFaces[0] = atoi(perlineData[2].c_str());
			leftPlaneFaces[1] = atoi(perlineData[3].c_str());

			rightPlaneFaces[0] = atoi(perlineData[0].c_str());
			rightPlaneFaces[1] = atoi(perlineData[1].c_str());

			medialVertices[0] = atoi(perlineData[4].c_str());
			medialVertices[1] = atoi(perlineData[5].c_str());

			// blockid  = atoi(perlineData[6].c_str());

			blockStride = atoi(perlineData[7].c_str());
			braceStride = atoi(perlineData[8].c_str());
		}
	}

	myfile.close();
}

void readBlocks(string dir, zTsSDFSlicer& _mySLicer, int _blockID, int* medialVertices, int* leftPlaneFaces, int* rightPlaneFaces, int& blockStride, int& braceStride, bool &_toLOCAL)
{
	
	readBlockAttributes(dir, _blockID, &medialVertices[0], &leftPlaneFaces[0], &rightPlaneFaces[0], blockStride, braceStride);

	mySlicer.setFromOBJ(dir, _blockID, medialVertices, leftPlaneFaces, rightPlaneFaces, blockStride, braceStride);

	//mySlicer.setFromJSON(fileDir, bID);

	deckBlock = mySlicer.onDeckBlock();
	bb = (!deckBlock) ? zDomain<zPoint>(zPoint(-0.9, -0.35, 0), zPoint(0.9, 1.5, 0)) : zDomain<zPoint>(zPoint(-1.5, -0.5, 0), zPoint(1.5, 0.5, 0));

	//--- FIELD
	if (bb_current.min != bb.min)
	{
		bb_current = bb;
		printf("\n bounds updated \n", blockID);
		mySlicer.createFieldMesh(bb_current, resX, resY);
	}

	_toLOCAL = true;
}

void exportPaths(zTsSDFSlicer &_mySLicer, int _blockID, string _exportPath, bool &_toLOCAL)
{
	bool chkDir = dirExists(_exportPath);
	if (!chkDir) _mkdir(_exportPath.c_str());


	string filename = _exportPath;
	filename += "/" + to_string(_blockID);

	bool chkSubDir = dirExists(filename);
	if (!chkSubDir) _mkdir(filename.c_str());

	bool chkTransform = false;

	if (!_toLOCAL)
	{
		_mySLicer.setTransforms(_toLOCAL);
		_toLOCAL = !_toLOCAL;

		chkTransform = true;
	}

	int numGraphs = 0;
	zObjGraphPointerArray graphs = _mySLicer.getBlockSectionGraphs(numGraphs);
	for (const auto& entry : std::filesystem::directory_iterator(filename)) std::filesystem::remove_all(entry.path());

	int graphID = 0;
	for (auto& g : graphs)
	{
		zFnGraph fnIsoGraph(*g);
		if (fnIsoGraph.numVertices() == 0)
		{
			graphID++;
			continue;
		}

		string outName1 = filename + "/outContour_";
		outName1 += to_string(graphID) + ".json";

		fnIsoGraph.to(outName1, zJSON);
		graphID++;

	}

	printf("\n %i graphs exported. ", graphID);

	if (chkTransform)
	{
		_mySLicer.setTransforms(_toLOCAL);
		_toLOCAL = !_toLOCAL;
	}
}

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);	

	bb_current = zDomain<zPoint>(zPoint(-1.5, -0.5, 0), zPoint(1.5, 0.5, 0));

	//--- FIELD
	mySlicer.createFieldMesh(bb_current, resX, resY);
		
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

	o_centerMesh.setDisplayElements(false, true, true);
	model.addObject(o_centerMesh);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	
	S.addSlider(&background, "blockID");
	S.sliders[1].attachToVariable(&blockID, 0, 110);

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

	B.addButton(&dTrimGraphs, "dTrimGraphs");
	B.buttons[9].attachToVariable(&dTrimGraphs);

	B.addButton(&dField, "dField");
	B.buttons[10].attachToVariable(&dField);
	
}

void update(int value)
{
	if (readBLOCK)
	{
		int bID = (int) blockID;

		zInt2 medialVertices;
		zInt2 leftPlaneFaces;
		zInt2 rightPlaneFaces;
		blockStride = 1;
		braceStride = 1;

		readBlocks(fileDir, mySlicer, bID, medialVertices, leftPlaneFaces, rightPlaneFaces, blockStride, braceStride, toLOCAL);

		readBLOCK = !readBLOCK;
	}

	if (computeHEIGHTS_Folder)
	{

		mySlicer.checkPrintLayerHeights_Folder(fileDir, printHeightDomain, neopreneOffset);
		computeHEIGHTS_Folder = !computeHEIGHTS_Folder;
	}

	if (computeFRAMES)
	{
		printf("\n ----------- \n BlockID %i \n", blockID);
		mySlicer.computePrintBlocks(printHeightDomain, printLayerWidth, raftLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, neopreneOffset, true, false);
	

		computeFRAMES = !computeFRAMES;
	}

	if (computeALL)
	{
		int numFiles = core.getNumfiles_Type( fileDir, zOBJ);
		printf("\n numFiles : %i ", numFiles);

		zInt2 medialVertices;
		zInt2 leftPlaneFaces;
		zInt2 rightPlaneFaces;
		blockStride = 1;
		braceStride = 1;

		for (int i =0; i< numFiles; i++)
		{
			int bID = i;

			//read blocks
			readBlocks(fileDir, mySlicer, bID, medialVertices, leftPlaneFaces, rightPlaneFaces, blockStride, braceStride, toLOCAL);

			// compute frames
			mySlicer.computePrintBlocks(printHeightDomain, printLayerWidth, raftLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, neopreneOffset, true, false);

			// export
			exportPaths(mySlicer, bID, exportPath, toLOCAL);

		}

		computeALL = !computeALL;
	}

	if (computeTRANSFORM)
	{
		mySlicer.setTransforms(toLOCAL);
		toLOCAL = !toLOCAL;

		computeTRANSFORM = !computeTRANSFORM;
	}

	if (exportContours)
	{
		exportPaths(mySlicer, blockID, exportPath, toLOCAL);		

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

	zTransform* tLeft = mySlicer.getRawBlockStartEnd(true);
	zTransform* tRight = mySlicer.getRawBlockStartEnd(false);

	for (int k = 0; k < 2; k++)
	{
		if(tLeft) model.displayUtils.drawTransform(tLeft[k], 0.5);
		if(tRight) model.displayUtils.drawTransform(tRight[k], 0.5);
	}

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

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));
	


	drawString("KEY Press", vec(winW - 350, winH - 650, 0));
	
	drawString("r - read block", vec(winW - 350, winH - 625, 0));
	drawString("h - compute Heights_Folder", vec(winW - 350, winH - 600, 0));
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

	if (k == 'r') readBLOCK = true;
	if (k == 'h') computeHEIGHTS_Folder = true;;
	if (k == 'p') computeFRAMES = true;;
	if (k == 'o') computeALL = true;;
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
