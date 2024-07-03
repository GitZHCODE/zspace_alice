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

void computeUV_Loop(zObjMesh& o_Mesh)
{

	zFnMesh fnMesh(o_Mesh);
	fnMesh.setEdgeColor(CYAN);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);	

	zVectorArray dirArray = { zVector(-0.757969,0.652291,0), zVector(0.399888,-0.916564,0), zVector(-0.467273,-0.884113,0), zVector(-0.982567,-0.185907,0 ), zVector(0.804601,0.593816,0),zVector(0.037396,0.999301,0), zVector(0.965925,-0.258824,0)};

	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		int id = e.getId();

		zItMeshVertexArray eVerts;
		e.getVertices(eVerts);

		if (e.getColor() == CYAN && !e.onBoundary())
		{
			zPoint v0 = eVerts[0].getPosition();
			zPoint v1 = eVerts[1].getPosition();

			v0.z = v1.z = 0;

			zVector eDir_1 = v1 - v0;
			eDir_1.normalize();

			zVector eDir_2 = v0 - v1;
			eDir_2.normalize();

			for (int i = 0; i < dirArray.size(); i++)
			{
				if (eDir_1.angle(dirArray[i]) < 0.2)
				{
					eColors[e.getId()] = MAGENTA;
					break;
				}

				if (eDir_2.angle(dirArray[i]) < 0.2)
				{
					eColors[e.getId()] = MAGENTA;
					break;
				}
			}

			//float dist = v0.distanceTo(v1);
			//if (dist > 0.830 && dist < 0.860) eColors[e.getId()] = MAGENTA;
		}
	}

	fnMesh.setEdgeColors(eColors, false);

}

void computeValley(zObjMesh& o_Mesh)
{


	zFnMesh fnMesh(o_Mesh);
	fnMesh.setEdgeColor(ORANGE);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	zColor* vColors = fnMesh.getRawVertexColors();;		

	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		int id = e.getId();

		zIntArray eVerts;
		e.getVertices(eVerts);

		if (vColors[eVerts[0]] == BLUE && vColors[eVerts[1]] == BLUE) eColors[e.getId()] = GREEN;	
		if (vColors[eVerts[0]] == BLUE && vColors[eVerts[1]] == BLACK) eColors[e.getId()] = GREEN;
		if (vColors[eVerts[0]] == BLACK && vColors[eVerts[1]] == BLUE) eColors[e.getId()] = GREEN;
	}
	

	fnMesh.setEdgeColors(eColors, false);

}

