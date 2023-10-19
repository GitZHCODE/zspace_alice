//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsStreamLines.h>

#include <igl/readOBJ.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// Custom Methods
zUtilsCore core;

void computeVLoops(zObjMesh& oMesh, int startVertexID0, int startVertexID1, vector<zItMeshHalfEdgeArray>& v_Loops)
{
	zFnMesh fnMesh_in(oMesh);

	zItMeshHalfEdge heStart;
	bool chk = fnMesh_in.halfEdgeExists(startVertexID1, startVertexID0, heStart);



	bool flip = false;
	if (!heStart.onBoundary())
	{
		heStart = heStart.getSym();
		flip = true;
	}

	zItMeshHalfEdge he_U = heStart;
	bool exit_1 = false;
	do
	{

		zItMeshHalfEdge he_V = (!flip) ? he_U.getSym().getNext() : he_U.getSym().getPrev();
		int startVertexValence = (!flip) ? he_V.getStartVertex().getValence() : he_V.getVertex().getValence();

		zItMeshHalfEdgeArray tempV;

		bool exit_2 = false;

		do
		{
			if (flip) tempV.push_back(he_V.getSym());
			else tempV.push_back(he_V);

			he_V.getEdge().setColor(zBLUE);

			if (flip)
			{
				if (he_V.getStartVertex().checkValency(startVertexValence)) exit_2 = true;
			}
			else
			{
				if (he_V.getVertex().checkValency(startVertexValence)) exit_2 = true;
			}


			if (!exit_2) he_V = (!flip) ? he_V.getNext().getSym().getNext() : he_V.getPrev().getSym().getPrev();

		} while (!exit_2);

		v_Loops.push_back(tempV);

		he_U.getEdge().setColor(zMAGENTA);

		if (flip)
		{
			if (he_U.getStartVertex().checkValency(2)) exit_1 = true;
		}
		else
		{
			if (he_U.getVertex().checkValency(2)) exit_1 = true;
		}

		//if (he == heStart) exit_1 = true;



		if (!exit_1) he_U = (!flip) ? he_U.getNext() : he_U.getPrev();
		else
		{
			tempV.clear();
			zItMeshHalfEdge he_V1 = (!flip) ? he_U.getNext() : he_U.getPrev();
			bool exit_3 = false;
			do
			{
				if (flip)tempV.push_back(he_V1.getSym());
				else tempV.push_back(he_V1);
				he_V1.getEdge().setColor(zBLUE);

				if (flip)
				{
					if (he_V1.getStartVertex().checkValency(2)) exit_3 = true;
				}
				else
				{
					if (he_V1.getVertex().checkValency(2)) exit_3 = true;
				}


				if (!exit_3) he_V1 = (!flip) ? he_V1.getNext() : he_V1.getPrev();

			} while (!exit_3);

			v_Loops.push_back(tempV);
		}

	} while (!exit_1);
}

void computeFieldVectors(zObjMesh& oMesh, vector<zItMeshHalfEdgeArray>& v_Loops, zVectorArray& fieldVectors)
{
	zFnMesh fnMesh_in(oMesh);

	fieldVectors.clear();
	fieldVectors.assign(fnMesh_in.numVertices(), zVector());

	for (int i = 0; i < v_Loops.size(); i++)
	{
		for (int j = 0; j < v_Loops[i].size(); j++)
		{
			
			if (j == 0)
			{
				int vStart = v_Loops[i][j].getStartVertex().getId();
				zVector dir = v_Loops[i][j].getVector();
				dir.normalize();
				fieldVectors[vStart] = dir;
			}
			else if (j == v_Loops[i].size() - 1)
			{
				int vStart = v_Loops[i][j].getStartVertex().getId();
				zVector dir = v_Loops[i][j - 1].getVector() + v_Loops[i][j].getVector();
				dir.normalize();
				fieldVectors[vStart] = dir;


				int vEnd = v_Loops[i][j].getVertex().getId();
				zVector dir_1 =  v_Loops[i][j].getVector();
				dir_1.normalize();
				fieldVectors[vEnd] = dir_1;
			}
			else
			{
				int vStart = v_Loops[i][j].getStartVertex().getId();
				zVector dir = v_Loops[i][j - 1].getVector() + v_Loops[i][j].getVector();
				dir.normalize();
				fieldVectors[vStart] = dir;
			}
		}
	}


}

