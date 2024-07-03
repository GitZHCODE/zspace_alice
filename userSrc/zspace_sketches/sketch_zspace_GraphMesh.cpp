//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMesh = true;
bool exportMesh = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjGraph oGraph;



zObjComputeMesh oCompMesh;

zPointArray positions;
zFloatArray positionsZ;

int id = 1;
string inFilePath = "data/toSTAD/TXT/3DPattern_040623_" + to_string(id) + ".txt ";
string outFilePath = "data/toSTAD/TXT/3DPattern_040623_" + to_string(id) + ".obj";

zColor BLACK(0, 0, 0, 1);
zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor MAGENTA(1, 0, 1, 1);
zColor CYAN(0, 1, 1, 1);
zColor ORANGE(1, 0.5, 0, 1);
zColor YELLOW(1, 1, 0, 1);

////// --- CUSTOM METODS ----------------------------------------------------

void drawTextAtVec(string s, zVector& pt)
{
	unsigned int i;
	glRasterPos3f(pt.x, pt.y, pt.z);

	for (i = 0; i < s.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
}

bool readTXT(string path, zObjGraph& _o_Graph, zFloatArray& positions_Z)
{
	ifstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	zPointArray positions;
	zBoolArray onBoundaryArray;
	zIntArray eConnects;

	positions_Z.clear();

	unordered_map <string, int> positionVertex;

	int vCounter = 0;
	zIntPair eVertexPair;

	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		vector<string> perlineData = core.splitString(str, " ");

		if (perlineData.size() > 0)
		{
			// vertex
			if (perlineData[0] == "v")
			{
				if (perlineData.size() == 5)
				{
					zVector pos;
					pos.x = atof(perlineData[1].c_str());
					pos.y = atof(perlineData[2].c_str());
					pos.z = 0; // atof(perlineData[3].c_str());

					bool onBoundary = (perlineData[4] == "True") ? true : false;

					//zPoint pos_factorised = core.factoriseVector(pos, 3);

					int vId = -1;
					//bool chk = core.checkRepeatVector(pos_factorised, positions, vId, 3);

					bool chk = false;
					for (int k = 0; k < positions.size(); k++)
					{
						if (positions[k].distanceTo(pos) < 0.01)
						{
							vId = k;
							chk = true;
						}

						if (chk) break;
					}

					if (chk)
					{
						onBoundaryArray[vId] = onBoundary;
					}
					else
					{
						vId = positions.size();
						positions.push_back(pos);
						onBoundaryArray.push_back(onBoundary);

						positions_Z.push_back(atof(perlineData[3].c_str()));
					}

					

					vCounter++;
					
					(vCounter % 2 == 0) ? eVertexPair.first = vId : eVertexPair.second = vId;

					if (vCounter % 2 == 0)
					{
						eConnects.push_back(eVertexPair.first);
						eConnects.push_back(eVertexPair.second);
					}

					
				}
				
			}

		
		}
	}

	myfile.close();

	zFnGraph fnGraph(_o_Graph);
	zVector up(0, 0, -1);
	fnGraph.create(positions, eConnects, up);

	zColor* vColors = fnGraph.getRawVertexColors();
	for (int i = 0; i < onBoundaryArray.size(); i++)
	{
		if (!onBoundaryArray[i]) vColors[i] = GREEN;
	}
	

	printf("\n v %i  e %i ", fnGraph.numVertices(), fnGraph.numEdges());
}

zPoint computeRefPoint(zItGraphVertexArray& vLoop)
{
	zPoint out;

	for (auto& v : vLoop) out += v.getPosition();
	out /= vLoop.size();

	return out;
}

