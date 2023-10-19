#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include <headers/zCudaToolsets/field/zTsGraphSDF.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS
zUtilsCore core;

#define RES 1024
#define FIELDRES RES * RES 
#define GRAPHMEMSIZE 128
#define NUMGRAPH 300
vector<zFloatArray> gpu_inGraphs;
zPointArray outLines;


void setGraphSDFData(zTsGraphSDF &graphSDF, zTsSDFSlicer &slicer)
{
	zObjMeshScalarField* oField = slicer.getRawFieldMesh();

	zFnMeshField fnField(*oField);
	zObjMesh* oMesh = fnField.getRawMesh();

	zFnMesh fnMesh(*oMesh);
	
	int num = FIELDRES * NUMGRAPH;

	zGraph_Face* fieldFaces;
	fieldFaces  = new zGraph_Face[num];

	std::chrono::time_point<std::chrono::system_clock> s_Time = std::chrono::system_clock::now();

	for (zItMeshFace f(*oMesh); !f.end(); f++)
	{

		int id = f.getId();
		zPointArray points;
		f.getVertexPositions(points);

		for (int i = 0; i < NUMGRAPH; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				fieldFaces[id].face_vPositions[j * 3 + 0] = points[j].x;
				fieldFaces[id].face_vPositions[j * 3 + 1] = points[j].y;
				fieldFaces[id].face_vPositions[j * 3 + 2] = points[j].z;
			}	

		}
	}

	graphSDF.setField(fieldFaces, num);

	std::chrono::time_point<std::chrono::system_clock> e_Time = std::chrono::system_clock::now();

	double mSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(e_Time - s_Time).count();
	printf("\n memory compute %1.2f seconds ", mSeconds / 1000);

}

void setGraphSDFData_A(zTsGraphSDF& graphSDF, zTsSDFSlicer& slicer)
{
	zObjMeshScalarField* oField = slicer.getRawFieldMesh();

	zFnMeshField fnField(*oField);
	zObjMesh* oMesh = fnField.getRawMesh();

	zFnMesh fnMesh(*oMesh);

	int num = FIELDRES * 4 * 3;

	float* fieldFacePositions;
	fieldFacePositions = new float[num];

	zPoint *vPositions = fnMesh.getRawVertexPositions();

	std::chrono::time_point<std::chrono::system_clock> s_Time = std::chrono::system_clock::now();

	for (zItMeshFace f(*oMesh); !f.end(); f++)
	{

		int id = f.getId();
		zIntArray fVerts;
		f.getVertices(fVerts);

		
		for (int j = 0; j < 4; j++)
		{
			fieldFacePositions[id * 12 + j * 3 + 0] = vPositions[fVerts[j]].x;
			fieldFacePositions[id * 12 + j * 3 + 1] = vPositions[fVerts[j]].y;
			fieldFacePositions[id * 12 + j * 3 + 2] = vPositions[fVerts[j]].z;
		}

		
	}

	graphSDF.setField(fieldFacePositions, num);

	// compute graphs
	int numGraphs = 0;
	zObjGraphPointerArray graphs = slicer.getBlockSectionGraphs(numGraphs);

	vector<zTransform> transforms = slicer.getBlockFrames();

	

	if (numGraphs > 0)
	{
		gpu_inGraphs.clear();
		gpu_inGraphs.assign(numGraphs, zFloatArray());
		
		int maxverts = 0;
		int graphCounter = 0; 
		for (auto& g : graphs)
		{
			zFloatArray temp;
			temp.assign(GRAPHMEMSIZE, INVALID_VAL);

			int counter = 0;
			
			zItGraphHalfEdge he_start(*g, 0);
			zItGraphHalfEdge he = he_start;

			zFnGraph fnGraph(*g);
			zPoint* vPositions = fnGraph.getRawVertexPositions();
			zColor* vColors = fnGraph.getRawVertexColors();

			zTransform t = transforms[graphCounter];
			fnGraph.setTransform(t, true, false);

			zTransform tLocal;
			tLocal.setIdentity();
			fnGraph.setTransform(tLocal, true, true);

			maxverts = (fnGraph.numVertices() > maxverts) ? fnGraph.numVertices() : maxverts;

			do
			{
				if (he == he_start)
				{
					int vID = he.getStartVertex().getId();

					temp[counter + 0] = vPositions[vID].x;
					temp[counter + 1] = vPositions[vID].y;
					temp[counter + 2] = vPositions[vID].z;

					temp[counter + 3] = vColors[vID].r;
					temp[counter + 4] = vColors[vID].g;
					temp[counter + 5] = vColors[vID].b;
					counter += 6;
				}
				
				
				int vID = he.getVertex().getId();

				temp[counter + 0] = vPositions[vID].x;
				temp[counter + 1] = vPositions[vID].y;
				temp[counter + 2] = vPositions[vID].z;

				temp[counter + 3] = vColors[vID].r;
				temp[counter + 4] = vColors[vID].g;
				temp[counter + 5] = vColors[vID].b;
				
				counter += 6;
				

				he = he.getNext();
				

			} while (he != he_start);

			fnGraph.setTransform(t, true, true);

			gpu_inGraphs[graphCounter] = temp;
			graphCounter++;
		}

		printf("\n maxVerts : %i | graphs %i %i ", maxverts, graphCounter, numGraphs);
	}
	

	std::chrono::time_point<std::chrono::system_clock> e_Time = std::chrono::system_clock::now();

	double mSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(e_Time - s_Time).count();
	printf("\n set memory compute %1.2f seconds ", mSeconds / 1000);

}

