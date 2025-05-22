//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>


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

pxr::UsdPrim rootPrim;
zObjMesh myObjMesh;
zObjMesh allObjMesh[3000];

//vector<pxr::UsdPrim> allPrim;
vector<pxr::UsdPrim> allMeshPrim;
pxr::UsdStageRefPtr myStage;


//
void zOBJMeshFromUsdMesh(UsdGeomMesh usdMesh, zObjMesh& oMesh)
{	
	zFnMesh fnMesh(oMesh);

	VtArray<GfVec3f> points;
	VtArray<GfVec3f> normals;
	VtArray<int>     faceVertexCounts;
	VtArray<int>     faceVertexIndices;
	GfMatrix4d transform;

	bool tmp = true;
	UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
	UsdAttribute normalsAttr = usdMesh.GetNormalsAttr();
	UsdAttribute faceVertexCountsAttr = usdMesh.GetFaceVertexCountsAttr();
	UsdAttribute faceVertexIndicesAttr = usdMesh.GetFaceVertexIndicesAttr();
	usdMesh.GetLocalTransformation(&transform, &tmp);

	pointsAttr.Get(&points);
	normalsAttr.Get(&normals);
	faceVertexCountsAttr.Get(&faceVertexCounts);
	faceVertexIndicesAttr.Get(&faceVertexIndices);

	zPointArray positions;
	zIntArray polyCounts;
	zIntArray polyConnects;
	zTransform myTransform;

	for (int i = 0; i < points.size() * 3; i += 3)
	{
		zPoint pos = zPoint(points.cdata()->GetArray()[i], points.cdata()->GetArray()[i + 1], points.cdata()->GetArray()[i + 2]);
		positions.push_back(pos);
	}

	for (int i = 0; i < faceVertexCounts.size(); i++)
	{
		polyCounts.push_back(faceVertexCounts.cdata()[i]);
	}

	for (int i = 0; i < faceVertexIndices.size(); i++)
	{
		polyConnects.push_back(faceVertexIndices.cdata()[i]);
	}

	double* data = transform.GetArray();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			myTransform(i, j) = data[i * 4 + j];			
		}
	}

	//cout << myTransform << endl;

	//get myTransform

	//(oMesh).mesh.clear();
	fnMesh.create(positions, polyCounts, polyConnects);
	fnMesh.setTransform(myTransform);
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
	
	//myStage = UsdStage::Open("data/USD/RhinoCube.usd");
	//myStage = UsdStage::Open("data/USD/GH_glass.usd");
	//myStage = UsdStage::Open("data/USD/Rhino_Glass_LM.usd");
	myStage = UsdStage::Open("data/USD/Rhino_3plane_LM.usd");
	//myStage = UsdStage::Open("data/USD/testMayaLive.usda");	
		
	rootPrim = myStage->GetDefaultPrim();

	for (pxr::UsdPrim prim : myStage->Traverse()) 
	{
		if (prim.IsA<UsdGeomMesh>())
		{
			allMeshPrim.push_back(prim);
		}
	}
	cout << allMeshPrim.size() << " UsdGeomMesh are found" << endl;

	for (int i = 0; i < allMeshPrim.size(); i++)
	{
		UsdGeomMesh usdMesh = UsdGeomMesh(allMeshPrim[i]);
		zOBJMeshFromUsdMesh(usdMesh, allObjMesh[i]);
		model.addObject(allObjMesh[i]);		
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
	model.draw();
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
