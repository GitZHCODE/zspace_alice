//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include<headers/zToolsets/geometry/zTsLatticeDeformer.h>

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

zTsLatticeDeformer deformer;

zObjMesh oMesh;
zObjMesh baseLattice;
zObjMesh targetLattice;

string path_baseLattice = "data/latticeTest/baseLattice.obj";
string path_targetLattice = "data/latticeTest/targetLattice.obj";
string path_deformObj = "data/latticeTest/deformObj.obj";


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
	B.addButton(&d_mesh, "d_mesh");
	B.buttons[1].attachToVariable(&d_mesh);
	B.addButton(&debug, "debug");
	B.buttons[2].attachToVariable(&debug);

	zFnMesh fn(baseLattice);
	fn.from(path_baseLattice, zOBJ);

	fn = zFnMesh(targetLattice);
	fn.from(path_targetLattice, zOBJ);

	fn = zFnMesh(oMesh);
	fn.from(path_deformObj, zOBJ);

	//lattice deform
	deformer.setBaseLattice(baseLattice);
	deformer.setTargetLattice(targetLattice);
	deformer.setInMesh(oMesh);

	model.addObject(baseLattice);
	model.addObject(targetLattice);
	model.addObject(oMesh);

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

	deformer.draw(true, true, true);

	//if (d_graph)
	//{
	//	zItGraphEdge e(*baseGraph);

	//	for (; !e.end(); e++)
	//	{
	//		zPointArray vPos;
	//		e.getVertexPositions(vPos);
	//		zVector move(0, 0, 0.1);
	//		model.displayUtils.drawLine(vPos[0] + move, vPos[1] + move, zMAGENTA, 4);

	//		zIntArray vIds;
	//		e.getVertices(vIds);
	//		for (auto& id : vIds)
	//		{
	//			zItGraphVertex v(*baseGraph, id);
	//			model.displayUtils.drawPoint(v.getPosition() + move, v.getColor(), 10);
	//		}
	//	}
	//}

	//if (d_mesh)
	//{
	//	for (auto& obj : gameObjs)
	//		obj.oMesh.setDisplayElements(false, true, true);
	//}

	//if (!d_mesh)
	//{
	//	for (auto& obj : gameObjs)
	//		obj.oMesh.setDisplayElements(false, false, false);
	//}

	//if (debug)
	//{
	//	for (auto& obj : gameObjs)
	//	{
	//		float angle = acosf(obj.transform(0, 0));
	//		angle *= RAD_TO_DEG;

	//		zPoint pos(obj.transform(3, 0), obj.transform(3, 1), obj.transform(3, 2) + 1);
	//		model.displayUtils.drawTextAtPoint(to_string(angle), pos);
	//	}
	//}

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
