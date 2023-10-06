//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>

using namespace zSpace;
using namespace std;
using namespace std::chrono;


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool toFile = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

//planarise
zObjMesh oMesh;
zFnMeshDynamics fnDyMesh;
zDoubleArray devs_planarity;
zIntArray fixedVertexIds;

double tol_d = 0.01;
double dT = 0.1;

int numMeshes;
int myCounter = 0;
string dir_in = "data/pSolver/in/";
//string dir_out = "data/pSolver/out/testAnim.usda";
string dir_out = "omniverse://nucleus.zaha-hadid.com/Projects/1453_CODE/UserTests/TZ/testAnim.usda";


//omniverse
zOmniCore omniCore;
UsdStageRefPtr stage;
UsdPrim usd;
int timeStamp = 0;
int timeSpace = 500;

void to(UsdPrim& usd, zObjMesh& meshObj, UsdTimeCode timeCode = UsdTimeCode::Default())
{
	zFnMesh fnMesh(meshObj);
	//mesh attributes
	VtArray<GfVec3f> points;
	VtArray<GfVec3f> normals;
	VtArray<int> fVCounts;
	VtArray<int> fVIDs;
	GfMatrix4d transform;

	//custom attributes
	VtArray<GfVec3f> vCols;
	VtArray<float> opacity;

	VtArray<GfVec3f> vCols_unique;
	VtArray<float> opacity_unique;
	VtArray<int> vCols_unique_index;

	//set mesh vertex attributes
	int numV = fnMesh.numVertices();
	zPoint* rawVPositions = fnMesh.getRawVertexPositions();
	zColor* rawVColor = fnMesh.getRawVertexColors();

	//get & set transformation
	zTransform t;
	fnMesh.getTransform(t);

	transform.Set(t(0, 0), t(1, 0), t(2, 0), t(3, 0),
		t(0, 1), t(1, 1), t(2, 1), t(3, 1),
		t(0, 2), t(1, 2), t(2, 2), t(3, 2),
		t(0, 3), t(1, 3), t(2, 3), t(3, 3));

	// v positions and color
	for (int i = 0; i < numV; i++)
	{
		GfVec3f v_attr, c_attr;
		//set vertex positions
		v_attr.Set(rawVPositions[i].x, rawVPositions[i].y, rawVPositions[i].z);
		points.push_back(v_attr);

		opacity.push_back(1.0);
		c_attr.Set(rawVColor[i].r, rawVColor[i].g, rawVColor[i].b);
		vCols.push_back(c_attr);

		int id = -1;
		bool chkRepeat = false;

		for (int j = 0; j < vCols_unique.size(); j++)
		{
			if (vCols_unique[j] == c_attr)
			{
				chkRepeat = true;
				id = j;
				break;
			}
		}

		if (chkRepeat)
		{
			vCols_unique_index.push_back(id);
		}
		else
		{
			vCols_unique_index.push_back(vCols_unique.size());
			vCols_unique.push_back(c_attr);
			opacity_unique.push_back(1.0);

		}

	}

	//set mesh face attributes
	for (zItMeshFace f(meshObj); !f.end(); f++)
	{
		zVector fNorm = f.getNormal();

		fVCounts.push_back(f.getNumVertices());

		zIntArray v_idx;
		f.getVertices(v_idx);
		for (auto& o : v_idx)
		{
			fVIDs.push_back(o);

			GfVec3f n_attr;
			n_attr.Set(fNorm.x, fNorm.y, fNorm.z);
			normals.push_back(n_attr);
		}
	}



	//create default attr
	UsdGeomMesh usdMesh(usd);
	TfToken interpolationType = pxr::UsdGeomTokens->uniform;

	if (timeCode.IsDefault() || timeCode == 0)
	{
		//create points & normals
		usdMesh.CreateFaceVertexCountsAttr().Set(fVCounts);
		usdMesh.CreateFaceVertexIndicesAttr().Set(fVIDs);

		usdMesh.CreatePointsAttr().Set(points, timeCode);
		usdMesh.CreateNormalsAttr().Set(normals, timeCode);

		//create display color
		auto displayColorPrimvar = usdMesh.CreateDisplayColorPrimvar();
		displayColorPrimvar.Set(vCols_unique, timeCode);
		displayColorPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayColorPrimvar.SetInterpolation(interpolationType);

		//create opacity
		auto displayOpacityPrimvar = usdMesh.CreateDisplayOpacityPrimvar();
		displayOpacityPrimvar.Set(opacity_unique, timeCode);
		displayOpacityPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayOpacityPrimvar.SetInterpolation(interpolationType);
	}
	else
	{
		//update points & normals
		usdMesh.GetPointsAttr().Set(points, timeCode);
		usdMesh.GetNormalsAttr().Set(normals, timeCode);

		//update display color
		auto displayColorPrimvar = usdMesh.GetDisplayColorPrimvar();
		displayColorPrimvar.Set(vCols_unique, timeCode);
		displayColorPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayColorPrimvar.SetInterpolation(interpolationType);

		//update opacity
		auto displayOpacityPrimvar = usdMesh.GetDisplayOpacityPrimvar();
		displayOpacityPrimvar.Set(opacity_unique, timeCode);
		displayOpacityPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayOpacityPrimvar.SetInterpolation(interpolationType);
	}

	//attr = meshPrim.AddTransformOp();
	//attr.Set(transform);
}

