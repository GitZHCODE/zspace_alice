//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <zCudaToolsets/energy/zTsSolarOcclusionGPU.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool COMPUTE_OCCLUSION = false;
bool DISPLAY_OCCLUSION = false;

float* sunPath_hour_pts;
float* sunPath_day_pts;
float* compass_pts;

zDomainColor colDomain(zColor(180, 1, 1), zColor(360, 1, 1));

double background = 0.35;

////////////////////////////////////////////////////////////////////////// zSpace Objects

/*!<model*/
zModel model;

/*!<zSpace Toolset*/
zTsSolarOcclusionGPU solar;

/*!<zSpace utils*/
zUtilsCore core;

// zSpace CUDA
bool GPU_COMPUTE;

zObjPointCloud cloud;
//define sun and buildings
zObjMesh oMesh_TestBuilding;
zObjMesh oMesh_VisualBuilding;
zObjMesh oMesh_Sun;

zColorArray cols;
zColor colA(0, 0, 0, 1);
zColor colB(1, 1, 1, 1);

int sunCount = 0;
int numSun;

int resX = 1024;
int resY = 1024;

ZSPACE_EXTERN bool checkCudaExists(string& version);

ZSPACE_EXTERN void cleanDeviceMemory();

ZSPACE_EXTERN bool cdComputeOcclusion(zTsSolarOcclusionGPU& sOcclusion);

void computeBuildingBBox(zObjMesh& _oMesh, zPoint& _minBB, zPoint& _maxBB)
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

void createBufferMap(zVector& _minBB3d, zVector& _maxBB3d, float* _bufferRawPos, float* _bufferValue, int resX, int resY)
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
			float unitX = (maxBB.x - minBB.x) / (resX - 1);
			float unitY = (maxBB.y - minBB.y) / (resY - 1);
			zPoint now(minBB.x + unitX * i, minBB.y + unitY * j / 3, 0.0f);
			_bufferRawPos[i * resY * 3 + j + 0] = now.x;
			_bufferRawPos[i * resY * 3 + j + 1] = now.y;
			_bufferRawPos[i * resY * 3 + j + 2] = now.z;

			_bufferValue[i * resY + j + 0] = -1;
			_bufferValue[i * resY + j + 1] = -1;
			_bufferValue[i * resY + j + 2] = -1;
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

void computeOcclusionCPU(float* buffer_facepositions, int* buffervalues, int numBuffer, int numFacePositions)
{
	/*for (int i = 0; i < numBuffer; i++)
	{
		zPoint bufferPos(buffer_facepositions[i + 0], buffer_facepositions[i + 1], buffer_facepositions[i + 2]);

		int faceposition_offset = numBuffer - i;

		float dist = 10000000;

		buffervalues[i] = -1;

		for (int o = i; o < i + numFacePositions; o += 16)
		{
			int j = o + faceposition_offset;
			int faceID = floorf((j - numBuffer) / 16);

			zPoint p0(buffer_facepositions[j + 0], buffer_facepositions[j + 1], buffer_facepositions[j + 2]);
			zPoint p1(buffer_facepositions[j + 4], buffer_facepositions[j + 5], buffer_facepositions[j + 6]);
			zPoint p2(buffer_facepositions[j + 8], buffer_facepositions[j + 9], buffer_facepositions[j + 10]);
			zPoint p3(buffer_facepositions[j + 12], buffer_facepositions[j + 13], buffer_facepositions[j + 14]);

			zPointArray poly;
			poly.push_back(p0);
			poly.push_back(p1);
			poly.push_back(p2);
			poly.push_back(p3);

			if (buffer_facepositions[j + 3] < dist)
			{
				if (core.pointInPlanarPolygon(bufferPos, p0, p1, p2, p3, zVector(0, 0, 1)))
				{
					buffervalues[i + 0] = faceID;
					dist = buffer_facepositions[j + 3];
				}
			}
		}
	}*/
}