// zSpace CUDA

bool GPU_COMPUTE;
string GPUString;

ZSPACE_EXTERN bool checkCudaExists(string& version);

ZSPACE_EXTERN void cleanDeviceMemory();

ZSPACE_EXTERN bool cdpSDFSlicer(zTsGraphSDF& graphSDF, float printWidth, int funcNum, bool &setup);


////////////////////////////////////////////////////////////////////////// General

bool computeHEIGHTS_Folder = false;
bool computeFRAMES = false;
bool computeSDF = false;
bool display = true;
bool readBLOCK = false;

bool gpuSETUP = false;
bool gpuSDF = false;

bool computeTRANSFORM = false;
bool toLOCAL = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

//zUtilsCore core;
zObjMesh o_centerMesh;

int blockStride = 4;
int braceStride = 1;


double blockID = 22;

int SDFFunc_Num = 1;
bool SDFFunc_NumSmooth = 2;

int numSDFLayers = 2;
bool allSDFLayers = true;




zDomainFloat neopreneOffset(0.000, 0.000);

bool deckBlock = true;
zDomain<zPoint> bb;
zDomain<zPoint> bb_current;

int resX = RES;
int resY = RES;

string fileDir = "data/striatus/";

string filePath = "data/striatus/test/deck_3.json";
string exportBRGPath = "data/striatus/out_PrintBlock_3.json";

string exportINC3DPath = "C:/Users/vishu.b/Desktop/Striatus_IO/test/blockTest";

string filePath_centerMesh = "data/striatus/out_BRG_220730.json";

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

float printPlaneSpace = 0.00025;
float printLayerWidth = 0.048;
float raftLayerWidth = 0.048;

zDomainFloat printHeightDomain(0.006, 0.012);

