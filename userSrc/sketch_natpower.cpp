#define _MAIN_
#define _HAS_STD_BYTE 0

#ifdef _MAIN_

#include "main.h"
#include <zApp/include/zViewer.h>


#include <zToolsets/natpower/zTsNatpowerSDF.h>
//#include <zCore/Geometry/zExtGraph.h>

//#include <zToolsets/geometry/zTsSDFSlicer.h>


//#include <include/zCore.h>
//#include <include/zGeometry.h>
//#include <include/zDisplay.h>
//#include <include/zData.h>
//#include <include/zIO.h> 
//

using namespace zSpace;
zModel model;

////////////////////////////////////////////////////////////////////////// General

bool readJson = false;

bool selectBlockFolder = false;


bool computeHEIGHTS_Folder = false;
bool computeFRAMES = false;
bool computeSDF = false;
bool exportSlice = false;
bool exportSections = false;
bool exportSDF = false;
bool readSDF = false;

bool runPlaneLeft = false;
bool runBothPlanes = false;

bool interpolateOrigins = false;


bool computeTRANSFORM = false;
bool toLOCAL = true;


bool dInputMesh = false;
bool dSliceLeft = true;
bool dSliceRight = false;
bool dMeshFlatten = false;
bool dMedialGraph = false;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = true;
bool dContourGraphs_FLT = true;
bool dTrimGraphs = true;
bool dTrimGraphs_flatten = true;
bool dField = false;
bool dCritical = false;
bool dOtherSide = true;
bool dCable = false;


bool displayAllGraphs = false;
int currentGraphId = 1;
int totalGraphs = 0;
bool frameCHECKS = false;


double background = 0.8;
//double _slider_blockID = 57;
double _slider_blockID = 64;
double _slider_SDF_Func =7;
double _slider_SDF_Layers =5;
double _slider_SDF_smooth = 2;

////////////////////////////////////////////////////////////////////////// zSpace Objects

//string mainDir = "C:/Users/heba.eiz/source/repos/GitZHCODE/0_APPS/Natpower_stereotomy/V2/Data/natpower/outFolder";
string mainDir = "//zaha-hadid.com/data/Projects/1453_CODE/1453___research/res_Navee/_NatPower/App/V3/Data/NatPower/outFolder";
//string mainDir = "C:/Users/heba.eiz/source/repos/GitZHCODE/zSpace_Viewer/EXE/Data/natpower/outFolder";
//string mainDir = "data/natpower/outFolder";
//string blockVersion = "16_1";
 //string blockVersion = "18_4";
string blockVersion = "19_5";
//string cablesDir = mainDir + "/cableGraphs";
string cablesDir = "//zaha-hadid.com/data/Projects/1453_CODE/1453___research/res_Navee/_NatPower/App/V3/Data/NatPower/outFolder/V17_6/shared/cableGraphs";
//string blockDir = "data/NatPower/blocks/v13/";
string blockDir = mainDir + "/V" + blockVersion + "/shared/blocks/";
//string expBlockDir = "data/NatPower/testSliceMesh/";
string expBlockDir = "data/NatPower/testSliceMesh/";
int blockID = 0;

//string cableGraphDir = blockDir + "inCableGraphs/";


zDomain<zPoint> bb;


int resX = 200;
int resY = 200;

float printPlaneSpace = 0.008;
float printLayerWidth = 0.048;
float raftLayerWidth = 0.048;

zDomainFloat neopreneOffset(0, 0);

int SDFFunc_Num = 2;
int SDFFunc_NumSmooth = 1;
int numSDFLayers = 5;
bool allSDFLayers = false;

zDomainFloat printHeightDomain(0.0057, 0.0123);
//zDomainFloat printHeightDomain(0.0055, 0.013);


zTsNatpowerSDF mySlicer;

