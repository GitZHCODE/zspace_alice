//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zColor BLACK(0, 0, 0, 1);
zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor MAGENTA(1, 0, 1, 1);
zColor CYAN(0, 1, 1, 1);
zColor ORANGE(1, 0.5, 0, 1);
zColor YELLOW(1, 1, 0, 1);

zUtilsCore core;

void computeV(zObjMesh &_oMesh , int dir_sVID, int dir_eVID, zInt2DArray &_vertex_graphIds, zObjGraphArray &_vGraphs)
{
	zFnMesh fnMesh(_oMesh);	

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	//printf("\n % i ", eColors.size());

	zItMeshHalfEdge U_startHE;
	bool chk = fnMesh.halfEdgeExists(dir_sVID, dir_eVID, U_startHE);

	int graphIds = 0;
	_vGraphs.clear();
	_vGraphs.assign(15,zObjGraph());

	if (chk)
	{
	
		bool onBoundary = U_startHE.onBoundary();
		if (onBoundary)U_startHE = U_startHE.getSym();

		printf("\n %i %i ", U_startHE.getStartVertex().getId(), U_startHE.getVertex().getId());

		bool exitU = false;

		zItMeshHalfEdge U_currentHE = U_startHE;
		U_currentHE = (onBoundary) ? U_currentHE.getPrev().getSym().getPrev() : U_currentHE.getNext().getSym().getNext();

		do
		{
			zPointArray gPositions;
			zIntArray eConnects;

			zItMeshHalfEdge V_startHE = (onBoundary) ? U_currentHE.getNext() : U_currentHE.getPrev();
			zItMeshHalfEdge V_currentHE = V_startHE;
			bool exitV = false;

			

			do
			{
				if (V_currentHE.getVertex().checkValency(3)) exitV = true;
				
				eColors[V_currentHE.getEdge().getId()] = MAGENTA;

				gPositions.push_back(V_currentHE.getStartVertex().getPosition());
				
				// add V graphId
				_vertex_graphIds[V_currentHE.getStartVertex().getId()][1] = graphIds;

				if (gPositions.size() > 1)
				{
					eConnects.push_back(gPositions.size() - 2);
					eConnects.push_back(gPositions.size() - 1);
				}

				if (exitV)
				{
					gPositions.push_back(V_currentHE.getVertex().getPosition());
					eConnects.push_back(gPositions.size() - 2);
					eConnects.push_back(gPositions.size() - 1);

					// add V graphId
					_vertex_graphIds[V_currentHE.getVertex().getId()][1] = graphIds;
				}

				 V_currentHE = V_currentHE.getNext().getSym().getNext();				

			} while (!exitV);

			// make graph
			
			zFnGraph fnGraph(_vGraphs[graphIds]);
			fnGraph.create(gPositions, eConnects);
			graphIds++;


			if (onBoundary)
			{
				if (U_currentHE.getStartVertex().checkValency(2)) exitU = true;
			}
			else
			{
				if (U_currentHE.getVertex().checkValency(2)) exitU = true;
			}				



			if (!exitU)
			{
				U_currentHE = (onBoundary) ? U_currentHE.getPrev().getSym().getPrev() : U_currentHE.getNext().getSym().getNext();
			}

			

		} while (!exitU);
	}

	fnMesh.setEdgeColors(eColors, false);
}

