//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
//#include <headers/zCudaToolsets/energy/zTsSolarOcclusion.h>
//#include <headers/zCudaToolsets/energy/zTsSolarOcclusionGPU.h>

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

//transform from sunPlane to world
zTransform t;
zObjPointCloud cloud;

//define pixel receiver
zObjMeshScalarField oMesh_Field;

////// --- GUI OBJECTS ----------------------------------------------------
#pragma region SOLAR METHODS

void getFieldBBox(zFnMesh& buildingMesh, zVectorArray& sunCenter, zVectorArray& sunVector, zVectorArray& outTransformCenters, zVector& outFieldMinBB, zVector& outFieldMaxBB)
{
	//buildingMesh bbox 3d
	zPoint minBB, maxBB;
	buildingMesh.getBounds(minBB, maxBB);
	zVectorArray buildingBBox3d;
	buildingBBox3d.assign(8, zVector(0, 0, 0));

	buildingBBox3d[0] = zPoint(minBB.x, minBB.y, minBB.z);
	buildingBBox3d[1] = zPoint(maxBB.x, minBB.y, minBB.z);
	buildingBBox3d[2] = zPoint(maxBB.x, maxBB.y, minBB.z);
	buildingBBox3d[3] = zPoint(minBB.x, maxBB.y, minBB.z);
	buildingBBox3d[4] = zPoint(minBB.x, minBB.y, maxBB.z);
	buildingBBox3d[5] = zPoint(maxBB.x, minBB.y, maxBB.z);
	buildingBBox3d[6] = zPoint(maxBB.x, maxBB.y, maxBB.z);
	buildingBBox3d[7] = zPoint(minBB.x, maxBB.y, maxBB.z);

	//for (auto& p : buildingBBox3d) cout << p << endl;

	zPointArray fieldBBoxTemp;
	for (int i = 0; i < sunCenter.size(); i++)
	{
		zVector O = sunCenter[i];
		zVector Z = sunVector[i];

		zVectorArray p = buildingBBox3d;
		//compute projection
		zPoint tCenter;
		for (int j = 0; j < 8; j++)
		{
			double dist = core.minDist_Point_Plane(p[j], O, Z);
			zVector target = Z * dist * -1;
			p[j] += target;
			tCenter += p[j];
		}
		tCenter /= 8;
		outTransformCenters[i] = tCenter;

		//compute transform
		zTransform t;
		zTransform tLocal;

		Z.normalize();
		zVector basis(0, 1, 0);
		zVector X = basis ^ Z;

		zVector Y = Z ^ X;
		Y.normalize();

		X = Y ^ Z;
		X.normalize();

		t.setIdentity();
		t(0, 0) = X.x; t(0, 1) = X.y; t(0, 2) = X.z;
		t(1, 0) = Y.x; t(1, 1) = Y.y; t(1, 2) = Y.z;
		t(2, 0) = Z.x; t(2, 1) = Z.y; t(2, 2) = Z.z;
		t(3, 0) = tCenter.x; t(3, 1) = tCenter.y; t(3, 2) = tCenter.z;

		tLocal.setIdentity();

		//transform building bbox 3d to field plane
		zObjPointCloud cloud;
		zFnPointCloud fnCloud(cloud);
		fnCloud.addPositions(p);
		fnCloud.setTransform(t, true, false);
		fnCloud.setTransform(tLocal, true, true);

		zPoint* vPositions;
		vPositions = fnCloud.getRawVertexPositions();

		//push transformed building bbox to array
		for (int k = 0; k < fnCloud.numVertices(); k++)
		{
			fieldBBoxTemp.push_back(vPositions[k]);
			cout << vPositions[k] << endl;
		}
	}
	core.getBounds(fieldBBoxTemp, outFieldMinBB, outFieldMaxBB);
}

#pragma endregion SOLAR METHODS

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
	testBuildingMesh.from("data/SolarMesh/genzon_HP.obj", zOBJ);
	//testBuildingMesh.from("data/SolarMesh/hotspot/hotspot_building.obj", zOBJ);

	zFnMesh sunMesh(oMesh_Sun);
	sunMesh.from("data/SolarMesh/solar_Sun_genzon_mul.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/hotspot/hotspot_sun.obj", zOBJ);

