//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

/// OV
#include "xformUtils.h"
#include "primUtils.h"
#include "OmniverseUsdLuxLightCompat.h"

#include <mutex>
#include <memory>
#include <map>
#include <condition_variable>

#include <OmniClient.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/usd/kind/registry.h>
#include <pxr/usd/usdUtils/pipeline.h>
#include <pxr/usd/usdUtils/sparseValueWriter.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/payloads.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/references.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usdShade/input.h>
#include <pxr/usd/usdShade/output.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdShade/shader.h>

#include <usdPhysics/scene.h>
#include <usdPhysics/rigidBodyAPI.h>
#include <usdPhysics/collisionAPI.h>
#include <usdPhysics/meshCollisionAPI.h>

/// ZSPACE
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;
using namespace pxr;

//zspace objects
//zObjMesh oMesh;
//zFnMesh fnMesh;
//zIntArray polyConnects;
//zIntArray polyCount;

pxr::UsdPrim rootPrim;
zObjMesh oMesh;
zObjMeshArray allObjMesh;
vector<SdfPath> primPaths;
vector<pxr::UsdPrim> allMeshPrim;

UsdStageRefPtr stage_in;
UsdStageRefPtr stage_out;

string destinationPath = "omniverse://nucleus.zaha-hadid.com";
//string filePath = "omniverse://nucleus.zaha-hadid.com/Projects/OV_IO/testBox.usda";
//string filePath_out = "omniverse://nucleus.zaha-hadid.com/Projects/OV_IO/testBox_out.usda";

string filePath = "omniverse://nucleus.zaha-hadid.com/Projects/1453_CODE/UserTests/VB/testCube_1.usda";
string filePath_out = "omniverse://nucleus.zaha-hadid.com/Projects/1453_CODE/UserTests/VB/testCube_1.out.usda";

////////////////////////////////////////////////////////////////////////// OV methods

PXR_NAMESPACE_USING_DIRECTIVE

// Globals for Omniverse Connection and base Stage
static UsdStageRefPtr gStage;

// Omniverse logging is noisy, only enable it if verbose mode (-v)
static bool gOmniverseLoggingEnabled = false;

// Global for making the logging reasonable
static std::mutex gLogMutex;

static GfVec3d gDefaultRotation(0);
static GfVec3i gDefaultRotationOrder(0, 1, 2);
static GfVec3d gDefaultScale(1);

// Private tokens for building up SdfPaths. We recommend
// constructing SdfPaths via tokens, as there is a performance
// cost to constructing them directly via strings (effectively,
// a table lookup per inPath element). Similarly, any API which
// takes a token as input should use a predefined token
// rather than one created on the fly from a string.
TF_DEFINE_PRIVATE_TOKENS(
	_tokens,
	(DistantLight)
	(DomeLight)
	(Looks)
	(World)
	(Shader)
	(st)

	// These tokens will be reworked or replaced by the official MDL schema for USD.
	// https://developer.nvidia.com/usd/MDLschema
	(Material)
	((_module, "module"))
	(name)
	(out)
	((shaderId, "mdlMaterial"))
	(mdl)

	// Tokens used for USD Preview Surface
	(diffuseColor)
	(normal)
	(file)
	(result)
	(varname)
	(rgb)
	(sourceColorSpace)
	(raw)
	(scale)
	(bias)
	(surface)
	(PrimST)
	(UsdPreviewSurface)
	((UsdShaderId, "UsdPreviewSurface"))
	((PrimStShaderId, "UsdPrimvarReader_float2"))
	(UsdUVTexture)
);


static void OmniClientConnectionStatusCallbackImpl(void* userData, const char* url, OmniClientConnectionStatus status) noexcept
{
	// Let's just print this regardless
	{
		std::unique_lock<std::mutex> lk(gLogMutex);
		std::cout << "Connection Status: " << omniClientGetConnectionStatusString(status) << " [" << url << "]" << std::endl;
	}
	if (status == eOmniClientConnectionStatus_ConnectError)
	{
		// We shouldn't just exit here - we should clean up a bit, but we're going to do it anyway
		std::cout << "[ERROR] Failed connection, exiting." << std::endl;
		exit(-1);
	}
}

