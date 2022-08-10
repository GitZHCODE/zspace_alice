//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

zColor MAGENTA(1, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor YELLOW(1, 1, 0, 1);
zColor ORANGE(1, 0.5, 0, 1);

bool compute = false;
bool exportMesh = false;
bool display = true;

bool compGRAD = false;

double background = 0.35;
double panelWidth = 5;
double structureDepth = 0.5;
double structureWidth = 0.1;
int startV;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjGraphArray oGraph_loops;
zObjGraphArray oGraph_structures;
zObjMesh oMesh_pattern;
zObjMesh oMesh_rib;

double p0Z = 0;
double p1Z = 10;

zPoint control_p0;
zPoint control_p1;

double minDivs = 4;
double maxDivs = 128;


bool readStartV(string path, int& startV)
{
	ifstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	

	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		vector<string> perlineData = core.splitString(str, " ");

		if (perlineData.size() > 0)
		{
			// Translation
			if (perlineData[0] == "startV")
			{
				startV = atoi(perlineData[1].c_str());
			}
			

		}


	}

	myfile.close();
	

}

int computeNumLoops(zObjMesh& oMesh, int startV, zItMeshHalfEdge& startHe)
{ 
	zItMeshVertex sV(oMesh, startV);	
	//zItMeshHalfEdge startHe;
	zItMeshHalfEdgeArray cHEdges;
	sV.getConnectedHalfEdges(cHEdges);

	zVector up(0, 0, 1);
	float minAngle = 360;

	for (auto& he : cHEdges)
	{
		zVector heVec = he.getVector();
		heVec.normalize();
		if (heVec.angle(up) < minAngle)
		{
			startHe = he;
			minAngle = heVec.angle(up);
		}
		
	}

	zItMeshHalfEdge currentHe = startHe;
	bool exit = false;
	int numLoops = 1;
	do
	{
		if (currentHe.getVertex().onBoundary()) exit = true;
		if (!exit) currentHe = currentHe.getNext().getSym().getNext();
		
		numLoops++;

	} while (!exit);

	return numLoops;
}

double getLoopLength(zObjMesh& oMesh, zItMeshHalfEdge& startHe)
{

	
	zItMeshHalfEdge he = startHe;
	double eLength = 0.0;

	bool exit = false;
	do
	{
		eLength += he.getLength();
		he = he.getNext().getSym().getNext();

		//printf("\n %i %i ", he.getId(), startHe.getId());

		if (he == startHe) exit = true;

	} while (!exit);

	return eLength;
}

