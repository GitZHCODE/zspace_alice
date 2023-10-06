//#define _MAIN_

#ifdef _MAIN_

#include "main.h"



/// ZSPACE
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>

using namespace zSpace;
using namespace std;


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMesh oMesh;

zOmniCore omniCore;


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

	// Connect to OV
	bool doLiveEdit = false;
	std::string existingStage;
	string destinationPath = "omniverse://nucleus.zaha-hadid.com/Projects";
	bool chk = omniCore.isValidOmniURL(destinationPath);
		
	// Startup Omniverse with the default login
	if (!omniCore.startOmniverse()) exit(1);
	omniCore.printConnectedUsername(destinationPath);	

	// read mesh
	zFnMesh fnMesh(oMesh);

	fnMesh.from("data/USD/panels/colorCube.json", zJSON);

	printf("\n v %i e %i f %i", fnMesh.numVertices(), fnMesh.numEdges(), fnMesh.numPolygons());

	//fnMesh.computeFaceColorfromVertexColor();

	fnMesh.to(destinationPath + "/testCube_fCol_out_3108.usda", zUSD);


	//string* s;
	//UsdStageRefPtr stage = omniCore.gStage;
	//const char* c = stage->GetDefaultPrim().GetPath().GetText();
	//cout << c << endl;

		//for (pxr::UsdPrim prim : stage->Traverse())
	//{
	//	if (prim.IsA<UsdGeomMesh>())
	//	{
	//		allMeshPrim.push_back(prim);
	//		SdfPath path;
	//		prim.GetPrimAtPath(path);
	//		primPaths.push_back(path);

	//		cout << "working" << endl;
	//	}
	//}
	//cout << "Found " << allMeshPrim.size() << " UsdGeomMesh" << endl;
	//for (auto& name : primPaths)
	//	cout << name.GetText() << endl;



	// All done, shut down our connection to Omniverse
	omniCore.shutdownOmniverse();
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	// set display element booleans
	oMesh.setDisplayElements(false, true, true);
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

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
		
		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;	
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
