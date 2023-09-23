#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


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
				if(flip)tempV.push_back(he_V1.getSym());
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

void computeNewPositions(float panelWidth, vector<zItMeshHalfEdgeArray>& v_Loops, vector<zPointArray>& divsPoints)
{
	divsPoints.clear();

	for (int l = 0; l < v_Loops.size(); l++)
	{
		//printf("\n loop %i | ", l);
		float length = 0;
		for (int j = 0; j < v_Loops[l].size(); j++)
		{
			//printf(" %i ", v_Loops[l][j].getId());
			length += v_Loops[l][j].getLength();
		}

		//printf(" | %1.2f ", length);

		int numDivs = floor(length / panelWidth);
		int actualDivs = -1;

		for (int i = 0; i < 6; i++)
		{
			if (pow(2, i) > numDivs)
			{
				actualDivs = pow(2, i);
				break;
			}
		}

		//printf(" | nD %i | aD %i ", numDivs, actualDivs);

		float actualPWidth = length / actualDivs;
		zPointArray loop_pts;
		loop_pts.push_back(v_Loops[l][0].getStartVertex().getPosition()); // first
		int currentindex = 0;

		//////////////////////// loop 
		int currentIndex = 0;
		zItMeshHalfEdge walkHe = v_Loops[l][currentIndex];
		zPoint pOnCurve = walkHe.getStartVertex().getPosition();;
		bool exit = false;

		zPoint start;

		float dStart = 0;
		float dIncrement = actualPWidth;
		while (!exit)
		{
			zPoint eEndPoint = walkHe.getVertex().getPosition();
			dStart += dIncrement;
			float distance_increment = dIncrement;
			while (pOnCurve.distanceTo(eEndPoint) < distance_increment)
			{
				distance_increment = distance_increment - pOnCurve.distanceTo(eEndPoint);
				pOnCurve = eEndPoint;

				currentIndex++;
				walkHe = v_Loops[l][currentIndex];
				eEndPoint = walkHe.getVertex().getPosition();
			}

			zVector he_vec = walkHe.getVector();
			he_vec.normalize();

			start = pOnCurve + he_vec * distance_increment;
			pOnCurve = start;

			loop_pts.push_back(pOnCurve);

			if (loop_pts.size() == actualDivs) exit = true;
		}

		///////////////////////////////////
		int index = v_Loops[l].size() - 1;
		loop_pts.push_back(v_Loops[l][index].getVertex().getPosition()); // last

		divsPoints.push_back(loop_pts);

	}

}

