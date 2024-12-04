#define _MAIN_

#ifdef _MAIN_

#include <iostream>

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/Landuse/zTsConfiguratorLanduse.h>

//
using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool reset = false;
bool d_mesh = true;
bool exportTo = false;

bool drawType = false;

double background = 1.0;
double cooldown = 1000;

////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zObjMesh* baseMesh;

zTsConfiguratorLanduse configurator;

string path_underlayMesh = "data/Landuse/underlayGrid.usda";
string path_export = "data/Landuse/result.usda";

vector<zPoint> fCentres;
vector<string> fTypes;

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
	B.addButton(&reset, "reset");
	B.buttons[1].attachToVariable(&reset);
	B.addButton(&d_mesh, "d_mesh");
	B.buttons[2].attachToVariable(&d_mesh);
	B.addButton(&exportTo, "exportTo");
	B.buttons[3].attachToVariable(&exportTo);


	//configurator
	configurator.initialise(path_underlayMesh);
}

void update(int value)
{
	if (compute)
	{
		configurator.compute(1000, 0.05, 0.001);

		compute = !compute;

		//std::exit(1);
	}

	if (reset)
	{
		configurator.initialise(path_underlayMesh);
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

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("Best Score:" + to_string(configurator.finalScore), vec(winW - 350, winH - 500, 0));
	drawString("Current Iteration:" + to_string((int)configurator.finalIteration), vec(winW - 350, winH - 475, 0));

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
		else if (cooldown <= 1) cooldown -= 0.1;
		else cooldown -= 100;
		if (cooldown <= 0) cooldown = 0.1;
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
				string s;

				zItMeshFace f(configurator.displayMesh, configurator.gridFaceIds[i][j]);

				CellAbbr abbr = configurator.cellAbbrs[configurator.grid[i][j]];
				s = std::visit([](auto&& arg) {
					std::cout << arg << std::endl;

					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, std::string>) {
						return arg;
					}
					else if constexpr (std::is_same_v<T, char>) {
						return std::string(1, arg);
					}
					else {
						return std::to_string(arg);
					}
					}, abbr);

				fTypes.push_back(s);
				fCentres.push_back(f.getCenter());
			}
		}

		drawType = true;
	}

	if (k == 'r')
	{
		configurator.displayMesh.setDisplayElements(false, true, true);
		drawType = false;
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
