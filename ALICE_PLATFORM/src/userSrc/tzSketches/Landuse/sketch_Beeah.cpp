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

map<CellType,std::pair<string,zColor>> legendTable;


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
	B.addButton(&reset, "reset");
	B.buttons[1].attachToVariable(&reset);
	B.addButton(&d_mesh, "d_mesh");
	B.buttons[2].attachToVariable(&d_mesh);
	B.addButton(&exportTo, "exportTo");
	B.buttons[3].attachToVariable(&exportTo);


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
				string s = configurator.cellAbbrs[configurator.grid[i][j]].second;

				zItMeshFace f(configurator.displayMesh, configurator.gridFaceIds[i][j]);

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
