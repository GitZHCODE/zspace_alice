#define _MAIN_

#ifdef _MAIN_

#include "main.h"



/// ZSPACE
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/OV/zTsOVdataProcessor.h>

using namespace zSpace;
using namespace std;


////////////////////////////////////////////////////////////////////////// General

bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMesh oMesh;

bool computeId = false;
bool computeVolume = false;
bool computePlanarity = false;
bool exportTo = false;
bool updateColour = false;


////// --- GUI OBJECTS ----------------------------------------------------
zTsOVdataProcessor zProcessor;

string ov_destinationPath = "omniverse://nucleus.zaha-hadid.com/Projects";
string in_usdPath = "omniverse://nucleus.zaha-hadid.com/Projects/0000_ChicagoInnovate/inFolder/example_natpower.usda";
string out_usdPath = "omniverse://nucleus.zaha-hadid.com/Projects/0000_ChicagoInnovate/outFolder/example_natpower_pp.usda";
string rootPrimName = "Blocks";
//string in_usdPath = "omniverse://nucleus.zaha-hadid.com/Projects/0000_ChicagoInnovate/inFolder/example_test.usda";
//string out_usdPath = "omniverse://nucleus.zaha-hadid.com/Projects/0000_ChicagoInnovate/outFolder/example_test_pp.usda";
//
//string in_usdPath = "omniverse://nucleus.zaha-hadid.com/Projects/0000_ChicagoInnovate/inFolder/threeBlocks.usda";
//string out_usdPath = "omniverse://nucleus.zaha-hadid.com/Projects/0000_ChicagoInnovate/outFolder/threeBlocks_pp.usda";

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	// json user config
	json j;
	string path = "data/OV/userConfig.json";
	ifstream in_myfile;
	in_myfile.open(path.c_str());

	if (in_myfile.fail())
		cout << " error in opening file  " << path.c_str() << endl;

	in_myfile >> j;
	in_myfile.close();

	// assign paths
	ov_destinationPath	= j["ov_destinationPath"].get<string>();
	in_usdPath			= j["in_usdPath"].get<string>();
	out_usdPath			= j["out_usdPath"].get<string>();
	rootPrimName		= j["rootPrimName"].get<string>();

	// initialise
	zProcessor.initialise(ov_destinationPath, in_usdPath, rootPrimName);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans
	//oMesh.setDisplayElements(false, true, true);
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&display, "display");
	B.buttons[0].attachToVariable(&display);
	B.addButton(&updateColour, "updateColour");
	B.buttons[1].attachToVariable(&updateColour);
	B.addButton(&computeId, "computeId");
	B.buttons[2].attachToVariable(&computeId);
	B.addButton(&computeVolume, "computeVolume");
	B.buttons[3].attachToVariable(&computeVolume);
	B.addButton(&computePlanarity, "computePlanarity");
	B.buttons[4].attachToVariable(&computePlanarity);
	B.addButton(&exportTo, "export");
	B.buttons[5].attachToVariable(&exportTo);

}

void update(int value)
{
	if (computeId)
	{
		zProcessor.compute(zProcessType::meshId, updateColour);
		computeId = !computeId;
	}

	if (computeVolume)
	{
		zProcessor.compute(zProcessType::meshVolume, updateColour);
		computeVolume = !computeVolume;
	}

	if (computePlanarity)
	{
		zProcessor.compute(zProcessType::meshPlanarity, updateColour);
		computePlanarity = !computePlanarity;
	}

	if (exportTo)
	{
		zProcessor.exportTo(out_usdPath);
		exportTo = !exportTo;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		zProcessor.draw();
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	//if (k == 'p') compute = true;;	
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