void createMesh(zObjGraph& o_inGraph, zObjMesh& o_outMesh,bool compMesh = false,  zVector graphNorm = zVector(0,0,1))
{
	zPointArray positions;
	zIntArray pConnects, pCounts;

	zFnGraph fnGraph(o_inGraph);

	zBoolArray heVisited;
	heVisited.assign(fnGraph.numHalfEdges(), false);

	int numVCounter = 0;
	for (zItGraphVertex v(o_inGraph); !v.end(); v++)
	{
		if (v.getColor() == GREEN)
		{
			numVCounter++;

			zItGraphHalfEdgeArray cHEdges;
			v.getConnectedHalfEdges(cHEdges);

			for (auto& he : cHEdges)
			{
				if (heVisited[he.getId()]) continue;

				zItGraphHalfEdge start = he;
				int counter = 0;

				bool exit = false;

				zIntArray tempConnects;

				// compute closed loops
				zItGraphVertexArray loopV;
				loopV.push_back(he.getStartVertex());
				

				zPoint pStart;
				zPoint pR;
				zVector vR;

				do
				{
					pStart = he.getVertex().getPosition();
					if(he.getVertex() != loopV[0]) loopV.push_back(he.getVertex());
					zPoint pR = computeRefPoint(loopV);
					zVector vR = pR - pStart;
					vR.normalize();

					zItGraphVertex currentV = he.getVertex();
					zItGraphHalfEdgeArray cHEdges_1;
					currentV.getConnectedHalfEdges(cHEdges_1);

					zItGraphHalfEdge next;
					int nextID = -1;
					float maxAngle = 0;
					for (auto& cHE : cHEdges_1)
					{
						if (cHE == he.getSym()) continue;
						//if (heVisited[cHE.getId()]) continue;

						zVector heDir = cHE.getVector();
						heDir.normalize();

						float ang = vR.angle360(heDir, graphNorm);
						if (ang > maxAngle)
						{
							maxAngle = ang;
							next = cHE;
							nextID = cHE.getId();
						}
					}
					
					heVisited[he.getId()] = true;

					if (nextID != -1)
					{
						he = next;
					}
					else exit = true;
					counter++;

					if (he == start) exit = true;
					if (counter == 9)
					{
						exit = true;
						start.getEdge().setColor(MAGENTA);
						start.getStartVertex().setColor(YELLOW);						

						for (auto& v : loopV)
						{						
							v.setColor(YELLOW);
						}
					}

				} while (!exit);

				if (!compMesh)
				{
					printf("\n %i | %i | ", start.getId(), counter);
					for (auto& v : loopV)
					{
						printf(" %i ", v.getId());
					}
				}
				

				if (counter >= 3 && counter < 9)
				{
					for (auto &v : loopV) pConnects.push_back(v.getId());
					pCounts.push_back(loopV.size());
				}
			}
		}

		//if (numVCounter == 100) break;
	}
	

	if (pCounts.size() > 0)
	{
		fnGraph.getVertexPositions(positions);		

		zFnMesh fnMesh(o_outMesh);


		if (compMesh)
		{
			fnMesh.create(positions, pCounts, pConnects);

			//printf("\n v %i %i  %i", positions.size(), pCounts.size(), pConnects.size());
			printf("\n v %i e %i  p %i", fnMesh.numVertices(), fnMesh.numEdges(), fnMesh.numPolygons());
		}

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

	// read Graph	
	readTXT(inFilePath, oGraph, positionsZ);

	zVector x(1, 0, 0);
	zVector y(0, -1, 0);
	zVector z(0, 0, 1);

	cout << endl << x.angle360(y, z) << endl;
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oGraph);
	model.addObject(oMesh);

	// set display element booleans
	oGraph.setDisplayElements(true, true);	
	oMesh.setDisplayElements(true, true, true);

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
		createMesh(oGraph, oMesh, computeMesh);

		// project to Z
		zFnGraph fnGraph(oGraph);
		zFnMesh fnMesh(oMesh);

		if (positionsZ.size() == fnMesh.numVertices())
		{
			zPoint* vPositions = fnMesh.getRawVertexPositions();

			for (int i = 0; i < fnMesh.numVertices(); i++)
			{
				vPositions[i].z = positionsZ[i];
			}
		}

		

		compute = !compute;	
	}

	if (exportMesh)
	{
		zFnMesh fnMesh(oMesh);
		fnMesh.to(outFilePath, zOBJ);

		exportMesh = !exportMesh;
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

		zFnGraph fnGraph(oGraph);
		zPoint* positions = fnGraph.getRawVertexPositions();

		for (int i = 0; i < fnGraph.numVertices(); i++)
		{
			drawTextAtVec(to_string(i), positions[i]);
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

	if (k == 'c')
	{
		computeMesh = true;
		compute = true;
	}

	if (k == 'e') exportMesh = true;;
	

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