template <typename T>
void addProperty(UsdPrim& usd, TfToken& token, SdfValueTypeName typeName, const T& propertyValue, UsdTimeCode timeCode = UsdTimeCode::Default())
{
	UsdGeomPrimvarsAPI primvar(usd);

	if (timeCode.IsDefault() || timeCode == 0)
	{
		primvar.CreatePrimvar(token, typeName).Set(propertyValue, timeCode);
	}
	else
	{
		primvar.GetPrimvar(token).Set(propertyValue, timeCode);
	}
}

bool checkClock(int ms) {
	using namespace std::chrono;

	// Get the current time
	auto startTime = steady_clock::now();

	while (true) {
		// Calculate the elapsed time since the start
		auto currentTime = steady_clock::now();
		auto elapsedTime = duration_cast<milliseconds>(currentTime - startTime);

		// Check if 0.5 seconds (500 milliseconds) have elapsed
		if (elapsedTime.count() >= ms) {
			return true; // Return true every 0.5 seconds
		}

		// Sleep for a short duration to avoid busy-waiting
		std::this_thread::sleep_for(milliseconds(10));
	}
}
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
	model.addObject(oMesh);
	oMesh.setDisplayElements(true, true, true);

	// read mesh

	zFnMesh fnMesh(oMesh);
	fnMesh.from(dir_in + "m_0.json", zJSON);
	fnDyMesh.create(oMesh, true);

	// Connect to OV
	bool doLiveEdit = false;
	std::string existingStage;
	string destinationPath = "omniverse://nucleus.zaha-hadid.com/Projects/1453_CODE/UserTests/TZ";
	bool chk = omniCore.isValidOmniURL(destinationPath);

	// Startup Omniverse with the default login
	if (!omniCore.startOmniverse()) exit(1);
	omniCore.printConnectedUsername(destinationPath);

	//--------------------- 3 - WRITE TO FILE --------------------------
	stage = UsdStage::CreateNew(dir_out);
	stage->SetMetadata(TfToken("defaultPrim"), VtValue("World"));
	stage->SetMetadata(TfToken("upAxis"), VtValue("Z"));
	stage->SetMetadata(TfToken("metersPerUnit"), VtValue(1));

	string s_root, s_layer, s_prim;
	s_root = "/World";
	s_layer = s_root + "/Geometry";
	s_prim = s_layer + "/mesh";
	UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath(s_root));
	UsdGeomXform layer = UsdGeomXform::Define(stage, SdfPath(s_layer));
	UsdGeomMesh meshPrim = UsdGeomMesh::Define(stage, SdfPath(s_prim));

	usd = meshPrim.GetPrim();

	to(usd, oMesh, timeStamp);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	// set display element booleans

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

		bool exit_planar = false;
		double mxDev = 0;
		zVectorArray forceDir;

		//fnDyMesh.addPlanarityForce(1.0, tol_d, zVolumePlanar, devs_planarity, forceDir, exit_planar);
		fnDyMesh.addPlanarityForce(1.0, tol_d, zQuadPlanar, devs_planarity,forceDir, exit_planar);
		fnDyMesh.update(dT, zRK4, true, true, true);

		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_d)
				f.setColor(zColor(0, 1, 0, 1));
			else f.setColor(zColor(1, 0, 1, 1));
		}
		fnDyMesh.computeVertexColorfromFaceColor();

		double fPlanar_max = core.zMax(devs_planarity);
		double fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);

		//if (checkClock(500))
		{
			to(usd, oMesh, timeStamp);

			VtArray<float> out_planarity;
			out_planarity.assign(devs_planarity.size(), 0.0f);
			for (int i = 0; i < devs_planarity.size(); i++) {
				out_planarity[i] = static_cast<float>(devs_planarity[i]);
			}
			addProperty(usd, TfToken("planarity"), SdfValueTypeNames->FloatArray, out_planarity, timeStamp);

			timeStamp++;
		}


		if (exit_planar)
		{
			compute = !compute;
			toFile = true;
		}
	}

	if (toFile)
	{

		//zFnMesh fn(oMesh);
		//fn.to(dir_out + "outMesh.json", zJSON);

		//stage->SetMetadata(TfToken("endTimeCode"), VtValue(timeStamp - 1));
		//stage->SetMetadata(TfToken("startTimeCode"), VtValue(0));
		//stage->SetMetadata(TfToken("framesPerSecond"), VtValue(24.0));
		//stage->SetMetadata(TfToken("timeCodesPerSecond"), VtValue(24.0));
		
		stage->SetStartTimeCode(0.0);
		stage->SetEndTimeCode(timeStamp - 1);
		stage->SetFramesPerSecond(24.0);
		stage->SetTimeCodesPerSecond(24.0);

		stage->GetRootLayer()->Save();
		cout << endl << "stage saved as: " << dir_out << endl;

		toFile = !toFile;
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

		//zPoint* vPositions = fnDyMesh.getRawVertexPositions();

		//for (int i = 0; i < fnDyMesh.numVertices(); i++) {
		//	bool fixedV = std::find(std::begin(fixedVertexIds), std::end(fixedVertexIds), i) != std::end(fixedVertexIds);

		//	if (fixedV)
		//		model.displayUtils.drawPoint(vPositions[i], zColor(1, 0, 0, 1), 5);
		//}
	}




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
