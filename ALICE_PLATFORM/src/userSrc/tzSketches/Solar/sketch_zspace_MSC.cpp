//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

//#include <headers/zCudaToolsets/energy/zTsSolarOcclusionGPU.h>

//#include <headers/zInterface/functionsets/zFnMeshComputeField.h>

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

//define sun and buildings
zObjMesh oMesh_TestBuilding;
zObjMesh oMesh_Sun;

//sunPlane
zPlane sunPlane;

//transform from sunPlane to world
zTransform t;
zObjPointCloud cloud;

zPoint tP;
//define pixel receiver
zObjMeshScalarField oMesh_Field;

////// --- GUI OBJECTS ----------------------------------------------------
void computeBuildingBBox(zObjMesh& _oMesh, zPoint &_minBB, zPoint &_maxBB)
{
	zPoint minBB, maxBB;
	zFnMesh fnMesh(_oMesh);
	fnMesh.getBounds(minBB, maxBB);

	_minBB = minBB;
	_maxBB = maxBB;
}

void computeTransform(zVector& _sunVec, zVector& _sunCenter, zVector& _minBB, zVector& _maxBB, zTransform& _t, zTransform& _tLocal)
{
	zVector O = _sunCenter;
	zVector Z = _sunVec;

	zVector tCenter = (_minBB + _maxBB) / 2;
	double dist = core.minDist_Point_Plane(tCenter, O, Z);
	zVector target = Z * dist * -1;
	tCenter += target;

	zTransform t;

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

	//output
	_t = t;
	_tLocal.setIdentity();
}

void flatternBuilding(zObjMesh& _oMesh, zVector _sunVec, zVector _sunCenter, zTransform _t, zTransform _tLocal)
{
	//project buildingMesh
	zVector O = _sunCenter;
	zVector Z = _sunVec;
	zFnMesh fnMesh(_oMesh);

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		double dist = core.minDist_Point_Plane(fnMesh.getRawVertexPositions()[i], O, Z);
		zVector target = Z * dist * -1;
		fnMesh.getRawVertexPositions()[i] += target;
	}

	//transform buildingMesh
	fnMesh.setTransform(_t, true, false);
	fnMesh.setTransform(_tLocal, true, true);
}

void createBufferMap(zVector& _minBB3d, zVector& _maxBB3d, float* _bufferRawPos, int* _bufferValue, int resX, int resY)
{
	float a = _maxBB3d.x - _minBB3d.x;
	float b = _maxBB3d.y - _minBB3d.y;
	float c = _maxBB3d.z - _minBB3d.z;

	float d = pow(pow(a, 2) + pow(b, 2) + pow(c, 2), 0.5);
	d /= 2;

	zPoint minBB(-d, -d, 0);
	zPoint maxBB(d, d, 0);

	for (int i = 0; i < resX; i++) {
		for (int j = 0; j < resY * 3; j += 3) {
			float unitX = (maxBB.x - minBB.x) / (resX-1);
			float unitY = (maxBB.y - minBB.y) / (resY-1);
			zPoint now(minBB.x + unitX * i, minBB.y + unitY * j/3, 0.0f);
			_bufferRawPos[i * resY * 3 + j + 0] = now.x;
			_bufferRawPos[i * resY * 3 + j + 1] = now.y;
			_bufferRawPos[i * resY * 3 + j + 2] = now.z;

			_bufferValue[i * resY + j / 3] = 0;
		}
	}

	//for (int i = 0; i < resX; i++) {
	//	for (int j = 0; j < resY; j ++) {
	//		float unitX = (maxBB.x - minBB.x) / resX;
	//		float unitY = (maxBB.y - minBB.y) / resY;
	//		zPoint now(minBB.x + unitX * i, minBB.y + unitY * j, 0.0f);
	//		_bufferRawPos[i * resY + j + 0] = now.x;
	//		_bufferRawPos[i * resY + j + 1] = now.y;
	//		_bufferRawPos[i * resY + j + 2] = now.z;

	//		_bufferValue[i * resY + j] = 0;

	//		cout << "x " << _bufferRawPos[i * resY + j + 0] << ",";
	//		cout << "y " << _bufferRawPos[i * resY + j + 1] << ",";
	//		cout << "z " << _bufferRawPos[i * resY + j + 2] << endl;
	//	}
	//}
}

void computeZBuffer(zObjMesh& _oMesh, zVector& _sunVec, zVector& _sunCenter, float* _ZBuffer)
{
	zVector O = _sunCenter;
	zVector Z = _sunVec;
	zFnMesh fnMesh(_oMesh);
	zVectorArray fCenters;
	fnMesh.getCenters(zFaceData, fCenters);

	for (int i = 0; i < fnMesh.numPolygons(); i++)
	{
		_ZBuffer[i] = core.minDist_Point_Plane(fCenters[i], O, Z);
	}
}

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
	//testBuildingMesh.from("data/SolarMesh/solar_Building.obj", zOBJ);
	//testBuildingMesh.from("data/SolarMesh/genzon_HP.obj", zOBJ);
	//testBuildingMesh.from("data/SolarMesh/hotspot/hotspot_building.obj", zOBJ);
	testBuildingMesh.from("data/SolarMesh/testMSC.obj", zOBJ);


	zFnMesh sunMesh(oMesh_Sun);
	sunMesh.from("data/SolarMesh/solar_Sun.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/solar_Sun_genzon.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/solar_Sun_genzon_mul.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/hotspot/hotspot_sun_two.obj", zOBJ);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh_TestBuilding);
	model.addObject(oMesh_Sun);
	//model.addObject(oMesh_Field);

	model.addObject(cloud);
	cloud.setDisplayElements(true);

	// set display element booleans
	//oMesh_Field.setDisplayElements(true, true, true);
	oMesh_TestBuilding.setDisplayElements(false, true, true);
	oMesh_Sun.setDisplayElements(true, true, true);