void computeLoopGraphs(zObjMesh& oMesh, zObjGraphArray &oGraphArray, int startV, double _pWidth, int maxDivs)
{
	
	zItMeshHalfEdge startHe;
	int nLoops = computeNumLoops(oMesh, startV,  startHe);

	printf("\n numLoops %i ", nLoops);

	oGraphArray.clear();
	oGraphArray.assign(nLoops - 1, zObjGraph());

	zItMeshHalfEdge currentHe = startHe;
	bool exitU = false;
	
	int numGraphs = 0;

	do
	{
		zItMeshHalfEdge startHE_V = currentHe.getSym().getNext();
		zItMeshHalfEdge currenHE_V = startHE_V;

		double loopLen = getLoopLength(oMesh, startHE_V);
		printf("\n loopLength %1.2f ", loopLen);
		int numDivs = loopLen / _pWidth;

		// V LOOP
		int tempNumDivs = 0;

		for (int i = 1; i < 10; i++)
		{
			if (numDivs >= pow(2, i)) tempNumDivs = pow(2, i);

			if (numDivs < pow(2, i))
			{
				if (tempNumDivs == 0) numDivs = 2;
				else numDivs = pow(2, i)/*tempNumDivs*/;
				break;
			}
		}

		if (numDivs > maxDivs) maxDivs = numDivs;

		float actualpWidth = (loopLen / (numDivs));
		int totaltInteriorVerts = numDivs;

		printf("\n numDivs %i, actualpWidth %1.2f ", numDivs, actualpWidth);

		zPoint pOnCurve = currenHE_V.getStartVertex().getPosition();;
		bool exitV = false;

		// compute Graph point
		zPointArray positions;
		zIntArray eConnects;
		int numVerts = 0;

		positions.push_back(pOnCurve);
		numVerts++;


		do
		{
			zPoint eEndPoint = currenHE_V.getVertex().getPosition();

			float distance_increment = actualpWidth;
			while (pOnCurve.distanceTo(eEndPoint) < distance_increment)
			{
				distance_increment = distance_increment - pOnCurve.distanceTo(eEndPoint);
				pOnCurve = eEndPoint;

				currenHE_V = (currenHE_V.onBoundary()) ? currenHE_V.getNext() : currenHE_V.getNext().getSym().getNext();
				eEndPoint = currenHE_V.getVertex().getPosition();
			}

			zVector he_vec = currenHE_V.getVector();
			he_vec.normalize();

			//O
			zPoint O = pOnCurve + he_vec * distance_increment;
			pOnCurve = O;
			positions.push_back(pOnCurve);
			numVerts++;

			if (positions.size() > 1)
			{
				eConnects.push_back(positions.size() - 2);
				eConnects.push_back(positions.size() - 1);
			}

			if (numVerts == totaltInteriorVerts) exitV = true;
			//if (currenHE_V == startHE_V) exitV = true;

			if (exitV)
			{
				eConnects.push_back(positions.size() - 1);
				eConnects.push_back(0);
			}

		} while (!exitV);

		// create GRAPH
		
		zFnGraph fnGraph(oGraphArray[numGraphs]);
		fnGraph.create(positions, eConnects);
		fnGraph.setEdgeColor(MAGENTA);
		//fnGraph.setEdgeWeight(3);

		printf("\n %i | %i %i ", numGraphs, fnGraph.numVertices(), fnGraph.numEdges());
		numGraphs++;


		//EXIT U
		if (currentHe.getVertex().onBoundary()) exitU = true;
		if (!exitU) currentHe = currentHe.getNext().getSym().getNext();		

	} while (!exitU);

	

}

