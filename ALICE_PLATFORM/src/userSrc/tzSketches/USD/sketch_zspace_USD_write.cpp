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
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usd/object.h>
#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primTypeInfo.h>


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

zObjMesh oMesh;
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

	zFnMesh fnMesh(oMesh);
	zFloat4 rot = { 0,0,45,1 };
	fnMesh.setRotation(rot, false);
	fnMesh.from("data/USD/colorCube.json", zJSON);
	
	string Dir = "data/USD/colorCube.usda";
	myStage = UsdStage::CreateNew(Dir);
	UsdGeomXform root = UsdGeomXform::Define(myStage, SdfPath("/World"));
	UsdGeomXform layer = UsdGeomXform::Define(myStage, SdfPath("/World/Geometry"));
	UsdGeomMesh meshPrim = UsdGeomMesh::Define(myStage, SdfPath("/World/Geometry/mesh"));

	//UsdPrim defaultPrim = myStage->CreateClassPrim(SdfPath("/World"));
	myStage->SetMetadata(TfToken("defaultPrim"), VtValue("World"));
	myStage->SetMetadata(TfToken("doc"), VtValue("Generated from zSpace"));
	myStage->SetMetadata(TfToken("upAxis"), VtValue("Z"));

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
		{m(0,3),m(1,3),m(2,3),m(3,3)}};

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
	VtArray<GfVec3f> col;
	col.push_back(GfVec3f(0.98, 0.98, 0.98));
	attr.Set(col);
	attr = meshPrim.GetPrimvar(TfToken("displayOpacity"));
	VtArray<float> opacity;
	opacity.push_back(0);
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



	myStage->GetRootLayer()->Save();




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
