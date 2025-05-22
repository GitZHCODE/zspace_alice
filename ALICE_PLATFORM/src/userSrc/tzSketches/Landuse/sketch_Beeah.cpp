#define _MAIN_

#ifdef _MAIN_

#include <iostream>

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/Landuse/zTsConfiguratorLanduse.h>
#include <userSrc/tzSketches/Configurator3d/zTsScanline.h>

//
using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool c_scanline = false;
bool reset = false;
bool d_mesh = true;
bool d_scanline = false;
bool exportTo = false;

bool drawType = false;
bool drawScore = false;

double background = 1.0;
double cooldown = 1000;

////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zObjMesh* baseMesh;

zTsConfiguratorLanduse configurator;
zScanline scanline;

string path_underlayMesh = "data/Landuse/underlayGrid.usda";
string path_export = "data/Landuse/result.usda";

// landuse
vector<zPoint> fCentres;
vector<string> fTypes;
vector<string> fScores;

map<CellType,std::pair<string,zColor>> legendTable;

//scanline
zPointArray positions;
std::vector<bool> matrix;
std::vector<bool> matrix_original;
vector<zSpace::Rectangle> rects;


std::string to_string_two_decimals(double value)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << value;
	return stream.str();
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
	B.addButton(&c_scanline, "c_scanline");
	B.buttons[1].attachToVariable(&c_scanline);
	B.addButton(&reset, "reset");
	B.buttons[2].attachToVariable(&reset);
	B.addButton(&d_mesh, "d_mesh");
	B.buttons[3].attachToVariable(&d_mesh);
	B.addButton(&d_scanline, "d_scanline");
	B.buttons[4].attachToVariable(&d_scanline);
	B.addButton(&exportTo, "exportTo");
	B.buttons[5].attachToVariable(&exportTo);

	//static_assert(alignof(zSpace::zColor) <= 8, "zColor alignment exceeds 8 bytes.");
	//static_assert(alignof(zSpace::zObjMesh) <= 8, "zObjMesh alignment exceeds 8 bytes.");

	//std::cout << "zColor Alignment: " << alignof(zSpace::zColor) << " bytes\n";
	//std::cout << "zObjMesh Alignment: " << alignof(zSpace::zObjMesh) << " bytes\n";

	//scanline
	rects.reserve(1000);


	//configurator
	configurator.initialise(path_underlayMesh);

	//legend
	for (auto& cell : configurator.cellColors)
	{
		string percentage = "    ";

		auto it = configurator.agentPercentages.find(cell.first);

		if (it != configurator.agentPercentages.end())
			percentage = to_string_two_decimals(configurator.agentPercentages[cell.first]);

		legendTable[cell.first].first = percentage  + "  " + configurator.cellAbbrs[cell.first].first;
		legendTable[cell.first].second = cell.second;
	}
}

void update(int value)
{
	if (compute)
	{
		configurator.compute(1000, 0.005, 0.001);

		c_scanline = true;

		compute = !compute;

		//std::exit(1);
	}

	if (c_scanline)
	{
		rects.clear();
		rects.reserve(1000);
		matrix.clear();
		positions.clear();

		int rows = configurator.ROWS;
		int cols = configurator.COLS;

		matrix.assign(rows * cols, false);

		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				//if (configurator.grid[i][j] == RESI_MODULAR)
				if (find(configurator.agentTypes.begin(), configurator.agentTypes.end(), configurator.grid[i][j]) != configurator.agentTypes.end())
					matrix[i * cols + j] = true;

				zItMeshFace f(configurator.displayMesh, configurator.gridFaceIds[i][j]);
				positions.push_back(f.getCenter());
			}
		}
		matrix_original = matrix;

		bool testAll = true;
		if (!testAll)
		{
			auto begin = std::chrono::high_resolution_clock::now();

			auto result = scanline.maximalRectangle(rows, cols, matrix, true);

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
			printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

			rects.push_back(result);

			std::cout << "MinBB: (" << result.minPos.rowId << ", " << result.minPos.colId << ") ";
			std::cout << "MaxBB: (" << result.maxPos.rowId << ", " << result.maxPos.colId << ")\n";
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

		c_scanline = !c_scanline;
	}

	if (reset)
	{
		configurator.initialise(path_underlayMesh);
		cooldown = 1000;

		reset = !reset;
	}

	if (exportTo)
	{
		configurator.to(path_export);

		exportTo = !exportTo;
	}
}

