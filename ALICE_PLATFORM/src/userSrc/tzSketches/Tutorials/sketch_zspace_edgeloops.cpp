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
bool display = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMesh oMesh;
zFnMesh fn;
zIntArray allEdges;

//define container of output halfedge ids
vector<int> edgeIds;
int vertexId;

void getEdgeLoops(zObjMesh& oMesh, bool includeBoundaries = true, bool flip = false)
{
	zItMeshVertex v(oMesh);
	zIntArray corners;
	for (v.begin(); !v.end(); v++)
	{
		if(v.checkValency(2))
		corners.push_back(v.getId());
	}

	int cornerId = flip ? corners[1] : corners[0];
	zIntArray cHes;
	zItMeshVertex corner(oMesh, cornerId);
	corner.getConnectedHalfEdges(cHes);

	zItMeshHalfEdge he;
	for (auto& id : corners)
	{
		zItMeshHalfEdge test(oMesh, id);
		if (!test.onBoundary())
			he = test;
	}

	int startId = he.getVertex().getId();
	zIntArray allStartHes;
	do
	{
		allStartHes.push_back(he.getId());
		he = he.getNext().getSym().getNext();
	} while (!he.getVertex().checkValency(2) && he.getVertex().getId() != startId);

}

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
	
	fn = zFnMesh(oMesh);
	fn.from("data/skeletonMesh.json", zJSON);

	model.addObject(oMesh);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	oMesh.setDisplayElements(false, true, false);

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
		//define vertex iterator
		zItMeshVertex v(oMesh);
		//define container of halfedges
		zItMeshHalfEdgeArray cHes;

		//iterate through all vertices
		for (; !v.end(); v++)
		{
			//check vertex valence to find the star vertex
			if (v.getValence() > 4)
			{
				vertexId = v.getId();
				//get the connected halfedges if found that vertex
				v.getConnectedHalfEdges(cHes);
			}
		}

		//traverse all halfedges in cHes
		for (auto& he : cHes)
		{
			//for each halfedge go to the boundary
			do
			{
				edgeIds.push_back(he.getId());
				he = he.getNext().getSym().getNext();
			} while (!he.getVertex().onBoundary());
			edgeIds.push_back(he.getId());
		}


		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	// zspace model draw
	model.draw();

	if (display)
	{

		zItMeshVertex v(oMesh, vertexId);
		model.displayUtils.drawPoint(v.getPosition(), zRED, 5);
		//visualise our edges
		for (auto& id : edgeIds)
		{
			zItMeshHalfEdge he(oMesh, id);
			model.displayUtils.drawLine(he.getStartVertex().getPosition(), he.getVertex().getPosition(), zMAGENTA, 5);
		}
		
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
