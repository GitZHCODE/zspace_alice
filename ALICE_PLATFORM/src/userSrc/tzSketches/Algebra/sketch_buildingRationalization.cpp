#define _MAIN_

#ifdef _MAIN_

#include "main.h"
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include "zBuilding.h"

using namespace zSpace;
using namespace std;
using namespace NS_hangzhou;

////////////////////////////////////////////////////////////////////////// General
bool readUSD = false;
bool compute = false;
bool display = true;
bool exportToFile = false;
double background = 0.85;

zModel model;
zUtilsCore core;

////////////////////////////////////////////////////////////////////////// zSpace Objects
unique_ptr<zBuilding> building;

// File paths
string importPath = "data/algebra/test_slabs_all.usda";
string exportPath = "data/algebra/rationalized_slabs_all.usda";

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
    B.addButton(&readUSD, "readUSD");
    B.buttons[0].attachToVariable(&readUSD);
    B.addButton(&compute, "compute");
    B.buttons[1].attachToVariable(&compute);
    B.addButton(&display, "display");
    B.buttons[2].attachToVariable(&display);
    B.addButton(&exportToFile, "export");
    B.buttons[3].attachToVariable(&exportToFile);
}

void update(int value)
{
    if (readUSD)
    {
        // Create new building
        building = make_unique<zBuilding>("Test Building");
        
        // Import from USD
        building->read(importPath);
        
        readUSD = !readUSD;
    }

    if (compute)
    {
        building->rationalise();

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

    if (display && building)
    {
        building->draw();
    }

    // Display file paths
    setup2d();
    glColor3f(0, 0, 0);
    restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
    switch (k)
    {
    case 'r':
    case 'R':
        compute = true;
        break;
    case 'i':
    case 'I':
        // TODO: Add file dialog for import path
        compute = true;
        break;
    case 'e':
    case 'E':
        // TODO: Add file dialog for export path
        exportToFile = true;
        compute = true;
        break;
    }
}

void mousePress(int b, int s, int x, int y)
{
}

void mouseMotion(int x, int y)
{
}

#endif // _MAIN_ 