void draw()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glEnable(GL_MULTISAMPLE);

	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	model.draw();

	if (d_mesh)
	{
		configurator.draw();
		//configurator.draw(false, false, false, true, false);

	}

	if (drawType)
	{
		for (int i = 0; i < fCentres.size(); i++)
			model.displayUtils.drawTextAtPoint(fTypes[i], fCentres[i]);
	}

	if (drawScore)
	{
		for (int i = 0; i < fCentres.size(); i++)
			model.displayUtils.drawTextAtPoint(fScores[i], fCentres[i]);
	}

	if (d_scanline)
	{
		//for (int i = 0; i < positions.size(); i++)
		//{
		//	zColor col;
		//	if (matrix_original[i]) col = zBLUE;
		//	else col = zRED;
		//	model.displayUtils.drawPoint(positions[i], col, 5);
		//}

		for (auto& rect : rects)
		{
			vector<zSpace::Position> edgeIds;

			if (rect.minPos.rowId == rect.maxPos.rowId || rect.minPos.colId == rect.maxPos.colId)
				edgeIds = scanline.getIdsWithinRectangle(rect);
			else
			{
				edgeIds = scanline.getEdgeIdsWithinRectangle(rect);
				edgeIds.push_back(edgeIds[0]);
			}


			for (int i = 0; i < edgeIds.size() - 1; i++)
			{
				int id_a = configurator.gridFaceIds[edgeIds[i].rowId][edgeIds[i].colId];
				int id_b = configurator.gridFaceIds[edgeIds[i + 1].rowId][edgeIds[i + 1].colId];

				zItMeshFace fa(configurator.displayMesh, id_a);
				zItMeshFace fb(configurator.displayMesh, id_b);

				if (rect.maxPos.rowId - rect.minPos.rowId > 1 && rect.maxPos.colId - rect.minPos.colId > 1)
					model.displayUtils.drawLine(fa.getCenter(), fb.getCenter(), zBLACK, 8);
				else
					model.displayUtils.drawLine(fa.getCenter(), fb.getCenter(), zGREEN, 8);
			}

			//if (rect.first.first != -1)
			//{
			//	zPoint minBB = { (float)rect.first.first, (float)rect.first.second,0 };
			//	zPoint maxBB = { (float)rect.second.first, (float)rect.second.second,0 };
			//	model.displayUtils.drawRectangle(minBB, maxBB, zGREEN, 5);
			//}
		}
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("Best Score:" + to_string(configurator.finalScore), vec(winW - 350, winH - 500, 0));
	drawString("Current Iteration:" + to_string((int)configurator.finalIteration), vec(winW - 350, winH - 475, 0));

	// draw legend
	vec pos_icon(winW - 350, winH - 100, 0);
	vec pos_legend(pos_icon.x + 20, pos_icon.y, pos_icon.z);
	vec move(0,-15,0);
	for (auto& legend :legendTable)
	{
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		drawString(legend.second.first, pos_legend);
		glColor4f(legend.second.second.r, legend.second.second.g, legend.second.second.b, legend.second.second.a);
		drawRectangle(vec(pos_icon.x - 5, pos_icon.y - 5, 0), vec(pos_icon.x + 5, pos_icon.y + 5, 0));
		pos_legend += move;
		pos_icon += move;
	}

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'f')
		setCamera(29, 0, 0.001, 62.9, -27.7);

	if (k == 'p')
	{
		configurator.compute(1000, cooldown, 0.001);
		if (cooldown <= 100) cooldown -= 10;
		else if (cooldown <= 5) cooldown -= 1;
		else if (cooldown <= 1) cooldown -= 0.01;
		else cooldown -= 100;
		if (cooldown <= 0) cooldown = 0.005;
	}

	if (k == 'w')
	{
		configurator.displayMesh.setDisplayElements(false, true, false);
	}

	if (k == 't')
	{
		configurator.displayMesh.setDisplayElements(false, true, false);

		fTypes.clear();
		fCentres.clear();
		// Print types
		for (int i = 0; i < configurator.ROWS; ++i)
		{
			for (int j = 0; j < configurator.COLS; ++j)
			{
				string s = configurator.cellAbbrs[configurator.grid[i][j]].second;

				zItMeshFace f(configurator.displayMesh, configurator.gridFaceIds[i][j]);

				fTypes.push_back(s);
				fCentres.push_back(f.getCenter());
			}
		}

		drawType = true;
	}

	if (k == 's')
	{
		configurator.displayMesh.setDisplayElements(false, true, false);

		fScores.clear();
		fCentres.clear();
		// Print types
		for (int i = 0; i < configurator.ROWS; ++i)
		{
			for (int j = 0; j < configurator.COLS; ++j)
			{
				string s = to_string_two_decimals(configurator.gridFaceScores[i][j]);

				zItMeshFace f(configurator.displayMesh, configurator.gridFaceIds[i][j]);

				fScores.push_back(s);
				fCentres.push_back(f.getCenter());
			}
		}

		drawScore = true;
	}

	if (k == 'r')
	{
		configurator.displayMesh.setDisplayElements(false, true, true);
		drawType = false;
		drawScore = false;
	}


	//	float z, ry, rx, tx, tz;
	//getCamera(z, rx, ry, tx, ty);
	//cout << z << "," << rx << "," << ry << "," << tx << "," << ty << endl;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