void computeLoopGraphs(zObjMesh& oMesh, zObjGraphArray& oGraphArray, int startV, zPoint &p0, zPoint&p1, float minDivs, float maxDivs)
{

	zItMeshHalfEdge startHe;
	int nLoops = computeNumLoops(oMesh, startV, startHe);

	printf("\n numLoops %i ", nLoops);

	oGraphArray.clear();
	oGraphArray.assign(nLoops - 1, zObjGraph());

	zItMeshHalfEdge currentHe = startHe;
	bool exitU = false;

	int numGraphs = 0;

	float maxDistance = p0.distanceTo(p1);
	float minDistance = 0;

	bool compMinDistance = true;
	do
	{
		zItMeshHalfEdge startHE_V = currentHe.getSym().getNext();
		zItMeshHalfEdge currenHE_V = startHE_V;

		zPoint pt = startHE_V.getStartVertex().getPosition();

		if (compMinDistance)
		{			
			minDistance = p0.distanceTo(pt);
		}
		
		float dist = p0.distanceTo(pt);;

		double loopLen = getLoopLength(oMesh, startHE_V);
		//printf("\n loopLength %1.2f ", loopLen);
		int numDivs = core.ofMap(dist, minDistance, maxDistance, minDivs,  maxDivs);

		if (numDivs < minDivs) numDivs = minDivs;

		// V LOOP
		int tempNumDivs = 0;

		for (int i = 1; i < 10; i++)
		{
			if (numDivs >= pow(2, i)) tempNumDivs = pow(2, i);

			if (numDivs < pow(2, i))
			{
				if (tempNumDivs == 0) numDivs = 2;
				else numDivs = pow(2, i)/*tempNumDivs*/;
				break;
			}
		}

		if (numDivs > maxDivs) maxDivs = numDivs;

		float actualpWidth = (loopLen / (numDivs));
		int totaltInteriorVerts = numDivs;

		printf("\n numDivs %i, actualpWidth %1.2f ", numDivs, actualpWidth);

		zPoint pOnCurve = currenHE_V.getStartVertex().getPosition();;
		bool exitV = false;

		// compute Graph point
		zPointArray positions;
		zIntArray eConnects;
		int numVerts = 0;

		positions.push_back(pOnCurve);
		numVerts++;


		do
		{
			zPoint eEndPoint = currenHE_V.getVertex().getPosition();

			float distance_increment = actualpWidth;
			while (pOnCurve.distanceTo(eEndPoint) < distance_increment)
			{
				distance_increment = distance_increment - pOnCurve.distanceTo(eEndPoint);
				pOnCurve = eEndPoint;

				currenHE_V = (currenHE_V.onBoundary()) ? currenHE_V.getNext() : currenHE_V.getNext().getSym().getNext();
				eEndPoint = currenHE_V.getVertex().getPosition();
			}

			zVector he_vec = currenHE_V.getVector();
			he_vec.normalize();

			//O
			zPoint O = pOnCurve + he_vec * distance_increment;
			pOnCurve = O;
			positions.push_back(pOnCurve);
			numVerts++;

			if (positions.size() > 1)
			{
				eConnects.push_back(positions.size() - 2);
				eConnects.push_back(positions.size() - 1);
			}

			if (numVerts == totaltInteriorVerts) exitV = true;
			//if (currenHE_V == startHE_V) exitV = true;

			if (exitV)
			{
				eConnects.push_back(positions.size() - 1);
				eConnects.push_back(0);
			}

		} while (!exitV);

		// create GRAPH

		zFnGraph fnGraph(oGraphArray[numGraphs]);
		fnGraph.create(positions, eConnects);
		fnGraph.setEdgeColor(MAGENTA);
		//fnGraph.setEdgeWeight(3);

		printf("\n %i | %i %i ", numGraphs, fnGraph.numVertices(), fnGraph.numEdges());
		numGraphs++;


		//EXIT U
		if (currentHe.getVertex().onBoundary()) exitU = true;
		if (!exitU) currentHe = currentHe.getNext().getSym().getNext();

	} while (!exitU);



}