static void failNotify(const char* msg, const char* detail = nullptr, ...)
{
	std::unique_lock<std::mutex> lk(gLogMutex);

	fprintf(stderr, "%s\n", msg);
	if (detail != nullptr)
	{
		fprintf(stderr, "%s\n", detail);
	}
}

// Shut down Omniverse connection
static void shutdownOmniverse()
{
	// Calling this prior to shutdown ensures that all pending live updates complete.
	omniClientLiveWaitForPendingUpdates();

	// The stage is a sophisticated object that needs to be destroyed properly.  
	// Since gStage is a smart pointer we can just reset it
	gStage.Reset();

	// This will prevent "Core::unregister callback called after shutdown"
	omniClientSetLogCallback(nullptr);

	omniClientShutdown();
}

// Omniverse Log callback
static void logCallback(const char* threadName, const char* component, OmniClientLogLevel level, const char* message) noexcept
{
	std::unique_lock<std::mutex> lk(gLogMutex);
	if (gOmniverseLoggingEnabled)
	{
		puts(message);
	}
}

// Startup Omniverse 
static bool startOmniverse()
{
	// Register a function to be called whenever the library wants to print something to a log
	omniClientSetLogCallback(logCallback);

	// The default log level is "Info", set it to "Debug" to see all messages
	omniClientSetLogLevel(eOmniClientLogLevel_Debug);

	// Initialize the library and pass it the version constant defined in OmniClient.h
	// This allows the library to verify it was built with a compatible version. It will
	// return false if there is a version mismatch.
	if (!omniClientInitialize(kOmniClientVersion))
	{
		return false;
	}

	omniClientRegisterConnectionStatusCallback(nullptr, OmniClientConnectionStatusCallbackImpl);

	return true;
}

// Returns true if the provided maybeURL contains a host and inPath
static bool isValidOmniURL(const std::string& maybeURL)
{
	bool isValidURL = false;
	OmniClientUrl* url = omniClientBreakUrl(maybeURL.c_str());
	if (url->host && url->path &&
		(std::string(url->scheme) == std::string("omniverse") ||
			std::string(url->scheme) == std::string("omni")))
	{
		isValidURL = true;
	}
	omniClientFreeUrl(url);
	return isValidURL;
}

// Create a new connection for this model in Omniverse, returns the created stage URL
static std::string createOmniverseModel(const std::string& destinationPath, string stageName, bool doLiveEdit)
{
	std::string stageUrl = destinationPath + "/" + stageName + (doLiveEdit ? ".live" : ".usd");

	// Delete the old version of this file on Omniverse and wait for the operation to complete
	{
		std::unique_lock<std::mutex> lk(gLogMutex);
		std::cout << "Waiting for " << stageUrl << " to delete... " << std::endl;
	}
	omniClientWait(omniClientDelete(stageUrl.c_str(), nullptr, nullptr));
	{
		std::unique_lock<std::mutex> lk(gLogMutex);
		std::cout << "finished" << std::endl;
	}

	// Create this file in Omniverse cleanly
	gStage = UsdStage::CreateNew(stageUrl);
	if (!gStage)
	{
		failNotify("Failure to create model in Omniverse", stageUrl.c_str());
		return std::string();
	}

	{
		std::unique_lock<std::mutex> lk(gLogMutex);
		std::cout << "New stage created: " << stageUrl << std::endl;
	}

	// Keep the model contained inside of "World", only need to do this once per model
	const SdfPath defaultPrimPath = SdfPath::AbsoluteRootPath().AppendChild(_tokens->World);
	UsdGeomXform defaultPrim = UsdGeomXform::Define(gStage, defaultPrimPath);

	// Set the /World prim as the default prim
	gStage->SetDefaultPrim(defaultPrim.GetPrim());

	// Set the default prim as an assembly to support using component references
	pxr::UsdModelAPI(defaultPrim).SetKind(pxr::KindTokens->assembly);

	// Always a good idea to declare your up-ness
	UsdGeomSetStageUpAxis(gStage, UsdGeomTokens->y);

	// For physics its important to set units!
	UsdGeomSetStageMetersPerUnit(gStage, 0.01);

	return stageUrl;
}

