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
// a table lookup per path element). Similarly, any API which
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

// Returns true if the provided maybeURL contains a host and path
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
	std::string stageUrl = destinationPath +"/" + stageName + (doLiveEdit ? ".live" : ".usd");

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

	// Connect to OV
	bool doLiveEdit = false;
	std::string existingStage;
	string destinationPath = "omniverse://nucleus.zaha-hadid.com/Projects";
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

	// Create the USD model in Omniverse
	string newStageName = "newBox";
	const std::string stageUrl = createOmniverseModel(destinationPath, newStageName, doLiveEdit);

	//open existing stage
	existingStage = "omniverse://nucleus.zaha-hadid.com/Projects/mayaTest_VB.usd";

	// Print the username for the server
	printConnectedUsername(existingStage);

	UsdStageRefPtr stage = UsdStage::Open(existingStage);
	if (!stage)
	{
		std::cout << "Failure to open stage.  Exiting." << std::endl;		
	}
	else
	{
		std::cout << "Opened Stage: " << existingStage << std::endl;

		// Print the up-axis
		std::cout << "Stage up-axis: " << UsdGeomGetStageUpAxis(stage) << std::endl;

		// Print the stage's linear units, or "meters per unit"
		std::cout << "Meters per unit: " << std::setprecision(5) << UsdGeomGetStageMetersPerUnit(stage) << std::endl;
	}

	// All done, shut down our connection to Omniverse
	shutdownOmniverse();
		
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
