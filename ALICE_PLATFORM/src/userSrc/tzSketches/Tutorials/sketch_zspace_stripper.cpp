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
zObjMesh oMesh;

zObjMeshArray strips_a;
zObjMeshArray strips_b;

void getStrips(int vId, zObjMesh& oMesh, zObjMeshArray& outMeshArray)
{
	zItMeshHalfEdge he(oMesh);
	zItMeshHalfEdge he_start(oMesh);
	zItMeshHalfEdgeArray hes;

	for (he.begin(); !he.end(); he++)
	{
		if (he.getVertex().getId() == vId && he.onBoundary())
		{
			he_start = he;
			break;
		}
	}

	//find all start hes
	do
	{
		hes.push_back(he_start);
		he_start = he_start.getPrev();
	} while (!he_start.getVertex().checkValency(2));

	cout << "start hes:" << hes.size() << endl;

	//find all faces in each strip
	for (auto& he : hes)
	{
		he = he.getSym();
		zObjMesh stripMesh;
		zFnMesh fm(stripMesh);

		zPointArray pVertices;
		zIntArray pCounts;
		zIntArray pConnects;
		zColor color;

		do
		{
			zItMeshFace f = he.getFace();
			zPointArray fVertices;
			f.getVertexPositions(fVertices);
			color = f.getColor();

			for (auto& v : fVertices)
			{
				int vID;
				bool check = core.checkRepeatVector(v, pVertices, vID);

				if (!check)
				{
					vID = pVertices.size();
					pVertices.push_back(v);
				}
				pConnects.push_back(vID);
			}
			pCounts.push_back(f.getNumVertices());

			he = he.getNext().getNext().getSym();

		} while (!he.onBoundary());

		fm.create(pVertices, pCounts, pConnects);
		fm.setVertexColor(color, true);
		outMeshArray.push_back(stripMesh);
	}
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
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/squareMesh.json",zJSON);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);
	for (auto& m : strips_a)
		model.addObject(m);
	for (auto& m : strips_b)
		model.addObject(m);
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
		strips_a.clear();
		strips_b.clear();
		
		zIntArray corners;
		zItMeshVertex v(oMesh);
		for (v.begin(); !v.end(); v++)
		{
			if (v.checkValency(2))
				corners.push_back(v.getId());
		}

		getStrips(corners[0], oMesh, strips_a);
		getStrips(corners[1], oMesh, strips_b);

		for (int i = 0; i < strips_a.size(); i++)
		{
			if (i % 2 != 0)
			{
				zItMeshFace f(strips_a[i]);
				for (f.begin(); f.end(); f++)
				{
					if (f.getId() % 2 != 0)
					{
						zIntArray fvIds;
						f.getVertices(fvIds);
						for (auto& vId : fvIds)
						{
							zItMeshVertex v(strips_a[i],vId);
							zVector pos = v.getPosition();
							pos += zVector(0, 0, 1);
							v.setPosition(pos);
						}
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