void computeGuideMesh(zObjGraphArray& oGraphArray,  zIntArray &startVperLoop, zObjMesh& outMesh)
{
	zPointArray positions;
	zIntArray polyCounts, polyConnects;

	startVperLoop.clear();
	startVperLoop.assign(oGraphArray.size(),0);

	
	for (int i =0; i< oGraphArray.size(); i++)
	{
		zFnGraph fnGraph_current(oGraphArray[i]);

		if (fnGraph_current.numVertices() == 0) continue;

		startVperLoop[i] = positions.size();
		zPoint* vPositions = fnGraph_current.getRawVertexPositions();

		//Positions
		for (int i = 0; i < fnGraph_current.numVertices(); i++)
		{
			positions.push_back(vPositions[i]);
		}

		//Polyconnects & counts
		if (i > 0)
		{
			int sVertCurrentGraph = startVperLoop[i];
			int sVertPrevGraph = startVperLoop[i - 1];

			zFnGraph fnGraph_prev(oGraphArray[i - 1]);

			int numVertsCurrentGraph = fnGraph_current.numVertices();
			int numVertsPrevGraph = fnGraph_prev.numVertices();

			int numDivsCurrentGraph = numVertsCurrentGraph;
			//if (!loopGraph[i]) numDivsCurrentGraph -= 1;

			int numDivsPrevGraph = numVertsPrevGraph;
			//if (!loopGraph[i - 1]) numDivsPrevGraph -= 1;

			if (numVertsCurrentGraph == numVertsPrevGraph)
			{
				for (int j = 0; j < numVertsPrevGraph; j++)
				{
					polyConnects.push_back(sVertPrevGraph + j);
					polyConnects.push_back(sVertPrevGraph + ((j + 1) % numVertsPrevGraph));
					polyConnects.push_back(sVertCurrentGraph + ((j + 1) % numVertsPrevGraph));
					polyConnects.push_back(sVertCurrentGraph + j);

					polyCounts.push_back(4);
				}
			}
			else
			{
				if (numDivsPrevGraph < numDivsCurrentGraph)
				{
					int factor = floor((numDivsCurrentGraph / numDivsPrevGraph));

					for (int j = 0; j < numVertsCurrentGraph; j += factor)
					{
						int prevGraphVert = floor(j / factor);

						if (j != numVertsCurrentGraph - 1)
						{
							for (int k = 0; k < floor(factor * 0.5); k++)
							{
								int vId_0 = sVertPrevGraph + prevGraphVert;
								int vId_1 = sVertCurrentGraph + j + k;
								int vId_2 = sVertCurrentGraph + ((j + k + 1) % numVertsCurrentGraph);

								polyConnects.push_back(vId_2);
								polyConnects.push_back(vId_1);
								polyConnects.push_back(vId_0);
								polyCounts.push_back(3);

								//printf("\n A1 %i , %i , %i ", vId_2, vId_1, vId_0);
							}
						}

						if (j != 0)
						{
							for (int k = 0; k < floor(factor * 0.5); k++)
							{
								int vId_0 = sVertPrevGraph + prevGraphVert;
								int vId_1 = sVertCurrentGraph + ((j + k - 1 + numVertsCurrentGraph) % numVertsCurrentGraph);
								int vId_2 = sVertCurrentGraph + j + k;

								polyConnects.push_back(vId_2);
								polyConnects.push_back(vId_1);
								polyConnects.push_back(vId_0);

								polyCounts.push_back(3);

								//printf("\n A2 %i , %i , %i ", vId_2, vId_1, vId_0);
							}
						}

						if (j != numVertsCurrentGraph - 1)
						{
							int fac = floor(factor * 0.5);

							int vId_0 = sVertPrevGraph + prevGraphVert;
							int vId_1 = sVertCurrentGraph + j + fac;
							int vId_2 = sVertPrevGraph + ((prevGraphVert + 1) % numVertsPrevGraph);

							polyConnects.push_back(vId_2);
							polyConnects.push_back(vId_1);
							polyConnects.push_back(vId_0);
							polyCounts.push_back(3);

							//printf("\n A2 %i , %i , %i ", vId_2, vId_1, vId_0);
						}
					}

					//printf("\n A2 %i , %i , %i ", sVertCurrentGraph + numVertsCurrentGraph - 1, sVertCurrentGraph, sVertPrevGraph);

					polyConnects.push_back(sVertCurrentGraph + numVertsCurrentGraph - 1);					
					polyConnects.push_back(sVertPrevGraph);
					polyConnects.push_back(sVertCurrentGraph);
					polyCounts.push_back(3);

				}

				if (numDivsPrevGraph > numDivsCurrentGraph)
				{
					int factor = floor((numDivsPrevGraph / numDivsCurrentGraph));

					for (int j = 0; j < numVertsPrevGraph; j += factor)
					{
						int currentGraphVert = floor(j / factor);

						if (j != numVertsPrevGraph - 1)
						{
							for (int k = 0; k < floor(factor * 0.5); k++)
							{

								int vId_0 = sVertPrevGraph + j + k;
								int vId_1 = sVertPrevGraph + ((j + k + 1) % numVertsPrevGraph);
								int vId_2 = sVertCurrentGraph + currentGraphVert;

								polyConnects.push_back(vId_0);
								polyConnects.push_back(vId_1);
								polyConnects.push_back(vId_2);

								polyCounts.push_back(3);

								//printf("\n B1 %i , %i , %i ", vId_0, vId_1, vId_2);
							}
						}

						if (j != 0)
						{
							for (int k = 0; k < floor(factor * 0.5); k++)
							{
								int vId_0 = sVertCurrentGraph + currentGraphVert;
								int vId_1 = sVertPrevGraph + ((j + k - 1 + numVertsPrevGraph) % numVertsPrevGraph);
								int vId_2 = sVertPrevGraph + j + k;

								polyConnects.push_back(vId_0);
								polyConnects.push_back(vId_1);
								polyConnects.push_back(vId_2);

								polyCounts.push_back(3);

								//printf("\n B2 %i , %i , %i ", vId_0, vId_1, vId_2);
							}
						}

						if (j != numVertsPrevGraph - 1)
						{
							int fac = floor(factor * 0.5);

							int vId_0 = sVertCurrentGraph + currentGraphVert;
							int vId_1 = sVertPrevGraph + j + fac;
							int vId_2 = sVertCurrentGraph + ((currentGraphVert + 1) % numVertsCurrentGraph);

							polyConnects.push_back(vId_0);
							polyConnects.push_back(vId_1);
							polyConnects.push_back(vId_2);

							polyCounts.push_back(3);

							//printf("\n B3 %i , %i , %i ", vId_0, vId_1, vId_2);
						}


					}

				}
			}
		}
				
	}

	zFnMesh fnMesh(outMesh);
	fnMesh.clear();
	fnMesh.create(positions, polyCounts, polyConnects);
	fnMesh.setEdgeColor(GREEN);

}

