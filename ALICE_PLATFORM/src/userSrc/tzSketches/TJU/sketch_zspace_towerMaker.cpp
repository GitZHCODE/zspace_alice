#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include "zTsTowerMaker.h"
#include "C:\Users\taizhong_chen\source\repos\GitZHCODE\zspace_alice\ALICE_PLATFORM\src\userSrc\tzSketches\TechLabSlicer\RapidMaker.h"

//#include <headers/zCore/zExtMesh.h>
//#include <headers/zCore/zExtPoint.h>

using namespace zSpace;
using namespace std;
using namespace zSpace::ABB;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool compute_union = false;
bool compute_union_mul = false;
bool display = true;
bool rapid = false;

double background = 0.85;
double displayScale = 1.0;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/


zTsTowerMaker TM;

string path_in = "data/TJU/input.usda";
string path_out = "data/TJU/output.usda";

int numX = 100;
int numY = 100;

double k1 = 0.07;
double k2 = 0.35;
double multiplier = 1.0;
double contourHeight = 1.2;

double sampleDist = 3.5;

////// --- GUI OBJECTS ----------------------------------------------------

// Helper function to create a transformation matrix for a given position and orientation
Matrix4d createTransform(double x, double y, double z, double angle_degrees = 0) {
	Matrix4d transform = Matrix4d::Identity();

	// Set rotation (rotate around Z axis)
	double angle_rad = angle_degrees * PI / 180.0;
	double cos_a = cos(angle_rad);
	double sin_a = sin(angle_rad);

	// Create rotation matrix for Z-axis rotation
	Matrix3d rotZ;
	rotZ << cos_a, -sin_a, 0,
		sin_a, cos_a, 0,
		0, 0, 1;

	// Set rotation part
	transform.block<3, 3>(0, 0) = rotZ;

	// Set translation part (in the correct column)
	transform.block<3, 1>(0, 3) = Vector3d(x, y, z);

	return transform;
}

vector<Target> createPathFromGraph(zObjGraph& graph, Pos& trans){
	vector<Target> path;
	path.reserve(10000);

	double multiplier = 1.0;


	zItGraphVertex v(graph);
	for (; !v.end(); v++)
	{
		double x, y, z;
		x = v.getPosition().x * multiplier + 800;
		y = v.getPosition().y * multiplier + 750;
		z = v.getPosition().z * multiplier + 0;

		path.push_back(Target(createTransform(x, y, z, 0.0)));
	}
	path.shrink_to_fit();

	return path;
};

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object

	// set display element booleans

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&displayScale, "displayScale");
	S.sliders[1].attachToVariable(&displayScale, 0, 5);
	S.addSlider(&k1, "k1");
	S.sliders[2].attachToVariable(&k1, -1, 1);
	S.addSlider(&k2, "k2");
	S.sliders[3].attachToVariable(&k2, -1, 1);
	S.addSlider(&multiplier, "multiplier");
	S.sliders[4].attachToVariable(&multiplier, -1, 1);
	S.addSlider(&contourHeight, "contourHeight");
	S.sliders[5].attachToVariable(&contourHeight, 0, 20);
	S.addSlider(&sampleDist, "sampleDist");
	S.sliders[6].attachToVariable(&sampleDist, 0, 20);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&compute_union_mul, "compute_union_mul");
	B.buttons[1].attachToVariable(&compute_union_mul);
	B.addButton(&rapid, "rapid");
	B.buttons[2].attachToVariable(&rapid);
	B.addButton(&display, "display");
	B.buttons[3].attachToVariable(&display);

	TM.from(path_in);
	TM.initialise(numX, numY);
}

void update(int value)
{
	if (compute)
	{
		TM.compute(zSMin::exponential, zSMin::exponential, k1 * multiplier, k2 * multiplier, contourHeight, sampleDist);

		compute = !compute;
	}

	if (rapid)
	{
		RapidMaker rapidMaker;

		// Create a module with custom settings
		PrinterSettings settings(46.06824, 20, 100, 0.2);
		Module& printModule = rapidMaker.createModule("TOWER_TEST", settings);

		// Create main procedure
		Procedure& mainProc = printModule.createProcedure("Main");

		// Add home J
		vector<double> home = { 0, 0, 10, 0, -10, 0 };
		printModule.addMoveAbsJ(mainProc, home, 100);

		WObjData wobj = printModule.findWObjData("defaultWObj");

		for (size_t i = 0; i < TM.layers.size(); i++)
		{
			mainProc.addCommand("!layer " + to_string(i));
			for (size_t j = 0; j < TM.layers[i]->graphs.size(); j++)
			{			
				// Create printpath from zObjGraph
				vector<Target> printPath = createPathFromGraph(TM.layers[i]->graphs[j], wobj.trans);

				// Print separated graph with approach and retract moves
				rapidMaker.addPrintMoveWithApproach(printModule, mainProc, printPath, 5.0);
			}
		}

		// Add home J
		printModule.addMoveAbsJ(mainProc, home, 100);


		string outputPath = "data/TechLabSlicer/testFolder/rapid";
		cout << "Generating RAPID files in: " << outputPath << endl;

		if (rapidMaker.generateRapidFiles(outputPath)) {
			cout << "RAPID file generation completed successfully." << endl;
		}
		rapid = !rapid;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	glPushMatrix();
	glScalef(displayScale, displayScale, displayScale);

	model.draw();

	if (display)
	{
		TM.draw();
	}

	glPopMatrix();

	//////////////////////////////////////////////////////////

	setup2d();

	//for (size_t i = 0; i < modeInfo.size(); i++)
	//{
	//	glColor3f(cols[i].r, cols[i].g, cols[i].b);
	//	drawString(modeInfo[i], Alice::vec(winW - 350, winH - 750 + i * 20, 0));
	//}

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	//if (k == 'p') compute = true;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
