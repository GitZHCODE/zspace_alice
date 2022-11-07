//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


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

void computeValley(zObjMesh& o_Mesh)
{


	zFnMesh fnMesh(o_Mesh);
	//fnMesh.setEdgeColor(CYAN);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		zItMeshVertexArray eVerts;
		e.getVertices(eVerts);
		if (eVerts[0].getColor() == RED && eVerts[1].getColor() == RED) eColors[e.getId()] = ORANGE;
		if (eVerts[0].getColor() == RED && eVerts[1].getColor() == BLACK) eColors[e.getId()] = ORANGE;
		if (eVerts[0].getColor() == BLACK && eVerts[1].getColor() == RED) eColors[e.getId()] = ORANGE;
		if (eVerts[0].getColor() == BLACK && eVerts[1].getColor() == BLACK) eColors[e.getId()] = ORANGE;

		if (eVerts[0].getColor() == GREEN && eVerts[1].getColor() == GREEN) eColors[e.getId()] = YELLOW;
		if (eVerts[0].getColor() == GREEN && eVerts[1].getColor() == BLACK) eColors[e.getId()] = YELLOW;
		if (eVerts[0].getColor() == BLACK && eVerts[1].getColor() == GREEN) eColors[e.getId()] = YELLOW;

	}

	fnMesh.setEdgeColors(eColors, false);

}

void computeDir(zObjMesh& o_Mesh, zVector& dir, zColor inCol)
{
	zFnMesh fnMesh(o_Mesh);
	//fnMesh.setEdgeColor(CYAN);

	zPoint* vPositions = fnMesh.getRawVertexPositions();

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		zItMeshVertexArray eVerts;
		e.getVertices(eVerts);

		if (eVerts[0].getColor() == RED && eVerts[1].getColor() == RED) continue;
		if (eVerts[0].getColor() == RED && eVerts[1].getColor() == BLACK) continue;
		if (eVerts[0].getColor() == BLACK && eVerts[1].getColor() == RED) continue;
		if (eVerts[0].getColor() == BLACK && eVerts[1].getColor() == BLACK) continue;
		
		zPoint v0 = vPositions[eVerts[0].getId()];
		zPoint v1 = vPositions[eVerts[1].getId()];

		v0.z = 0;
		v1.z = 0;

		zVector projects_eDir = v1 - v0;
		projects_eDir.normalize();

		if (projects_eDir * dir >= 0.9 || projects_eDir * dir <= -0.9)
		{
			eColors[e.getId()] = inCol;
		}

	}

	fnMesh.setEdgeColors(eColors, false);
}

void computeUV_Shell(zObjMesh& o_Mesh, int start_eV0, int start_eV1)
{


	zFnMesh fnMesh(o_Mesh);
	//fnMesh.setEdgeColor(CYAN);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	//printf("\n % i ", eColors.size());

	zItMeshHalfEdge U_startHE;
	bool chk = fnMesh.halfEdgeExists(start_eV0, start_eV1, U_startHE);

	if (chk)
	{
		printf("\n %i %i ", U_startHE.getStartVertex().getId(), U_startHE.getVertex().getId());

		bool exitU = false;

		zItMeshHalfEdge U_currentHE = U_startHE;
		U_currentHE = U_currentHE.getNext().getSym().getNext();

		do
		{
			zItMeshHalfEdge V_startHE = U_currentHE.getPrev();
			zItMeshHalfEdge V_currentHE = V_startHE;
			bool exitV = false;

			do
			{
				//printf("\n %i ", V_currentHE.getEdge().getId());
				eColors[V_currentHE.getEdge().getId()] = MAGENTA;

				if (V_currentHE.getVertex().checkValency(3))
				{
					zVector curV = V_currentHE.getVector();
					zVector nextV = V_currentHE.getNext().getVector();

					if(curV * nextV > 0.8) 	V_currentHE = V_currentHE.getNext();
					else  V_currentHE = V_currentHE.getNext().getSym().getNext();
				}
				else V_currentHE = V_currentHE.getNext().getSym().getNext();

				if (V_currentHE == V_startHE) exitV = true;

				if (V_currentHE.getStartVertex().getColor() == RED) exitV = true;				

			} while (!exitV);


			V_currentHE = U_currentHE.getSym().getNext();;
			exitV = false;
			do
			{
				eColors[V_currentHE.getEdge().getId()] = MAGENTA;


				if (V_currentHE.getStartVertex().checkValency(3))
				{
					zVector curV = V_currentHE.getVector();
					zVector prevV = V_currentHE.getPrev().getVector();

					if (curV * prevV > 0.8) 	V_currentHE = V_currentHE.getPrev();
					else  V_currentHE = V_currentHE.getPrev().getSym().getPrev();
					
				}
				else V_currentHE = V_currentHE.getPrev().getSym().getPrev();				

				if (V_currentHE.getVertex().getColor() == RED) exitV = true;

			} while (!exitV);


			if (U_currentHE.getVertex().onBoundary()) exitU = true;



			if (!exitU)
			{
				U_currentHE = U_currentHE.getNext().getSym().getNext();
			}

			if (U_currentHE.getStartVertex().getColor() == RED)
			{				
				exitU = true;
			}

		} while (!exitU);
	}

	fnMesh.setEdgeColors(eColors, false);

}

