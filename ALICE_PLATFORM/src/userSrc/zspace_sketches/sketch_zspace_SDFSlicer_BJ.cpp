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

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

void indentifyFeatureEdges(zObjMesh& o_inMesh)
{
	zFnMesh fnMesh(o_inMesh);
	zColor *vColors = fnMesh.getRawVertexColors();

	zColor magenta(1, 0, 1, 1);

	for (zItMeshEdge e(o_inMesh); !e.end(); e++)
	{
		zIntArray eVerts;
		e.getVertices(eVerts);

		if (vColors[eVerts[0]] == magenta && vColors[eVerts[1]] == magenta)
		{
			e.setColor(magenta);
		}
	}
}

////////
bool checkFOLDER = false;
bool runAll = false;
bool computeFRAMES = false;
bool computeSDF = false;
bool computeMap = false;
bool computePLYS = false;
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
zDomainFloat oDomain(0.0001, 0.03);

zDomain<zPoint> bb(zPoint(-1.5, -1, 0), zPoint(2.5, 1, 0));
int resX = 512;
int resY = 512;

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

float printPlaneSpace = 0.004;
float printLayerWidth = 0.008;
//float printLayerWidth = 0.022; // 0.002 for verticals

zDomainFloat printHeightDomain(0.00175, 0.004);

int blockID = 17;
string geom_location = "top";

int SDFFunc_Num = 3;
bool SDFFunc_NumSmooth = 1;

int numSDFLayers = 3;
bool allSDFLayers = true;