void computeValley_1(zObjMesh& o_Mesh)
{


	zFnMesh fnMesh(o_Mesh);
	fnMesh.setEdgeColor(ORANGE);

	zColorArray eColors;
	fnMesh.getEdgeColors(eColors);

	zColor* vColors = fnMesh.getRawVertexColors();;

	//for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	//{
	//	int id = e.getId();

	//	zIntArray eVerts;
	//	e.getVertices(eVerts);

	//	if (vColors[eVerts[0]] == BLUE && vColors[eVerts[1]] == BLUE) eColors[e.getId()] = GREEN;
	//	if (vColors[eVerts[0]] == BLUE && vColors[eVerts[1]] == BLACK) eColors[e.getId()] = GREEN;
	//	if (vColors[eVerts[0]] == BLACK && vColors[eVerts[1]] == BLUE) eColors[e.getId()] = GREEN;
	//}

	zItMeshEdgeArray boundaryEdges;

	zBoolArray eVisited;
	eVisited.assign(fnMesh.numEdges(), false);

	for (zItMeshEdge e(o_Mesh); !e.end(); e++)
	{
		if (e.onBoundary())boundaryEdges.push_back(e);
	}


	for (int i = 0; i < boundaryEdges.size(); i++)
	{
		if (eVisited[boundaryEdges[i].getId()]) continue;

		zItMeshHalfEdge heStart, he;

		he = (boundaryEdges[i].getHalfEdge(0).onBoundary()) ? boundaryEdges[i].getHalfEdge(0) : boundaryEdges[i].getHalfEdge(1);
		heStart = he;

		bool exit1 = false;

		do
		{
			//if (he.getSym().onBoundary()) exit1 = true;
			//if(he.onBoundary())  he = he.getSym();

			zItMeshHalfEdge he2 = he.getSym().getNext();
			
			bool exit2 = false;
			
			do
			{
				if (he2.getVertex().onBoundary()) exit2 = true;

				eVisited[he2.getEdge().getId()] = true;
				eColors[he2.getEdge().getId()] = GREEN;

				he2 = he2.getNext().getSym().getNext();				

			} while (!exit2);

			eVisited[he.getEdge().getId()] = true;
			he = he.getNext();
			if(heStart == he) exit1 = true;

		} while (!exit1);
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

bool toSTAD(string path, zObjMesh& o_Mesh_Shell, zObjMesh& o_Mesh_Valley, zIntArray &supports, float &totalArea, zObjGraph& o_Graph_Stad, zVectorArray &forceVecs, zPointArray &forcePts, zDomainFloat &minMaxForce )
{
	ofstream myfile;
	myfile.open(path.c_str());

	zPointArray positions;
	zIntArray eConnects;
	zColorArray eColor;

	zFnMesh fnMeshValley(o_Mesh_Valley);
	zFnMesh fnMeshShell(o_Mesh_Shell);

	forceVecs.clear();
	forcePts.clear();
	minMaxForce = zDomainFloat(10000, 0);

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
	for (zItMeshVertex v(o_Mesh_Valley); !v.end(); v++ )
	{
		zPoint vPos = v.getPosition();		
		myfile << "\n " << (v.getId() + 1) << " " << vPos.x << " " << vPos.z << " " << vPos.y << ";";

		positions.push_back(vPos);
	}

	int currentVertexID = positions.size();
	zIntArray shellVertexMap;
	shellVertexMap.assign(fnMeshShell.numVertices(), 0);

	zBoolArray use_shellVertex; 
	use_shellVertex.assign(fnMeshShell.numVertices(), true);

	for (zItMeshVertex v(o_Mesh_Shell); !v.end(); v++)
	{
		zPoint vPos = v.getPosition();
		int v0_ID = -1;

		if (v.onBoundary())
		{			
			bool chkV0 = core.checkRepeatVector(vPos, positions, v0_ID,2);
			if (!chkV0)
			{
				printf("\n %i | %i", v.getId(), v0_ID);

				myfile << "\n " << (currentVertexID + 1) << " " << vPos.x << " " << vPos.z << " " << vPos.y << ";";

				v0_ID = positions.size();
				positions.push_back(vPos);
				currentVertexID++;
			}
			else use_shellVertex[v.getId()] = false;
		}

		else
		{
			
			myfile << "\n " << (currentVertexID + 1) << " " << vPos.x << " " << vPos.z << " " << vPos.y << ";";

			v0_ID = positions.size();
			positions.push_back(vPos);
			currentVertexID++;
		}
		
		shellVertexMap[v.getId()] = v0_ID;

		
	}

	//MEMBER INCIDENCES
	zIntArray members_U;
	zIntArray members_V;
	zIntArray members_W;
	zIntArray members_Valley_large;
	zIntArray members_Valley_small;


	myfile << "\nMEMBER INCIDENCES ";
	for (zItMeshEdge e(o_Mesh_Valley); !e.end(); e++)
	{
		int id = e.getId();

		zItMeshVertexArray eVerts;
		e.getVertices(eVerts);

		myfile << "\n " << (e.getId() + 1) << " " << (eVerts[0].getId() + 1) << " " << (eVerts[1].getId() + 1) << ";";
			
		eConnects.push_back(eVerts[0].getId());
		eConnects.push_back(eVerts[1].getId());

		if (e.getColor() == ORANGE)
		{
			members_Valley_large.push_back(id + 1);
			eColor.push_back(ORANGE);			
		}
		else if (e.getColor() == GREEN)
		{
			members_Valley_small.push_back(id + 1);
			eColor.push_back(GREEN);		

		}
		else
		{
			eColor.push_back(BLACK);
		}

	}


	// VALLEY BRACING 	
	int currentId = fnMeshValley.numEdges();

	zPointArray pts;
	fnMeshValley.getVertexPositions(pts);

	zBoolArray vertVisited;
	vertVisited.assign(fnMeshValley.numVertices(), false);

	for (zItMeshVertex v(o_Mesh_Valley); !v.end(); v++)
	{
		if (vertVisited[v.getId()]) continue;

		//printf("\n %i " , v.getId());

		if (v.getColor() == BLUE)
		{
			zItMeshHalfEdgeArray cHEdges;
			v.getConnectedHalfEdges(cHEdges);

			zItMeshHalfEdge he = (cHEdges[0].getVertex().getColor() == BLUE) ? cHEdges[1] : cHEdges[0];

			int numVerts = positions.size();
			zItMeshHalfEdge start = he;
			
			zIntArray tempIDs;

			do
			{
				if (he.getStartVertex().getColor() == BLUE)
				{
					tempIDs.push_back(he.getStartVertex().getId());
					vertVisited[he.getStartVertex().getId()] = true;
					
				}

				he = he.getNext().getSym().getNext();


			} while (he != start);

			if (tempIDs.size() == 3)
			{
				eConnects.push_back(tempIDs[0]);
				eConnects.push_back(tempIDs[1]);

				myfile << "\n " << (currentId + 1) << " " << (tempIDs[0] + 1) << " " << (tempIDs[1] + 1) << ";";
				members_W.push_back(currentId + 1);
				eColor.push_back(YELLOW);
				currentId++;

				// -------

				eConnects.push_back(tempIDs[1]);
				eConnects.push_back(tempIDs[2]);

				myfile << "\n " << (currentId + 1) << " " << (tempIDs[1] + 1) << " " << (tempIDs[2] + 1) << ";";
				members_W.push_back(currentId + 1);
				eColor.push_back(YELLOW);
				currentId++;

				// --------

				eConnects.push_back(tempIDs[2]);
				eConnects.push_back(tempIDs[0]);

				myfile << "\n " << (currentId + 1) << " " << (tempIDs[2] + 1) << " " << (tempIDs[0] + 1) << ";";
				members_W.push_back(currentId + 1);
				eColor.push_back(YELLOW);
				currentId++;
			}
		}
	}

	printf("\n total edges before shell %i ", currentId);

	// SHELL EDGES

	for (zItMeshEdge e(o_Mesh_Shell); !e.end(); e++)
	{	
		zItMeshVertexArray eVerts;
		e.getVertices(eVerts);

		if (!use_shellVertex[eVerts[0].getId()] && !use_shellVertex[eVerts[1].getId()]) continue;

		int v0 = shellVertexMap[eVerts[0].getId()];
		int v1 = shellVertexMap[eVerts[1].getId()];

		myfile << "\n " << (currentId + 1) << " " << (v0 + 1) << " " << (v1 + 1) << ";";

		eConnects.push_back(v0);
		eConnects.push_back(v1);

		if (e.getColor() == CYAN)
		{
			members_U.push_back(currentId + 1);
			eColor.push_back(CYAN);
		}
		else if (e.getColor() == MAGENTA)
		{
			members_V.push_back(currentId + 1);
			eColor.push_back(MAGENTA);

		}
		else
		{
			eColor.push_back(BLACK);
		}
		

		currentId++;
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

	
	for (int id : members_Valley_large)
	{
		myfile << " " << id ;
	}
	if (members_Valley_large.size() > 0) myfile << " TABLE ST 165.1X5.4RHS \n";

	for (int id : members_Valley_small)
	{
		myfile << " " << id;
	}
	if (members_Valley_small.size() > 0) myfile << " TABLE ST 114.3X5.4CHS \n";
	
	for (int id : members_U)
	{
		myfile << " " << id;
	}
	if (members_U.size() > 0) myfile << " TABLE ST 114.3X5.4CHS \n";

	for (int id : members_V)
	{
		myfile << " " << id;
	}
	if (members_V.size() > 0) myfile << " TABLE ST 114.3X5.4CHS \n";

	for (int id : members_W)
	{
		myfile << " " << id;
	}
	if (members_W.size() > 0) myfile << " TABLE ST 114.3X5.4CHS \n";

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
	fnMeshValley.getCenters(zFaceData, fCenters);

	zPointArray heCenters;
	fnMeshValley.getCenters(zHalfEdgeData, heCenters);

	zFloatArray vAreas;
	totalArea = fnMeshValley.getVertexAreas(fCenters, heCenters, vAreas);
	
	
	myfile << "\nJOINT LOAD ";

	//	1 2 3 4 5 6 7 8 9 10 FY - 2.5
	for (zItMeshVertex v(o_Mesh_Valley); !v.end(); v++)
	{
		int id = v.getId();
		forcePts.push_back(v.getPosition());

		if (v.getColor() == RED)
		{
			float val = -3.5 * vAreas[id];
			myfile << "\n " << (id + 1) << " FY " << val;

			forceVecs.push_back(zVector(0, 0, val));			
			minMaxForce.min = (val < minMaxForce.min) ? val : minMaxForce.min;
			minMaxForce.max = (val > minMaxForce.max) ? val : minMaxForce.max;
		}
		else if (v.getColor() == BLUE)
		{
			float val = -2.5 * vAreas[id];
			myfile << "\n " << (id + 1) << " FY " << val;

			forceVecs.push_back(zVector(0, 0, val));			
			minMaxForce.min = (val < minMaxForce.min) ? val : minMaxForce.min;
			minMaxForce.max = (val > minMaxForce.max) ? val : minMaxForce.max;
		}
		else
		{
			forceVecs.push_back(zVector(0, 0, 0));
		}
	}


	fCenters.clear();
	heCenters.clear();
	vAreas.clear();

	fnMeshShell.getCenters(zFaceData, fCenters);
	fnMeshShell.getCenters(zHalfEdgeData, heCenters);

	totalArea += fnMeshShell.getVertexAreas(fCenters, heCenters, vAreas);


	for (zItMeshVertex v(o_Mesh_Shell); !v.end(); v++)
	{	

		//if (!use_shellVertex[v.getId()]) continue;

		int id = shellVertexMap[ v.getId()];
		forcePts.push_back(v.getPosition());

		if (v.getColor() == RED)
		{
			float val = -3.5 * vAreas[v.getId()];
			myfile << "\n " << (id + 1) << " FY " << val;

			forceVecs.push_back(zVector(0, 0, val));
			minMaxForce.min = (val < minMaxForce.min) ? val : minMaxForce.min;
			minMaxForce.max = (val > minMaxForce.max) ? val : minMaxForce.max;
		}
		else if (v.getColor() == BLUE)
		{
			float val = -2.5 * vAreas[v.getId()];
			myfile << "\n " << (id + 1) << " FY " << val;

			forceVecs.push_back(zVector(0, 0, val));
			minMaxForce.min = (val < minMaxForce.min) ? val : minMaxForce.min;
			minMaxForce.max = (val > minMaxForce.max) ? val : minMaxForce.max;
		}
		else
		{
			forceVecs.push_back(zVector(0, 0, 0));
		}
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
	// MAKE GRAPH

	zFnGraph fnGraph(o_Graph_Stad);
	fnGraph.create(positions, eConnects);
	
	//zColorArray eCols_temp;
	//fnGraph.getEdgeColors(eCols_temp);

	//printf("\n edgecolor %i %i ", eCols_temp.size(), eColor.size());
	
	fnGraph.setEdgeColors(eColor,false);

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
bool displayMESH = true;
bool displayGRAPH = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh_shell;
zObjMesh oMesh_valley;

zObjGraph oGraph_stad;
zIntArray supports;

zVectorArray forceVecs;
zPointArray forcePts;
zDomainFloat minMaxForces;

zDomainFloat mappedDomain(0.1, 1);

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
	zFnMesh fnMesh_shell(oMesh_shell);
	fnMesh_shell.from("data/toSTAD/toStad_040623_shell.json", zJSON);

	
	zFnMesh fnMesh_valley(oMesh_valley);
	fnMesh_valley.from("data/toSTAD/toStad_040623_valley_3.json", zJSON);

	//  supports
	//supports = zIntArray{ 2,3,5,9,13,15,17,21,23,25,29,31,33,37,39,41,45,47,49,53,55,896,899,900,902,903,905,906,907,909,911,912,914,915,916 };
	int redCounter = 0;
	int blueCounter = 0;
	int greenCounter = 0;
	for (zItMeshVertex v(oMesh_valley); !v.end(); v++)
	{
		if (v.onBoundary())
		{
			supports.push_back(v.getId());
			v.setColor(BLACK);			
		}	

		if (v.getColor() == RED)redCounter++;
		if (v.getColor() == BLUE)blueCounter++;
	}
	
	printf("\n num supports %i ", supports.size());
	printf("\n red %i blue %i green %i ", redCounter, blueCounter, greenCounter);
	

	computeUV_Loop(oMesh_shell);
	//computeValley(oMesh_valley);

	computeValley_1(oMesh_valley);
	// --- 
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh_shell);
	model.addObject(oMesh_valley);
	model.addObject(oGraph_stad);
	
	// set display element booleans
	oMesh_shell.setDisplayElements(true, true, false);
	oMesh_valley.setDisplayElements(true, true, false);

	oGraph_stad.setDisplayElements(true, true);
	
	
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

	B.addButton(&displayMESH, "displayMESH");
	B.buttons[2].attachToVariable(&displayMESH);

	B.addButton(&displayGRAPH, "displayGRAPH");
	B.buttons[3].attachToVariable(&displayGRAPH);
	

}

void update(int value)
{
	oMesh_valley.setDisplayObject(displayMESH);
	oMesh_shell.setDisplayObject(displayMESH);

	oGraph_stad.setDisplayObject(displayGRAPH);

	if (compute)
	{
		float totalArea;
		toSTAD("data/toSTAD/outTest.std", oMesh_shell, oMesh_valley, supports, totalArea, oGraph_stad, forceVecs, forcePts, minMaxForces);
		printf("\n totalArea: %1.3f ", totalArea);

		zFnGraph fnGraph(oGraph_stad);
		fnGraph.to("data/toSTAD/stadGraph.json", zJSON);

		compute = !compute;	
	}

	if (importSTAD)
	{
		fromSTAD("data/toSTAD/STAD_in_3005.txt", oMesh_shell);

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
		int i = 0;
		for (auto& f : forceVecs)
		{
			float len = f.length();
			zVector tmp = f * -1;;
			tmp.normalize();

			float mLen = core.ofMap(len, minMaxForces, mappedDomain);
			tmp *= mLen;

			zPoint p0 = forcePts[i] + tmp;
			model.displayUtils.drawLine(p0, forcePts[i], GREEN, 0.5);
			model.displayUtils.drawPoint(forcePts[i], GREEN, 3);
			i++;
		}
	}

	model.draw();

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
