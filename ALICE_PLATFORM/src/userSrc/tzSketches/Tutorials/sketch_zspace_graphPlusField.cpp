//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.75;

////////////////////////////////////////////////////////////////////////// zSpace Objects

zModel model;
zObjGraph oGraph;
zObjPointScalarField oField;

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	////////////////////////////////////////////////////////////////////////// zSpace
	// initialise model
	model = zModel(100000);

	//graph
	zFnGraph fnGraph(oGraph);
	fnGraph.from("data/tutorials/simpleGraph.json", zJSON);
	fnGraph.setEdgeColor(zMAGENTA);
	fnGraph.setEdgeWeight(3);

	//field
	zVector minBB(-5, -5, -5);
	zVector maxBB(5, 5, 5);
	zFnPointScalarField fnField(oField);
	fnField.create(minBB, maxBB, 20, 20, 20);
	fnField.setFieldColorDomain(zDomainColor(zBLACK, zRED));

	//set field color domain
	zDomainColor colDomain(zColor(1, 0, 0, 0), zColor(0, 1, 0, 1));
	fnField.setFieldColorDomain(colDomain);
	
	//set field values based on distance to graph
	zScalarArray vals;
	fnField.getScalarsAsEdgeDistance(vals, oGraph, 0.1, false);
	fnField.setFieldValues(vals);
	fnField.updateColors();

	//add objects to scene
	model.addObject(oGraph);
	model.addObject(oField);

	//set display
	oField.setDisplayElements(true);
	oGraph.setDisplayElements(false, true);
}

void update(int value)
{
	if (compute)
	{


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
		model.draw();
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