// Stage URL really only needs to contain the server in the URL.  eg. omniverse://ov-prod
static void printConnectedUsername(const std::string& stageUrl)
{
	// Get the username for the connection
	std::string userName("_none_");
	omniClientWait(omniClientGetServerInfo(stageUrl.c_str(), &userName, [](void* userData, OmniClientResult result, struct OmniClientServerInfo const* info) noexcept
		{
			std::string* userName = static_cast<std::string*>(userData);
			if (userData && userName && info && info->username)
			{
				userName->assign(info->username);
			}
		}));
	{
		std::unique_lock<std::mutex> lk(gLogMutex);
		std::cout << "Connected username: " << userName << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////
void OV_Connect(string destinationPath, string filePath)
{
	// Connect to OV
	bool doLiveEdit = false;
	if (!isValidOmniURL(destinationPath))
	{
		std::cout << "This is not an Omniverse Nucleus URL: " << destinationPath << std::endl;
		std::cout << "Correct Omniverse URL format is: omniverse://server_name/Path/To/Example/Folder" << std::endl;
		std::cout << "Allowing program to continue because file paths may be provided in the form: C:/Path/To/Stage" << std::endl;
	}
	else std::cout << "Connected to Omniverse Nucleus URL: " << destinationPath << std::endl;

	// Startup Omniverse with the default login
	if (!startOmniverse())
		exit(1);

	//// Create the USD model in Omniverse
	//string newStageName = "newBox";
	//const std::string stageUrl = createOmniverseModel(destinationPath, newStageName, doLiveEdit);

	// Print the username for the server
	printConnectedUsername(filePath);

}

void OV_Disconnect()
{
	// All done, shut down our connection to Omniverse
	shutdownOmniverse();
}

void fromUSDFile(UsdGeomMesh usdMesh, zObjMesh& oMesh)
{
	// Create a function object
	zFnMesh fnMesh(oMesh);

	// Declare arrays to store mesh data
	VtArray<GfVec3f> u_points;
	VtArray<GfVec3f> u_normals;
	VtArray<GfVec3f> u_vertexColors;
	VtArray<int>     faceVertexCounts;
	VtArray<int>     faceVertexIndices;
	VtArray<int>	 u_vertexColorIndices;

	GfMatrix4d transform;
	bool tmp = true;

	// Retrieve attributes from `usdMesh`
	UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
	UsdAttribute normalsAttr = usdMesh.GetNormalsAttr();
	UsdAttribute faceVertexCountsAttr = usdMesh.GetFaceVertexCountsAttr();
	UsdAttribute faceVertexIndicesAttr = usdMesh.GetFaceVertexIndicesAttr();
	UsdAttribute vertexColorAttr = usdMesh.GetDisplayColorAttr();
	UsdAttribute vertexColorIndicesAttr = usdMesh.GetPrim().GetAttribute(pxr::TfToken("primvars:displayColor:indices"));

	// Retrieve the local transformation of the mesh
	usdMesh.GetLocalTransformation(&transform, &tmp);

	// Get data from the USD attributes
	/*
	pointsAttr.Get(&u_points);
	normalsAttr.Get(&u_normals);
	faceVertexCountsAttr.Get(&faceVertexCounts);
	faceVertexIndicesAttr.Get(&faceVertexIndices);
	vertexColorAttr.Get(&u_vertexColors);
	vertexColorIndicesAttr.Get(&u_vertexColorIndices);
	*/

	// Prepare data structures for the zObjMesh
	zPointArray positions;
	zIntArray polyCounts;
	zIntArray polyConnects;
	zTransform myTransform;
	zColorArray palette;
	zColorArray vertexColors;

	if (pointsAttr.Get(&u_points))
		for (int i = 0; i < u_points.size() * 3; i += 3)
		{
			zPoint pos = zPoint(u_points.cdata()->GetArray()[i], u_points.cdata()->GetArray()[i + 1], u_points.cdata()->GetArray()[i + 2]);
			positions.push_back(pos);
		}

	if (faceVertexCountsAttr.Get(&faceVertexCounts))
		for (int i = 0; i < faceVertexCounts.size(); i++)
		{
			polyCounts.push_back(faceVertexCounts.cdata()[i]);
		}

	if (faceVertexIndicesAttr.Get(&faceVertexIndices))
		for (int i = 0; i < faceVertexIndices.size(); i++)
		{
			polyConnects.push_back(faceVertexIndices.cdata()[i]);
		}

	if (vertexColorAttr.Get(&u_vertexColors))
		for (int i = 0; i < u_vertexColors.size() * 3; i += 3)
		{
			zColor col = zColor(u_vertexColors.cdata()->GetArray()[i], u_vertexColors.cdata()->GetArray()[i + 1], u_vertexColors.cdata()->GetArray()[i + 2], 1);
			palette.push_back(col);
		}

	if (vertexColorIndicesAttr.Get(&u_vertexColorIndices))
		for (size_t i = 0; i < u_vertexColorIndices.size(); i++)
		{
			int id = u_vertexColorIndices[i];
			vertexColors.push_back(palette[id]);
		}

	// Convert the GfMatrix4d to a `zTransform` matrix
	if (usdMesh.GetLocalTransformation(&transform, &tmp))
	{
		double* data = transform.GetArray();
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				myTransform(i, j) = data[i * 4 + j];
			}
		}
	}

	// Create the zObjMesh
	fnMesh.create(positions, polyCounts, polyConnects);
	fnMesh.computeMeshNormals();

	// Set vertex colors
	fnMesh.setVertexColors(vertexColors, true);

	// Set the transformation matrix
	fnMesh.setTransform(myTransform);

}