zUtilsCore core;

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&_slider_blockID, "blockID");
	S.sliders[1].attachToVariable(&_slider_blockID, 0, 80);

	S.addSlider(&_slider_SDF_Func, "sdfFuncNum");
	S.sliders[2].attachToVariable(&_slider_SDF_Func, 0, 7);

	S.addSlider(&_slider_SDF_smooth, "sdf_Smooth");
	S.sliders[3].attachToVariable(&_slider_SDF_smooth, 0, 7);

	S.addSlider(&_slider_SDF_Layers, "SDF_Layers");
	S.sliders[4].attachToVariable(&_slider_SDF_Layers, 0, 100);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	int bcounter = 0;
	

	B.addButton(&allSDFLayers, "all_sdf_layer");
	B.buttons[bcounter++].attachToVariable(&allSDFLayers);

	/*B.addButton(&runBothPlanes, "runBothPlanes");
	B.buttons[bcounter++].attachToVariable(&runBothPlanes);

	B.addButton(&interpolateOrigins, "interpolateOrigins");
	B.buttons[bcounter++].attachToVariable(&interpolateOrigins);

	B.addButton(&computeFRAMES, "computeFrames");
	B.buttons[bcounter++].attachToVariable(&computeFRAMES);*/

	B.addButton(&computeHEIGHTS_Folder, "exportLayerHeightChk");
	B.buttons[bcounter++].attachToVariable(&computeHEIGHTS_Folder);

	B.addButton(&dInputMesh, "displayInputMesh");
	B.buttons[bcounter++].attachToVariable(&dInputMesh);

	B.addButton(&dSliceLeft, "displayLeftBlock");
	B.buttons[bcounter++].attachToVariable(&dSliceLeft);

	B.addButton(&dSliceRight, "displayRightBlock");
	B.buttons[bcounter++].attachToVariable(&dSliceRight);

	B.addButton(&dMeshFlatten, "displayFlattenMesh");
	B.buttons[bcounter++].attachToVariable(&dMeshFlatten);

	B.addButton(&dSectionGraphs, "dSectionGraphs");
	B.buttons[bcounter++].attachToVariable(&dSectionGraphs);


	B.addButton(&dContourGraphs, "dContourGraphs");
	B.buttons[bcounter++].attachToVariable(&dContourGraphs);


	B.addButton(&dContourGraphs_FLT, "dContourGraphs_FLT");
	B.buttons[bcounter++].attachToVariable(&dContourGraphs_FLT);

	B.addButton(&dTrimGraphs, "dTrimGraphs");
	B.buttons[bcounter++].attachToVariable(&dTrimGraphs);

	B.addButton(&dTrimGraphs_flatten, "dTrimGraphs_FLT");
	B.buttons[bcounter++].attachToVariable(&dTrimGraphs_flatten);


	B.addButton(&dField, "dField");
	B.buttons[bcounter++].attachToVariable(&dField);

	B.addButton(&dOtherSide, "dOtherSide");
	B.buttons[bcounter++].attachToVariable(&dOtherSide);
	B.addButton(&dCable, "dCable");
	B.buttons[bcounter++].attachToVariable(&dCable);

	B.addButton(&dCritical, "dCritical");
	B.buttons[bcounter++].attachToVariable(&dCritical);



}

zFloatArray getArrayFromTransform(zTransform& transform)
{
	zFloatArray vals;
	vals.assign(16, 0);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			vals[i * 4 + j] = transform(i, j);
		}
	}
	return vals;
}

void get2DArrayFromTransform(zTransform& transform, vector<zDoubleArray>& arr)
{
	//vector<zDoubleArray> arr;
	arr.clear();
	arr.assign(4, zDoubleArray());
	for (int i = 0; i < 4; i++)
	{
		arr[i].assign(4, 0.0);
		for (int j = 0; j < 4; j++)
		{
			arr[i][j] = transform(i, j);
		}
	}
}

