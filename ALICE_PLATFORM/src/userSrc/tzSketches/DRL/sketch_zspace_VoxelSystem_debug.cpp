//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include "VoxelSystem.h"

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

bool showGraph = true;
bool showPoints = true;
bool showMesh = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oBbox;
zObjMeshArray comps;
zObjMeshArray fieldMesh;

double fieldOffset = 0.0;
double meshScale = 0.0;
double threshold = 0;


zObjGraph oGraph;
zObjPointScalarField oField;

VoxelSystem vs;
zFloatArray fieldValues;
zIntArray meshID;
zPointArray positions;

////// --- GUI OBJECTS ----------------------------------------------------


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
	model.addObject(oBbox);

	// set display element booleans
	oBbox.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// VoxelSystem

		// read mesh
	zFnMesh fnBbox(oBbox);
	fnBbox.from("data/drl/bbox.obj", zOBJ);

	zPoint minBB, maxBB;
	fnBbox.getBounds(minBB, maxBB);

	vs.setFieldBounds(minBB, maxBB);
	vs.setFieldDim(10, 10, 10);

	string pathGraph = "data/drl/graph.obj";
	string pathMesh = "data/drl/voxelMesh";

	vs.setGraphFromFile(oGraph, pathGraph, zOBJ);
	vs.setMeshFromDirectory(comps, pathMesh, zOBJ);
	vs.setDisplayElements(true, false, true, false, true, true);

	vs.create();

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&fieldOffset, "fieldOffset");
	S.sliders[1].attachToVariable(&fieldOffset, -10, 10);
	S.addSlider(&meshScale, "meshScale");
	S.sliders[2].attachToVariable(&meshScale, -1, 1);
	S.addSlider(&threshold, "threshold");
	S.sliders[3].attachToVariable(&threshold, -1, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{
		vs.setFieldOffset(fieldOffset);
		vs.update();

		fieldMesh = vs.getFieldMesh();
		fieldValues = vs.getFieldValues(false);
		meshID = vs.getFieldMeshId();
		positions = vs.getFieldPosition();

		//for (int i = 0; i < 20; i++) cout << fieldValues[i] << ",";
		//cout << endl;

		//for (int i = 0; i < 20; i++) cout << meshID[i] << ",";
		//cout << endl;

		for (int i = 0; i < fieldMesh.size(); i++)
		{
			if (fieldValues[i] < threshold)
			{
				fieldMesh[i] = zObjMesh();
			}
			else
			{
				zItMeshVertex v(fieldMesh[i]);
				for (v.begin(); !v.end(); v++)
				{
					if (!v.onBoundary())
					{
						zVector target = v.getPosition() - positions[i];
						target.normalize();
						target = target * fieldValues[i] * meshScale;

						zPoint pt = v.getPosition();
						pt += target;
						v.setPosition(pt);
					}
				}
			}
		}

		compute = !compute;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();

		vs.draw(showPoints, showGraph, false);

		if(showMesh)
		for (auto& m : fieldMesh) m.draw();
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;

	if (k == 'm') showMesh = !showMesh;

	if (k == 'v') showPoints = !showPoints;

	if (k == 'g') showGraph = !showGraph;

	if (k == 'i')
	{
		
	}

	if (k == 'o')
	{
		string path = "data/drl/output.obj";
		vs.exportToObj(path, fieldMesh);
	}

}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
