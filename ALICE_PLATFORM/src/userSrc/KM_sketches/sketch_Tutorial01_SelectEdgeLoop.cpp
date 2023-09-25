//#define _MAIN_

//#ifdef _MAIN_

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
zObjMesh oMesh;


////// --- GUI OBJECTS ----------------------------------------------------

vector<zItMeshHalfEdge> selectEdgeLoop(zItMeshHalfEdge startHalfEdge) {
	vector<zItMeshHalfEdge> edgeLoop;

	// Check starting half edge is valid and active
	if (!startHalfEdge.isActive()) return edgeLoop;
	if (!startHalfEdge.onBoundary()) {
		startHalfEdge = startHalfEdge.getSym();
	}
	zItMeshHalfEdge current = startHalfEdge;
	int boundaryCounter = 0;
	int loopCounter = 0;

	bool forwardTraversal = true; 

	do {
		edgeLoop.push_back(current);
		loopCounter++;
		bool endVertexOnBoundary = current.getVertex().onBoundary();
		bool startVertexOnBoundary = current.getSym().getVertex().onBoundary();

		if (boundaryCounter == 2 && (!endVertexOnBoundary || !startVertexOnBoundary)) {
			break;  // If a boundary encountered for the second time and either vertex is not on boundary, exit loop
		}

		else if (endVertexOnBoundary && startVertexOnBoundary) {
			// Both start and end vertices are on boundary, get the next HE
			
			
			current = current.getNext();
			
		}
		
		else if (endVertexOnBoundary && !startVertexOnBoundary) {
			boundaryCounter++;
			forwardTraversal = !forwardTraversal;  // Reverse the direction	
			current = current.getPrev().getSym().getPrev();
			
		}

		else if (!endVertexOnBoundary && startVertexOnBoundary) {
			boundaryCounter++;
			current = current.getNext().getSym().getNext();
		
		}

		else {
			
			current = current.getNext().getSym().getNext();
		}
		

		//if (forwardTraversal) {
		//	current = current.getNext().getSym().getNext();
		//}
		//else {
		//	current = current.getPrev().getSym().getPrev(); // reverse direction
		//}

		
		/*if (loopCounter > 2) {
			cout << "Loop has run more than 5 iterations. Breaking." << endl;
			break;
		}*/
	} while (current != startHalfEdge && current.isActive());
	cout << "Loop ran for " << loopCounter << " steps." << endl;
	return edgeLoop;
}

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
	
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/TestData/alicetest3.obj", zOBJ, false);


	zItMeshHalfEdge myStartEdge(oMesh, 200);
	vector<zItMeshHalfEdge> myEdgeLoop = selectEdgeLoop(myStartEdge);
	zColor magenta(1.0, 0.0, 1.0, 1.0);

	//myStartEdge.getEdge().setColor(magenta);

	for (auto& edge : myEdgeLoop) {
		edge.getEdge().setColor(magenta);
	}
	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

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