void update(int value)
{
	if (selectBlockFolder)
	{
		selectBlockFolder = !selectBlockFolder;

		string readCIN = "n";
		cout << "\n Current main directory is: \n " << mainDir << endl;
		cout << "Change main dir? 'y' or 'n'" << endl;
		cin >> readCIN;

		string tempMainDir = mainDir;
		if (readCIN == "y" || readCIN == "Y")
		{
			cout << "\n Enter main directory";
			cin >> tempMainDir;
		}


		cout << "\n Enter blocks version number: (for example; 13_1 or 13) " << endl;
		cin >> blockVersion;

		//check if the directory exist
		//string tempDir = tempMainDir + "/" + "V" + blockVersion + "/shared/blocks/";
		string tempDir = tempMainDir + "/" + blockVersion + "/shared/blocks/";

		if (!filesystem::exists(tempDir))
		{
			cout << "\n the following directory does NOT exist! \n " << tempDir;
			cout << "\n Try again? 'y' or 'n' ";
			cin >> readCIN;

			if (readCIN == "y" || readCIN == "Y")
			{
				selectBlockFolder = true;
			}

		}
		else
		{
			blockDir = tempDir;
			cout << "\n Block Directory updated! " << endl << blockDir;
		}


	}
	if (readJson)
	{
		blockID = (int)_slider_blockID;
		_slider_blockID = blockID;
		mySlicer = zTsNatpowerSDF();
		mySlicer.setFromJSON(blockDir, blockID, runBothPlanes, runPlaneLeft);

		bb = zDomain<zPoint>(zPoint(-2.5, -2.5, 0), zPoint(2.5, 2.5, 0));
		//mySlicer.createFieldMesh(bb, resX, resY);
		mySlicer.createFieldMeshFromMeshBounds(0.01f, 0.1);
		//get cable graph
		
		mySlicer.setCableGraph(cablesDir);

		toLOCAL = true;

		zFnMesh fnm(mySlicer.o_GuideMesh);
		zDomainColor col_domain(zRED, zBLUE);



		//zDoubleArray vertexCurvature;
		//fnm.getGaussianCurvature(vertexCurvature);

		/*float min_gc = core.zMin(vertexCurvature);
		float max_gc = core.zMax(vertexCurvature);

		zDomainFloat gc_domain(min_gc, max_gc);
		zDomainFloat out_domain(0.0, 1.0);*/

		//printf("\n GaussianCurvature Min | Max %1.4f | %1.4f", min_gc, max_gc);

		//for (zItMeshVertex v(mySlicer.o_GuideMesh); !v.end(); v++)
		//{
		//	zColor v_blendColor = core.blendColor(vertexCurvature[v.getId()], gc_domain, col_domain, zHSV);
		//	v.setColor(v_blendColor);

		//	float remapValue = core.ofMap((float)vertexCurvature[v.getId()], gc_domain, out_domain);
		//	vertexCurvature[v.getId()] = remapValue;

		//}

		//fnm.computeFaceColorfromVertexColor();

		readJson = !readJson;
	}
	if (computeFRAMES)
	{
		mySlicer._interpolateFramesOrigins = interpolateOrigins;
		//bool chkSDF = false;
		//bool chkGeo = true;
		//bool layerChk = natpower.checkPrintLayerHeights(chkSDF, chkGeo);
		mySlicer.compute_PrintBlocks(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, true, false);
		//printf("\n layerChk = %s | chkSDF %s | chkGeo %s", to_string(layerChk), to_string(chkSDF), to_string(chkGeo));
		float cellSize = 0.007f;
		//printf("\n ", mySlicer.)
		if(mySlicer.planarBlock) mySlicer.createFieldMeshFromSectionBounds(cellSize, 0.2);
		else
		{
			bb = zDomain<zPoint>(zPoint(-0.3, -0.3, 0), zPoint(1.8, 0.8, 0));
			float lenX = bb.max.x - bb.min.x;
			float lenY = bb.max.y - bb.min.y;
			int resx = ceil(lenX / cellSize);
			int resy = ceil(lenY / cellSize);
			mySlicer.createFieldMesh(bb, resx, resy);

		}
		//if (mySlicer.planarBlock) mySlicer.createFieldMeshFromSectionBounds(0.01f, 0.2);


		/*int numMinPts = 0;
		zFnPointCloud fn;
		zObjPointCloud* o_minPts = mySlicer.getRawCriticalPoints(false);
		fn = zFnPointCloud(*o_minPts);
		zPointArray maxPoints;
		fn.getVertexPositions(maxPoints);
		for (auto& p : maxPoints)
		{
			cout << endl << p;
		}*/

		computeFRAMES = !computeFRAMES;
	}
	if (computeSDF)
	{
		printf("\n SDF smooth %i", SDFFunc_NumSmooth);
		mySlicer.compute_PrintBlocks(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, false, true);

		//mySlicer.computeSDF(allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, printLayerWidth, 0, raftLayerWidth);;

		computeSDF = !computeSDF;


	}

	if (computeHEIGHTS_Folder)
	{
		//bool chkSDF = false;
		//bool chkGeo = true;
		//bool layerChk = natpower.checkPrintLayerHeights(chkSDF, chkGeo);
		//natpower.computePrintBlocks(printHeightDomain, printLayerWidth, raftLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, neopreneOffset, true, false);
		//printf("\n layerChk = %s | chkSDF %s | chkGeo %s", to_string(layerChk), to_string(chkSDF), to_string(chkGeo));

		mySlicer.check_PrintLayerHeights_Folder(blockDir, printHeightDomain, neopreneOffset, runBothPlanes, runPlaneLeft);

		computeHEIGHTS_Folder = !computeHEIGHTS_Folder;
	}
	if (computeTRANSFORM)
	{
		mySlicer.setTransforms(toLOCAL);
		toLOCAL = !toLOCAL;

		computeTRANSFORM = !computeTRANSFORM;
	}
	if (exportSlice)
	{
		expBlockDir = blockDir + "exported";

		if (!filesystem::is_directory(expBlockDir) || !filesystem::exists(expBlockDir)) filesystem::create_directory(expBlockDir);


		zFnMesh fnmesh;
		string path;

		fnmesh = zFnMesh(*mySlicer.getRawLeftMesh());
		path = expBlockDir + "BlockSlice_" + to_string(blockID) + "_left.json";
		fnmesh.to(path, zJSON);

		fnmesh = zFnMesh(*mySlicer.getRawRightMesh());
		path = expBlockDir + "BlockSlice_" + to_string(blockID) + "_right.json";
		fnmesh.to(path, zJSON);

		exportSlice = !exportSlice;
	}

	if (exportSections)
	{

		expBlockDir = blockDir + "exportedSDF";


		if (!filesystem::is_directory(expBlockDir) || !filesystem::exists(expBlockDir)) filesystem::create_directory(expBlockDir);


		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockSectionGraphs(numGraphs);


		//totalGraphs = mySlicer.isRegular? numGraphs : floor(numGraphs * 0.5);
		totalGraphs = numGraphs;

		zFnGraph fn;
		int counter = 0;
		/*for (auto& g : graphs)
		{
			fn = zFnGraph(*g);
			fn.to(expBlockDir + "section_" + to_string(++counter) + ".json", zJSON);


		}*/
		string OutputFolder = expBlockDir +"/" + to_string(blockID) + "/";

		cout << endl << "ExportDir: \n" << OutputFolder;


		if (!filesystem::is_directory(OutputFolder) || !filesystem::exists(OutputFolder)) filesystem::create_directory(OutputFolder);


		vector<zTransform> transforms = mySlicer.getBlockFrames();

		for (int i = 0; i < graphs.size(); i++)
		{
			json j;
			fn = zFnGraph(*graphs[i]);
			fn.to(j);
			vector<zDoubleArray> frames;
			get2DArrayFromTransform(transforms[i], frames);

			//core.json_writeAttribute(j, "frames", frames);
			j["Frame"] = frames;



			string path = OutputFolder + "_section_" + core.getPaddedIndexString(i, 3) + ".json";


			core.json_write(path, j);

			//try to read one of the json
			//zExtGraph extGraph;
			////string path2 = OutputFolder + "section_" + core.getPaddedIndexString(0, 3) + ".json";
			//char* chr = path.data();

			//ext_graph_from(chr, extGraph);

		}

		exportSections = !exportSections;
	}
	
	if (exportSDF)
	{
		exportSDF = !exportSDF;
		expBlockDir = blockDir + "exportedSDF_2";


		bool chkTransform = false;

		if (!toLOCAL)
		{
			mySlicer.setTransforms(toLOCAL);
			toLOCAL = !toLOCAL;

			chkTransform = true;
		}

		if (!filesystem::is_directory(expBlockDir) || !filesystem::exists(expBlockDir)) filesystem::create_directory(expBlockDir);


		string OutputFolder = expBlockDir;// +"/Block_" + to_string(blockID);
		cout << endl << "output folder :\n" << OutputFolder;
		if (!filesystem::is_directory(OutputFolder) || !filesystem::exists(OutputFolder)) filesystem::create_directory(OutputFolder);

		//mySlicer.exportJSON(blockDir, OutputFolder, "3dp_block", printLayerWidth, raftLayerWidth);
		mySlicer.exportJSON_update(blockDir, OutputFolder);
	}
	if (readSDF)
	{
		readSDF = !readSDF;

		string readDir = blockDir + "exportedSDF" + to_string(blockID);



		

	}
	if (SDFFunc_Num != _slider_SDF_Func)
	{
		SDFFunc_Num = (int)_slider_SDF_Func;
		_slider_SDF_Func = SDFFunc_Num;
	}

	if (blockID != _slider_blockID)
	{
		blockID = (int)_slider_blockID;
		_slider_blockID = blockID;
	}
	if (numSDFLayers != _slider_SDF_Layers)
	{
		numSDFLayers = (int)_slider_SDF_Layers;
		_slider_SDF_Layers = numSDFLayers;
	}
	if (SDFFunc_NumSmooth != _slider_SDF_smooth)
	{
		_slider_SDF_smooth = (int)_slider_SDF_smooth;
		SDFFunc_NumSmooth = (int)_slider_SDF_smooth;
	}


}