void setColor(zObjMesh& _oMesh, float* _bufferValue, int _numBuffer, zColor colA, zColor colB)
{
	// Sorting the given array
	sort(_bufferValue, _bufferValue + _numBuffer);

	zFnMesh fnMesh(_oMesh);
	zColorArray cols;
	cols.assign(fnMesh.numPolygons(), colA);

	// Finding unique numbers
	int counter = 0;
	for (int i = 0; i < _numBuffer; i++)
	{
		if (_bufferValue[i] == _bufferValue[i + 1])
		{
			continue;
		}
		else
		{
			if (_bufferValue[i] != -1)
			{
				cols[_bufferValue[i]] = colB;
				counter++;
			}
		}
	}
	cout << endl;
	cout << counter << "faces in sun" << endl;

	fnMesh.setFaceColors(cols, true);
}

void setColorGPU(zObjMesh& _oMesh, int* _bufferValue, int _numBuffer, zColor colA, zColor colB)
{
	// Sorting the given array
	sort(_bufferValue, _bufferValue + _numBuffer);

	zFnMesh fnMesh(_oMesh);
	zColorArray cols;
	cols.assign(fnMesh.numPolygons(), colA);

	// Finding unique numbers
	for (int i = 0; i < _numBuffer; i++)
	{
		if (_bufferValue[i] == _bufferValue[i + 1])
		{
			continue;
		}
		else
		{
			if (_bufferValue[i] != -1)
			{
				cols[_bufferValue[i]] = colB;
			}
		}
	}

	fnMesh.setFaceColors(cols, true);
}