void computeUV_Funnel(zObjMesh& o_Mesh, int start_eV0 , int start_eV1)
{
	

	zFnMesh fnMesh(o_Mesh);
	//fnMesh.setEdgeColor(CYAN);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	//printf("\n % i ", eColors.size());

	zItMeshHalfEdge U_startHE;
	bool chk = fnMesh.halfEdgeExists(start_eV0, start_eV1, U_startHE);

	if (chk)
	{
		printf("\n %i %i ", U_startHE.getStartVertex().getId(), U_startHE.getVertex().getId());

		bool exitU = false;

		zItMeshHalfEdge U_currentHE = U_startHE;
		U_currentHE = U_currentHE.getNext().getSym().getNext();

		

		do
		{
			zItMeshHalfEdge V_startHE = U_currentHE.getPrev();
			zItMeshHalfEdge V_currentHE = V_startHE;
			bool exitV = false;

			int redCounter = 0;
			do
			{
				//printf("\n %i ", V_currentHE.getEdge().getId());
				if(redCounter == 3 || redCounter == 4 ) eColors[V_currentHE.getEdge().getId()] = BLUE;
				else eColors[V_currentHE.getEdge().getId()] = MAGENTA;

				V_currentHE = V_currentHE.getNext().getSym().getNext();

				if (V_currentHE == V_startHE) exitV = true;

				if (V_currentHE.getStartVertex().getColor() == RED || V_currentHE.getStartVertex().getColor() == GREEN)
				{
					V_currentHE = V_currentHE.getNext().getSym().getNext();
					V_currentHE = V_currentHE.getNext().getSym().getNext();

					redCounter++;
					
				}
				
			} while (!exitV);

			//printf("\n redCounter %i ", redCounter);
			
			if(U_currentHE.getVertex().onBoundary()) exitU = true;
			
			

			if (!exitU)
			{
				U_currentHE = U_currentHE.getNext().getSym().getNext();
			}

			if (U_currentHE.getStartVertex().getColor() == RED)
			{
				//U_currentHE = U_currentHE.getNext().getSym().getNext();
				//U_currentHE = U_currentHE.getNext().getSym().getNext();
				exitU = true;
			}

		} while (!exitU);
	}

	fnMesh.setEdgeColors(eColors,false);

}