void computeVerticalLoopMesh(zObjGraphArray& oGraphArray, zIntArray& startVperLoop, zObjMesh& guideMesh, zObjMesh &ribMesh, zObjGraphArray& outGraphs, float structureDEPTH , float structureWIDTH)
{
	
	zPointArray mesh_positions;
	zIntArray polyCounts, polyConnects;
	
	
	int maxDivs = 0;
	int minDivs = 1000000;
	for (auto& g : oGraphArray)
	{
		zFnGraph fnGraph_current(g);
		if (fnGraph_current.numVertices() > maxDivs) maxDivs = fnGraph_current.numVertices();
		if (fnGraph_current.numVertices() < minDivs) minDivs = fnGraph_current.numVertices();
	}

	outGraphs.clear();
	outGraphs.assign(maxDivs * 2, zObjGraph());

	zFnMesh fnGuideMesh(guideMesh);
	zInt2DArray stripIDPerGuideVertex;
	stripIDPerGuideVertex.assign(fnGuideMesh.numVertices(), zIntArray());

	int graphID = 0;
	for (int j = 1; j < maxDivs * 4 + 1 ; j+=2)
	{
		zPointArray positions;
		zIntArray edgeConnects;

		for (int i = 0; i < oGraphArray.size(); i++)
		{
			zFnGraph fnGraph_current(oGraphArray[i]);
			if (fnGraph_current.numVertices() == 0) continue;

			int n_v = fnGraph_current.numVertices();
			int startVert = startVperLoop[i];
					
			zPoint* vPositions = fnGraph_current.getRawVertexPositions();

			//if (n_v % 2 == 1) n_v -= 1;

			int factor = floor(maxDivs / n_v);
			//printf("\n factor %i ", factor);

			if (factor != 0)
			{
				int id = (int)floor(j / 4);

				float val = (float)id / factor;


				int cVertId = (int)floor(id / factor);

				val -= cVertId;				

				if (j % 4 == 3 || j % 4 == 2)
				{
					if (val >= 0.5) cVertId += 1;
					
				}


				if (j % 4 == 0 || j % 4 == 1)
				{

					if (val > 0.5) cVertId += 1;					
				}

				cVertId = cVertId % n_v;

				int numVerts = positions.size();
				positions.push_back(vPositions[cVertId]);					
							

				if (numVerts > 0)
				{
					edgeConnects.push_back(numVerts - 1);
					edgeConnects.push_back(numVerts);
				}

				/// Mesh

				int vID = startVert + cVertId;
				zItMeshVertex guideMesh_v(guideMesh, vID);
				zVector vNorm = guideMesh_v.getNormal();
				vNorm.normalize();

				int numVerts_mesh = mesh_positions.size();
				mesh_positions.push_back(guideMesh_v.getPosition() + vNorm * structureDEPTH);
				mesh_positions.push_back(guideMesh_v.getPosition() - vNorm * structureDEPTH);

				stripIDPerGuideVertex[vID].push_back(graphID);
				stripIDPerGuideVertex[vID].push_back(vID);
				stripIDPerGuideVertex[vID].push_back(numVerts_mesh);
				stripIDPerGuideVertex[vID].push_back(numVerts_mesh + 1);

				if (numVerts > 0)
				{
					polyConnects.push_back(numVerts_mesh - 2);
					polyConnects.push_back(numVerts_mesh - 1);
					polyConnects.push_back(numVerts_mesh + 1);
					polyConnects.push_back(numVerts_mesh + 0);

					polyCounts.push_back(4);
				}

			}
		}

		zFnGraph fnOutGraph(outGraphs[graphID]);
		fnOutGraph.create(positions, edgeConnects);
		fnOutGraph.setEdgeColor(YELLOW);

		graphID++;

		//printf("\n %i : %i %i ", j, fnOutGraph.numVertices(), fnOutGraph.numEdges());
	}

	zFnMesh fnMesh(ribMesh);
	fnMesh.clear();
	fnMesh.create(mesh_positions, polyCounts, polyConnects);
	fnMesh.setEdgeColor(ORANGE);

	zVector up(0, 0, 1);

	zPoint* rib_VPositions = fnMesh.getRawVertexPositions();

	for (int i = 0; i < stripIDPerGuideVertex.size(); i++)
	{
		int numGraphs = floor(stripIDPerGuideVertex[i].size() / 4);
		int midId = floor(numGraphs / 2);

		//printf("\n gV %i | %i | ", i, numGraphs);

		for (int j = 0; j < stripIDPerGuideVertex[i].size(); j += 4)
		{
			//printf(" %i ", stripIDPerGuideVertex[i][j]);

			
			int graphID = stripIDPerGuideVertex[i][j];
			int guideMeshVertID = stripIDPerGuideVertex[i][j + 1];

			int index = (j == 0) ? 0 : floor(j / 4);
			

			int patternmesh_v0 = stripIDPerGuideVertex[i][j + 2];
			int patternmesh_v1 = stripIDPerGuideVertex[i][j + 3];

			zItMeshVertex v(guideMesh, guideMeshVertID);
			zItMeshHalfEdgeArray cHEdges;
			v.getConnectedHalfEdges(cHEdges);

			// get HE going in up direction
			zItMeshHalfEdge heUP;
			zVector heUPVec;
			float minAngle = 360;
			for (auto& he : cHEdges)
			{
				zVector heVec = he.getVector();
				if (heVec.angle(up) < minAngle)
				{
					heUP = he;
					heUPVec = heVec;
					minAngle = heVec.angle(up);
				}
			}
			//cout << endl << heUPVec;

			bool left = (index < midId) ? true : false;
			zVector vNorm = v.getNormal();
			vNorm.normalize();			


			if (left)
			{
				
				int multFac = midId - index;
				//cout << "\n vNorm " << vNorm << " | heUp " << heUPVec;

				zItMeshHalfEdge heLeft;
				zVector heLeftVec;
				float minAnglePlane = 360;

				for (auto& he : cHEdges)
				{
					if (he == heUP) continue;

					zVector heVec = he.getVector();
					float ang = heVec.angle360(vNorm, up);
					//printf("\n ang %1.2f ", ang);

					if (ang < minAnglePlane)
					{
						heLeft = he;
						heLeftVec = heVec;
						minAnglePlane = ang;
					}
				}

				heLeftVec.normalize();
				rib_VPositions[patternmesh_v0] += (heLeftVec * multFac * structureWIDTH);
				rib_VPositions[patternmesh_v1] += (heLeftVec * multFac * structureWIDTH);

				///printf("\n multFac %i  %i %i \n ", multFac, patternmesh_v0, patternmesh_v1);
				//cout << endl << heLeftVec;

			}
			else
			{
				
				int multFac = (index + 1) - midId ;
				//cout << "\n vNorm " << vNorm << " | heUp " << heUPVec;

				zItMeshHalfEdge heRight;
				zVector heRightVec;
				float maxAnglePlane = 0;

				for (auto& he : cHEdges)
				{
					if (he == heUP) continue;

					zVector heVec = he.getVector();
					float ang = heVec.angle360(vNorm, up);
					//printf("\n ang %1.2f ", ang);

					if (ang > maxAnglePlane)
					{
						heRight = he;
						heRightVec = heVec;
						maxAnglePlane = ang;
					}
				}

				heRightVec.normalize();
				rib_VPositions[patternmesh_v0] += (heRightVec * multFac * structureWIDTH);
				rib_VPositions[patternmesh_v1] += (heRightVec * multFac * structureWIDTH);
			}

			
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

	// read mesh
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/yate/inMesh.obj", zOBJ);	
	readStartV("data/yate/inData.txt", startV);	
		
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_pattern);
	model.addObject(oMesh_rib);

	// set display element booleans
	oMesh.setDisplayElements(false, true, false);
	oMesh_pattern.setDisplayElements(false, true, false);
	oMesh_rib.setDisplayElements(false, true, true);
	for (zObjGraph& g : oGraph_loops)
	{
		model.addObject(g);
		g.setDisplayElements(true, true);
	}

	for (zObjGraph& g : oGraph_structures)
	{
		model.addObject(g);
		g.setDisplayElements(true, true);
	}
	


	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&panelWidth, "panelWidth");
	S.sliders[1].attachToVariable(&panelWidth, 0.1, 10);

	S.addSlider(&structureDepth, "structureDepth");
	S.sliders[2].attachToVariable(&structureDepth, 0.1, 2);

	S.addSlider(&structureWidth, "structureWidth");
	S.sliders[3].attachToVariable(&structureWidth, 0.1, 2);

	S.addSlider(&p0Z, "p0Z");
	S.sliders[4].attachToVariable(&p0Z, -10, 20);

	S.addSlider(&p1Z, "p1Z");
	S.sliders[5].attachToVariable(&p1Z, -10, 20);

	S.addSlider(&minDivs, "minDivs");
	S.sliders[6].attachToVariable(&minDivs, 4, 128);

	S.addSlider(&maxDivs, "maxDivs");
	S.sliders[7].attachToVariable(&maxDivs, 4, 128);
		

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	B.addButton(&compGRAD, "compGRAD");
	B.buttons[2].attachToVariable(&compGRAD);

}