void setup()
{

	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(200000);
	// read mesh
	zFnMesh testBuildingMesh(oMesh_TestBuilding);
	testBuildingMesh.from("data/SolarMesh/testMSC.obj", zOBJ);
	//testBuildingMesh.smoothMesh(1,false);

	oMesh_VisualBuilding = oMesh_TestBuilding;

	zFnMesh sunMesh(oMesh_Sun);
	sunMesh.from("data/SolarMesh/solar_Sun.obj", zOBJ);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh_TestBuilding);
	model.addObject(oMesh_VisualBuilding);
	model.addObject(oMesh_Sun);

	// set display element booleans
	oMesh_TestBuilding.setDisplayElements(false, true, true);
	oMesh_Sun.setDisplayElements(true, true, true);

	//////////////////////////////////////////////////////////  OCCLUSION SETUP
	auto begin = std::chrono::high_resolution_clock::now();

	zVectorArray rawSunPos;
	zVectorArray rawSunVecs;
	sunMesh.getCenters(zFaceData, rawSunPos);
	sunMesh.getFaceNormals(rawSunVecs);

	zVector sunVec = rawSunVecs[0];
	zVector sunPos = rawSunPos[0];

	float* bufferRawPos;
	float* bufferValues;
	float* ZBuffer;

	const int NUM_SUN = sunMesh.numPolygons();
	const int NUM_V = testBuildingMesh.numVertices();
	const int NUM_F = testBuildingMesh.numPolygons() * 16;
	const int NUM_B = resX * resY * 3;

	bufferRawPos = new float[resX * resY * 3];
	bufferValues = new float[resX * resY * 3];
	ZBuffer = new float[NUM_F];

	//initalise buffer positions and values in CPU
	computeZBuffer(oMesh_TestBuilding, sunVec, sunPos, ZBuffer);

	zPoint minBB3d;
	zPoint maxBB3d;
	zTransform t;
	zTransform tLocal;
	computeBuildingBBox(oMesh_TestBuilding, minBB3d, maxBB3d);
	computeTransform(sunVec, sunPos, minBB3d, maxBB3d, t, tLocal);
	flatternBuilding(oMesh_TestBuilding, sunVec, sunPos, t, tLocal);
	createBufferMap(minBB3d, maxBB3d, bufferRawPos, bufferValues, resX, resY);

	//assign face positions
	zVector* rawVPos = testBuildingMesh.getRawVertexPositions();
	float* facePositions = new float[NUM_F * 4 * 4];
	for (zItMeshFace f(oMesh_TestBuilding); !f.end(); f++)
	{
		zIntArray fVerts;
		f.getVertices(fVerts);

		int fID = f.getId();

		for (int i = 0; i < fVerts.size(); i++)
		{
			facePositions[fID * 16 + i * 4 + 0] = rawVPos[fVerts[i]].x;
			facePositions[fID * 16 + i * 4 + 1] = rawVPos[fVerts[i]].y;
			facePositions[fID * 16 + i * 4 + 2] = rawVPos[fVerts[i]].z;
			facePositions[fID * 16 + i * 4 + 3] = ZBuffer[fID];
		}
	}

	//set values in host
	solar.setBuffer(bufferRawPos, bufferValues, NUM_B);
	solar.setFacePositions(facePositions, NUM_F);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	printf("\n CPU Time: %.7f seconds.", elapsed.count() * 1e-9);

	//////////////////////////////////////////////////////////  OCCLUSION COMPUTE
	//compute occlusion GPU
	bool out = cdComputeOcclusion(solar);
	float* bufferValueGPU = solar.getRawBufferValues();

	////compute occlusion CPU
	//float* buffer_facePositions;
	//std::copy(bufferRawPos, bufferRawPos + NUM_B, buffer_facePositions);
	//std::copy(facePositions, facePositions + NUM_F, buffer_facePositions + NUM_B);
	//computeOcclusionCPU(buffer_facePositions, bufferValues, NUM_B, NUM_F);

	//////////////////////////////////////////////////////////  OCCLUSION VISUALISE
	//visualise buffer map
	zFnPointCloud fnCloud(cloud);
	for (int i = 0; i < NUM_B/3; i++)
	{
		zVector tP(bufferRawPos[i * 3 + 0], bufferRawPos[i * 3 + 1], bufferRawPos[i * 3 + 2]);
		fnCloud.addPosition(tP);
		//cout<< "bufferValue" << i / 3 << "_ " << bufferValues[i] << endl;
		//cout << "bufferValueCPU" << i << "_ " << bufferValues[i*3] << "  |  bufferValueGPU" << i << "_ " << bufferValueGPU[i * 3] << endl;
	}
	//setColor(oMesh_VisualBuilding, bufferValues, NUM_B, colA, colB);
	setColor(oMesh_VisualBuilding, bufferValueGPU, NUM_B, colA, colB);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&GPU_COMPUTE, "GPU_COMPUTE");
	B.buttons[0].attachToVariable(&GPU_COMPUTE);

	B.addButton(&DISPLAY_OCCLUSION, "DISPLAY_OCCLUSION");
	B.buttons[1].attachToVariable(&DISPLAY_OCCLUSION);
}

void update(int value)
{
	if (GPU_COMPUTE)
	{
		bool out = cdComputeOcclusion(solar);

		GPU_COMPUTE = !GPU_COMPUTE;
	}
	if (DISPLAY_OCCLUSION)
	{
		float* bufferValue = solar.getRawBufferValues();

		//setColorGPU(oMesh_VisualBuilding, solar.numFacePositions(), bufferValue, solar.numBuffer(), colA, colB);

		//for (int i = 0; i < solar.numBuffer(); i+=3)
		//{
		//	cout << "bufferVal_" << i << " " << bufferValue[i] << endl;
		//}

		DISPLAY_OCCLUSION = !DISPLAY_OCCLUSION;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	// zspace model draw

	for (zItPointCloudVertex v(cloud); !v.end(); v++)
	{
		model.displayUtils.drawPoint(v.getPosition(), zColor(), 1);
	}

	model.draw();

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') GPU_COMPUTE = true;
	if (k == 'd') DISPLAY_OCCLUSION = true;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
