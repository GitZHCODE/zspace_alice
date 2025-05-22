//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
//#include <headers/zCudaToolsets/energy/zTsSolarOcclusion.h>
//#include <headers/zCudaToolsets/energy/zTsSolarOcclusionGPU.h>
#include <stdio.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool write = false;
bool computeMap = false;
bool display = true;

int faceCount = 0;

//double background = 0.35;
double background = 1;


////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;
/*!<Objects*/

zBoolArray occlusionLong;
zColorArray cols;
zColor colA(0, 0, 0, 1);
zColor colB(1, 1, 1, 1);
int sunCount = 0;
int numSun;

zUtilsCore core;
//zTsSolarOcclusion solar;
//zTsSolarOcclusionGPU solar;

//define sun and buildings
zObjMesh oMesh_TestBuilding;
zObjMesh oMesh_Sun;

//sunPlane
zPlane sunPlane;

// zSpace CUDA

//bool GPU_COMPUTE;
//string GPUString;
//
//ZSPACE_EXTERN bool checkCudaExists(string& version);
//
//ZSPACE_EXTERN void cleanDeviceMemory();
//
//ZSPACE_EXTERN float cdOcclusionValue(zTsSolarOcclusionGPU& solarOcclusion, bool EPWRead);

////// --- GUI OBJECTS ----------------------------------------------------
void setup()
{
#pragma region "AlicePreparation"

	////////////////////////////////////////////////////////////////////////// Enable smooth display
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(200000);
	// read mesh
	zFnMesh testBuildingMesh(oMesh_TestBuilding);
	//testBuildingMesh.from("data/SolarMesh/genzon_HP.obj", zOBJ);
	//testBuildingMesh.from("data/SolarMesh/hotspot/hotspot_building.obj", zOBJ);
	testBuildingMesh.from("data/SolarMesh/solar_Building.obj", zOBJ);
	

	zFnMesh sunMesh(oMesh_Sun);
	sunMesh.from("data/SolarMesh/solar_Sun_genzon_mul.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/hotspot/hotspot_sun.obj", zOBJ);

	/*
#pragma region DATA PREPARATION
	//////////////////////////////////////////////////////////  DATA PREPARATION
	auto begin = std::chrono::high_resolution_clock::now();

	const int NUM_SUN = sunMesh.numPolygons();
	const int NUM_V = testBuildingMesh.numVertices();
	const int NUM_F = testBuildingMesh.numPolygons();
	float* sunVecs;
	float* sunCenters;
	float* vertices;
	float* normals;
	float* fCenters;
	float* fNormals;
	int* polyConnects;
	float* occlusionVal;

	//get sun vectors
	zVectorArray rawSunVecs;
	sunMesh.getFaceNormals(rawSunVecs);
	sunVecs = new float[NUM_SUN * 3];
	for (int i = 0; i < NUM_SUN; i++)
	{
		sunVecs[i * 3 + 0] = rawSunVecs[i].x;
		sunVecs[i * 3 + 1] = rawSunVecs[i].y;
		sunVecs[i * 3 + 2] = rawSunVecs[i].z;
	}

	//get sun centers
	zVectorArray rawSunPos;
	sunMesh.getCenters(zFaceData, rawSunPos);
	sunCenters = new float[NUM_SUN * 3];
	for (int i = 0; i < NUM_SUN; i++)
	{
		sunCenters[i * 3 + 0] = rawSunPos[i].x;
		sunCenters[i * 3 + 1] = rawSunPos[i].y;
		sunCenters[i * 3 + 2] = rawSunPos[i].z;
	}

	//get building vertices
	zVector* rawVPos = testBuildingMesh.getRawVertexPositions();
	vertices = new float[NUM_V * 3];
	for (int i = 0; i < NUM_V; i++)
	{
		vertices[i * 3 + 0] = rawVPos[i].x;
		vertices[i * 3 + 1] = rawVPos[i].y;
		vertices[i * 3 + 2] = rawVPos[i].z;
	}

	//get building vertex normals
	zVector* rawVNormals = testBuildingMesh.getRawVertexNormals();
	normals = new float[NUM_V * 3];
	for (int i = 0; i < NUM_V; i++)
	{
		normals[i * 3 + 0] = rawVNormals[i].x;
		normals[i * 3 + 1] = rawVNormals[i].y;
		normals[i * 3 + 2] = rawVNormals[i].z;
	}
	core.ray_triangleIntersection()

	//get building face centers
	zVectorArray rawFPos;
	testBuildingMesh.getCenters(zFaceData, rawFPos);
	fNormals = new float[NUM_F * 3];
	for (int i = 0; i < NUM_F; i++)
	{
		fCenters[i * 3 + 0] = rawFPos[i].x;
		fCenters[i * 3 + 1] = rawFPos[i].y;
		fCenters[i * 3 + 2] = rawFPos[i].z;
	}

	//get building face normals
	zVectorArray rawFNormals;
	testBuildingMesh.getFaceNormals(rawFNormals);
	fNormals = new float[NUM_F * 3];
	for (int i = 0; i < NUM_F; i++)
	{
		fNormals[i * 3 + 0] = rawFNormals[i].x;
		fNormals[i * 3 + 1] = rawFNormals[i].y;
		fNormals[i * 3 + 2] = rawFNormals[i].z;
	}

	//get building polyconnects
	polyConnects = new int[NUM_F * 4];
	for (int i = 0; i < NUM_F; i++)
	{
		zItMeshFace face(oMesh_TestBuilding, i);
		zIntArray pID;
		face.getVertices(pID);
		for (int j = 0; j < pID.size(); j++)
		{
			polyConnects[i * 4 + j] = pID[j];
		}
	}

	//setup occlusion value
	occlusionVal = new float[NUM_SUN * NUM_V];
	//for (int i = 0; i < NUM_SUN; i++)
	//{
	//	for (int j = 0; j < NUM_V; j++)
	//	{
	//		occlusionVal[i * j + j] = (rawVNormals[j] * rawSunVecs[i] > 0)? rawVNormals[j] * rawSunVecs[i] : 0;
	//	}
	//}
	for (size_t i = 0; i < NUM_SUN * NUM_V; i++)
	{
		cout << occlusionVal[i] << endl;
	}

auto end = std::chrono::high_resolution_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
printf("\n Data Preparation Time: %.7f seconds.", elapsed.count() * 1e-9);


#pragma endregion DATA PREPARATION
*/

//declaration
const int NUN_V = testBuildingMesh.numVertices();
const int NUN_F = testBuildingMesh.numPolygons();

zBoolArray occlusion;
occlusion.assign(NUN_V, false);

zVectorArray rawSunVecs;
zVectorArray rawSunPos;

sunMesh.getFaceNormals(rawSunVecs);
sunMesh.getCenters(zFaceData, rawSunPos);

zVector rawSunVec = rawSunVecs[5];
zVector rawSunCenter = rawSunPos[5];

zVector* rawVPos = testBuildingMesh.getRawVertexPositions();
zVector* rawVNormals = testBuildingMesh.getRawVertexNormals();
zVectorArray rawFPos;
zVectorArray rawFNormals;

testBuildingMesh.getCenters(zFaceData, rawFPos);
testBuildingMesh.getFaceNormals(rawFNormals);

zBoolArray Vinsun;
zBoolArray Finsun;

Vinsun.assign(NUN_V, false);
Finsun.assign(NUN_F, false);

//check in sun
for (size_t i = 0; i < NUN_V; i++) Vinsun[i] = (rawSunVec * rawVNormals[i] > 0) ? true : false;
for (size_t i = 0; i < NUN_F; i++) Finsun[i] = (rawSunVec * rawFNormals[i] > 0) ? true : false;

int counter = 0;
auto begin = std::chrono::high_resolution_clock::now();
//compute occlusion
for (size_t i = 0; i < NUN_V; i++)
{
	if (Vinsun[i])
	{
		for (size_t j = 0; j < NUN_F; j++)
		{
			float distV = core.minDist_Point_Plane(rawVPos[i], rawSunCenter, rawSunVec);
			float distF = core.minDist_Point_Plane(rawFPos[j], rawSunCenter, rawSunVec);
			if (Finsun[j] && distV > distF)
			{
				zItMeshFace face(oMesh_TestBuilding, j);
				zIntArray pID;
				face.getVertices(pID);
				zPointArray poly;
				poly.assign(4, zPoint());

				bool self = false;
				for (int k = 0; k < pID.size(); k++)
				{
					poly[k] = rawVPos[pID[k]];
					if (i == pID[k]) self = true;
				}
				if (self) continue;
				else
				{
					bool inPoly = core.pointInPlanarPolygon(rawVPos[i], poly, rawSunVec);
					counter++;
					if (inPoly)
					{
						occlusion[i] = true;
						break;
					}
				}
			}
		}
	}
	else occlusion[i] = true;
}
auto end = std::chrono::high_resolution_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

//for (auto& o : occlusion) cout << o << endl;

zColorArray cols;
zColor colA(0, 0, 0, 1);
zColor colB(1, 1, 1, 1);

		zFnMesh fnMesh(oMesh_TestBuilding);
		int size = fnMesh.numVertices();
		cols.assign(size, colA);

		for (int i = 0; i < NUN_V; i++)
		{
			if (occlusion[i]) cols[i] = colB;
		}
		fnMesh.setVertexColors(cols, true);



#pragma region AlicePreparation

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh_TestBuilding);
	model.addObject(oMesh_Sun);

	// set display element booleans
	oMesh_TestBuilding.setDisplayElements(false, true, true);
	oMesh_Sun.setDisplayElements(true, true, true);