void createNewMesh(vector<zItMeshHalfEdgeArray>& v_Loops, vector<zPointArray>& divsPoints, zObjMesh& oMesh)
{
	zFnMesh fnMesh_out(oMesh);


	zPointArray positions;
	zIntArray pCounts, pConnects;

	for (int i = 0; i < v_Loops.size() - 1; i++)
	{
		int numV_current = divsPoints[i].size();
		int numV_next = divsPoints[i + 1].size();

		int current_pow = -1;
		for (int m = 0; m < 6; m++) if (numV_current - pow(2, m) == 1) current_pow = m;

		int next_pow = -1;
		for (int m = 0; m < 6; m++) if (numV_next - pow(2, m) == 1) next_pow = m;

		int powerDiff = abs(current_pow - next_pow);

		if (numV_current == numV_next)
		{
			int startId_current = 0;
			int startId_next = 0;

			int numFaces = numV_current - 1;
			//printf("\n %i %i numF %i", numV_current, numV_next, numFaces);

			for (int j = 0; j < numFaces; j++)
			{
				// add pposition to array

				int v0 = -1;
				core.checkRepeatVector(divsPoints[i][startId_current], positions, v0, 3);
				if (v0 == -1)
				{
					v0 = positions.size();
					positions.push_back(divsPoints[i][startId_current]);
				}

				int v1 = -1;
				core.checkRepeatVector(divsPoints[i][startId_current + 1], positions, v1, 3);
				if (v1 == -1)
				{
					v1 = positions.size();
					positions.push_back(divsPoints[i][startId_current + 1]);
				}

				int v2 = -1;
				core.checkRepeatVector(divsPoints[i + 1][startId_next + 1], positions, v2, 3);
				if (v2 == -1)
				{
					v2 = positions.size();
					positions.push_back(divsPoints[i + 1][startId_next + 1]);
				}

				int v3 = -1;
				core.checkRepeatVector(divsPoints[i + 1][startId_next], positions, v3, 3);
				if (v3 == -1)
				{
					v3 = positions.size();
					positions.push_back(divsPoints[i + 1][startId_next]);
				}


				pConnects.push_back(v0);
				pConnects.push_back(v1);
				pConnects.push_back(v2);
				pConnects.push_back(v3);

				pCounts.push_back(4);

				startId_current += 1;
				startId_next += 1;
			}

		}
		else if (numV_current < numV_next)
		{
			int startId_current = 0;
			int startId_next = 0;

			int numFaces = numV_current - 1;
			//printf("\n %i %i numF %i |", numV_current, numV_next, numFaces);
			for (int j = 0; j < numFaces; j++)
			{
				// add pposition to array

				int numFacePoint = 0;

				int v0 = -1;
				core.checkRepeatVector(divsPoints[i][startId_current], positions, v0, 3);
				if (v0 == -1)
				{
					v0 = positions.size();
					positions.push_back(divsPoints[i][startId_current]);
				}
				pConnects.push_back(v0);
				numFacePoint++;
				//printf(" %i ", v0);

				int v1 = -1;
				core.checkRepeatVector(divsPoints[i][startId_current + 1], positions, v1, 3);
				if (v1 == -1)
				{
					v1 = positions.size();
					positions.push_back(divsPoints[i][startId_current + 1]);
				}
				pConnects.push_back(v1);
				numFacePoint++;
				//printf(" %i ", v1);

				int v2 = -1;
				core.checkRepeatVector(divsPoints[i + 1][startId_next + pow(2, powerDiff)], positions, v2, 3);
				if (v2 == -1)
				{
					v2 = positions.size();
					positions.push_back(divsPoints[i + 1][startId_next + pow(2, powerDiff)]);
				}
				pConnects.push_back(v2);
				numFacePoint++;
				//printf(" %i ", v2);

				for (int k = startId_next + pow(2, powerDiff) - 1; k > startId_next ; k--)
				{
					int v = -1;
					core.checkRepeatVector(divsPoints[i + 1][k], positions, v, 3);
					if (v == -1)
					{
						v = positions.size();
						positions.push_back(divsPoints[i + 1 ][k]);
					}
					pConnects.push_back(v);
					numFacePoint++;
					//printf(" %i ", v);

				}

				int v3 = -1;
				core.checkRepeatVector(divsPoints[i + 1][startId_next], positions, v3, 3);
				if (v3 == -1)
				{
					v3 = positions.size();
					positions.push_back(divsPoints[i + 1][startId_next]);
				}
				pConnects.push_back(v3);
				numFacePoint++;
				//printf(" %i ", v3);




				pCounts.push_back(numFacePoint);

				startId_current += 1;
				startId_next += pow(2, powerDiff);
			}


		}
		else if (numV_current > numV_next)
		{

			int startId_current = 0;
			int startId_next = 0;

			int numFaces = numV_next - 1;
			//printf("\n %i %i numF %i", numV_current, numV_next, numFaces);
		

			for (int j = 0; j < numFaces; j++)
			{
				// add pposition to array

				int numFacePoint = 0;

				int v0 = -1;
				core.checkRepeatVector(divsPoints[i][startId_current], positions, v0, 3);
				if (v0 == -1)
				{
					v0 = positions.size();
					positions.push_back(divsPoints[i][startId_current]);
				}
				pConnects.push_back(v0);
				numFacePoint++;

				for (int k = startId_current + 1; k < startId_current + pow(2, powerDiff); k++)
				{
					int v = -1;
					core.checkRepeatVector(divsPoints[i][k], positions, v, 3);
					if (v == -1)
					{
						v = positions.size();
						positions.push_back(divsPoints[i][k]);
					}
					pConnects.push_back(v);
					numFacePoint++;

				}

				int v1 = -1;
				core.checkRepeatVector(divsPoints[i][startId_current + pow(2, powerDiff)], positions, v1, 3);
				if (v1 == -1)
				{
					v1 = positions.size();
					positions.push_back(divsPoints[i][startId_current + pow(2, powerDiff)]);
				}
				pConnects.push_back(v1);
				numFacePoint++;

				int v2 = -1;
				core.checkRepeatVector(divsPoints[i + 1][startId_next + 1], positions, v2, 3);
				if (v2 == -1)
				{
					v2 = positions.size();
					positions.push_back(divsPoints[i + 1][startId_next + 1]);
				}
				pConnects.push_back(v2);
				numFacePoint++;

				int v3 = -1;
				core.checkRepeatVector(divsPoints[i + 1][startId_next], positions, v3, 3);
				if (v3 == -1)
				{
					v3 = positions.size();
					positions.push_back(divsPoints[i + 1][startId_next]);
				}
				pConnects.push_back(v3);
				numFacePoint++;




				pCounts.push_back(numFacePoint);

				startId_current += pow(2, powerDiff);
				startId_next += 1;
			}

		}
	}


	fnMesh_out.create(positions, pCounts, pConnects);

	printf("\n newMesh %i %i %i ", fnMesh_out.numVertices(), fnMesh_out.numEdges(), fnMesh_out.numPolygons());

}
////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;

zObjMesh oMeshOut;

int startVertexID0 = 10;
int startVertexID1 = 524;

float panelWidth = 0.9;

vector<zPointArray> divsPoints;

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
	fnMesh_in.from("data/sanya/inMesh_1.json", zJSON);
	
	
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
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{
		vector<zItMeshHalfEdgeArray> v_Loops;

		computeVLoops(oMesh, startVertexID0, startVertexID1, v_Loops);

		computeNewPositions(panelWidth, v_Loops, divsPoints);

		// create new mesh
		createNewMesh(v_Loops, divsPoints, oMeshOut);

		zFnMesh fnMesh_out(oMeshOut);
		fnMesh_out.to("data/sanya/outMesh.json", zJSON);

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

	for (int i = 0; i < divsPoints.size(); i++)
	{
		for (int j = 0; j < divsPoints[i].size(); j++)
		{
			model.displayUtils.drawPoint(divsPoints[i][j], zGREEN, 5);
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