void computeSeedPoints(zObjMesh& oMesh, int startVertexID0, int startVertexID1, float panelWidth, zPointArray &seedPoints)
{
	zFnMesh fnMesh_in(oMesh);
	seedPoints.clear();

	zItMeshHalfEdgeArray v_Loop;
	// compute vLoop

	zItMeshHalfEdge heStart;
	bool chk = fnMesh_in.halfEdgeExists(startVertexID1, startVertexID0, heStart);

	zItMeshHalfEdge he_U = heStart;
	bool exit_1 = false;

	do
	{
		v_Loop.push_back(he_U);

		he_U.getEdge().setColor(zMAGENTA);
	
		if (he_U.getVertex().checkValency(3)) exit_1 = true;

		if (!exit_1) he_U =  he_U.getNext().getSym().getNext();		

	} while (!exit_1);
	

	// length 
	float length = 0;
	for (int j = 0; j < v_Loop.size(); j++)
	{
		//printf(" %i ", v_Loops[l][j].getId());
		length += v_Loop[j].getLength();
	}

	int numDivs = ceil(length / panelWidth);
	fnMesh_in.computeEdgeLoop_Split(v_Loop, numDivs, seedPoints);
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool d_inMesh = true;
bool d_outMesh = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;
zObjMesh oMesh_quad;

zObjMesh oMeshOut;

int startVertexID0 = 4270;
int startVertexID1 = 1024;

float panelWidth = 0.1;

zTsStreamsMesh myStreams;
zVectorArray fVectors;
zPoint* vPositons;

vector<zStreamLine> streams;

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
	

	zFnMesh fnMesh_in(oMesh);
	fnMesh_in.from("data/sanya/inMesh_3.json", zJSON);
	
	vector<zItMeshHalfEdgeArray> v_Loops;
	computeVLoops(oMesh, startVertexID0, startVertexID1, v_Loops);
	computeFieldVectors(oMesh, v_Loops, fVectors);

	vPositons = fnMesh_in.getRawVertexPositions();
	myStreams = zTsStreamsMesh(oMesh, fVectors);


	

	zPointArray seeds = { zPoint(-32.024,-21.357,15.146) };
	float pWidth = 1;
	computeSeedPoints(oMesh, 1035,3, pWidth, seeds);

	printf("\n seeds %i ", seeds.size());

	myStreams.setMaxLength(20);

	myStreams.setSeperationDistance(0.5);
	myStreams.setTestSeperationDistance(0.4);

	myStreams.createStreams(streams, seeds, false, 50);

	//zPoint p(-34.721, -15.123, 10.058);
	//int faceId = -1;
	//zPoint cP;
	//myStreams.computeClosestPointToMesh(p, faceId, cP);
	//cout << "\n " << faceId << ", " << cP;
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMeshOut);

	// set display element booleans
	oMesh.setDisplayElements(false, true, false);
	oMeshOut.setDisplayElements(false, true, true);
	
	//oMesh.setFaceCenters(fCens);
	//oMesh.setEdgeCenters(eCens);
	//oMesh.setDisplayElementIds(true, false, true);
	//oMesh.setDisplayFaceNormals(true, 0.1);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	
	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_outMesh, "d_outMesh");
	B.buttons[2].attachToVariable(&d_outMesh);

}

void update(int value)
{
	oMesh.setDisplayObject(d_inMesh);
	oMeshOut.setDisplayObject(d_outMesh);

	if (compute)
	{
		vector<zItMeshHalfEdgeArray> v_Loops;
		zIntArray loopDivs;
		int MaxDIVS = -1;

		//computeVLoops(oMesh, startVertexID0, startVertexID1, v_Loops);
		//computeNewPositions(oMesh, v_Loops, panelWidth, divsPoints);
		//computeNewPositions_Quads(oMesh, v_Loops, panelWidth, divsPoints, loopDivs, MaxDIVS, 10);

		// create new mesh
		//createNewMesh(v_Loops, divsPoints, oMeshOut);
		//createNewMesh_Quads(v_Loops, divsPoints, loopDivs, MaxDIVS, oMeshOut, 10);

		//zFnMesh fnMesh_out(oMeshOut);
		//fnMesh_out.to("data/sanya/outMesh.json", zJSON);

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
		
	
	zFnMesh fnMesh_in(oMesh);
	for (int i = 0; i < fnMesh_in.numVertices(); i++)
	{
		zPoint p1 = vPositons[i] + (fVectors[i] * 0.1);
		model.displayUtils.drawLine(vPositons[i], p1, zGREEN);
		
	}

	for (int i = 0; i < streams.size(); i++)
	{
		streams[i].graphObj.draw();
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
