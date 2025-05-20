//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include <headers/zToolsets/geometry/zTsColorSplit.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool exportToFile = false;

double background = 0.35;

////////////////////////////////////////////////////////////////////////// zSpace Objects

/*!<model*/
zModel model;
//zModel resultModel;
/*!<Objects*/
zObjMesh oMesh;
zObjMeshPointerArray result;
zTsColorSplit cs;


int numMesh;
int currentMeshID;

zUtilsCore core;

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
	zFnMesh fnMesh(oMesh); 
		//fnMesh.from("data/testColorMesh.json", zJSON);
	fnMesh.from("data/colorMesh.json", zJSON);

	//model.addObject(oMesh);
	oMesh.setDisplayElements(true, true, true);





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
	B.addButton(&exportToFile, "export");
	B.buttons[2].attachToVariable(&exportToFile);


}

void update(int value)
{
	if (compute)
	{
		cs.setInMesh(oMesh);
		cs.compute();
		result = cs.getRawSplitMesh(numMesh);

		compute = !compute;
	}

	if (exportToFile)
	{
		if (numMesh > 0)
		{
			string dir = "data/out/";
			cs.exportTo(dir, zJSON);
		}
		exportToFile = !exportToFile;

	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	model.draw();
	S.draw();
	B.draw();


	if (display)
	{
		if(numMesh > 0)
		for (auto& m : result)
		{
			m->setDisplayElements(true, true,true);
			m->draw();
		}
	}
	else
	{
		int i = currentMeshID;

		if (numMesh > 0 && i >= 0 && i < numMesh)
		{
			result[i]->setDisplayElements(true, true, true);
			result[i]->draw();
		}

	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));
	drawString("Total num of meshes #:" + to_string(numMesh), vec(50, 200, 0));
	drawString("Current mesh #:" + to_string(currentMeshID), vec(50, 225, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{

	if (k == 'w')
	{
		if (currentMeshID < numMesh - 1)currentMeshID++;;
	}
	if (k == 's')
	{
		if (currentMeshID > 0)currentMeshID--;;
	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
