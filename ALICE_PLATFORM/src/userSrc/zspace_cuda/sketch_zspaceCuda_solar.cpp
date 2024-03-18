#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zCudaToolsets/energy/zTsSolarAnalysis.h>

using namespace zSpace;

////////////////////////////////////////////////////////////////////////// General

bool COMPUTE_SUNPATH = false;
bool BAKE_SUNPATH = false;
bool COMPUTE_RADIATION = false;

bool READ_EPW = false;
bool COLORMESH = false;
bool EXPORTMESH = false;

zLocation LOCATION;

string inPath;
string outPath;

int dummyYEAR = 0;
int YEAR = 2020;
zInt2 MONTHS = { 6,6 };
zInt2 HOURS = { 6,18 };
zInt2 DATES = { 21,21 };

float* sunPath_hour_pts;
float* sunPath_day_pts;
float* compass_pts;

zDomainColor colDomain(zColor(180, 1, 1), zColor(360, 1, 1));
zDomainFloat radiationDomain(RAD_MIN, RAD_MAX);

double background = 0.35;
double compScale = 1.0;

////////////////////////////////////////////////////////////////////////// zSpace Objects

/*!<objects*/
zObjGraphArray o_sunPaths;

zObjMesh o_Mesh;

/*!<model*/
zModel model;

/*!<zSpace Toolset*/
zTsSolarAnalysis solar;

/*!<zSpace utils*/
zUtilsCore core;

// zSpace CUDA

bool GPU_COMPUTE;
string GPUString;

ZSPACE_EXTERN bool checkCudaExists(string& version);

ZSPACE_EXTERN void cleanDeviceMemory();

ZSPACE_EXTERN bool cdpCummulativeRadiation(zTsSolarAnalysis& sAnalysis, bool EPWRead);

ZSPACE_INLINE void loadConfig(string& _inPath, string& _outPath, zDomainDate& dDates, zLocation& dLocation)
{
	json j;
	string file = "data/solarRadiationConfig.json";
	bool chk = core.readJSON(file, j);

	if (chk)
	{
		_inPath = j["InMesh"].get<string>();
		_outPath = j["OutMesh"].get<string>();

		int year = j["Year"].get<int>();
		const json& months = j["Months"].get<vector<int>>();
		const json& dates = j["Dates"].get<vector<int>>();
		const json& hours = j["Hours"].get<vector<int>>();

		const json& location = j["Location"].get<vector<float>>();

		dDates = zDomainDate(zDate(year, months[0], dates[0], hours[0], 1), zDate(year, months[1], dates[1], hours[1], 1));
		dLocation.latitude = location[0];
		dLocation.longitude = location[1];
		dLocation.timeZone = location[2];
	}
}

void setup()
{

	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	char* x = "mystring";
	x = x + (char)10;
	printf("\n x %s ", x);

	// ---- check if cuda exists
	GPU_COMPUTE = checkCudaExists(GPUString);
	cout << "\n " << GPUString;

	//----------------------------------
	zDomainDate dDates;
	loadConfig(inPath, outPath, dDates, LOCATION);
	
	//----------------------------------


	//zDomainDate dDates(zDate(YEAR, MONTHS[0], DATES[0], HOURS[0], 1), zDate(YEAR, MONTHS[1], DATES[1], HOURS[1], 1));
	solar.setDomain_Dates(dDates);

	//LOCATION.latitude = 51.50000; 
	//LOCATION.longitude = -0.11700; 
	//LOCATION.timeZone = 0;
	solar.setLocation(LOCATION);

	// sunpaths
	o_sunPaths.clear();
	o_sunPaths.assign(33, zObjGraph());

	// in mesh
	zFnMesh fnMesh(o_Mesh);
	fnMesh.from(inPath, zOBJ);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	// set display element booleans
	for (auto& g : o_sunPaths)
	{
		model.addObject(g);
		g.setDisplayElements(false, true);
	}

	model.addObject(o_Mesh);
	o_Mesh.setDisplayElements(false, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&compScale, "compScale");
	S.sliders[1].attachToVariable(&compScale, 0.1, 100);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&COMPUTE_SUNPATH, "COMPUTE_SUNPATH");
	B.buttons[0].attachToVariable(&COMPUTE_SUNPATH);

	B.addButton(&COMPUTE_RADIATION, "COMPUTE_RADIATION");
	B.buttons[1].attachToVariable(&COMPUTE_RADIATION);

}