#pragma endregion AlicePreparation

#pragma region SolarPreparation

	//solar.setSunVec(sunVecs, NUM_SUN);
	//solar.setSunCenter(sunCenters, NUM_SUN);
	//solar.setResolution(RES_X, RES_Y);
	//solar.setVertices(vertices, NUM_V);
	//solar.setFaceCenters(faceCenters, NUM_F);
	//solar.setPolyConnects(polyConnects, NUM_F);
	//solar.setTransformCenters(tCenters, NUM_SUN);

	//solar.computeOcclusion();

#pragma endregion SolarPreparation

		//////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	//////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
}

void update(int value)
{
	//if (compute)
	//{
	//	zFnMesh fnMesh(oMesh_TestBuilding);
	//	int size = fnMesh.numVertices();
	//	cols.assign(size, colA);

	//		for (int i = sunCount * size; i < (sunCount + 1) * size; i++)
	//		{
	//			int id = i - (sunCount * size);
	//			if (occlusionLong[i]) cols[id] = colB;
	//		}
	//		fnMesh.setVertexColors(cols, true);

	//	sunCount++;
	//	if (sunCount > numSun) sunCount = 0;
	//	compute = !compute;	
	//	cout << sunCount << endl;
	//}
	//if (write)
	//{
	//	string dir = "data/SolarMesh/hotspot/occlusion.txt";
	//	ofstream myfile;
	//	myfile.open(dir);
	//	for (int i = 0; i < occlusionLong.size(); i++)
	//	{
	//		myfile << occlusionLong[i];
	//	}
	//	myfile.close();
	//	write = !write;
	//}
}

void draw()
{
	backGround(background);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		model.displayUtils.drawTransform(sunPlane);
	}



	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("Sun #:" + to_string(sunCount-1), Alice::vec(50, 400, 0));
	restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == '+') compute = true;
	if (k == '.') write = true;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