bool toSTAD(string path, zObjMesh& o_Mesh, zObjMesh& o_Mesh_cap, zIntArray &supports, float &totalArea)
{
	ofstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	//INPUT WIDTH 79
	int width = 79;
	myfile << "\nINPUT WIDTH " << width;
	
	//UNIT METER KN
	myfile << "\nUNIT METER KN ";
	
	//JOINT COORDINATES
	myfile << "\nJOINT COORDINATES ";
	
	// vertex positions
	// 1 -0.771834 0 1.26659;
	for (zItMeshVertex v(o_Mesh); !v.end(); v++ )
	{
		zPoint vPos = v.getPosition();		
		myfile << "\n " << (v.getId() + 1) << " " << vPos.x << " " << vPos.z << " " << vPos.y << ";";
	}

	//MEMBER INCIDENCES
	zIntArray members_U;
	zIntArray members_V;
	zIntArray members_W;
	zIntArray members_Valley;
	zIntArray members_Cap;


	myfile << "\nMEMBER INCIDENCES ";
	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		int id = e.getId();

		zItMeshVertexArray eVerts;
		e.getVertices(eVerts);

		myfile << "\n " << (e.getId() + 1) << " " << (eVerts[0].getId() + 1) << " " << (eVerts[1].getId() + 1) << ";";
			

		if(e.getColor() == ORANGE) members_Valley.push_back(id + 1);
		else if (e.getColor() == CYAN) members_V.push_back(id + 1);
		else if (e.getColor() == YELLOW) members_U.push_back(id + 1);
		else if (e.getColor() == GREEN) members_W.push_back(id + 1);

	}

	zFnMesh fnMesh(o_Mesh);
	int currentId = fnMesh.numEdges();

	zPointArray pts;
	fnMesh.getVertexPositions(pts);

	for (zItMeshEdge e(o_Mesh_cap); !e.end(); e++)
	{
		bool addEdge = false;
		if (e.onBoundary())
		{
			zItMeshVertexArray eVerts;
			e.getVertices(eVerts);

			if (eVerts[0].checkValency(2) && eVerts[1].checkValency(2))  addEdge = true;
		}
		else addEdge = true;

		if(addEdge)
		{
			int id = currentId;

			zItMeshVertexArray eVerts;
			e.getVertices(eVerts);

			int v0_ID, v1_ID;
			bool chkV0 = core.checkRepeatVector(eVerts[0].getPosition(), pts, v0_ID);
			bool chkV1 = core.checkRepeatVector(eVerts[1].getPosition(), pts, v1_ID);

			//zItMeshVertex v0;
			//bool chkV0 = fnMesh.vertexExists(eVerts[0].getPosition(), v0);

			//zItMeshVertex v1;
			//bool chkV1 = fnMesh.vertexExists(eVerts[1].getPosition(), v1);

			if (chkV0 && chkV1)
			{
				myfile << "\n " << (id + 1) << " " << (v0_ID + 1) << " " << (v1_ID + 1) << ";";

				members_Cap.push_back(id + 1);

				currentId++;
			}
			else
			{
				printf("\n %i %i ", eVerts[0].getId(), eVerts[1].getId());
			}

		}
		
	}

	printf("\n total edges %i ", currentId);

	//DEFINE MATERIAL START
	myfile << "\nDEFINE MATERIAL START ";

	//ISOTROPIC STEEL
	myfile << "\nISOTROPIC STEEL ";
	
	//	E 2.05e+008
	myfile << "\nE " << 2.05e+008;

	//	POISSON 0.3
	myfile << "\nPOISSON " << 0.3;

	//DENSITY 76.8195
	myfile << "\nDENSITY " << 76.8195;

	//ALPHA 1.2e-005
	myfile << "\nALPHA " << 1.2e-005;
	
	//DAMP 0.03
	myfile << "\nDAMP " << 0.03;

	//	TYPE STEEL
	myfile << "\nTYPE STEEL ";

	//STRENGTH FY 253200 FU 407800 RY 1.5 RT 1.2
	myfile << "\nSTRENGTH FY " << 253200 << " FU " << 407800 << " RY " << 1.5 << " RT " << 1.2;
	
	//ISOTROPIC CONCRETE
	myfile << "\nISOTROPIC CONCRETE ";
	
	//E 2.17185e+007
	myfile << "\nE " << 2.17185e+007;

	//POISSON 0.17
	myfile << "\nPOISSON " << 0.17;

	//DENSITY 23.5616
	myfile << "\nDENSITY " << 23.5616;

	//ALPHA 1e-005
	myfile << "\nALPHA " << 1e-005;

	//DAMP 0.05
	myfile << "\nDAMP " << 0.05;

	//TYPE CONCRETE
	myfile << "\nTYPE CONCRETE ";

	//STRENGTH FCU 27579
	myfile << "\nSTRENGTH FCU "<< 27579;
	
	//END DEFINE MATERIAL
	myfile << "\nEND DEFINE MATERIAL ";

	// MEMBER PROPERTY TATASTRUCTURA
	myfile << "\nMEMBER PROPERTY TATASTRUCTURA \n";

	
	for (int id : members_Valley)
	{
		myfile << " " << id ;
	}
	myfile << " TABLE ST 165.1X5.4RHS \n";

	for (int id : members_U)
	{
		myfile << " " << id;
	}
	myfile << " TABLE ST 114.3X5.4CHS \n";
	
	for (int id : members_V)
	{
		myfile << " " << id;
	}
	myfile << " TABLE ST 76.1X4.5CHS \n";

	for (int id : members_W)
	{
		myfile << " " << id;
	}
	myfile << " TABLE ST 114.3X5.4CHS \n";

	for (int id : members_Cap)
	{
		myfile << " " << id;
	}
	myfile << " TABLE ST 114.3X5.4CHS \n";

	

	//CONSTANTS
	myfile << "\nCONSTANTS ";

	//MATERIAL STEEL ALL
	myfile << "\nMATERIAL STEEL ALL ";

	//SUPPORTS
	myfile << "\nSUPPORTS ";
	myfile << "\n ";

	for (int sID : supports)
	{
		myfile << (sID + 1) << " ";
	}
	myfile << "PINNED ";

	//LOAD 1 LOADTYPE None  TITLE LOAD CASE 1
	myfile << "\nLOAD " << 1 << " LOADTYPE None  TITLE LOAD CASE  " << 1;

	//SELFWEIGHT Y -1.5 
	myfile << "\nSELFWEIGHT Y " << -1.5;

	//JOINT LOAD
		
	zPointArray fCenters;
	fnMesh.getCenters(zFaceData, fCenters);

	zPointArray heCenters;
	fnMesh.getCenters(zHalfEdgeData, heCenters);

	zFloatArray vAreas;
	totalArea = fnMesh.getVertexAreas(fCenters, heCenters, vAreas);
	
	
	myfile << "\nJOINT LOAD ";

	//	1 2 3 4 5 6 7 8 9 10 FY - 2.5
	for (zItMeshVertex v(o_Mesh); !v.end(); v++)
	{
		int id = v.getId();
		if (v.getColor() == RED) myfile << "\n " << (id + 1) << " FY " << -3.5 * vAreas[id];
		if (v.getColor() == BLUE)myfile << "\n " << (id + 1) << " FY " << -2.5 * vAreas[id];
	}
	
	//PERFORM ANALYSIS
	myfile << "\nPERFORM ANALYSIS ";
	
	//PARAMETER 1
	myfile << "\nPARAMETER " << 1;
	
	//CODE IS800 LSD
	myfile << "\nCODE IS800 LSD " ;
	
	//BEAM 1 ALL
	myfile << "\nBEAM 1 ALL ";

	//FYLD 250000 ALL
	myfile << "\nFYLD 250000 ALL ";
	
	//CHECK CODE ALL
	myfile << "\nCHECK CODE ALL ";

	//FINISH
	myfile << "\nFINISH ";

	myfile.close();
	cout << endl << "STAD exported. File:   " << path.c_str() << endl;
}