void toUSDFile(UsdGeomMesh meshPrim, zObjMesh& oMesh)
{
	zFnMesh fnMesh(oMesh);

	//mesh attributes
	VtArray<GfVec3f> points;
	VtArray<GfVec3f> normals;
	VtArray<int> fVCounts;
	VtArray<int> fVIDs;
	GfMatrix4d transform;

	//custom attributes
	VtArray<GfVec4i> hes;
	VtArray<int> vertices;
	VtArray<int> faces;
	VtArray<GfVec3d> faceNormals;
	VtArray<GfVec3d> faceColors;
	VtArray<GfVec3f> vCols;
	VtArray<int> vColsIndices;

	//set mesh vertex attributes
	float** rawPos;
	float** rawNorm;

	int numV = fnMesh.numVertices();
	rawPos = new float* [fnMesh.numVertices() * 3];
	rawNorm = new float* [fnMesh.numVertices() * 3];
	fnMesh.getRawVertexPositions(rawPos);
	fnMesh.getRawVertexNormals(rawNorm);
	zColor* rawColor = fnMesh.getRawVertexColors();

	//set transformation
	Matrix4f t;
	fnMesh.getTransform(t);
	Matrix4d m;
	m = t.cast<double>();

	double val[4][4] = {
		{m(0,0),m(1,0),m(2,0),m(3,0)},
		{m(0,1),m(1,1),m(2,1),m(3,1)},
		{m(0,2),m(1,2),m(2,2),m(3,2)},
		{m(0,3),m(1,3),m(2,3),m(3,3)} };

	transform.Set(val);

	for (int i = 0; i < numV; i++)
	{
		GfVec3f v_attr, n_attr, c_attr;
		//set vertex positions
		v_attr.Set(*rawPos[i * 3 + 0], *rawPos[i * 3 + 1], *rawPos[i * 3 + 2]);
		points.push_back(v_attr);

		//set vertex normals
		n_attr.Set(*rawNorm[i * 3 + 0], *rawNorm[i * 3 + 1], *rawNorm[i * 3 + 2]);
		normals.push_back(n_attr);

		//set vertex colors
		c_attr.Set(rawColor[i].r, rawColor[i].g, rawColor[i].b);
		vCols.push_back(c_attr);
	}

	//set mesh face attributes
	for (zItMeshFace f(oMesh); !f.end(); f++)
	{
		fVCounts.push_back(f.getNumVertices());
		zIntArray v_idx;
		f.getVertices(v_idx);
		for (auto& o : v_idx)
			fVIDs.push_back(o);

		if (f.getHalfEdge().isActive())
			faces.push_back(f.getHalfEdge().getId());
		else faces.push_back(-1);
	}

	zVector* fn = fnMesh.getRawFaceNormals();
	zColor* fc = fnMesh.getRawFaceColors();
	for (int i = 0; i < fnMesh.numPolygons(); i++)
	{
		GfVec3d n;
		GfVec3d c;

		n[0] = fn[i].x;
		n[0] = fn[i].y;
		n[0] = fn[i].z;

		c[0] = fc[i].r;
		c[0] = fc[i].g;
		c[0] = fc[i].b;

		faceNormals.push_back(n);
		faceColors.push_back(c);
	}


	//create default attr
	meshPrim.CreateFaceVertexCountsAttr();
	meshPrim.CreateFaceVertexIndicesAttr();
	meshPrim.CreateNormalsAttr();
	meshPrim.CreatePointsAttr();
	meshPrim.CreateOrientationAttr();

	//create custom attr
	meshPrim.CreatePrimvar(TfToken("displayColor"), SdfValueTypeNames->Int3Array);
	meshPrim.CreatePrimvar(TfToken("displayOpacity"), SdfValueTypeNames->Int);

	meshPrim.CreatePrimvar(TfToken("HE_Halfedges"), SdfValueTypeNames->Int4Array);
	meshPrim.CreatePrimvar(TfToken("HE_Vertices"), SdfValueTypeNames->IntArray);
	meshPrim.CreatePrimvar(TfToken("HE_VertexColors"), SdfValueTypeNames->Float3Array);
	meshPrim.CreatePrimvar(TfToken("HE_Faces"), SdfValueTypeNames->IntArray);
	meshPrim.CreatePrimvar(TfToken("HE_FaceNormals"), SdfValueTypeNames->Double3Array);
	meshPrim.CreatePrimvar(TfToken("HE_FaceColors"), SdfValueTypeNames->Double3Array);

	//half edges
	for (zItMeshHalfEdge he(oMesh); !he.end(); he++)
	{
		GfVec4i he_attr;

		he_attr[0] = (he.getPrev().isActive()) ? he.getPrev().getId() : -1;
		he_attr[1] = (he.getNext().isActive()) ? he.getNext().getId() : -1;
		he_attr[2] = (he.getVertex().isActive()) ? he.getVertex().getId() : -1;
		he_attr[3] = (!he.onBoundary()) ? he.getFace().getId() : -1;

		hes.push_back(he_attr);
	}

	//half edge vertices
	for (zItMeshVertex v(oMesh); !v.end(); v++)
	{
		if (v.getHalfEdge().isActive()) vertices.push_back(v.getHalfEdge().getId());
		else vertices.push_back(-1);
	}

	//set mesh attributes
	UsdAttribute attr;
	attr = meshPrim.GetPointsAttr();
	attr.Set(points);
	attr = meshPrim.GetNormalsAttr();
	attr.Set(normals);
	attr = meshPrim.GetFaceVertexCountsAttr();
	attr.Set(fVCounts);
	attr = meshPrim.GetFaceVertexIndicesAttr();
	attr.Set(fVIDs);
	attr = meshPrim.GetOrientationAttr();
	attr.Set(TfToken("leftHanded"));
	attr = meshPrim.AddTransformOp();
	attr.Set(transform);

	//set customised attributes
	attr = meshPrim.GetPrimvar(TfToken("displayColor"));
	VtArray<GfVec3f> cols;
	attr.Set(vCols);
	attr = meshPrim.GetPrimvar(TfToken("displayOpacity"));
	VtArray<float> opacity;
	opacity.push_back(1);
	attr.Set(opacity);
	attr = meshPrim.GetPrimvar(TfToken("HE_Halfedges"));
	attr.Set(hes);
	attr = meshPrim.GetPrimvar(TfToken("HE_Vertices"));
	attr.Set(vertices);
	attr = meshPrim.GetPrimvar(TfToken("HE_VertexColors"));
	attr.Set(vCols);
	attr = meshPrim.GetPrimvar(TfToken("HE_Faces"));
	attr.Set(faces);
	attr = meshPrim.GetPrimvar(TfToken("HE_FaceNormals"));
	attr.Set(faceNormals);
	attr = meshPrim.GetPrimvar(TfToken("HE_FaceColors"));
	attr.Set(faceColors);



	stage_out->GetRootLayer()->Save();

}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMeshArray oMeshes;


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
		//--------------------- 1 - READ FILE --------------------------
		OV_Connect(destinationPath, filePath);

		stage_in = UsdStage::Open(filePath);
		if (!stage_in)
		{
			std::cout << "Failure to open stage.  Exiting." << std::endl;
		}
		else
		{
			std::cout << "Opened Stage: " << filePath << std::endl;

			// Print the up-axis
			std::cout << "Stage up-axis: " << UsdGeomGetStageUpAxis(stage_in) << std::endl;

			// Print the stage_in's linear units, or "meters per unit"
			std::cout << "Meters per unit: " << std::setprecision(5) << UsdGeomGetStageMetersPerUnit(stage_in) << std::endl;
		}

		for (pxr::UsdPrim prim : stage_in->Traverse())
		{
			if (prim.IsA<UsdGeomMesh>())
			{
				allMeshPrim.push_back(prim);
				SdfPath path;
				prim.GetPrimAtPath(path);
				primPaths.push_back(path);

				cout << "working" << endl;
			}
		}
		cout << "Found " << allMeshPrim.size() << " UsdGeomMesh" << endl;
		for (auto& name : primPaths)
			cout << name.GetText() << endl;

		allObjMesh.assign(allMeshPrim.size(), zObjMesh());

		for (int i = 0; i < allMeshPrim.size(); i++)
		{
			UsdGeomMesh usdMesh = UsdGeomMesh(allMeshPrim[i]);

			// Convert usd mesh to zMesh
			fromUSDFile(usdMesh, allObjMesh[i]);

			model.addObject(allObjMesh[i]);
			allObjMesh[i].setDisplayElements(true, true, true);
		}


		//--------------------- 2 - MESH OPERATION --------------------------
		for (int i = 0; i < allObjMesh.size(); i++)
		{
			zFnMesh fn(allObjMesh[i]);
			fn.smoothMesh(1, true);
		}

		//--------------------- 3 - WRITE TO FILE --------------------------
		stage_out = UsdStage::CreateNew(filePath_out);
		stage_out->SetMetadata(TfToken("defaultPrim"), VtValue("World"));
		//stage_out->SetMetadata(TfToken("doc"), VtValue("Generated from zSpace"));
		stage_out->SetMetadata(TfToken("upAxis"), VtValue("Z"));

		string s_root, s_layer, s_prim;
		s_root = "/World";
		s_layer = s_root + "/Geometry";
		s_prim = s_layer + "/mesh";
		UsdGeomXform root = UsdGeomXform::Define(stage_out, SdfPath(s_root));
		UsdGeomXform layer = UsdGeomXform::Define(stage_out, SdfPath(s_layer));

		// add all meshes to the usd file
		for (int i = 0; i < allObjMesh.size(); i++)
		{
			s_prim += i;
			UsdGeomMesh meshPrim = UsdGeomMesh::Define(stage_out, SdfPath(s_prim));

			// Convert zMesh to usd mesh
			toUSDFile(meshPrim, allObjMesh[i]);
		}

		cout << endl;
		cout << "Stage saved to: " << filePath_out << endl;

		OV_Disconnect();

		compute = !compute;
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
