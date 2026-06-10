//#include "zInterface/objects/zObjMeshField.h"
#define _MAIN_
#define _HAS_STD_BYTE 0

#ifdef _MAIN_

#include "main.h"
#include <zApp/include/zViewer.h>


#include <zToolsets/carbcomn/zTsCarbcomn.h>
//#include <zCore/Geometry/zExtGraph.h>

#if defined ZSPACE_USD_INTEROP
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/valueTypeName.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/xform.h>

using namespace pxr;
#endif

//#include <zToolsets/geometry/zTsSDFSlicer.h>


//#include <include/zCore.h>
//#include <include/zGeometry.h>
//#include <include/zDisplay.h>
//#include <include/zData.h>
//#include <include/zIO.h> 
//

using namespace zSpace;
zModel model;

#if defined ZSPACE_USD_INTEROP
namespace zSpace
{
	void zFnMesh::from(pxr::UsdPrim& usd, bool staticGeom) {}
	void zFnMesh::to(pxr::UsdPrim& usd) {}
	void zFnGraph::from(pxr::UsdPrim& usd, bool staticGeom) {}
	void zFnGraph::to(pxr::UsdPrim& usd) {}
	void zFnParticle::from(pxr::UsdPrim& usd, bool staticGeom) {}
	void zFnParticle::to(pxr::UsdPrim& usd) {}
	void zFnPointCloud::from(pxr::UsdPrim& usd, bool staticGeom) {}
	void zFnPointCloud::to(pxr::UsdPrim& usd) {}
}
#endif

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
//double _slider_blockID = 44;
double _slider_blockID = 64;
double _slider_SDF_Func = 7;
double _slider_SDF_Layers = 3;
double _slider_SDF_smooth = 1;

////////////////////////////////////////////////////////////////////////// zSpace Objects

string mainDir = "//zaha-hadid.com/data/Projects/1453_CODE/1453___research/res_Navee/_NatPower/App/V3/Data/NatPower/outFolder";

string blockVersion = "20_3";
//string blockVersion = "20_2";

string cablesDir = "//zaha-hadid.com/data/Projects/1453_CODE/1453___research/res_Navee/_NatPower/App/V3/Data/NatPower/outFolder/V19_11/shared/cableGraphs";
string blockDir = mainDir + "/V" + blockVersion + "/shared/blocks/";
string expBlockDir = "data/Carbcomn/testSliceMesh/";
int blockID = 0;
vector<int> export_block_id_on_q{-1};

zDomain<zPoint> bb;

int resX = 200;
int resY = 200;

float printPlaneSpace = 0.008;
float printLayerWidth = 0.048;
float raftLayerWidth = 0.048;

zDomainFloat neopreneOffset(0.0f, 0.0f);

int SDFFunc_Num = 2;
int SDFFunc_NumSmooth = 1;
int numSDFLayers = 5;
bool allSDFLayers = false;

zDomainFloat printHeightDomain_wall(0.006f, 0.009f);
zDomainFloat printHeightDomain(0.0057f, 0.0123f);
//zDomainFloat printHeightDomain(0.0055, 0.013);



zTsCarbcomn mySlicer;

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
	S.sliders[1].attachToVariable(&_slider_blockID, 0, 82);

	S.addSlider(&_slider_SDF_Func, "sdfFuncNum");
	S.sliders[2].attachToVariable(&_slider_SDF_Func, 0, 7);

	S.addSlider(&_slider_SDF_smooth, "sdf_Smooth");
	S.sliders[3].attachToVariable(&_slider_SDF_smooth, 0, 7);

	S.addSlider(&_slider_SDF_Layers, "SDF_Layers");
	S.sliders[4].attachToVariable(&_slider_SDF_Layers, 0, 150);

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
	B.addButton(&dField, "dField");
	B.buttons[bcounter++].attachToVariable(&dField);

	B.addButton(&dContourGraphs_FLT, "dContourGraphs_FLT");
	B.buttons[bcounter++].attachToVariable(&dContourGraphs_FLT);

	B.addButton(&dTrimGraphs, "dTrimGraphs");
	B.buttons[bcounter++].attachToVariable(&dTrimGraphs);

	B.addButton(&dTrimGraphs_flatten, "dTrimGraphs_FLT");
	B.buttons[bcounter++].attachToVariable(&dTrimGraphs_flatten);




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

