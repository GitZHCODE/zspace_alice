//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>
#include <zApp/include/zTsGeometry.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General
bool color_mesh = false;
bool compute = false;
bool export_guidemesh = false;
bool export_panels = false;

bool displayGUIDE = false;
bool displayCOLORS = false;
bool displayPANELS = false;
bool displayPANELSTRI = false;
bool displayAllPanels = false;

int currentPanelId = 0;
int totalPanels = 0;

double background = 1.00;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
string fileDir = "data/Panels/";
string outPath = "data/Panels/Export/";
zUtilsCore core;
zObjMesh o_guideMesh;
int panel_num = 0;
vector<string> paneltypes;
zPointArray positions;

/*!<Tool sets*/
zTsPaneling myPaneling;

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
	zFnMesh fnPanelMesh(o_guideMesh);	
	//fnPanelMesh.from("data/Panels/JinanSamplePanels.json", zJSON);
	fnPanelMesh.from("data/Panels/inMesh.obj", zOBJ);


	//set paneling
	myPaneling.setGuideMesh(o_guideMesh);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(o_guideMesh);

	// set display element booleans
	zObjMesh* guideMesh = myPaneling.getRawGuideMesh();
	guideMesh->setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&color_mesh, "color_mesh");
	B.buttons[0].attachToVariable(&color_mesh);
	
	B.addButton(&compute, "compute");
	B.buttons[1].attachToVariable(&compute);

	B.addButton(&displayGUIDE, "dGUIDE");
	B.buttons[2].attachToVariable(&displayGUIDE);

	B.addButton(&displayCOLORS, "dCOLORS");
	B.buttons[3].attachToVariable(&displayCOLORS);

	B.addButton(&displayPANELS, "dPANELS");
	B.buttons[4].attachToVariable(&displayPANELS);

	B.addButton(&displayPANELSTRI, "dPANELSTRI");
	B.buttons[5].attachToVariable(&displayPANELSTRI);

	B.addButton(&export_guidemesh, "export_guidemesh");
	B.buttons[6].attachToVariable(&export_guidemesh);
	
	B.addButton(&export_panels, "export_panels");
	B.buttons[7].attachToVariable(&export_panels);

}

void update(int value)
{	
	if (color_mesh)
	{
		// Start measuring time
		auto begin = std::chrono::high_resolution_clock::now();
	
		//myPaneling.colorPanels();
		myPaneling.colorGuideMesh();
		//myPaneling.colorGuideMeshFour();
		color_mesh=!color_mesh;

		// Stop measuring time
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n Time: %.7f seconds.", elapsed.count() * 1e-9);
	}

	if (compute)
	{
		myPaneling.createPanels();
		myPaneling.computePanelType(0.015);
		compute = !compute;
	}

	if (export_guidemesh)
	{
		zObjMesh* exportMesh = myPaneling.getRawGuideMesh();
	    zFnMesh json_mesh= (*exportMesh);
		string filename = outPath + "panels"+ ".json";
		json_mesh.to(filename, zJSON);
		
		export_guidemesh =!export_guidemesh;
	}
	if (export_panels)
	{
		int numPanels = 0;
		zPanel* panels = myPaneling.getRawPanels(numPanels);
		paneltypes.clear();

		for (int i = 0; i < numPanels; i++)
		{
				zObjMesh* m = panels[i].getRawPanelMesh();
				zFnMesh json_mesh = (*m);
				string filename = outPath + "panel"+ to_string(i) + ".json";
				json_mesh.to(filename, zJSON);	
		}
		myPaneling.exportPanelTypes(outPath);
		export_panels = !export_panels;
	}
}

void draw()
{
	backGround(background);

	S.draw();
	B.draw();

	// zspace model draw
	model.draw();

	if (displayGUIDE)
	{		
		zObjMesh* guideMesh = myPaneling.getRawGuideMesh();
		guideMesh->setDisplayElements(false, true, true);
		guideMesh->draw();
	}
	if (displayCOLORS)
	{
		zObjMesh* colorMesh = myPaneling.getRawGuideMesh();
		colorMesh->setDisplayElements(false, true, true);
		colorMesh->draw();
	}
	if (displayPANELS || displayPANELSTRI)
	{
		int numPanels = 0;
		zPanel* panels = myPaneling.getRawPanels(numPanels);

		totalPanels = numPanels;
		if(displayAllPanels)
		{
			for (int i = 0; i < numPanels; i++)
			{
				if (displayPANELS)
				{
					zObjMesh* m = panels[i].getRawPanelMesh();
					m->setDisplayElements(false, true, true);
					m->draw();
				}
				if (displayPANELSTRI)
				{
					zObjMesh* m = panels[i].getRawPanelMesh_Tri();
					m->setDisplayElements(false, true, true);
					m->draw();
				}
			}
		}
		else
		{
			for (int i = 0; i < numPanels; i++)
			{
				if (displayPANELS)
				{
					zObjMesh* m = panels[i].getRawPanelMesh();
					m->setDisplayElements(false, true, false);
					m->draw();
				}
				if (displayPANELSTRI)
				{
					zObjMesh* m = panels[i].getRawPanelMesh_Tri();
					m->setDisplayElements(false, true, false);
					m->draw();
				}
			}
			if (displayPANELS)
			{
				zObjMesh* m = panels[currentPanelId].getRawPanelMesh();
				m->setDisplayElements(false, true, true);
				m->draw();
			}
			if (displayPANELSTRI)
			{
				zObjMesh* m = panels[currentPanelId].getRawPanelMesh_Tri();
				m->setDisplayElements(false, true, true);
				m->draw();
			}
		}	
	}

	//////////////////////////////////////////////////////////

	setup2d();
	glColor3f(0, 0, 0);

	drawString("Total Panels #:" + to_string(totalPanels), vec(winW - 350, winH - 800, 0));
	drawString("Current Panel #:" + to_string(currentPanelId), vec(winW - 350, winH - 775, 0));


	drawString("Planar - green", vec(winW - 350, winH - 400, 0));
	drawString("Concave Ellipsoid - yellow", vec(winW - 350, winH - 375, 0));
	drawString("Concave Cylinder - orange", vec(winW - 350, winH - 350, 0));
	drawString("Hyperboloid - blue", vec(winW - 350, winH - 325, 0));
	drawString("ConvexCylinder - magenta", vec(winW - 350, winH - 300, 0));
	drawString("ConvexEllipsoid - cyan", vec(winW - 350, winH - 275, 0));

	drawString("Display:", vec(winW - 350, winH - 225, 0));
	drawString("w - Increment Current Graph ID", vec(winW - 350, winH - 200, 0));
	drawString("s - Decrement Current Graph ID", vec(winW - 350, winH - 175, 0));
	drawString("d - Display all graphs", vec(winW - 350, winH - 150, 0));

	restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'o') color_mesh = true;;

	if (k == 'p') compute = true;;

	if (k == 'w')
	{
		if (currentPanelId < totalPanels - 1)currentPanelId++;;
	}
	if (k == 's')
	{
		if (currentPanelId > 0)currentPanelId--;;
	}

	if (k == 'd') displayAllPanels = !displayAllPanels;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
