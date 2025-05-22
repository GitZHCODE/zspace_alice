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
zObjMesh outMesh;


////// --- GUI OBJECTS ----------------------------------------------------

void bevelHe(zItMeshHalfEdge& he, bool& absoluteDist, float& bevelVal, zPoint& out_pt0, zPoint& out_pt1)
{
	zVector vec_a = he.getNext().getVector();
	zVector vec_b = he.getPrev().getSym().getVector();

	zVector newPos_a = he.getVertex().getPosition();
	zVector newPos_b = he.getSym().getVertex().getPosition();

	if (absoluteDist)
	{
		vec_a.normalize();
		vec_b.normalize();
	}
	vec_a *= bevelVal;
	vec_b *= bevelVal;

	newPos_a += vec_a;
	newPos_b += vec_b;

	out_pt0 = newPos_b;
	out_pt1 = newPos_a;
}

void bevelQuad(zObjMesh& oMesh, const zIntArray& bevelIds, float bevelVal, zObjMesh& outMesh, bool keepBevelledFace = true, bool absoluteDist = false)
{
	// map container to store the new vertex positions for each face
	std::map<int, zVectorArray> fv_map;

	//iterate on each face
	zItMeshFace f(oMesh);

	for (f.begin(); !f.end(); f++)
	{
		//get face vertices 
		zVectorArray vPositions;
		zIntArray heIds;
		f.getHalfEdges(heIds);

		bool found = false;
		zItMeshHalfEdge he_start;
		zItMeshHalfEdge he_opposite;

		//check if this face needs bevel
		for (auto& heId : heIds)
		{
			zItMeshHalfEdge he(oMesh, heId);
			int eId = he.getEdge().getId();
			if (std::find(bevelIds.begin(), bevelIds.end(), eId) != bevelIds.end())
			{
				found = true;
				he_start = he;
				break;
			}
		}
		
		if (found)
		{
			zPoint pt0, pt1;
			bevelHe(he_start, absoluteDist, bevelVal, pt0, pt1);
			vPositions.push_back(pt0);
			vPositions.push_back(pt1);

			//check the oppsite side
			he_opposite = he_start.getNext().getNext();
			int oppsiteId = he_opposite.getEdge().getId();
			bool chk = std::find(bevelIds.begin(), bevelIds.end(), oppsiteId) != bevelIds.end();
			if (chk)
			{
				zPoint pt2, pt3;
				bevelHe(he_opposite, absoluteDist, bevelVal, pt2, pt3);
				vPositions.push_back(pt2);
				vPositions.push_back(pt3);
			}
			else
			{
				vPositions.push_back(he_opposite.getSym().getVertex().getPosition());
				vPositions.push_back(he_opposite.getVertex().getPosition());
			}
		}
		else
		{
			f.getVertexPositions(vPositions);
		}
		fv_map[f.getId()] = vPositions;
	}

	//add face back
	if (keepBevelledFace)
	{
		int counter = fv_map.size();
		for (auto& eId : bevelIds)
		{
			zPointArray vPositions;
			zItMeshEdge e(oMesh, eId);
			zItMeshHalfEdge he_a = e.getHalfEdge(0);
			zItMeshHalfEdge he_b = e.getHalfEdge(1);

			zPoint pt0, pt1, pt2, pt3;
			if (!e.onBoundary())
			{
				bevelHe(he_a, absoluteDist, bevelVal, pt0, pt1);
				bevelHe(he_b, absoluteDist, bevelVal, pt2, pt3);
			}
			else
			{
				if (he_a.onBoundary())
				{
					bevelHe(he_b, absoluteDist, bevelVal, pt0, pt1);
					pt2 = he_a.getSym().getVertex().getPosition();
					pt3 = he_a.getVertex().getPosition();
				}
				else if (he_b.onBoundary())
				{
					bevelHe(he_a, absoluteDist, bevelVal, pt0, pt1);
					pt2 = he_b.getSym().getVertex().getPosition();
					pt3 = he_b.getVertex().getPosition();
				}
			}
			vPositions.push_back(pt3);
			vPositions.push_back(pt2);
			vPositions.push_back(pt1);
			vPositions.push_back(pt0);

			fv_map[counter] = vPositions;
			counter++;
		}
	}

    // Update the mesh
    zVectorArray positions;
    zIntArray pCounts;
    zIntArray pConnects;

    for (auto& map : fv_map)
    {
        for (auto& v : map.second)
        {
            int vID;
            bool check = core.checkRepeatVector(v, positions, vID);

            if (!check)
            {
                vID = positions.size();
                positions.push_back(v);
            }
            pConnects.push_back(vID);
        }
        pCounts.push_back(map.second.size());
    }

    zFnMesh fn(outMesh);
    fn.create(positions, pCounts, pConnects);
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
	fnMesh.from("data/bevel.json",zJSON);
		
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
		zIntArray bevelEdges;
		//bevelEdges.push_back(4);
		zItMeshEdge e(oMesh);
		for (e.begin(); !e.end(); e++)
		{
			if (!e.onBoundary())
				bevelEdges.push_back(e.getId());

			if (e.getHalfEdge(0).getVertex().checkValency(2) && e.getHalfEdge(1).getVertex().checkValency(2))
				bevelEdges.push_back(e.getId());
		}


		bevelQuad(oMesh, bevelEdges, 0.25, outMesh, true);

		oMesh = outMesh;
		zFnMesh fn(outMesh);
		cout << endl << "V:" << fn.numVertices() << "E:" << fn.numEdges() << "F:" << fn.numPolygons() << endl;

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
