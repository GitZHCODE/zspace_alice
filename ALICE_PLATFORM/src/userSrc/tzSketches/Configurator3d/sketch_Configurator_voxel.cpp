#define _MAIN_

#ifdef _MAIN_

#include <iostream>

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/Configurator3d/zTsConfigurator3d.h>

#include <userSrc/tzSketches/Configurator3d/zTsScanline.h>
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

zObjMesh* baseMesh;
zObjGraph* baseGraph;

zConfigurator3d configurator;
zScanline scanline;

zPointArray positions;
std::vector<bool> matrix;
std::vector<bool> matrix_original;

vector<pair<pair<int, int>, pair<int, int>>> rects;

void drawCoordinates(zTransform& trans, float scale, double wt)
{
	zPoint p(trans(0, 3), trans(1, 3), trans(2, 3));
	zVector frameX(trans(0, 0), trans(1, 0), trans(2, 0));
	zVector frameY(trans(0, 1), trans(1, 1), trans(2, 1));
	zVector frameZ(trans(0, 2), trans(1, 2), trans(2, 2));

	model.displayUtils.drawLine(p, p + frameX * scale, zRED, wt);
	model.displayUtils.drawLine(p, p + frameY * scale, zGREEN, wt);
	model.displayUtils.drawLine(p, p + frameZ * scale, zBLUE, wt);
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
	B.addButton(&d_graph, "d_graph");
	B.buttons[1].attachToVariable(&d_graph);
	B.addButton(&d_mesh, "d_mesh");
	B.buttons[2].attachToVariable(&d_mesh);
	B.addButton(&debug, "debug");
	B.buttons[3].attachToVariable(&debug);

	rects.reserve(1000);
	compute = true;
}

void update(int value)
{
	if (compute)
	{
		//configurator
		configurator.initialise();
		//configurator.compute();

		compute = !compute;

		//std::exit(1);
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
		configurator.connectionGraph.draw();
	}

	if (d_mesh)
	{
		for (auto& gameObj : configurator.gameObjs)
		{
			gameObj->voxel->setDisplayElements(false, true, true);
			gameObj->voxel->draw();
		}
	}

	if (!d_mesh)
	{
		for (auto& gameObj : configurator.gameObjs)
		{
			gameObj->voxel->setDisplayElements(false, true, false);
			gameObj->voxel->draw();
		}
	}

	if (debug)
	{
		for (auto& gameObj : configurator.gameObjs)
		{
			//if (gameObj.orientation == zOrientation::top) model.displayUtils.drawTextAtPoint("TOP", gameObj.vertex.getPosition());
			//if (gameObj.orientation == zOrientation::middle) model.displayUtils.drawTextAtPoint("MIDDLE", gameObj.vertex.getPosition());
			//if(gameObj.orientation == zOrientation::bottom) model.displayUtils.drawTextAtPoint("BOTTOM", gameObj.vertex.getPosition());

			model.displayUtils.drawTextAtPoint(to_string(gameObj->vertex->getId()), gameObj->vertex->getPosition());

			drawCoordinates(gameObj->transformation, 0.2, 2);
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