#if 0 && defined ZSPACE_USD_INTEROP
namespace
{
	UsdStageRefPtr createUsdLiteStage(const std::string& path)
	{
		std::filesystem::path outPath(path);
		std::error_code ec;
		if (outPath.has_parent_path()) std::filesystem::create_directories(outPath.parent_path(), ec);
		if (std::filesystem::exists(outPath, ec)) std::filesystem::remove(outPath, ec);

		std::string usdPath = outPath.generic_string();
		printf("\n creating USD file: %s", usdPath.c_str());

		UsdStageRefPtr stage = UsdStage::CreateNew(usdPath);
		if (!stage)
		{
			printf("\n error creating USD file: %s", usdPath.c_str());
			return stage;
		}

		UsdPrim world = stage->DefinePrim(SdfPath("/World"), TfToken("Xform"));
		stage->DefinePrim(SdfPath("/World/Geometry"), TfToken("Xform"));
		if (!world) return UsdStageRefPtr();

		if (stage->GetRootLayer()) stage->GetRootLayer()->SetDefaultPrim(TfToken("World"));
		return stage;
	}

	GfMatrix4d toUsdMatrix(const zTransform& transform)
	{
		GfMatrix4d matrix;
		matrix.Set(
			transform(0, 0), transform(0, 1), transform(0, 2), transform(0, 3),
			transform(1, 0), transform(1, 1), transform(1, 2), transform(1, 3),
			transform(2, 0), transform(2, 1), transform(2, 2), transform(2, 3),
			transform(3, 0), transform(3, 1), transform(3, 2), transform(3, 3));
		return matrix;
	}

	TfToken tokenFromName(const std::string& name)
	{
		std::string clean = name;
		for (char& c : clean)
		{
			const bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
			if (!valid) c = '_';
		}
		if (clean.empty() || (clean[0] >= '0' && clean[0] <= '9')) clean = "_" + clean;
		return TfToken(clean);
	}

	void applyFrame(UsdGeomXform& xform, const zTransform* frame)
	{
		if (!frame) return;

		auto matrix = toUsdMatrix(*frame);
		xform.AddTransformOp().Set(matrix);
		xform.GetPrim().CreateAttribute(TfToken("Frame"), SdfValueTypeNames->Matrix4d).Set(matrix);
	}

	void saveUsdStage(const UsdStageRefPtr& stage)
	{
		if (stage) stage->GetRootLayer()->Save();
	}

	bool addMeshToStage(const UsdStageRefPtr& stage, zObjMesh& meshObj, const std::string& primName, const zTransform* frame = nullptr)
	{
		if (!stage) return false;

		SdfPath meshPath("/World/Geometry");
		meshPath = meshPath.AppendChild(tokenFromName(primName));

		UsdGeomXform meshXform = UsdGeomXform::Define(stage, meshPath);
		applyFrame(meshXform, frame);

		UsdGeomMesh mesh = UsdGeomMesh::Define(stage, meshPath.AppendChild(TfToken("Mesh")));

		zFnMesh fnMesh(meshObj);
		zPointArray positions;
		zIntArray polyConnects;
		zIntArray polyCounts;
		fnMesh.getVertexPositions(positions);
		fnMesh.getPolygonData(polyConnects, polyCounts);

		VtArray<GfVec3f> points;
		points.reserve(positions.size());
		for (const zPoint& p : positions) points.emplace_back((float)p.x, (float)p.y, (float)p.z);

		VtArray<int> faceVertexIndices;
		VtArray<int> faceVertexCounts;
		faceVertexIndices.reserve(polyConnects.size());
		faceVertexCounts.reserve(polyCounts.size());
		for (int id : polyConnects) faceVertexIndices.push_back(id);
		for (int count : polyCounts) faceVertexCounts.push_back(count);

		mesh.CreatePointsAttr(VtValue(points), true);
		mesh.CreateFaceVertexIndicesAttr(VtValue(faceVertexIndices), true);
		mesh.CreateFaceVertexCountsAttr(VtValue(faceVertexCounts), true);
		mesh.CreateSubdivisionSchemeAttr(VtValue(UsdGeomTokens->none), true);

		return true;
	}

	bool exportMeshUsd(const std::string& path, zObjMesh& meshObj, const std::string& primName, const zTransform* frame = nullptr)
	{
		UsdStageRefPtr stage = createUsdLiteStage(path);
		if (!addMeshToStage(stage, meshObj, primName, frame)) return false;
		saveUsdStage(stage);
		return true;
	}