#pragma endregion "AlicePreparation"

	//new//////////////////////////////////////////////////////////////////////////

	float* bufferRawPos;
	int* bufferValue;
	float* ZBuffer;

	float* vertices ;
	float* occlusionVal;

	int resX = 10;
	int resY = 10;

	const int NUM_SUN = sunMesh.numPolygons();
	const int NUM_V = testBuildingMesh.numVertices();
	const int NUM_F = testBuildingMesh.numPolygons();
	const int NUM_B = resX * resY * 3;

	bufferRawPos = new float[resX * resY * 3];
	bufferValue = new int[resX * resY];
	ZBuffer = new float[NUM_F];

	zVectorArray rawSunPos;
	zVectorArray rawSunVecs;
	sunMesh.getCenters(zFaceData, rawSunPos);
	sunMesh.getFaceNormals(rawSunVecs);

	zVector sunVec = rawSunVecs[0];
	zVector sunPos = rawSunPos[0];

	computeZBuffer(oMesh_TestBuilding, sunVec, sunPos, ZBuffer);

	zPoint minBB3d;
	zPoint maxBB3d;
	zPoint tCenter;
	zTransform t;
	zTransform tLocal;
	computeBuildingBBox(oMesh_TestBuilding, minBB3d, maxBB3d);
	computeTransform(sunVec, sunPos, minBB3d, maxBB3d, t, tLocal);
	flatternBuilding(oMesh_TestBuilding, sunVec, sunPos, t, tLocal);

	createBufferMap(minBB3d, maxBB3d, bufferRawPos, bufferValue, resX, resY);

	auto begin = std::chrono::high_resolution_clock::now();
	//test some point
	int id = 45;
	tP = zVector(bufferRawPos[(id)*3 + 0], bufferRawPos[(id) * 3 + 1], bufferRawPos[(id) * 3 + 2]);

	int bufferID = -1;
	float dist = 99999.9f;
	for (size_t i = 0; i < NUM_F; i++)
	{
		zPointArray poly;
		zItMeshFace f(oMesh_TestBuilding, i);
		f.getVertexPositions(poly);

		if (ZBuffer[i] < dist)
		{
			if (core.pointInPlanarPolygon(tP, poly, zVector(0, 0, 1)))
			{
				bufferID = i;
				dist = ZBuffer[i];
			}
		}
	}

	cout << "buffer id "<< bufferID << endl;

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

	//get building vertices
	zVector* rawVPos = testBuildingMesh.getRawVertexPositions();

	float* facePositions = new float[testBuildingMesh.numPolygons() * 4];

	for (zItMeshFace f(oMesh_TestBuilding); !f.end(); f++)
	{
		zIntArray fVerts;
		f.getVertices(fVerts);

		int fID = f.getId();

		for (int i =0; i< fVerts.size(); i++)
		{
			facePositions[fID * 4 + i * 3 + 0] = rawVPos[fVerts[i]].x;
			facePositions[fID * 4 + i * 3 + 1] = rawVPos[fVerts[i]].y;
			facePositions[fID * 4 + i * 3 + 2] = rawVPos[fVerts[i]].z;
		}

	}
	
	vertices = new float[NUM_V * 3];
	for (int i = 0; i < NUM_V; i++)
	{
		vertices[i * 3 + 0] = rawVPos[i].x;
		vertices[i * 3 + 1] = rawVPos[i].y;
		vertices[i * 3 + 2] = rawVPos[i].z;
	}

	zVector testMin;
	zVector testMax;
	zFnMesh fnMesh(oMesh_TestBuilding);
	fnMesh.getBounds(testMin, testMax);

	cout << "testmin " << testMin << endl;
	cout << "testmax " << testMax << endl;
	cout << "bufferMin " << bufferRawPos[0] << "," << bufferRawPos[1] << "," << bufferRawPos[2] << endl;
	cout << "bufferMax " << bufferRawPos[resX*resY*3-3]<<","<<bufferRawPos[resX * resY*3 - 2] <<","<< bufferRawPos[resX * resY*3 - 1] << endl;

	zPointArray pts;
	zFnPointCloud fnCloud(cloud);
	for (size_t i = 0; i < resX*resY; i++)
	{
		pts.push_back(zVector(bufferRawPos[i * 3 + 0], bufferRawPos[i * 3 + 1], bufferRawPos[i * 3 + 2]));
	}
	fnCloud.addPositions(pts);

	//setup occlusion value
	occlusionVal = new float[NUM_SUN * NUM_V];




	//new//////////////////////////////////////////////////////////////////////////

	#pragma region "ComputeOcclusion"


		//for (auto& o : occlusion) cout << "occlusion " << o << endl;

	#pragma endregion "ComputeOcclusion"

		//////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	//////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	//B.addButton(&display, "display");
	//B.buttons[1].attachToVariable(&display);

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
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		model.displayUtils.drawTransform(sunPlane);

		for (zItPointCloudVertex v(cloud); !v.end(); v++)
		{
			model.displayUtils.drawPoint(v.getPosition(), zColor(), 1);
		}

		model.displayUtils.drawPoint(tP, zColor(), 10);


		//zObjMeshScalarField field(oMesh_Field);
		//field.draw();
		//for(auto &p : positions) model.displayUtils.drawPoint(p);
	}



	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Sun #:" + to_string(sunCount), Alice::vec(winW - 350, winH - 500, 0));
	drawString("Sun #:" + to_string(sunCount-1), Alice::vec(50, 400, 0));

	

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == '+') compute = true;;
	if (k == '.') write = true;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
