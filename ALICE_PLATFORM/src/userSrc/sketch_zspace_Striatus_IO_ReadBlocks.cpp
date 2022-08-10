//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool displayBlocks = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMeshArray oMeshArray;
zObjMesh o_centerMesh;

string fileDir = "data/striatus/100_Draft/";
string filePath_centerMesh = "data/striatus/out_BRG_220730.json";


zPointArray points;

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

	// read mesh
	zStringArray files;
	core.getFilesFromDirectory(files, fileDir, zJSON);

	oMeshArray.assign(files.size(), zObjMesh());
	points.assign(files.size(), zPoint());
	
	for (auto& s : files)
	{
		zStringArray split_0 = core.splitString(s, ".");
		zStringArray split_1 = core.splitString(split_0[0], "_");
	
		int meshID = atoi(split_1[split_1.size() - 1].c_str());

		zFnMesh fnMesh(oMeshArray[meshID]);
		fnMesh.from(s, zJSON);

		points[meshID] = (fnMesh.getCenter());

		points[meshID].z += 0.5;

		meshID++;
	}

	
	// read Center mesh

	zFnMesh fnCenterMesh(o_centerMesh);
	fnCenterMesh.from(filePath_centerMesh, zJSON);

	// get transform attribute
	json j;

	bool chkFile = core.readJSON(filePath_centerMesh, j);

	zFloatArray bTransform_array;
	bool chkAttr = core.readJSONAttribute(j, "BridgeTransform", bTransform_array);

	zTransform bTranform = core.getTransformFromArray(bTransform_array);
	

	fnCenterMesh.setTransform(bTranform, true, true);

		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	for (auto& m : oMeshArray)
	{
		model.addObject(m);
		m.setDisplayElements(false, true, false);
	}

	// set display element booleans
	model.addObject(o_centerMesh);
	o_centerMesh.setDisplayElements(false, false, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&displayBlocks, "displayBlocks");
	B.buttons[0].attachToVariable(&displayBlocks);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	for (auto& m : oMeshArray) m.setDisplayObject(displayBlocks);
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	// zspace model draw
	model.draw();


	if (display)
	{
		glColor3f(1, 1, 1);
		for (int i = 0; i < points.size(); i++)
		{
			model.displayUtils.drawTextAtPoint(to_string(i), points[i]);
		}
		
	}



	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
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