	bool addGraphToStage(const UsdStageRefPtr& stage, zObjGraph& graphObj, const std::string& primName, const zTransform* frame = nullptr)
	{
		if (!stage) return false;

		SdfPath graphPath("/World/Geometry");
		graphPath = graphPath.AppendChild(tokenFromName(primName));

		UsdGeomXform graphXform = UsdGeomXform::Define(stage, graphPath);
		applyFrame(graphXform, frame);

		UsdGeomBasisCurves curves = UsdGeomBasisCurves::Define(stage, graphPath.AppendChild(TfToken("Curves")));
		curves.CreateTypeAttr(VtValue(UsdGeomTokens->linear), true);
		curves.CreateWrapAttr(VtValue(UsdGeomTokens->nonperiodic), true);

		zFnGraph fnGraph(graphObj);
		zPointArray positions;
		zIntArray edgeConnects;
		fnGraph.getVertexPositions(positions);
		fnGraph.getEdgeData(edgeConnects);

		VtArray<GfVec3f> points;
		VtArray<int> curveVertexCounts;
		points.reserve(edgeConnects.size());
		curveVertexCounts.reserve(edgeConnects.size() / 2);

		for (size_t i = 0; i + 1 < edgeConnects.size(); i += 2)
		{
			const zPoint& start = positions[edgeConnects[i]];
			const zPoint& end = positions[edgeConnects[i + 1]];

			points.emplace_back((float)start.x, (float)start.y, (float)start.z);
			points.emplace_back((float)end.x, (float)end.y, (float)end.z);

			curveVertexCounts.push_back(2);
		}

		curves.CreatePointsAttr(VtValue(points), true);
		curves.CreateCurveVertexCountsAttr(VtValue(curveVertexCounts), true);
		return true;
	}

	bool exportGraphUsd(zUtilsCore&, const std::string& path, zObjGraph& graphObj, const std::string& primName, const zTransform* frame = nullptr)
	{
		UsdStageRefPtr stage = createUsdLiteStage(path);
		if (!addGraphToStage(stage, graphObj, primName, frame)) return false;
		saveUsdStage(stage);
		return true;
	}

	void addGraphArrayToStage(const UsdStageRefPtr& stage, zObjGraphArray& graphs, const std::string& groupName)
	{
		for (int i = 0; i < graphs.size(); i++)
		{
			addGraphToStage(stage, graphs[i], groupName + "_" + core.getPaddedIndexString(i, 3));
		}
	}

	void addMeshArrayToStage(const UsdStageRefPtr& stage, zObjMeshArray& meshes, const std::string& groupName)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			addMeshToStage(stage, meshes[i], groupName + "_" + core.getPaddedIndexString(i, 3));
		}
	}

	bool exportSlicerUsdLite(const std::string& outputFolder)
	{
		const std::string path = outputFolder + "/Block_" + to_string(blockID) + "_carbcomn.usda";
		UsdStageRefPtr stage = createUsdLiteStage(path);
		if (!stage) return false;

		if (mySlicer.getRawLeftMesh()) addMeshToStage(stage, *mySlicer.getRawLeftMesh(), "slice_left");
		if (mySlicer.getRawRightMesh()) addMeshToStage(stage, *mySlicer.getRawRightMesh(), "slice_right");

		int numGraphs = 0;
		zObjGraphPointerArray sections = mySlicer.getBlockSectionGraphs(numGraphs);
		vector<zTransform> transforms = mySlicer.getBlockFrames();
		for (int i = 0; i < sections.size(); i++)
		{
			const zTransform* frame = (i < transforms.size()) ? &transforms[i] : nullptr;
			addGraphToStage(stage, *sections[i], "section_" + core.getPaddedIndexString(i, 3), frame);
		}

		addGraphArrayToStage(stage, mySlicer.o_contourGraphs, "contour");
		addGraphArrayToStage(stage, mySlicer.o_contourGraphs_flatten, "contour_flatten");
		addGraphArrayToStage(stage, mySlicer.o_trimGraphs, "trim");
		addGraphArrayToStage(stage, mySlicer.o_trimGraphs_bracing, "trim_bracing");
		addGraphArrayToStage(stage, mySlicer.o_trimGraphs_cableprofile, "trim_cableprofile");
		addGraphArrayToStage(stage, mySlicer.o_raftGraphs, "raft");
		addMeshArrayToStage(stage, mySlicer.o_sectionMeshes, "section_mesh");
		addMeshArrayToStage(stage, mySlicer.o_sectionMeshesPar, "section_mesh_flatten");

		saveUsdStage(stage);
		return true;
	}
}
#else
namespace
{
	bool exportMeshUsd(const std::string&, zObjMesh&, const std::string&, const zTransform* = nullptr)
	{
		return false;
	}