bool fromSTAD(string path, zObjMesh& o_Mesh)
{
	ifstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	zFnMesh fnMesh(o_Mesh);
	fnMesh.setEdgeColor(zColor());

	int numE = fnMesh.numEdges();

	int countExist = 0;
	int countNonExist = 0;
	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		vector<string> perlineData = core.splitString(str, " ");

		if (perlineData.size() > 0)
		{
			int id = atoi(perlineData[0].c_str()) - 1;
			
			if (id < numE && id >= 0)
			{
				zItMeshEdge e(o_Mesh, id);
				e.setColor(RED);

				//printf("\n %1.2f ", e.getLength());
				countExist++;
			}
			else  countNonExist++;

			//printf("\n %i ", atoi(perlineData[0].c_str()));
			
		}
	}

	printf("\n  countExist %i | countNonExist %i ", countExist, countNonExist);

	return true;
}
////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool importSTAD = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;
zObjMesh oMesh_cap;
zIntArray supports;


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
	fnMesh.from("data/toSTAD/toStad_0507.json", zJSON);

	
	zFnMesh fnMesh_cap(oMesh_cap);
	fnMesh_cap.from("data/toSTAD/toStad_0507_Cap.obj", zOBJ);

	//  supports
	//supports = zIntArray{ 2,3,5,9,13,15,17,21,23,25,29,31,33,37,39,41,45,47,49,53,55,896,899,900,902,903,905,906,907,909,911,912,914,915,916 };
	int redCounter = 0;
	int blueCounter = 0;
	int greenCounter = 0;
	for (zItMeshVertex v(oMesh); !v.end(); v++)
	{
		if (v.getColor() == BLACK)
		{
			supports.push_back(v.getId());
		}

		if (v.getColor() == RED)redCounter++;
		if (v.getColor() == BLUE)blueCounter++;
		if (v.getColor() == GREEN)greenCounter++;
	}
	
	printf("\n num supports %i ", supports.size());
	printf("\n red %i blue %i green %i ", redCounter, blueCounter, greenCounter);

	fnMesh.setEdgeColor(CYAN, false);

	computeValley(oMesh);

	zVector dir_U(-0.666292, 0.745691, 0);
	computeDir(oMesh, dir_U, YELLOW);

	zVector dir_W(0.037396, 0.999301, 0);
	computeDir(oMesh, dir_W, GREEN);


	
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_cap);
	
	// set display element booleans
	oMesh.setDisplayElements(true, true, false);
	oMesh_cap.setDisplayElements(true, true, false);
	
	
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
		float totalArea;
		toSTAD("data/toSTAD/outTest.std", oMesh, oMesh_cap, supports, totalArea);

		printf("\n totalArea: %1.3f ", totalArea);

		compute = !compute;	
	}

	if (importSTAD)
	{
		fromSTAD("data/toSTAD/STAD_in_3005.txt", oMesh);

		importSTAD = !importSTAD;
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
		model.draw();
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'i') importSTAD = true;
	

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
