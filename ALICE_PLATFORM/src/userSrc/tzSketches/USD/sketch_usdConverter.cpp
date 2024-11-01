//#define _MAIN_

#ifdef _MAIN_

#include <iostream>

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/Configurator3d/zTsConfigurator3d.h>

//
using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;

double background = 0.85;

////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

void usdConverter(string inFile, string outFile)
{
	UsdStageRefPtr stage = UsdStage::Open(inFile);
	stage->Export(outFile);
}


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	//////////////////////////////////////////////////////////////////////////

	// initialise model
	model = zModel(100000);
	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	string folder = "data/";
	string fileName;
	string inFile;
	string outFile;

	cout << "file name - eg:myUsd.usd" << endl;
	cin >> fileName;

	inFile = folder + fileName;

	// Convert .usda to .usd or .usd to .usda
	if (inFile.find(".usda") != std::string::npos)
	{
		outFile = inFile.substr(0, inFile.size() - 1); // Remove 'a' to convert to .usd
	}
	else if (inFile.find(".usd") != std::string::npos)
	{
		outFile = inFile + "a"; // Add 'a' to convert to .usda
	}
	else
	{
		cout << "Invalid file format. Only .usd and .usda are supported." << endl;
		throw;
	}

	usdConverter(inFile, outFile);

	std::exit(1);
}

void update(int value)
{
	if (compute)
	{
		compute = !compute;

		std::exit(1);
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

}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
