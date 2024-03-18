//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zCudaToolsets/energy/zTsSolarOcclusionGPU.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool COMPUTE_OCCLUSION = false;
bool DISPLAY_OCCLUSION = false;
bool NEXTSUN = false;
bool SMOOTH = false;

bool QUALITY_LOW = false;
bool QUALITY_MEDIUM = false;
bool QUALITY_HIGH = false;
bool QUALITY_EXTREME = false;

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
zObjMesh oMesh_ContextBuilding;
zObjMesh oMesh_VisualContext;
zObjMesh oMesh_Sun;

zColorArray cols;
zColor colA(0, 0, 0, 1);
zColor colB(1, 1, 1, 1);

int sunCount = -1;
int numSun;

#define RES_LOW 256
#define RES_MEDIUM 512
#define RES_HIGH 1024
#define RES_EXTREME 2048

int resX = RES_EXTREME;
int resY = RES_EXTREME;

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

void flatternBuilding(zObjMesh& _oMesh, zObjMesh& _oMesh_context, zVector _sunVec, zVector _sunCenter, zTransform _t, zTransform _tLocal)
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

	//context
	zFnMesh fnMesh_context(_oMesh_context);
	for (int i = 0; i < fnMesh_context.numVertices(); i++)
	{
		double dist = core.minDist_Point_Plane(fnMesh_context.getRawVertexPositions()[i], O, Z);
		zVector target = Z * dist * -1;
		fnMesh_context.getRawVertexPositions()[i] += target;
	}

	//transform buildingMesh
	fnMesh_context.setTransform(_t, true, false);
	fnMesh_context.setTransform(_tLocal, true, true);
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

void computeZBuffer(zObjMesh& _oMesh, zObjMesh& _oMesh_context, zVector& _sunVec, zVector& _sunCenter, float* _ZBuffer)
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

	zFnMesh fnMesh_context(_oMesh_context);
	zVectorArray fCenters_context;
	fnMesh_context.getCenters(zFaceData, fCenters_context);
	for (int i = fnMesh.numPolygons(); i < fnMesh.numPolygons()+ fnMesh_context.numPolygons(); i++)
	{
		_ZBuffer[i] = core.minDist_Point_Plane(fCenters_context[i- fnMesh.numPolygons()], O, Z);
	}
}

void computeOcclusionCPU(float* buffer_facepositions, int* buffervalues, int numBuffer, int numFacePositions)
{
	for (int i = 0; i < numBuffer; i++)
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
	}
}

void setColor(zObjMesh& _oMesh, int* _bufferValue, int _numBuffer, zColor colA, zColor colB)
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
			if (_bufferValue[i] != -1 && _bufferValue[i] < fnMesh.numPolygons())
			{
				cols[_bufferValue[i]] = colB;
				counter++;
			}
		}
	}
	cout << endl;
	cout << counter << "faces in sun" << endl;

	fnMesh.setFaceColors(cols, true);
	fnMesh.computeVertexColorfromFaceColor();
	//fnMesh.smoothColors(1, zFaceData);
	//zColorArray vcols;
	//fnMesh.getVertexColors(vcols);
	//for (auto &c : vcols) cout << c.r << endl;
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
	zFnMesh contextBuildingMesh(oMesh_ContextBuilding);
	//testBuildingMesh.from("data/SolarMesh/testMSC.obj", zOBJ);
	//testBuildingMesh.from("data/SolarMesh/hotspot/hotspot_building.obj", zOBJ);
	testBuildingMesh.from("data/SolarMesh/genzon_HP.obj", zOBJ);
	//testBuildingMesh.from("data/honduras/0823_building.obj", zOBJ);
	//testBuildingMesh.smoothMesh(1, false);


	//contextBuildingMesh.from("data/SolarMesh/testMSC.obj", zOBJ);
	//contextBuildingMesh.from("data/SolarMesh/solar_occluder.obj", zOBJ);
	contextBuildingMesh.from("data/SolarMesh/solar_occluder_city.obj", zOBJ);

	oMesh_VisualBuilding = oMesh_TestBuilding;
	oMesh_VisualContext = oMesh_ContextBuilding;

	zFnMesh sunMesh(oMesh_Sun);
	//sunMesh.from("data/SolarMesh/solar_Sun.obj", zOBJ);
	sunMesh.from("data/SolarMesh/solar_Sun_genzon_mul.obj", zOBJ);
	//sunMesh.from("data/honduras/sun.obj", zOBJ);


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh_TestBuilding);
	model.addObject(oMesh_VisualBuilding);
	model.addObject(oMesh_VisualContext);
	model.addObject(oMesh_Sun);

	// set display element booleans
	oMesh_TestBuilding.setDisplayElements(false, true, true);
	oMesh_VisualBuilding.setDisplayElements(false, true, true);
	oMesh_Sun.setDisplayElements(true, true, true);

	//////////////////////////////////////////////////////////  OCCLUSION SETUP





	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&GPU_COMPUTE, "GPU_COMPUTE");
	B.buttons[0].attachToVariable(&GPU_COMPUTE);

	B.addButton(&NEXTSUN, "NEXTSUN");
	B.buttons[1].attachToVariable(&NEXTSUN);

	B.addButton(&QUALITY_LOW, "QUALITY_LOW");
	B.buttons[2].attachToVariable(&QUALITY_LOW);

	B.addButton(&QUALITY_MEDIUM, "QUALITY_MEDIUM");
	B.buttons[3].attachToVariable(&QUALITY_MEDIUM);

	B.addButton(&QUALITY_HIGH, "QUALITY_HIGH");
	B.buttons[4].attachToVariable(&QUALITY_HIGH);

	B.addButton(&QUALITY_EXTREME, "QUALITY_EXTREME");
	B.buttons[5].attachToVariable(&QUALITY_EXTREME);

	B.addButton(&SMOOTH, "SMOOTH");
	B.buttons[5].attachToVariable(&SMOOTH);
}