void update(int value)
{
	for (auto& g : o_sunPaths)
	{
		zFnGraph fnGraph(g);
		zFloat4 scale = { compScale, compScale , compScale , 1 };
		fnGraph.setScale(scale);
	}

	// ----compute sun path
	if (COMPUTE_SUNPATH)
	{
		solar.computeSunVectors_Year();
		sunPath_hour_pts = solar.getRawSunVectors_hour();
		sunPath_day_pts = solar.getRawSunVectors_day();

		solar.computeCompass();
		compass_pts = solar.getRawCompassPts();


		BAKE_SUNPATH = true;
		COMPUTE_RADIATION = true;
		EXPORTMESH = true;

		COMPUTE_SUNPATH = !COMPUTE_SUNPATH;
	} 

	if (BAKE_SUNPATH)
	{

		unsigned int deg = 3;

		
		int id = 0;
		for (int i = 0; i < 7; i++)
		{
			zPointArray ctrlPts;
			zIntArray eConnects;

			for (int j = 0; j <= 24; j++)
			{
				int id = ((i * 24) + j % 24) * 3;

				if (sunPath_day_pts[id] == INVALID_VAL || sunPath_day_pts[id + 2] < 0) continue;
				

				if (ctrlPts.size() > 0)
				{
					eConnects.push_back(ctrlPts.size() - 1);
					eConnects.push_back(ctrlPts.size());
				}

				ctrlPts.push_back(zPoint(sunPath_day_pts[id + 0], sunPath_day_pts[id + 1], sunPath_day_pts[id + 2]));
			}

			zFnGraph fnGraph(o_sunPaths[id]);
			fnGraph.create(ctrlPts, eConnects);

			fnGraph.setEdgeColor(zColor(1, 0, 0, 1));
			fnGraph.setEdgeWeight(3);
			id++;
		}

		for (int i = 0; i < 24; i++)
		{
			zPointArray ctrlPts;
			zIntArray eConnects;

			for (int j = 0; j < 366; j++)
			{
				int id = ((i * 366) + j) * 3;

				if (sunPath_hour_pts[id] == INVALID_VAL) continue;

				if (ctrlPts.size() > 0)
				{
					eConnects.push_back(ctrlPts.size() - 1);
					eConnects.push_back(ctrlPts.size());
				}

				ctrlPts.push_back(zPoint(sunPath_hour_pts[id + 0], sunPath_hour_pts[id + 1], sunPath_hour_pts[id + 2]));
			}


			zFnGraph fnGraph(o_sunPaths[id]);
			fnGraph.create(ctrlPts, eConnects);
			id++;
		}

		/////////compass
		for (int i = 0; i < 2; i++)
		{
			zPointArray ctrlPts(13, zPoint());
			zIntArray eConnects;

			for (int j = 0; j < 13; j++)
			{
				int id = (i * 12 + j % 12) * 3;
				ctrlPts[j] = zPoint(compass_pts[id], compass_pts[id + 1], compass_pts[id + 2]);

				if (j > 0)
				{
					eConnects.push_back(j - 1);
					eConnects.push_back(j);
				}
			}

			zFnGraph fnGraph(o_sunPaths[id]);
			fnGraph.create(ctrlPts, eConnects);
			id++;
		}

	

		BAKE_SUNPATH = !BAKE_SUNPATH;
		
	}

	if (COMPUTE_RADIATION)
	{
		
		// set color
		solar.setDomain_Colors(colDomain);

		// get vertex normals
		zFnMesh fnMesh(o_Mesh);
		const int numNorms = fnMesh.numVertices();
		zVector* rawVNorms = fnMesh.getRawVertexNormals();

		float* vNorms = new float[numNorms * 3];
		for (int i = 0; i < numNorms; i++)
		{
			vNorms[i * 3 + 0] = rawVNorms[i].x;
			vNorms[i * 3 + 1] = rawVNorms[i].y;
			vNorms[i * 3 + 2] = rawVNorms[i].z;
		}

		solar.setNormals(vNorms, numNorms * 3, READ_EPW);

		if (GPU_COMPUTE)
		{
			bool out = cdpCummulativeRadiation(solar, READ_EPW);
		}
		else
		{
			solar.computeCummulativeRadiation();
		}

		COLORMESH = true;
		

		COMPUTE_RADIATION = !COMPUTE_RADIATION;
	}

	if (COLORMESH)
	{
		// default radiation domain - cumulative angle
		radiationDomain.min = RAD_ANGLE_MAX;
		radiationDomain.max = RAD_ANGLE_MIN;

		if (READ_EPW)
		{
			radiationDomain.min = RAD_MIN;
			radiationDomain.max = RAD_MAX;
		}


		float* cols = solar.getRawColors();

		zIntArray  	normalCounts;
		zIntArray  	normals;

		zFnMesh fnMesh(o_Mesh);
		const int numNorms = fnMesh.numVertices();
		zColor* rawVCols = fnMesh.getRawVertexColors();

		int normalsCount = 0;

		for (int i = 0; i < numNorms; i++)
		{
			int numFaceNormals = normalCounts[i];

			zColor col;

			int id = (int)i * 3;
			col = zColor(cols[id + 0], cols[id + 1], cols[id + 2]);;
		
			rawVCols[i] = col;		
			
		}

		fnMesh.computeFaceColorfromVertexColor();
		COLORMESH = !COLORMESH;
	}

	if (EXPORTMESH)
	{
		zFnMesh fnMesh(o_Mesh);
		fnMesh.to(outPath, zJSON);

		EXPORTMESH = !EXPORTMESH;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	model.draw();

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') COMPUTE_SUNPATH = true;;
	
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