	bool exportGraphUsd(zUtilsCore&, const std::string&, zObjGraph&, const std::string&, const zTransform* = nullptr)
	{
		return false;
	}

	bool exportSlicerUsdLite(const std::string&)
	{
		return false;
	}
}
#endif

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
		mySlicer = zTsCarbcomn();

		//iscableblock is manually added
		if (blockID == 0 || blockID == 15||blockID == 44 || blockID == 52)
			mySlicer.isCableBlock = true;
		cout << "\n iscableblock " << mySlicer.isCableBlock << endl;
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
		//bool layerChk = Carbcomn.checkPrintLayerHeights(chkSDF, chkGeo);
		mySlicer.compute_PrintBlocks(printHeightDomain, printLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, true, false);
		//printf("\n layerChk = %s | chkSDF %s | chkGeo %s", to_string(layerChk), to_string(chkSDF), to_string(chkGeo));
		float cellSize = 0.006f;
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

		mySlicer.check_SDF_LayerHeights();


		computeSDF = !computeSDF;


	}

	if (computeHEIGHTS_Folder)
	{
		//bool chkSDF = false;
		//bool chkGeo = true;
		//bool layerChk = Carbcomn.checkPrintLayerHeights(chkSDF, chkGeo);
		//Carbcomn.computePrintBlocks(printHeightDomain, printLayerWidth, raftLayerWidth, allSDFLayers, numSDFLayers, SDFFunc_Num, SDFFunc_NumSmooth, neopreneOffset, true, false);
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


		string path;

		path = expBlockDir + "BlockSlice_" + to_string(blockID) + "_left.usda";
		exportMeshUsd(path, *mySlicer.getRawLeftMesh(), "slice_left");

		path = expBlockDir + "BlockSlice_" + to_string(blockID) + "_right.usda";
		exportMeshUsd(path, *mySlicer.getRawRightMesh(), "slice_right");

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
			string path = OutputFolder + "_section_" + core.getPaddedIndexString(i, 3) + ".usda";
			exportGraphUsd(core, path, *graphs[i], "section", &transforms[i]);

		}

		exportSections = !exportSections;
	}
	
	if (exportSDF)
	{
		exportSDF = !exportSDF;
		expBlockDir = blockDir + "exportedSDF_carbcomn";


		bool chkTransform = false;

		if (!toLOCAL)
		{
			mySlicer.setTransforms(toLOCAL);
			toLOCAL = !toLOCAL;

			chkTransform = true;
		}

		if (!filesystem::is_directory(expBlockDir) || !filesystem::exists(expBlockDir)) filesystem::create_directory(expBlockDir);

		string OutputFolder = expBlockDir;
		cout << endl << "output folder :\n" << OutputFolder;
		if (!filesystem::is_directory(OutputFolder) || !filesystem::exists(OutputFolder)) filesystem::create_directory(OutputFolder);

		mySlicer.exportUSD_update(blockDir, OutputFolder);
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

	mySlicer.o_debug_sectiongraph.draw();
	mySlicer.o_debug_slotgraph.draw();
	mySlicer.o_debug_splitgraph.draw();
	mySlicer.o_debug_bracinggraph.draw();
	mySlicer.o_debug_bracingslotsgraph.draw();
	mySlicer.o_debug_trims.draw();
	mySlicer.o_debug_cutout.draw();

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

		int numGraphs_1 = 0;
		zObjGraphPointerArray graphs_1 = mySlicer.getBlockCableProfileGraphs(numGraphs_1);


		if (displayAllGraphs)
		{
			for (auto& g : graphs)
			{
				g->draw();
			}

			for (auto& g : graphs_1)
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
				if(numGraphs_1 > 0) graphs_1[i]->draw();

				if (pentagon)
				{
					graphs[i + end]->draw();
					if(numGraphs_1 > 0) graphs_1[i + end]->draw();
				}
					

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

		for (auto c : mySlicer.o_CableMeshes)
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
	if (k == 'q') 
	{
		for (int i = 0; i < export_block_id_on_q.size();i++)
		{
			allSDFLayers = true;
			_slider_blockID = export_block_id_on_q[i];
			readJson = true;
			computeFRAMES = true;
			computeSDF = true;
			exportSDF = true;
		}

	}
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