void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	mySlicer.debug_graph.draw();

	if (dSliceLeft)
	{
		//mySlicer.getRawLeftMesh()->draw();

		zObjMesh mesh = *mySlicer.getRawLeftMesh();
		mesh.draw();
		for (zItMeshEdge e(mesh); !e.end(); e++)
		{
			if (e.getColor() == zCYAN)
			{
				model.displayUtils.drawLine(
					e.getHalfEdge(0).getStartVertex().getPosition(),
					e.getHalfEdge(0).getVertex().getPosition(),
					zCYAN, 3);
			}
			if (e.getColor() == zRED || e.getColor() == zBLUE)
			{

				//printf("\n r");
				model.displayUtils.drawLine(
					e.getHalfEdge(0).getStartVertex().getPosition(),
					e.getHalfEdge(0).getVertex().getPosition(),
					e.getColor(), 5);



				//if (e.getColor() == zBLUE)	model.displayUtils.drawTextAtPoint(to_string(e.getId()), e.getCenter());
			}
		}
		for (zItMeshVertex v(mesh); !v.end(); v++)
		{
			//if (!(v.getColor() == zBLACK))
			if (!(v.getColor() == zBLACK))
			{
				model.displayUtils.drawPoint(v.getPosition(), v.getColor(), 15);
			}
			/*if (v.getColor() == zORANGE )
			{
				model.displayUtils.drawPoint(v.getPosition(), v.getColor(), 15);
			}*/
		}



	}
	if (dSliceRight)
	{
		//mySlicer.getRawRightMesh()->draw();
		if (mySlicer.planarBlock)
		{


			zObjMesh mesh = *mySlicer.getRawRightMesh();
			mesh.draw();
			for (zItMeshEdge e(mesh); !e.end(); e++)
			{
				if (e.getColor() == zCYAN)
				{
					model.displayUtils.drawLine(
						e.getHalfEdge(0).getStartVertex().getPosition(),
						e.getHalfEdge(0).getVertex().getPosition(),
						zCYAN, 3);
				}
				if (e.getColor() == zRED || e.getColor() == zBLUE)
				{

					//printf("\n r");
					model.displayUtils.drawLine(
						e.getHalfEdge(0).getStartVertex().getPosition(),
						e.getHalfEdge(0).getVertex().getPosition(),
						e.getColor(), 5);
					if (e.getColor() == zBLUE)	model.displayUtils.drawTextAtPoint(to_string(e.getId()), e.getCenter());

				}
			}
		}
		else
		{
			/*for (auto& m : mySlicer.o_sectionMeshesPar)
			{
				m.draw();
			}
			if (displayAllGraphs)
			{
				for (auto& g : graphs)
				{
					g->setDisplayVertices(true);
					g->draw();
				}
			}*/

			zObjMeshArray geo = mySlicer.o_sectionMeshes;
			int numGraphs = geo.size();

			int i = (currentGraphId -1);
			
			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				geo[i].setDisplayElements(true, true, true);
				geo[i].draw();
			}


		}

	}
	if (dMeshFlatten)
	{
		//mySlicer.getRawRightMesh()->draw();
		if (!mySlicer.planarBlock)
		{
			zObjMeshArray graphs = mySlicer.o_sectionMeshesPar;
			int numGraphs = graphs.size();

			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i].draw();
			}


		}

	}
	if (dContourGraphs_FLT)
	{
		//mySlicer.getRawRightMesh()->draw();
		if (!mySlicer.planarBlock)
		{
			zObjGraphArray graphs = mySlicer.o_contourGraphs_flatten;
			int numGraphs = graphs.size();

			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i].draw();
			}


		}

	}

	if (dCritical)
	{
		int numMinPts = 0;
		zFnPointCloud fn;
		zObjPointCloud* o_minPts = mySlicer.getRawCriticalPoints(true);
		fn = zFnPointCloud(*o_minPts);
		if (fn.numVertices() > 0) model.displayUtils.drawPoints(fn.getRawVertexPositions(), zBLUE, 15, fn.numVertices());

		zObjPointCloud* o_maxPts = mySlicer.getRawCriticalPoints(false);
		fn = zFnPointCloud(*o_maxPts);
		if (fn.numVertices() > 0) model.displayUtils.drawPoints(fn.getRawVertexPositions(), zRED, 15, fn.numVertices());
		//printf("\n d %i", fn.numVertices());
	}

	bool pentagon = !mySlicer.isRegular&& dOtherSide;

	if (dSectionGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockSectionGraphs(numGraphs);


		//totalGraphs = floor(numGraphs * 0.5);
		totalGraphs = !pentagon ? numGraphs : floor(numGraphs * 0.5);

		if (displayAllGraphs)
		{


			for (auto& g : graphs)
			{
				g->setDisplayVertices(false);
				g->draw();

				for (zItGraphVertex e(*g); !e.end(); e++)
				{
					if (e.getColor() == zORANGE)
					{
						//model.displayUtils.drawPoint(e.getPosition(), e.getColor(), 15);
						//printf("\n m");
					}
					else
					{
						//model.displayUtils.drawPoint(e.getPosition(), e.getColor(), 2);

					}
				}

			}
		}
		else
		{
			int i = currentGraphId;
			int  end = !pentagon ? numGraphs : floor(numGraphs * 0.5);
			//int  end = floor(numGraphs ); 

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i]->setDisplayVertices(true);
				graphs[i]->draw();



				for (zItGraphVertex v(*graphs[i]); !v.end(); v++)
				{
					if (!(v.getColor() == zBLACK))
						model.displayUtils.drawPoint(v.getPosition(), v.getColor(), 15);

				}
				if (pentagon)
				{


					graphs[i + end]->setDisplayVertices(true);
					graphs[i + end]->draw();

					for (zItGraphVertex v(*graphs[i + end]); !v.end(); v++)
					{
						if(!(v.getColor() == zBLACK))
						model.displayUtils.drawPoint(v.getPosition(), v.getColor(), 15);

					}
				}


			}

		}


		vector<zTransform> transforms = mySlicer.getBlockFrames();
		for (int k = 0; k < transforms.size(); k++) model.displayUtils.drawTransform(transforms[k], 0.05);

	}
	if (dContourGraphs)
	{
		int numGraphs = 0;
		zObjGraphPointerArray graphs = mySlicer.getBlockContourGraphs(numGraphs);
		//printf("\n numGraphs %i ", numGraphs);
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
			//int  end =floor(numGraphs * 0.5) ;
			int  end = !pentagon ? numGraphs : floor(numGraphs * 0.5);


			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i]->draw();

				if (pentagon)
					graphs[i + end]->draw();

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
			//int end = floor(numGraphs * 0.5);
			int  end = !pentagon ? numGraphs : floor(numGraphs * 0.5);


			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i]->draw();

				if (pentagon)
					graphs[i + end]->draw();

			}

		}
	}
	if (dTrimGraphs_flatten)
	{
		if (!mySlicer.planarBlock)
		{
			zObjGraphArray graphs = mySlicer.o_trimGraphs_bracing_flat;
			int numGraphs = graphs.size();

			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				graphs[i].draw();
			}


		}
	}
	if (dField)
	{
		zObjMeshScalarField* o_field = mySlicer.getRawFieldMesh();

		o_field->setDisplayElements(false, true, true);
		o_field->draw();

	}
	if (dInputMesh)
	{
		mySlicer.o_GuideMesh.draw();
	}
	
	if (dCable)
	{
		for(auto c: mySlicer.o_CableGraphs)
		{
			c.draw();
		}
	}
	
	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("currentGraphId #:" + to_string(currentGraphId), vec(winW - 350, winH - 500, 0));
	drawString("isRegual? #:" + to_string(mySlicer.isRegular), vec(winW - 350, winH - 200, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'r')
	{
		readJson = true;
	}
	if (k == 'i') dInputMesh = true;
	if (k == 'p')
	{
		computeFRAMES = true;
	}

	if (k == 'o') computeSDF = true;;

	if (k == 'e')
	{
		//exportSlice = true;
		exportSections = true;
		exportSDF = true;
	}
	if (k == 'E')
	{
		exportSections = true;

	}
	if (k == 't') computeTRANSFORM = true;;


	if (k == 'w')
	{
		if (currentGraphId < totalGraphs - 1)currentGraphId++;;
	}
	if (k == 's')
	{
		if (currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') displayAllGraphs = !displayAllGraphs;

	if (k == 'i')
	{
		selectBlockFolder = true;
	}
	if (k == 'M')
	{
		readSDF = true;
	}
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
