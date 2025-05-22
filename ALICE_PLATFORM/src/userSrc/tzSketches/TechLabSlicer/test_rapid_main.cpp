#define _MAIN_

#ifdef _MAIN_

#include "main.h"


#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include "RapidMaker.h"
#include <iostream>

using namespace zSpace;
using namespace std;


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

zModel model;
////////////////////////////////////////////////////////////////////////// zSpace Objects

using namespace zSpace::ABB;

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

// Helper function to create a square path using transformation matrices
vector<Target> createSquarePath(double x, double y, double size, double z, double angle_degrees = 0, const ConfData* config = nullptr) {
    vector<Target> path;

    // Create corner points with consistent orientation
    path.push_back(Target(createTransform(x, y, z, angle_degrees), config ? *config : ConfData(), "corner1"));
    path.push_back(Target(createTransform(x + size, y, z, angle_degrees), config ? *config : ConfData(), "corner2"));
    path.push_back(Target(createTransform(x + size, y + size, z, angle_degrees), config ? *config : ConfData(), "corner3"));
    path.push_back(Target(createTransform(x, y + size, z, angle_degrees), config ? *config : ConfData(), "corner4"));
    path.push_back(Target(createTransform(x, y, z, angle_degrees), config ? *config : ConfData(), "corner5")); // Close the loop

    return path;
}

void setup()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	model = zModel(100000);



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


}

void update(int value)
{
	if (compute)
	{
        RapidMaker rapidMaker;

        // Create a module with custom settings
        PrinterSettings settings(46.06824, 20, 100, 0.2);
        Module& printModule = rapidMaker.createModule("ZHA_PRINT", settings);

        // Define the standard tool with correct values
        ToolData standardTool("standardTool");
        standardTool.robhold = true;
        standardTool.tframe = Pos(325.451, 2.555, 91.138);
        standardTool.tframe_rot = Orient(0.707107, 0.0, 0.707107, 0.0);
        standardTool.mass = 25.0;
        standardTool.cog = Pos(1.0, 0.0, 0.0);
        standardTool.inertia = Orient(1.0, 0.0, 0.0, 0.0);
        standardTool.aios = 0.0;
        standardTool.aios_unit = 0.0;
        standardTool.aios_value = 0.0;
        printModule.addToolData(standardTool);

        // Define zones with different precision levels
        ZoneData preciseZone("preciseZone");
        preciseZone.finep = false;
        preciseZone.pzone_tcp = 0.05;
        preciseZone.pzone_ori = 0.05;
        preciseZone.pzone_eax = 0.1;
        preciseZone.zone_ori = 0.01;
        preciseZone.zone_leax = 0.1;
        preciseZone.zone_reax = 0.01;
        printModule.addZoneData(preciseZone);

        ZoneData roughZone("roughZone");
        roughZone.finep = false;
        roughZone.pzone_tcp = 0.2;
        roughZone.pzone_ori = 0.2;
        roughZone.pzone_eax = 0.1;
        roughZone.zone_ori = 0.01;
        roughZone.zone_leax = 0.1;
        roughZone.zone_reax = 0.01;
        printModule.addZoneData(roughZone);

        // Create main procedure
        Procedure& mainProc = printModule.createProcedure("Main");

        // Print outer wall with standard tool and rough precision
        mainProc.addCommand("!PATH ROUGH PRECISION");

        // Create outer wall path with 0-degree orientation
        vector<Target> outerPath = createSquarePath(450, 150, 100, 450, 0);

        // Print outer wall with approach and retract moves
        rapidMaker.addPrintMoveWithApproach(printModule, mainProc, outerPath, 5.0,
            &roughZone, &standardTool);

        // Print inner details with precise movements
        mainProc.addCommand("!PATH ACC PRECISION");

        // Create inner wall path with 45-degree orientation
        vector<Target> innerPath = createSquarePath(450, 150, 100, 500, 45);

        // Print inner wall with approach and retract moves
        rapidMaker.addPrintMoveWithApproach(printModule, mainProc, innerPath, 5.0,
            &preciseZone, &standardTool);

		string outputPath = "data/TechLabSlicer/testFolder/rapid";
		cout << "Generating RAPID files in: " << outputPath << endl;

		if (rapidMaker.generateRapidFiles(outputPath)) {
			cout << "RAPID file generation completed successfully." << endl;
		}
		compute = !compute;
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