void update(int value)
{
	if (GPU_COMPUTE)
	{
		cout << endl << "------------------------------------------------------------" <<endl;
		cout << "resX_" << resX << endl;
		cout << "resY_" << resY << endl;
		cout << "current sun_ " << sunCount << endl;
		cout << "current sun_ " << sunCount << endl;

		auto begin = std::chrono::high_resolution_clock::now();

		zVectorArray rawSunPos;
		zVectorArray rawSunVecs;
		zFnMesh sunMesh(oMesh_Sun);

		sunMesh.getCenters(zFaceData, rawSunPos);
		sunMesh.getFaceNormals(rawSunVecs);

		zVector sunVec = rawSunVecs[sunCount];
		//zVector sunPos = rawSunPos[sunCount];
		zVector sunPos(0, 0, 0);

		zObjMesh oMesh_B = oMesh_TestBuilding;
		zObjMesh oMesh_C = oMesh_ContextBuilding;
		zFnMesh testBuildingMesh(oMesh_B);
		zFnMesh contextBuildingMesh(oMesh_C);
		float* bufferRawPos;
		int* bufferValues;
		float* ZBuffer;

		const int NUM_SUN = sunMesh.numPolygons();
		const int NUM_V = testBuildingMesh.numVertices();
		const int NUM_F_test = testBuildingMesh.numPolygons() * 16;
		const int NUM_F_context = contextBuildingMesh.numPolygons() * 16;
		int NUM_F = NUM_F_test + NUM_F_context;
		const int NUM_B = resX * resY * 3;

		bufferRawPos = new float[resX * resY * 3];
		bufferValues = new int[resX * resY * 3];
		ZBuffer = new float[NUM_F];

		zPoint minBB3d;
		zPoint maxBB3d;
		computeBuildingBBox(oMesh_B, minBB3d, maxBB3d);

		float a = maxBB3d.x - minBB3d.x;
		float b = maxBB3d.y - minBB3d.y;
		float c = maxBB3d.z - minBB3d.z;
		float d = pow(pow(a, 2) + pow(b, 2) + pow(c, 2), 0.5);
		d /= 2;

		sunPos += sunVec * d * -1;
		//initalise buffer positions and values in CPU
		computeZBuffer(oMesh_B, oMesh_C, sunVec, sunPos, ZBuffer);

		zTransform t;
		zTransform tLocal;
		computeTransform(sunVec, sunPos, minBB3d, maxBB3d, t, tLocal);
		flatternBuilding(oMesh_B, oMesh_C, sunVec, sunPos, t, tLocal);
		createBufferMap(minBB3d, maxBB3d, bufferRawPos, bufferValues, resX, resY);

		//assign test face positions
		float* facePositions = new float[NUM_F * 4 * 4];

		zVector* rawVPos = testBuildingMesh.getRawVertexPositions();
		for (zItMeshFace f(oMesh_B); !f.end(); f++)
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

		//assign context face positions
		zPoint minBB2d(-d, -d, 0);
		zPoint maxBB2d(d, d, 0);

		zVector* rawVPos_context = contextBuildingMesh.getRawVertexPositions();
		for (zItMeshFace f(oMesh_C); !f.end(); f++)
		{
			zIntArray fVerts;
			f.getVertices(fVerts);
			zPoint center = f.getCenter();
			if (center.x > minBB2d.x && center.y > minBB2d.y && center.x < maxBB2d.x && center.y < maxBB2d.y)
			{

				int fID = f.getId() + NUM_F_test / 16;

				for (int i = 0; i < fVerts.size(); i++)
				{
					facePositions[fID * 16 + i * 4 + 0] = rawVPos_context[fVerts[i]].x;
					facePositions[fID * 16 + i * 4 + 1] = rawVPos_context[fVerts[i]].y;
					facePositions[fID * 16 + i * 4 + 2] = rawVPos_context[fVerts[i]].z;
					facePositions[fID * 16 + i * 4 + 3] = ZBuffer[fID];
				}
			}
		}

		//set values in host
		solar.setBuffer(bufferRawPos, bufferValues, NUM_B);
		solar.setFacePositions(facePositions, NUM_F);

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n CPU Time: %1.8f ms.", elapsed.count() * 1e-6);

		//////////////////////////////////////////////////////////  OCCLUSION COMPUTE
		//compute occlusion GPU
		bool out = cdComputeOcclusion(solar);
		int* bufferValueGPU = solar.getRawBufferValues();

		//////////////////////////////////////////////////////////  OCCLUSION VISUALISE
		//visualise buffer map
		//zFnPointCloud fnCloud(cloud);
		//for (int i = 0; i < NUM_B / 3; i++)
		//{
		//	zVector tP(bufferRawPos[i * 3 + 0], bufferRawPos[i * 3 + 1], bufferRawPos[i * 3 + 2]);
		//	fnCloud.addPosition(tP);
		//	//cout<< "bufferValue" << i / 3 << "_ " << bufferValues[i] << endl;
		//	//cout << "bufferValueCPU" << i << "_ " << bufferValues[i*3] << "  |  bufferValueGPU" << i << "_ " << bufferValueGPU[i * 3] << endl;
		//}

		auto begin1 = std::chrono::high_resolution_clock::now();

		setColor(oMesh_VisualBuilding, bufferValueGPU, NUM_B, colA, colB);

		auto end1 = std::chrono::high_resolution_clock::now();
		auto elapsed1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - begin1);
		printf("\n post Time: %1.8f ms.", elapsed1.count() * 1e-6);

		GPU_COMPUTE = !GPU_COMPUTE;
	}

	if (DISPLAY_OCCLUSION)
	{

		//setColorGPU(oMesh_VisualBuilding, solar.numFacePositions(), bufferValue, solar.numBuffer(), colA, colB);

		//for (int i = 0; i < solar.numBuffer(); i+=3)
		//{
		//	cout << "bufferVal_" << i << " " << bufferValue[i] << endl;
		//}

		DISPLAY_OCCLUSION = !DISPLAY_OCCLUSION;
	}

	if (NEXTSUN)
	{
		zFnMesh fnMesh(oMesh_Sun);
		int numSun = fnMesh.numPolygons();

		sunCount = (sunCount < numSun - 1) ? sunCount + 1 : 0;
		GPU_COMPUTE = true;
		
		NEXTSUN = !NEXTSUN;
	}
	if (SMOOTH)
	{
		zFnMesh fnMesh(oMesh_TestBuilding);
		//fnMesh.smoothMesh(1,false);

		SMOOTH = !SMOOTH;
	}

	if (QUALITY_LOW) {resX = RES_LOW; resY = RES_LOW;  QUALITY_LOW = !QUALITY_LOW;}
	if (QUALITY_MEDIUM) { resX = RES_MEDIUM; resY = RES_MEDIUM;  QUALITY_MEDIUM = !QUALITY_MEDIUM;}
	if (QUALITY_HIGH) { resX = RES_HIGH; resY = RES_HIGH; QUALITY_HIGH = !QUALITY_HIGH;}
	if (QUALITY_EXTREME) { resX = RES_EXTREME; resY = RES_EXTREME;QUALITY_EXTREME = !QUALITY_EXTREME;}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	// zspace model draw

	//for (zItPointCloudVertex v(cloud); !v.end(); v++)
	//{
	//	model.displayUtils.drawPoint(v.getPosition(), zColor(), 1);
	//}

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
	if (k == '+') NEXTSUN = true;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
