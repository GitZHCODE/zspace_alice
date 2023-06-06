//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


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
zObjMeshArray oMeshes;


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
	

	zStringArray fileMeshes;
	core.getFilesFromDirectory(fileMeshes, "data/OV",zOBJ);
	
	oMeshes.assign(fileMeshes.size(), zObjMesh());

	for (int i =0; i< fileMeshes.size(); i++)
	{
		zFnMesh fnMesh(oMeshes[i]);
		cout << fileMeshes[i] << endl ;
		fnMesh.from(fileMeshes[i], zOBJ);

		for (zItMeshEdge e(oMeshes[i]); !e.end(); e++)
		{
			if (!e.onBoundary())
			{
				e.setColor(zColor(1, 0, 0, 1));
				cout << "\n";
				cout << e.getHalfEdge(0).getVector() << "\n";
				cout << e.getHalfEdge(1).getVector() << "\n";
			}
		}

		model.addObject(oMeshes[i]);
		oMeshes[i].setDisplayElements(true, true, true);

		zStringArray s = core.splitString(fileMeshes[i], ".");

		fnMesh.to(s[0] + ".json", zJSON);

		json j;
		bool chk = core.readJSON(s[0] + ".json", j);
		
		zDoubleArray fabBase = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1.7, -1, -0.8, 1 };
		j["FabBase"] = fabBase;

		zDoubleArray worldBase = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		j["WorldBase"] = worldBase;

		// EXPORT	
		ofstream myfile;
		string f = s[0] + ".json";
		myfile.open(f.c_str());		
		
		myfile << j.dump();
		myfile.close();
	}
	

	/*json j;
	bool chk = core.readJSON("C:/Users/vishu.b/Desktop/test.json", j);*/

	//vector<zDoubleArray> creaseData;
	//core.readJSONAttribute(j, "FaceAttributes", creaseData);

	//zTransform t;
	//t.setIdentity();

	//fnMesh.setTransform(t);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	

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