void computeU(zObjMesh& _oMesh, int dir_sVID, int dir_eVID, zInt2DArray& _vertex_graphIds, zObjGraphArray& _uGraphs)
{
	zFnMesh fnMesh(_oMesh);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	//printf("\n % i ", eColors.size());

	zItMeshHalfEdge U_startHE;
	bool chk = fnMesh.halfEdgeExists(dir_sVID, dir_eVID, U_startHE);

	int graphIds = 0;
	_uGraphs.clear();
	_uGraphs.assign(23, zObjGraph());

	if (chk)
	{
		bool onBoundary = U_startHE.onBoundary();
		if (onBoundary)U_startHE = U_startHE.getSym();

		printf("\n %s %i %i ", (onBoundary) ? "true": "false", U_startHE.getStartVertex().getId(), U_startHE.getVertex().getId());

		bool exitU = false;

		zItMeshHalfEdge U_currentHE = U_startHE;
		U_currentHE = (onBoundary) ? U_currentHE.getPrev().getSym().getPrev() : U_currentHE.getNext().getSym().getNext();

		do
		{
			zPointArray gPositions;
			zIntArray eConnects;

			zItMeshHalfEdge V_startHE = (onBoundary) ? U_currentHE.getNext() : U_currentHE.getPrev();
			zItMeshHalfEdge V_currentHE = V_startHE;
			bool exitV = false;

			do
			{
				if (onBoundary)
				{
					if (V_currentHE.getVertex().checkValency(3) && V_currentHE.getVertex().onBoundary()) exitV = true;
				}
				else
				{
					if (V_currentHE.getStartVertex().checkValency(3) && V_currentHE.getStartVertex().onBoundary()) exitV = true;
				}

				eColors[V_currentHE.getEdge().getId()] = CYAN;

				(onBoundary) ? gPositions.push_back(V_currentHE.getStartVertex().getPosition()) : gPositions.push_back(V_currentHE.getVertex().getPosition());

				// add V graphId
				_vertex_graphIds[V_currentHE.getStartVertex().getId()][0] = graphIds;

				if (gPositions.size() > 1)
				{
					eConnects.push_back(gPositions.size() - 2);
					eConnects.push_back(gPositions.size() - 1);
				}

				if (exitV)
				{
					(onBoundary) ? 	gPositions.push_back(V_currentHE.getVertex().getPosition()) : gPositions.push_back(V_currentHE.getStartVertex().getPosition());
					eConnects.push_back(gPositions.size() - 2);
					eConnects.push_back(gPositions.size() - 1);

					// add V graphId
					_vertex_graphIds[V_currentHE.getVertex().getId()][0] = graphIds;
				}


				if (onBoundary)
				{
					if (V_currentHE.getVertex().checkValency(3))
					{
						zVector curV = V_currentHE.getVector();
						zVector nextV = V_currentHE.getNext().getVector();
						

						if (curV * nextV > 0.5) 	V_currentHE = V_currentHE.getNext();
						else  V_currentHE = V_currentHE.getNext().getSym().getNext();
					}
					else V_currentHE = V_currentHE.getNext().getSym().getNext();
				}
				else
				{
					if (V_currentHE.getStartVertex().checkValency(3))
					{
						zVector curV = V_currentHE.getVector();
						zVector prevV = V_currentHE.getPrev().getVector();
						

						if (prevV * curV > 0.3) 	V_currentHE = V_currentHE.getPrev();
						else  V_currentHE = V_currentHE.getPrev().getSym().getPrev();
					}
					else V_currentHE = V_currentHE.getPrev().getSym().getPrev();
				}


			} while (!exitV);

			// make graph

			zFnGraph fnGraph(_uGraphs[graphIds]);
			fnGraph.create(gPositions, eConnects);
			graphIds++;


			if (onBoundary)
			{
				if (U_currentHE.getStartVertex().checkValency(2)) exitU = true;
			}
			else
			{
				if (U_currentHE.getVertex().checkValency(2)) exitU = true;
			}



			if (!exitU)
			{
				U_currentHE = (onBoundary) ? U_currentHE.getPrev().getSym().getPrev() : U_currentHE.getNext().getSym().getNext();
			}



		} while (!exitU);
	}

	fnMesh.setEdgeColors(eColors, false);
}

