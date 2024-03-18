//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include <headers/zApp/include/zTsGeometry.h>

#define M_PI 3.14159265358979323846  /* pi */

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool reset = false;

double background = 0.35;
double dT = 0.5;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zIntArray fixedVertices;
zIntArray arcVertices;
zIntArray nodeVertices;

zUtilsCore core;
zObjGraph oGraph;

zObjGraph oGraph_copy;
zFnGraphDynamics dyGraph;

////// --- GUI OBJECTS ----------------------------------------------------

void setupGraph(zObjGraph& oGraph)
{
	zItGraphHalfEdge he_start;

	//traverse all halfedges in a graph
	for (zItGraphHalfEdge he(oGraph); !he.end(); he++)
	{
		if (he.getVertex().checkValency(1))
		{
			he_start = he;
			fixedVertices.push_back(he.getVertex().getId());
		}
	}

	int counter = 0;

	do
	{
		int currentV = he_start.getVertex().getId();
		if (he_start.getVertex().checkValency(2))
		{
			if (counter % 2 == 0)
			{
				nodeVertices.push_back(currentV);
			}
			else
			{
				arcVertices.push_back(currentV);
			}
		}

		counter++;
		he_start = he_start.getPrev();
	} while (!he_start.getVertex().checkValency(1));

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
	dyGraph = zFnGraphDynamics(oGraph);
	dyGraph.from("data/biArc/inGraph.json", zJSON);
	dyGraph.makeDynamic();

	setupGraph(oGraph);
	
	//dyGraph.setFixed(nodeVertices);

	oGraph_copy = oGraph;


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oGraph);

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
	B.addButton(&reset, "reset");
	B.buttons[2].attachToVariable(&reset);

}

void update(int value)
{
	if (compute)
	{
		//solver
		//set fixed vertices
		dyGraph.setFixed(fixedVertices);

		//add equal length forces
		for (auto& vId : arcVertices)
		{
			float meanLength = 0;

			zItGraphVertex v(oGraph, vId);
			zIntArray edges;
			v.getConnectedEdges(edges);

			zItGraphEdge e0(oGraph, edges[0]);
			zItGraphEdge e1(oGraph, edges[1]);

			//compute average length of two arc edges;
			meanLength += e0.getLength();
			meanLength += e1.getLength();
			meanLength *= 0.5;

			dyGraph.addSpringForce(1.0, edges[0], meanLength);
			dyGraph.addSpringForce(1.0, edges[1], meanLength);
		}

		//make 180 degrees
		float restAngle = M_PI;
		for (auto& vId : nodeVertices)
		{
			dyGraph.addAngleForce(1.0, vId, restAngle, false);
		}

		//add vector forces
		//for (auto& vId : fixedVertices)
		//{
		//	zItGraphVertex v(oGraph_copy, vId);
		//	zItGraphHalfEdge he = v.getHalfEdge();

		//	zVector origin = he.getVertex().getPosition();
		//	he = he.getSym();
		//	zVector vec = he.getVector();
		//	int vId = he.getVertex().getId();

		//	dyGraph.addVectorForce(1.0, vId, origin, vec);
		//}

		//run the solver
		dyGraph.update(dT, zEuler, true, true, true);

		//compute = !compute;	
	}

	if (reset)
	{
		oGraph = oGraph_copy;
		reset = false;
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

		zPoint* vPositions = dyGraph.getRawVertexPositions();

		for (int i = 0; i < dyGraph.numVertices(); i++) {
			bool nodeV = std::find(std::begin(nodeVertices), std::end(nodeVertices), i) != std::end(nodeVertices);
			bool arcV = std::find(std::begin(arcVertices), std::end(arcVertices), i) != std::end(arcVertices);
			bool fixedV = std::find(std::begin(fixedVertices), std::end(fixedVertices), i) != std::end(fixedVertices);

			if (nodeV)
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 1, 0, 1), 5);
			else if (fixedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(1, 0, 0, 1), 5);
			//else if (alignedV)
			//	model.displayUtils.drawPoint(vPositions[i], zColor(0, 1, 0, 1), 5);
			else if (arcV)
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 0, 1, 1), 5);
			else
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 0, 0, 1), 5);
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