#pragma region "DATA PREPARATION"
	//////////////////////////////////////////////////////////  DATA PREPARATION
	auto begin = std::chrono::high_resolution_clock::now();

	const int RES_X = 300;
	const int RES_Y = 300;
	const int NUM_SUN = sunMesh.numPolygons();
	const int NUM_V = testBuildingMesh.numVertices();
	const int NUM_F = testBuildingMesh.numPolygons();
	float* sunVecs;
	float* sunCenters;
	float* vertices;
	float* faceCenters;
	int* polyConnects;
	float* fieldPos;
	int* fieldVal;
	float* tCenters;
	vector<zTransform> tArr;
	vector<zTransform> tLocalArr;

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
	zPointArray rawSunCenters;
	sunMesh.getCenters(zFaceData, rawSunCenters);
	sunCenters = new float[NUM_SUN * 3];
	for (int i = 0; i < NUM_SUN; i++)
	{
		sunCenters[i * 3 + 0] = rawSunCenters[i].x;
		sunCenters[i * 3 + 1] = rawSunCenters[i].y;
		sunCenters[i * 3 + 2] = rawSunCenters[i].z;
	}

	//get building vertices
	zVector* rawVPos = testBuildingMesh.getRawVertexPositions();
	vertices = new float[NUM_V * NUM_SUN * 3];
	for (int i = 0; i < NUM_SUN; i++)
	{
		for (int j = 0; j < NUM_V; j++)
		{
			vertices[i * j * 3 + 0] = rawVPos[j].x;
			vertices[i * j * 3 + 1] = rawVPos[j].y;
			vertices[i * j * 3 + 2] = rawVPos[j].z;
		}
	}

	//get building face centers
	zPointArray rawFPos;
	testBuildingMesh.getCenters(zFaceData, rawFPos);
	faceCenters = new float[NUM_V * NUM_SUN * 3];
	for (int i = 0; i < NUM_SUN; i++)
	{
		for (int j = 0; j < NUM_F; j++)
		{
			faceCenters[i * j * 3 + 0] = rawVPos[j].x;
			faceCenters[i * j * 3 + 1] = rawVPos[j].y;
			faceCenters[i * j * 3 + 2] = rawVPos[j].z;
		}
	}

	//get building polyconnects
	polyConnects = new int[NUM_F * NUM_SUN * 4];
	for (int i = 0; i < NUM_SUN; i++)
	{
		for (int j = 0; j < NUM_F; j++)
		{
			zItMeshFace face(oMesh_TestBuilding, j);
			zIntArray pID;
			face.getVertices(pID);
			for (int k = 0; k < pID.size(); k++)
			{
				polyConnects[i * j * 4 + k] = pID[k];
			}
		}
	}

	//get field position and value
	zObjMeshScalarField oField;
	zFnMeshScalarField fnField(oField);
	zVector fieldMinBB, fieldMaxBB;
	zVectorArray transformCenters;
	transformCenters.assign(NUM_SUN, zVector());
	getFieldBBox(testBuildingMesh, rawSunCenters, rawSunVecs, transformCenters, fieldMinBB, fieldMaxBB);
	fnField.create(fieldMinBB, fieldMaxBB, RES_X, RES_Y, 1, true, false);
	tCenters = new float[NUM_SUN * 3];
	for (int i = 0; i < NUM_SUN; i++)
	{
		tCenters[i * 3 + 0] = transformCenters[i].x;
		tCenters[i * 3 + 1] = transformCenters[i].y;
		tCenters[i * 3 + 2] = transformCenters[i].z;
	}

	//init field value
	zFloatArray value;
	value.assign(RES_X * RES_Y, 0);
	fnField.setFieldValues(value);
	zVector* fRawPos = fnField.fnMesh.getRawVertexPositions();
	zScalar* fRawVal = fnField.getRawFieldValues();
	const int NUM_F_POS = fnField.fnMesh.numVertices();
	const int NUM_F_VAL = fnField.numFieldValues();
	fieldPos = new float[NUM_F_POS * 3];
	for (int i = 0; i < NUM_F_POS; i++)
	{
		fieldPos[i * 3 + 0] = fRawPos[i].x;
		fieldPos[i * 3 + 1] = fRawPos[i].y;
		fieldPos[i * 3 + 2] = fRawPos[i].z;
	}
	fieldVal = new int[NUM_F_VAL];
	for (int i = 0; i < NUM_F_VAL; i++)
	{
		fieldPos[i] = fRawVal[i];
	}

	cout << "minbb" << fieldMinBB << endl;
	cout << "maxBB" << fieldMaxBB << endl;
	

auto end = std::chrono::high_resolution_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

#pragma endregion "DATA PREPARATION"


#pragma region "AlicePreparation"

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh_TestBuilding);
	model.addObject(oMesh_Sun);

	model.addObject(cloud);
	cloud.setDisplayElements(true);

	// set display element booleans
	oMesh_TestBuilding.setDisplayElements(false, true, true);
	oMesh_Sun.setDisplayElements(true, true, true);

#pragma endregion "AlicePreparation"

	#pragma region "SolarPreparation"

	//solar.setSunVec(sunVecs, NUM_SUN);
	//solar.setSunCenter(sunCenters, NUM_SUN);
	//solar.setResolution(RES_X, RES_Y);
	//solar.setVertices(vertices, NUM_V);
	//solar.setFaceCenters(faceCenters, NUM_F);
	//solar.setPolyConnects(polyConnects, NUM_F);
	//solar.setTransformCenters(tCenters, NUM_SUN);

	//solar.computeOcclusion();

		//zVectorArray sunVecs;
		//zVectorArray sunCenters;
		//int resX = 300;
		//int resY = 300;
	
		//sunMesh.getFaceNormals(sunVecs);
		//sunMesh.getCenters(zFaceData, sunCenters);
	
		//numSun = sunVecs.size();
		//solar.setSunVec(&sunVecs[0], sunVecs.size());
		//solar.setSunCenter(&sunCenters[0], sunVecs.size());
		//solar.setBuildingMesh(oMesh_TestBuilding);
		//solar.setResolution(resX, resY);
		////-=------------------------------
		////sun
		//const int NUM_SUN = 6;
		//float sunVecs[NUM_SUN] = {};
		//float sunCenters[NUM_SUN] = {};

		////field
		//const int resX = 300;
		//const int resY = 300;


	#pragma endregion "SolarPreparation"

		//cout << endl;
		//cout << "resX " << resX << endl;
		//cout << "resY " << resX << endl;
	
	#pragma region "ComputeOcclusion"
		//auto begin = std::chrono::high_resolution_clock::now();

		//solar.computeOcclusion();
		//zBoolArray occlusion = solar.getOcclusion();

		//auto end = std::chrono::high_resolution_clock::now();
		//auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		//printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

		//for (auto& o : occlusion) cout << "occlusion " << o << endl;

	#pragma endregion "ComputeOcclusion"

		//occlusionLong = solar.getOcclusionLong();

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

		for (zItPointCloudVertex v(cloud); !v.end(); v++)
		{
			model.displayUtils.drawPoint(v.getPosition(), zColor(), 10);
		}

		//zObjMeshScalarField field = solar.getField();
		//field.draw();
		//for(auto &p : positions) model.displayUtils.drawPoint(p);
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
