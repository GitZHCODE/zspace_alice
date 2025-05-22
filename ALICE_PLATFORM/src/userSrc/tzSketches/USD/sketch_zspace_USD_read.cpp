//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/api.h>
#include <pxr/usd/usd/object.h>
#include <pxr/usd/usd/schemaBase.h>


using namespace zSpace;

using namespace pxr;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

//zUtilsCore core;

//zObjMesh oMesh;
//zObjGraph oGraph;

//zPointArray positions;


pxr::UsdPrim myPrim;
pxr::UsdStageRefPtr myStage;




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

	myStage = UsdStage::Open("data/USD/RhinoCube.usd");
	
	//pxr::SdfPathSet pathSet = myStage->GetLoadSet();

	myPrim = myStage->GetPrimAtPath(SdfPath("/World/Default/Geometry/Default/Mesh0"));
	cout << endl << myPrim.GetDescription();
	
	UsdGeomMesh usdMesh(myPrim);
	size_t numF = usdMesh.GetFaceCount();


	VtArray<GfVec3f> points;
	VtArray<GfVec3f> normals;
	VtArray<int> fVCounts;
	VtArray<int> fVIDs;
	VtArray<GfVec3f> vCols;

	TfTokenVector tokens = myPrim.GetPropertyNames();
	for (auto& o : tokens) cout << o << endl;

	//faceCount
	cout << endl;
	cout << "FaceCount_" << numF << endl;

	//faceVertexCounts 
	if (UsdAttribute attr = myPrim.GetAttribute(TfToken("faceVertexCounts")))
	{ 
		attr.Get(&fVCounts);
		cout << endl << "faceVertexCounts_" << endl;
		for (int i = 0; i < fVCounts.size(); i++)
			cout << fVCounts.cdata()[i] << ",";
	}

	//faceVertexIndices  
	if (UsdAttribute attr = myPrim.GetAttribute(TfToken("faceVertexIndices")))
	{
		attr.Get(&fVIDs);
		cout << endl << "faceVertexIndices_" << endl;
		for (int i = 0; i < fVIDs.size(); i++)
			cout << fVIDs.cdata()[i] << ",";
	}

	//normals  
	if (UsdAttribute attr = myPrim.GetAttribute(TfToken("normals")))
	{
		attr.Get(&normals);
		cout << endl << "normals_" << endl;
		for (int i = 0; i < normals.size()*3; i++)
		{
			cout << normals.cdata()->GetArray()[i + 0] << ",";
			cout << normals.cdata()->GetArray()[i + 1] << ",";
			cout << normals.cdata()->GetArray()[i + 2] << ",";
		}
	}

	//points  
	if (UsdAttribute attr = myPrim.GetAttribute(TfToken("points")))
	{
		attr.Get(&points);
		cout << endl << "points_" << endl;
		for (int i = 0; i < points.size() * 3; i += 3)
		{
			cout << points.cdata()->GetArray()[i+0] << ",";
			cout << points.cdata()->GetArray()[i+1] << ",";
			cout << points.cdata()->GetArray()[i+2] << ",";
		}
	}

	//displayColor  
	if (UsdAttribute attr = myPrim.GetAttribute(TfToken("displayColor")))
	{
		attr.Get(&vCols);
		cout << endl << "displayColor_" << endl;
		for (int i = 0; i < vCols.size()*3; i++)
		{
			cout << vCols.cdata()->GetArray()[i + 0] << ",";
			cout << vCols.cdata()->GetArray()[i + 1] << ",";
			cout << vCols.cdata()->GetArray()[i + 2] << ",";
		}
	}





	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object

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
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{		
		
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
