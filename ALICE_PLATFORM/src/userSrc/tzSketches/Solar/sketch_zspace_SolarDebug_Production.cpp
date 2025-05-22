//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
//#include <headers/zApp/include/zTsStatics.h>
#include <headers/zCudaToolsets/energy/zTsSolarOcclusion.h>
//#include <headers/zCudaToolsets/energy/zTsSolarAnalysis.h>
//#include<headers/zToolsets/geometry/zTsSDFBridge.h>


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
zTsSolarOcclusion solar;

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
	testBuildingMesh.from("data/SolarMesh/hotspot/hotspot_building.obj", zOBJ);

	zFnMesh sunMesh(oMesh_Sun);
	//sunMesh.from("data/SolarMesh/solar_Sun.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/solar_Sun_genzon.obj", zOBJ);
	//sunMesh.from("data/SolarMesh/solar_Sun_genzon_mul.obj", zOBJ);
	sunMesh.from("data/SolarMesh/hotspot/hotspot_sun_two.obj", zOBJ);

	
	zDoubleArray radValue;
	radValue.assign(testBuildingMesh.numVertices(), 0);

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

	#pragma region "SolarPreparation"

		zVectorArray sunVecs;
		zVectorArray sunCenters;
		int resX = 2400;
		int resY = 2400;
	
		sunMesh.getFaceNormals(sunVecs);
		sunMesh.getCenters(zFaceData, sunCenters);
	
		numSun = sunVecs.size();
		solar.setSunVec(&sunVecs[0], sunVecs.size());
		solar.setSunCenter(&sunCenters[0], sunVecs.size());
		solar.setBuildingMesh(oMesh_TestBuilding);
		solar.setResolution(resX, resY);
	#pragma endregion "SolarPreparation"

		cout << endl;
		cout << "resX " << resX << endl;
		cout << "resY " << resX << endl;
	
	#pragma region "ComputeOcclusion"
		auto begin = std::chrono::high_resolution_clock::now();

		solar.computeOcclusion();
		zBoolArray occlusion = solar.getOcclusion();
		//zIntArray occValue = solar.getOcclusionValue();

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

		//for (auto& o : occlusion) cout << "occlusion " << o << endl;

	#pragma endregion "ComputeOcclusion"

		occlusionLong = solar.getOcclusionLong();

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
	if (compute)
	{
		zFnMesh fnMesh(oMesh_TestBuilding);
		int size = fnMesh.numVertices();
		cols.assign(size, colA);

			for (int i = sunCount * size; i < (sunCount + 1) * size; i++)
			{
				int id = i - (sunCount * size);
				if (occlusionLong[i]) cols[id] = colB;
			}
			fnMesh.setVertexColors(cols, true);

		sunCount++;
		if (sunCount > numSun) sunCount = 0;
		compute = !compute;	
		cout << sunCount << endl;
	}
	if (write)
	{
		string dir = "data/SolarMesh/hotspot/occlusion.txt";
		ofstream myfile;
		myfile.open(dir);
		for (int i = 0; i < occlusionLong.size(); i++)
		{
			myfile << occlusionLong[i];
		}
		myfile.close();
		write = !write;
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