/*!<Tool sets*/
zTsSDFSlicer mySlicer;
zTsGraphSDF myGraphSDF;

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


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);	

	// ---- check if cuda exists
	GPU_COMPUTE = checkCudaExists(GPUString);
	cout << "\n " << GPUString;

	// read mesh
	//bb_current = zDomain<zPoint>(zPoint(-1.5, -0.5, 0), zPoint(1.5, 0.5, 0));
	bb_current = zDomain<zPoint>(zPoint(-1.5, -1.5, 0), zPoint(1.5, 1.5, 0));
	

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
		mySlicer.setFromJSON(fileDir, bID);

		deckBlock = mySlicer.onDeckBlock();
		//bb = (!deckBlock) ? zDomain<zPoint>(zPoint(-0.9, -0.35, 0), zPoint(0.9, 1.5, 0)) : zDomain<zPoint>(zPoint(-1.5, -0.5, 0), zPoint(1.5, 0.5, 0));
		bb = (!deckBlock) ? zDomain<zPoint>(zPoint(-0.9, -0.35, 0), zPoint(0.9, 1.5, 0)) : zDomain<zPoint>(zPoint(-1.5, -1.5, 0), zPoint(1.5, 1.5, 0));

		//--- FIELD
		if (bb_current.min != bb.min)
		{
			bb_current = bb;
			printf("\n bounds updated \n", blockID);
			mySlicer.createFieldMesh(bb_current, resX, resY);
		}

		toLOCAL = true;

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

	if (computeSDF)
	{
		double total;
		std::chrono::time_point<std::chrono::system_clock> s_Time = std::chrono::system_clock::now();

		mySlicer.computePrintBlocks(printHeightDomain, printLayerWidth, raftLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, neopreneOffset, false, true);

		std::chrono::time_point<std::chrono::system_clock> e_Time = std::chrono::system_clock::now();
		total = std::chrono::duration_cast<std::chrono::milliseconds>(e_Time - s_Time).count();

		printf("\n ----------------");
		int bID = (int)blockID;
		printf("\n cpu all graph compute %1.2f seconds | block %i | %i graphs | %i %i resolution | funcNUM %i ", total / 1000, bID, gpu_inGraphs.size(), resX, resY, SDFFunc_Num);

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
		bool chkDir = dirExists(exportINC3DPath);
		if(!chkDir) _mkdir(exportINC3DPath.c_str());

		string filename = exportINC3DPath;
		filename += (!deckBlock) ? "/Balustrade" : "/Deck";

		bool chkSubDir = dirExists(filename);
		if (!chkSubDir) _mkdir(filename.c_str());

		string currentPath = fileDir;
		currentPath += (deckBlock) ? "deck_" : "balustrade_";
		currentPath += to_string((int) blockID);
		currentPath += ".json";

		bool chkTransform = false;

		if (!toLOCAL)
		{
			mySlicer.setTransforms(toLOCAL);
			toLOCAL = !toLOCAL;

			chkTransform = true;
		}

		mySlicer.exportJSON(currentPath, filename,"3dp_block", printLayerWidth, raftLayerWidth);

		if (chkTransform)
		{
			mySlicer.setTransforms(toLOCAL);
			toLOCAL = !toLOCAL;
		}

		exportContours = !exportContours;
	}

	//-----------------------------
	if (gpuSETUP)
	{
		// setfielddata
		setGraphSDFData_A(myGraphSDF, mySlicer);



		gpuSETUP = !gpuSETUP;
	}

	if (gpuSDF)
	{
		vector<zTransform> transforms = mySlicer.getBlockFrames();

		bool gpu_deveiceMemorySetup = true;

		// setGraphdata
		double total;
		outLines.clear();
		std::chrono::time_point<std::chrono::system_clock> s_Time = std::chrono::system_clock::now();
		for (int i = 1; i < gpu_inGraphs.size(); i++)
		{
			printf("\n --");
			myGraphSDF.setGraph(&gpu_inGraphs[i][0], GRAPHMEMSIZE);

			if (GPU_COMPUTE)
			{
				bool out = cdpSDFSlicer(myGraphSDF, printLayerWidth, SDFFunc_Num, gpu_deveiceMemorySetup);

				myGraphSDF.computeOutlines(outLines, transforms[i]);
			}
		}
		
		std::chrono::time_point<std::chrono::system_clock> e_Time = std::chrono::system_clock::now();
		total = std::chrono::duration_cast<std::chrono::milliseconds>(e_Time - s_Time).count();
		
		printf("\n ----------------");
		int bID = (int)blockID;
		printf("\n gpu all graph compute %1.2f seconds | block %i | %i graphs | %i %i resolution | funcNUM %i ", total / 1000, bID, gpu_inGraphs.size(), resX, resY, SDFFunc_Num);

		gpuSDF = !gpuSDF;
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

	for (int i = 0; i < outLines.size(); i+= 2)
	{
		model.displayUtils.drawLine(outLines[i], outLines[i + 1]);
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

	if (k == 'g') gpuSETUP = !gpuSETUP;

	if (k == 'y') gpuSDF = !gpuSDF;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