void makePlanesFromGraph(zObjGraphArray& _Graphs, vector<zTransform>& _transforms)
{
	_transforms.clear();
	for (auto& g : _Graphs)
	{
		zFnGraph fnGraph(g);
		zPoint* vPositions = fnGraph.getRawVertexPositions();

		int startID = 0;
		int endID = fnGraph.numVertices() - 1;
		int midID = floor(fnGraph.numVertices() * 0.5);

		zVector X = vPositions[startID] - vPositions[midID];
		X.normalize();

		zVector Y = vPositions[endID] - vPositions[midID];
		Y.normalize();

		zVector Z = X ^ Y;
		Z.normalize();

		Y = Z ^ X;
		Y.normalize();

		zPoint O = vPositions[midID];

		zTransform out;

		out(0, 0) = X.x; out(0, 1) = X.y; out(0, 2) = X.z; out(0, 3) = 1;
		out(1, 0) = Y.x; out(1, 1) = Y.y; out(1, 2) = Y.z; out(1, 3) = 1;
		out(2, 0) = Z.x; out(2, 1) = Z.y; out(2, 2) = Z.z; out(2, 3) = 1;
		out(3, 0) = O.x; out(3, 1) = O.y; out(3, 2) = O.z; out(3, 3) = 1;
		_transforms.push_back(out);

		// distance from Plane
		zDomainFloat minMaxDistance(10000, -10000);
		printf("\n \n graph %i ", _transforms.size() -1);
		for(int i =0; i< fnGraph.numVertices(); i++)
		{
			double dist = abs(core.minDist_Point_Plane(vPositions[i], O, Z));

			if (dist < minMaxDistance.min) minMaxDistance.min = dist;
			if (dist > minMaxDistance.max) minMaxDistance.max = dist;		

		}
		printf(" | %1.4f  %1.4f", minMaxDistance.min, minMaxDistance.max);
	}
}

bool exportToJSON(string dir, zObjGraphArray& _Graphs, vector<zTransform>& _transforms ,  bool UGraphs)
{
	string fileName = (UGraphs) ? "UGraph" : "VGraph";

	string filePath = dir;
	filePath += (UGraphs) ? "/U" : "/V";

	for (int i = 0; i < _Graphs.size(); i++)
	{
		string tmpName = filePath + "/" + fileName;
		tmpName += "_" + to_string(i) + ".json";

		zFnGraph fnGraph(_Graphs[i]);
		fnGraph.to(tmpName, zJSON);
	}
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/


zObjMesh oMesh;
zObjGraph oGraph;

zObjGraphArray uGraphs;
zObjGraphArray vGraphs;

zInt2DArray vertex_graphIds;
vector<zTransform> uPlanes;
vector<zTransform> vPlanes;

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
	fnMesh.from("data/gridShell/shell_00.obj", zOBJ);

	vertex_graphIds.clear();
	vertex_graphIds.assign(fnMesh.numVertices(), zIntArray());

	for (int i = 0; i < vertex_graphIds.size(); i++)
	{
		// u GraphID
		vertex_graphIds[i].push_back(-1);

		//v GraphID
		vertex_graphIds[i].push_back(-1);
	}

	
	computeV(oMesh, 6, 248, vertex_graphIds,vGraphs);
	makePlanesFromGraph(vGraphs, vPlanes);
	exportToJSON("data/gridShell/out", vGraphs, vPlanes, false);

	computeU(oMesh, 6, 249, vertex_graphIds, uGraphs);
	makePlanesFromGraph(uGraphs, uPlanes);
	exportToJSON("data/gridShell/out", uGraphs, uPlanes, true);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	for (auto& g : vGraphs)
	{
		model.addObject(g);
		g.setDisplayElements(true, true);
	}

	for (auto& g : uGraphs)
	{
		model.addObject(g);
		g.setDisplayElements(true, true);
	}

	// set display element booleans
	oMesh.setDisplayElements(false, false, false);

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
		zFnMesh fnMesh(oMesh);
		fnMesh.smoothMesh(1);
		fnMesh.to("data/outMesh.json", zJSON);;

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
		
	}

	for (auto& t : vPlanes)
	{
		model.displayUtils.drawTransform(t, 0.5);
	}

	for (auto& t : uPlanes)
	{
		model.displayUtils.drawTransform(t, 0.5);
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
