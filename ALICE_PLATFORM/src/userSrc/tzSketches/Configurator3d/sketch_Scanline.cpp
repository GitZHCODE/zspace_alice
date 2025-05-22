//#define _MAIN_

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
		//configurator.initialise();
		//configurator.compute();

		int rows = 5;
		int cols = 5;

		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				positions.push_back(zPoint(i, j, 0));
			}
		}

		//matrix = { 0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,1,1,1,0,1 };
		//matrix = { 1,1,1,0,0,
		//			1,1,1,1,1,
		//			1,1,1,1,1,
		//			1,1,1,0,0,
		//			1,1,0,0,0 };
		
		matrix = { 0,1,1,1,1,
			1,1,1,1,1,
			1,1,1,1,1,
			0,0,1,1,1,
			0,0,1,1,1 };

		//matrix = scanline.generateRandomMatrix(rows, cols);
		matrix_original = matrix;

		//cout << "------" << endl;
		//cout << "matrix:" << endl;
		//for (auto& m : matrix)
		//	cout << m << ",";
		//cout << endl;

		bool testAll = true;
		if(!testAll)
		{
			auto begin = std::chrono::high_resolution_clock::now();

			auto result = scanline.maximalRectangle(rows, cols, matrix, true);

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
			printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

			rects.push_back(result);

			std::cout << "MinBB: (" << result.first.first << ", " << result.first.second << ") ";
			std::cout << "MaxBB: (" << result.second.first << ", " << result.second.second << ")\n";
		}
		else
		{
			auto begin = std::chrono::high_resolution_clock::now();

			auto result = scanline.findAllRectangles(rows, cols, matrix, false);

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
			printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

			rects = result;
		}
		rects.shrink_to_fit();

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
		//for (auto& gameObj : configurator.gameObjs)
		//{
		//	//if (gameObj.orientation == zOrientation::top) model.displayUtils.drawTextAtPoint("TOP", gameObj.vertex.getPosition());
		//	//if (gameObj.orientation == zOrientation::middle) model.displayUtils.drawTextAtPoint("MIDDLE", gameObj.vertex.getPosition());
		//	//if(gameObj.orientation == zOrientation::bottom) model.displayUtils.drawTextAtPoint("BOTTOM", gameObj.vertex.getPosition());

		//	model.displayUtils.drawTextAtPoint(to_string(gameObj->vertex->getId()), gameObj->vertex->getPosition());

		//	drawCoordinates(gameObj->transformation, 0.2, 2);
		//}

		for (int i = 0; i < positions.size(); i++)
		{
			zColor col;
			if (matrix_original[i]) col = zBLUE;
			else col = zRED;
			model.displayUtils.drawPoint(positions[i], col, 5);
		}

		for (auto& rect : rects)
		{
			if (rect.first.first != -1)
			{
				zPoint minBB = { (float)rect.first.first, (float)rect.first.second,0 };
				zPoint maxBB = { (float)rect.second.first, (float)rect.second.second,0 };
				model.displayUtils.drawRectangle(minBB, maxBB, zGREEN, 5);
			}
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
