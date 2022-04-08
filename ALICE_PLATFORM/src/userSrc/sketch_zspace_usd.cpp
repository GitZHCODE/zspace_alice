#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usd/primRange.h>


#include <ctime>
#include <iostream>

using namespace zSpace;
using namespace std;
using namespace pxr;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;


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


	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);



	///////////////////////////////////////////////////////////////

	UsdStageRefPtr stage = UsdStage::CreateNew("hello_zSpace.usda");

	SdfPath pathRoot = SdfPath("/Root");
	SdfPath pathSphere = SdfPath("/SphereMesh");

	UsdGeomXform root = UsdGeomXform::Define(stage, pathRoot);

	//root.AddXformOp(UsdGeomXformOp::Type::TypeTransform);
	
	GfMatrix4d t;
	t.SetIdentity();
	root.AddTransformOp().Set(t);

	UsdGeomSphere::Define(stage, pathSphere);


	//////Metadata

	time_t time_now = std::time(nullptr);

	struct tm timeinfo;

	timeinfo.tm_year = 2025 - 1900;
	timeinfo.tm_mon = 12 - 1;
	timeinfo.tm_mday = 13;
	timeinfo.tm_hour = 16;
	timeinfo.tm_min = 46;
	timeinfo.tm_sec = 45;

	time_t end_time = mktime(&timeinfo);


	string transforms = "";

	for (UsdPrim prim : stage->Traverse()) {
		if (prim.IsA<UsdGeomXform>())
		{
			if (UsdAttribute attr = prim.GetAttribute(TfToken("xformOp:transform")))
			{
				VtValue val;
				attr.Get(&val);

				auto data = val.Get<GfMatrix4d>().data();

				for (int i = 0; i < 16; i++)
				{
					transforms += " " + std::to_string(data[i]);
				}
	
				cout << transforms << endl;
			}
		}
	}



	stage->SetMetadata(TfToken("owner"), "treasury_x_ZHA");
	stage->SetMetadata(TfToken("sessionOwner"), "Cesar");
	stage->SetMetadata(TfToken("startTimeCode"), time_now);
	stage->SetMetadata(TfToken("endTimeCode"), end_time);
	stage->SetMetadata(TfToken("documentation"), transforms);

	stage->GetRootLayer()->Save();


	system("usdview C:/Users/cesar.fragachan/Documents/Repos/zSpace_Alice/ALICE_PLATFORM/x64/Release/EXE/hello_zSpace.usda");
}

void update(int value)
{
	if (compute)
	{

		
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
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