vector<zTransform> startPlanes;
vector<zTransform> endPlanes;

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

	cout << "Please enter geometry location (top/ mid/ bottom/all): ";
	cin >> geom_location;

	cout << "Please enter block ID (0 -20): ";
	cin >> blockID;

	cout << "Please enter SDF Functions (0 - 3): ";
	cin >> SDFFunc_Num;

	// read mesh
	zFnMesh fnSliceMesh(oSliceMesh);
	fnSliceMesh.from("data/Slicer/sliceMesh_geom_" + geom_location + ".json", zJSON);
	fnSliceMesh.setEdgeColor(zColor(0.8, 0.8, 0.8, 1));	
	indentifyFeatureEdges(oSliceMesh);

	// read planes from JSON
	json j;
	bool chk = core.readJSON("data/Slicer/sliceMesh_planes_" + geom_location + ".json", j);

	vector<zDoubleArray> startPlaneAttr;
	core.readJSONAttribute(j, "startPlane", startPlaneAttr);

	vector<zDoubleArray> endPlaneAttr;
	core.readJSONAttribute(j, "endPlane", endPlaneAttr);

	printf("\n planes %i %i ", startPlaneAttr.size(), endPlaneAttr.size());

	for (int i = 0; i < startPlaneAttr.size(); i++)
	{
		zTransform sPlane, ePlane;
		

		zVector basis(1, 1, 0);
		zPoint sO = zPoint(startPlaneAttr[i][9], startPlaneAttr[i][10], startPlaneAttr[i][11]);
		zVector sZ = zVector(startPlaneAttr[i][6], startPlaneAttr[i][7], startPlaneAttr[i][8]);

		zPoint eO = zPoint(endPlaneAttr[i][9], endPlaneAttr[i][10], endPlaneAttr[i][11]);
		zVector eZ = zVector(endPlaneAttr[i][6], endPlaneAttr[i][7], endPlaneAttr[i][8]);

		sPlane = core.getTransformFromOrigin_Normal(sO, sZ, basis);
		ePlane = core.getTransformFromOrigin_Normal(eO, eZ, basis);

		//printf("\n %i %1.2f %1.7f ", i, sZ.angle(eZ), sZ *eZ);

		startPlanes.push_back(sPlane);
		endPlanes.push_back(ePlane);
	}

	
	cout << endl << startPlanes[blockID];
	cout << endl << endl << endPlanes[blockID];	

	// make graph
	zPointArray vPositions;
	vPositions.push_back(zPoint(startPlanes[blockID](3, 0), startPlanes[blockID](3, 1), startPlanes[blockID](3, 2)));
	vPositions.push_back(zPoint(endPlanes[blockID](3, 0), endPlanes[blockID](3, 1), endPlanes[blockID](3, 2)));

	zIntArray eConnects = { 0,1 };
	zFnGraph fnGraph(oGuideGraph);

	//for (auto& p : fCens) p += zVector(0, 0, 0.05);
	fnGraph.create(vPositions, eConnects);

	// graadient mesh
	zFnMesh fnGradientMesh(oGradientMesh);
	fnGradientMesh.from("data/Slicer/sliceMesh_colorMap.json", zJSON);

	// read color
	//json j;
	//bool chk = core.readJSON("data/Slicer/colorMesh_v3_2.json", j);

	//vector<zDoubleArray> faceAttributes;
	//core.readJSONAttribute(j, "FaceAttributes", faceAttributes);	

	//zColorArray fColors;
	//if (faceAttributes.size() > 0)
	//{
	//	for (zItMeshFace f(oGradientMesh); !f.end(); f++)
	//	{
	//		int fID = f.getId();
	//		if (faceAttributes[fID].size() > 0)
	//		{
	//			fColors.push_back(zColor(faceAttributes[fID][3], faceAttributes[fID][4], faceAttributes[fID][5], 1));
	//		}
	//	}
	//}
	//
	//fnGradientMesh.setFaceColors(fColors, true);

	//set slicer
	mySlicer.setSliceMesh(oSliceMesh, true);
	mySlicer.setStartEndPlanes(startPlanes[blockID], endPlanes[blockID], true);
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

	if (computePLYS)
	{

		vector<zPlane> frames;

		for (int i = 0; i < startPlanes.size(); i++)
		{
			frames.push_back(startPlanes[i]);
			if(i==0) frames.push_back(startPlanes[i]);
			frames.push_back(endPlanes[i]);

		}

		mySlicer.setFrames(frames);
		mySlicer.computePrintBlockSections(true);

		mySlicer.computeSDF_Generic(allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, printLayerWidth);


		// export
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockContourGraphs(numGraphs);

		string dir = "data/Slicer/out_BJ/contours/verticals";
		//for (const auto& entry : std::filesystem::directory_iterator(dir)) std::filesystem::remove_all(entry.path());

		int graphID = 0;
		for (auto& g : graphs)
		{
			zFnGraph fnIsoGraph(*g);
			if (fnIsoGraph.numVertices() == 0)
			{
				graphID++;
				continue;
			}

			string dir_1 = (graphID % 2 == 1) ? dir + "/start" : dir + "/end";
			if(graphID == 0) for (const auto& entry : std::filesystem::directory_iterator(dir_1)) std::filesystem::remove_all(entry.path());

			string outName1 =  dir_1 + "/outContour_"  ;
			outName1 += to_string(graphID) + ".json";

			fnIsoGraph.to(outName1, zJSON);
			graphID++;

		}

		computePLYS = !computePLYS;
	}

	if (runAll)
	{

		for (int i = 0; i < startPlanes.size(); i++)
		{
			blockID = i;
			// create graph 
			zObjGraph* o_graph = mySlicer.getRawMedialGraph();
			zFnGraph fnGraph(*o_graph);
			zPoint* vPositions = fnGraph.getRawVertexPositions();
			vPositions[0] = zPoint(startPlanes[i](3, 0), startPlanes[i](3, 1), startPlanes[i](3, 2));
			vPositions[1] = zPoint(endPlanes[i](3, 0), endPlanes[i](3, 1), endPlanes[i](3, 2));

			// setPlanes
			mySlicer.setStartEndPlanes(startPlanes[i], endPlanes[i], true);

			printf("\n ----------- \n Block %i \n", i);

			mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, true, false);

			float printLength = 0;
			float weight = 0;
			bool frameCHECKS = mySlicer.checkPrintLayerHeights_Generic(printLength);;

			if (!frameCHECKS)  continue;

			//---------------------
			
			mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, false, true);

			// -------------------

			string folderName = "data/Slicer/out_BJ/contours/" + geom_location;

			string currentPath = folderName;
			currentPath += "/";
			currentPath += to_string((int)blockID);

			bool chkDir = dirExists(currentPath);
			if (!chkDir) _mkdir(currentPath.c_str());

			mySlicer.exportJSON_Generic(currentPath, "3dp_block", printLayerWidth);

		}

		runAll = !runAll;
	}

	if (computeFRAMES)
	{
		printf("\n ----------- \n Block %i \n", blockID);
		mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, true, false);


		vector<zTransform> planes = mySlicer.getBlockFrames();
		for (int i = 1; i < planes.size(); i++)
		{
			zPoint A(planes[i - 1](3, 0), planes[i - 1](3, 1), planes[i - 1](3, 2));

			zPoint O(planes[i](3, 0), planes[i](3, 1), planes[i](3, 2));
			zVector N(planes[i](2, 0), planes[i](2, 1), planes[i](2, 2));

			float d = core.minDist_Point_Plane(A, O, N);
			printf("\n plane dist %i %i  | %1.4f ", i-1, i, abs(d));
		}

		computeFRAMES = !computeFRAMES;
	}
	
	if (checkFOLDER)
	{
		string outFileName = "data/Slicer/out_BJ/printLayerHeights_allBlocks.csv";

		ofstream myfile;
		myfile.open(outFileName.c_str());

		if (myfile.fail())
		{
			cout << " error in opening file  " << outFileName.c_str() << endl;
			return;
		}

		myfile << "\n" << "geoLocation" << "," << "blockID" << "," << "frameCHECKS" << "," << "minHeight" << "," << "maxHeight" << "," << "planeAngles" << "," << "printLength" << "," << "weight" << endl;

		for (int i = 0; i < startPlanes.size(); i++)
		{
			blockID = i;
			// create graph 
			zObjGraph *o_graph = mySlicer.getRawMedialGraph();
			zFnGraph fnGraph(*o_graph);
			zPoint* vPositions = fnGraph.getRawVertexPositions();
			vPositions[0] = zPoint(startPlanes[i](3, 0), startPlanes[i](3, 1), startPlanes[i](3, 2));
			vPositions[1] = zPoint(endPlanes[i](3, 0), endPlanes[i](3, 1), endPlanes[i](3, 2));

			// setPlanes
			mySlicer.setStartEndPlanes(startPlanes[i], endPlanes[i], true);

			printf("\n ----------- \n Block %i \n", i);

			mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, true, false);

			float printLength = 0;
			float weight = 0;
			bool frameCHECKS = mySlicer.checkPrintLayerHeights_Generic(printLength);;

			if(!frameCHECKS) printLength = 0;
			else
			{
				//(length in cm) * (cross section in cm )* (material density in g/cm3)
				weight = ((printLength * 100) * (0.4 * 0.9)  * 1.37) / 1000;
			}

			zVector sNorm = zVector(startPlanes[i](2, 0), startPlanes[i](2, 1), startPlanes[i](2, 2));
			zVector eNorm = zVector(endPlanes[i](2, 0), endPlanes[i](2, 1), endPlanes[i](2, 2));

			myfile << "\n" << geom_location << "," << blockID << "," << ((frameCHECKS) ? "True" : "False") << "," << mySlicer.actualPrintHeightDomain.min << "," << mySlicer.actualPrintHeightDomain.max << "," << sNorm.angle(eNorm) << "," << printLength << "," << weight;

		}

		myfile.close();

		cout << " \n file exported : " << outFileName.c_str() << endl;
		
		checkFOLDER = !checkFOLDER;
	}

	if (computeSDF)
	{
		mySlicer.computePrintBlock_Generic(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, false, true);

		computeSDF = !computeSDF;
	}

	if (exportSDF)
	{

		string folderName = "data/Slicer/out_BJ/contours/" + geom_location;
		//for (const auto& entry : std::filesystem::directory_iterator(folderName)) std::filesystem::remove_all(entry.path());

		string currentPath = folderName;
		currentPath += "/";
		currentPath += to_string((int)blockID);

		bool chkDir = dirExists(currentPath);
		if (!chkDir) _mkdir(currentPath.c_str());

		mySlicer.exportJSON_Generic(currentPath, "3dp_block", printLayerWidth);

		//////------
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockContourGraphs(numGraphs);
		
		string dir = "data/Slicer/out_BJ/contours/test";
		for (const auto& entry : std::filesystem::directory_iterator(dir)) std::filesystem::remove_all(entry.path());

		int graphID = 0;
		for (auto& g : graphs)
		{
			zFnGraph fnIsoGraph(*g);
			if (fnIsoGraph.numVertices() == 0)
			{
				graphID++;
				continue;
			}

			string outName1 = dir + "/outContour_";
			outName1 += to_string(graphID) + ".json";

			fnIsoGraph.to(outName1, zJSON);
			graphID++;
			
		}


	/*	folderName = "data/Slicer/out/trims";
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

		}*/
		
		//printf("\n %i graphs exported. ", graphID);
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
		
		int numMinPts = 0;
		zObjPointCloud* o_minPts = mySlicer.getRawCriticalPoints(true);
		o_minPts->draw();

		int numMaxPts = 0;
		zObjPointCloud* o_maxPts = mySlicer.getRawCriticalPoints(false);
		o_maxPts->draw();

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

	model.displayUtils.drawTransform(startPlanes[blockID]);
	model.displayUtils.drawTransform(endPlanes[blockID]);

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Layers #:" + to_string(totalGraphs), vec(winW - 350, winH - 800, 0));
	drawString("Current Layer #:" + to_string(currentGraphId), vec(winW - 350, winH - 775, 0));

	
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{

	if (k == 'a') runAll = true;;

	if (k == 'c') checkFOLDER = true;;

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

	if (k == 'v') computePLYS = true;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
