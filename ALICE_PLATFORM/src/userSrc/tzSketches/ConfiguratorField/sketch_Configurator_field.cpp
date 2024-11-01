#define _MAIN_

#ifdef _MAIN_

#include <iostream>

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/ConfiguratorField/zTsConfiguratorField.h>

//
using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool reset = false;
bool computeMap = false;
bool d_graph = true;
bool d_field = true;
bool d_parcel = false;
bool d_bounds = false;

double background = 0.125;

int resX = 40;
int resY = 40;
double displayVecLength = 0.2;

double minNum = 1.0f;
double maxNum = 4.0f;
double minRadius = 0.15f;
double maxRadius = 0.35f;
double parcelOffset = 0.15f;
double parcelRotation = 0.0;

////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zObjMesh* baseMesh;
zObjGraph* baseGraph;

zTsConfiguratorField configurator;

string path_graphMesh = "data/Configurator_field/inFolder/graphMesh.json";
string path_underlayMesh = "data/Configurator_field/inFolder/underlayMesh.json";

zPointArray positions;
zVectorArray vectors;

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
	S.addSlider(&minNum, "minNum");
	S.sliders[1].attachToVariable(&minNum, 0, 10);
	S.addSlider(&maxNum, "maxNum");
	S.sliders[2].attachToVariable(&maxNum, 0, 10);
	S.addSlider(&minRadius, "minRadius");
	S.sliders[3].attachToVariable(&minRadius, 0, 0.5);
	S.addSlider(&maxRadius, "maxRadius");
	S.sliders[4].attachToVariable(&maxRadius, 0, 0.5);
	S.addSlider(&parcelOffset, "parcelOffset");
	S.sliders[5].attachToVariable(&parcelOffset, 0, 5);
	S.addSlider(&parcelRotation, "parcelRotation");
	S.sliders[6].attachToVariable(&parcelRotation, 0, 180);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&reset, "reset");
	B.buttons[1].attachToVariable(&reset);
	B.addButton(&d_graph, "d_graph");
	B.buttons[2].attachToVariable(&d_graph);
	B.addButton(&d_field, "d_field");
	B.buttons[3].attachToVariable(&d_field);
	B.addButton(&d_parcel, "d_parcel");
	B.buttons[4].attachToVariable(&d_parcel);
	B.addButton(&d_bounds, "d_bounds");
	B.buttons[5].attachToVariable(&d_bounds);

	//configurator
	configurator.initialise(path_graphMesh, path_underlayMesh, resX, resY, minNum, maxNum, minRadius, maxRadius);
}

void update(int value)
{
	if (compute)
	{

		configurator.compute(parcelRotation, parcelOffset);


		//configurator.getVectorField(positions, vectors);


		//compute = !compute;

		//std::exit(1);
	}

	if (reset)
	{
		configurator.initialise(path_graphMesh, path_underlayMesh, resX, resY, minNum, maxNum, minRadius, maxRadius);

		reset = !reset;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	model.draw();

	if (d_graph)
	{
		configurator.draw(true, false);

	}

	if (d_field)
	{
		configurator.draw(false, true);
	}

	if (!d_field)
	{
	}

	if (d_parcel)
	{
		configurator.draw(false, false, true);

		//for (int i = 0; i < positions.size(); i++)
		//{
		//	model.displayUtils.drawLine(positions[i], positions[i] + vectors[i] * displayVecLength, zColor(1, 1, 1, 1), 2);
		//}
	}

	if (d_bounds)
	{
		configurator.draw(false, false, false, true);
	}

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