void update(int value)
{
	control_p0.z = p0Z;
	control_p1.z = p1Z;

	if (compute)
	{
		if (compGRAD)
		{	
			computeLoopGraphs(oMesh, oGraph_loops, startV, control_p0, control_p1,minDivs,maxDivs);
		}
		else computeLoopGraphs(oMesh, oGraph_loops, startV, panelWidth, 16);

		zIntArray startVperLoop;
		computeGuideMesh(oGraph_loops, startVperLoop, oMesh_pattern);

		computeVerticalLoopMesh(oGraph_loops, startVperLoop, oMesh_pattern, oMesh_rib, oGraph_structures, structureDepth,structureWidth);

		compute = !compute;	
	}
	if (exportMesh)
	{
		zFnMesh fnMesh_pattern(oMesh_pattern);
		fnMesh_pattern.to("data/yate/outMesh_pattern.obj", zOBJ);

		zFnMesh fnMesh_rib(oMesh_rib);
		fnMesh_rib.to("data/yate/outMesh_rib.obj", zOBJ);

		exportMesh = !exportMesh;
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

		//for (auto& g : oGraph_loops) g.draw();
		//for (auto& g : oGraph_structures) g.draw();
		

		model.displayUtils.drawPoint(control_p0,zColor(1,0,0,1), 5);
		model.displayUtils.drawPoint(control_p1, zColor(0, 0, 1, 1), 5);
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);


	drawString("KEY Press", vec(winW - 350, winH - 600, 0));
	drawString("p - ComputePattern", vec(winW - 350, winH - 575, 0));
	drawString("e - ExportOBJ", vec(winW - 350, winH - 550, 0));


	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'e') exportMesh = true;;


}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
