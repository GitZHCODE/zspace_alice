//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include<headers/zToolsets/configurator/zTsConfigurator.h>

//
using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool d_graph = true;
bool d_mesh = true;
bool debug = false;

double background = 0.85;

////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zTsConfigurator configurator;
zObjMesh* baseMesh;
zObjGraph* baseGraph;
string path_baseMesh = "data/Configurator/UE/baseMesh.json";
vector<zGameObj> gameObjs;

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
	B.addButton(&d_graph, "d_graph");
	B.buttons[1].attachToVariable(&d_graph);
	B.addButton(&d_mesh, "d_mesh");
	B.buttons[2].attachToVariable(&d_mesh);
	B.addButton(&debug, "debug");
	B.buttons[3].attachToVariable(&debug);

	//configurator
	string dir = "data/Configurator/UE";
	configurator.setDirectory(dir);

	configurator.setBaseMeshFromFile(path_baseMesh);
	baseMesh = configurator.getRawBaseMesh();

	//initialisation
	configurator.initialise();
	baseGraph = configurator.getRawBaseGraph();

	//compute methods
	configurator.compute();

	//load methods
	configurator.loadConfig();
	configurator.loadMesh();

	gameObjs = configurator.getGameObjs();

	//add to model
	for (auto& obj : gameObjs)
		model.addObject(obj.oMesh);

	model.addObject(*baseMesh);
	//model.addObject(*baseGraph);

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

	model.draw();

	if (d_graph)
	{
		zItGraphEdge e(*baseGraph);

		for (; !e.end(); e++)
		{
			zPointArray vPos;
			e.getVertexPositions(vPos);
			zVector move(0, 0, 0.1);
			model.displayUtils.drawLine(vPos[0] + move, vPos[1] + move, zMAGENTA, 4);

			zIntArray vIds;
			e.getVertices(vIds);
			for (auto& id : vIds)
			{
				zItGraphVertex v(*baseGraph, id);
				model.displayUtils.drawPoint(v.getPosition() + move, v.getColor(), 10);
			}
		}
		baseMesh->setDisplayElements(false, true, false);
	}

	if (d_mesh)
	{
		for (auto& obj : gameObjs)
			obj.oMesh.setDisplayElements(false, true, true);
		baseMesh->setDisplayElements(false, true, false);

	}

	if (!d_mesh)
	{
		for (auto& obj : gameObjs)
			obj.oMesh.setDisplayElements(false, false, false);
		baseMesh->setDisplayElements(false, true, true);

	}

	if (debug)
	{
		for (auto& obj : gameObjs)
		{
			float angle = acosf(obj.transform(0, 0));
			angle *= RAD_TO_DEG;

			zPoint pos(obj.transform(3, 0), obj.transform(3, 1), obj.transform(3, 2) + 1);
			model.displayUtils.drawTextAtPoint(to_string(angle), pos);
		}
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
