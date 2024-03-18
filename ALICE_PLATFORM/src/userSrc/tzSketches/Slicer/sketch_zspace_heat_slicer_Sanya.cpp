//#define _MAIN_
#define _HAS_STD_BYTE 0

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>
#include <headers/zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;
////////////////////////////////////////////////////////////////////////// CUSTOM OBJECTS

struct zMegaPanel
{
	int id;
	zObjMesh o_panelMesh;
	zFloatArray scalars;

	zObjMeshArray o_stripMeshes;
	zObjMeshArray o_unrollMeshes;

	zObjGraphArray o_railGraphs;
	zObjGraph o_boundaryGraph;

	zObjGraphArray verticalLines_upper;
	zObjGraphArray verticalLines_bound;

	int numActiveStrips;
};

struct zPair_hash {
	template <class T1, class T2>
	size_t operator()(const pair<T1, T2>& p) const
	{
		auto hash1 = hash<T1>{}(p.first);
		auto hash2 = hash<T2>{}(p.second);

		if (hash1 != hash2) {
			return hash1 ^ hash2;
		}

		// If hash1 == hash2, their XOR is zero.
		return hash1;
	}
};

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;

//nurbs
zObjNurbsCurveArray displayCurves;
zPointArray displayPts;
zObjGraphArray displayVerticals;

void computeDualGraph_BST(zObjMesh& o_mesh, zObjGraph& o_graph, zItGraphVertexArray& bsf_Vertices, zIntPairArray& bsf_vertexPairs)
{
	zFnMesh fnMesh(o_mesh);

	zIntArray inEdge_dualEdge;
	zIntArray dualEdge_inEdge;
	fnMesh.getDualGraph(o_graph, inEdge_dualEdge, dualEdge_inEdge, true, false, false);

	zFnGraph fnGraph(o_graph);
	fnGraph.setEdgeColor(zColor(1, 1, 0, 1));

	zItGraphVertex v_MaxValence;
	int maxValence = 0;;

	for (zItGraphVertex v(o_graph); !v.end(); v++)
	{
		if (v.getValence() > maxValence)
		{
			v_MaxValence = v;
			maxValence = v.getValence();
		}
	}

	maxValence += 1;

	// breadth search first sorting

	v_MaxValence.getBSF(bsf_Vertices, bsf_vertexPairs);


}

void creatUnrollMesh(zObjMesh& o_mesh, zObjMesh& o_mesh_unroll, zObjGraph& o_dualgraph, zInt2DArray& oriVertex_UnrollVertex_map, unordered_map<zIntPair, int, zPair_hash>& oriFaceVertex_UnrollVertex, zItGraphVertexArray& bsf_Vertices, zIntPairArray& bsf_vertexPairs)
{
	zFnMesh fnMesh(o_mesh);

	computeDualGraph_BST(o_mesh, o_dualgraph, bsf_Vertices, bsf_vertexPairs);

	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zPointArray positions;
	zIntArray pConnects, pCounts;

	oriVertex_UnrollVertex_map.clear();
	oriVertex_UnrollVertex_map.assign(fnMesh.numVertices(), zIntArray());

	for (zItMeshFace f(o_mesh); !f.end(); f++)
	{
		zIntArray fVerts;
		f.getVertices(fVerts);

		for (auto fV : fVerts)
		{
			int numVerts = positions.size();

			pConnects.push_back(numVerts);
			oriVertex_UnrollVertex_map[fV].push_back(numVerts);

			zIntPair hashKey(f.getId(), fV);
			oriFaceVertex_UnrollVertex[hashKey] = numVerts;

			positions.push_back(vPositions[fV]);
		}

		pCounts.push_back(fVerts.size());
	}


	zFnMesh fnMesh_unroll(o_mesh_unroll);
	fnMesh_unroll.create(positions, pCounts, pConnects);

}

void computeVLoops_Interior(zObjMesh& oMesh, int startVertexID0, int startVertexID1, vector<zItMeshHalfEdgeArray>& v_Loops)
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

//void splitVLoops(zObjMesh& oMesh, vector<zItMeshHalfEdgeArray>& v_Loops, vector<zPointArray>& divsPoints)
//{
//	zFnMesh fnMesh_in(oMesh);
//	divsPoints.clear();
//
//	for (int l = 0; l < v_Loops.size(); l++)
//	{
//		zPointArray loop_pts;
//		fnMesh_in.computeEdgeLoop_SplitLength(v_Loops[l], 0.1, loop_pts);
//
//		divsPoints.push_back(loop_pts);
//	}
//
//}

void colorMesh(zObjMesh& o_mesh, zFloatArray& scalars)
{
	zFnMesh fnMesh(o_mesh);

	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}

void computeHeatScalars(zObjMesh& oMesh, vector<zItMeshHalfEdgeArray>& v_Loops, zScalarArray& scalars)
{
	zFnMesh fnMesh(oMesh);

	scalars.clear();
	scalars.assign(fnMesh.numVertices(), -1);

	for (int l = 0; l < v_Loops.size(); l++)
	{
		float length = 0;
		for (int j = 0; j < v_Loops[l].size(); j++)
		{
			if (j == 0) scalars[v_Loops[l][j].getStartVertex().getId()] = length;

			length += v_Loops[l][j].getLength();
			scalars[v_Loops[l][j].getVertex().getId()] = length;
		}
	}

	colorMesh(oMesh, scalars);

	/*zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();*/
}

void boolean_union(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result)
{

	fieldValues_Result.clear();
	for (int i = 0; i < fieldValues_A.size(); i++)
	{
		fieldValues_Result.push_back(core.zMin(fieldValues_A[i], fieldValues_B[i]));
	}

}

void computeBoundaryVertices_A(zObjMesh& oMesh, int startVertexID0, int startVertexID1, zIntArray& bIDs)
{
	zFnMesh fnMesh_in(oMesh);

	bIDs.clear();

	zItMeshHalfEdge heStart;
	bool chk = fnMesh_in.halfEdgeExists(startVertexID1, startVertexID0, heStart);



	bool flip = false;
	if (!heStart.onBoundary())
	{
		heStart = heStart.getSym();
		flip = true;
	}

	zItMeshHalfEdge he_U = heStart;

	if (flip) bIDs.push_back(he_U.getVertex().getId());
	else bIDs.push_back(he_U.getStartVertex().getId());

	bool exit_1 = false;
	do
	{
		if (flip) bIDs.push_back(he_U.getStartVertex().getId());
		else bIDs.push_back(he_U.getVertex().getId());

		he_U.getEdge().setColor(zMAGENTA);

		if (flip)
		{
			if (he_U.getStartVertex().checkValency(2)) exit_1 = true;
		}
		else
		{
			if (he_U.getVertex().checkValency(2)) exit_1 = true;
		}


		if (!exit_1) he_U = (!flip) ? he_U.getNext() : he_U.getPrev();


	} while (!exit_1);
}

void computeBoundaryVertices(zObjMesh* o_mesh, zIntArray& vIDs, zIntArray& bIDs)
{
	bIDs.clear();

	for (int j = 0; j < vIDs.size(); j++)
	{
		zItMeshVertex v(*o_mesh, vIDs[j]);
		zItMeshHalfEdgeArray cHEdges;
		v.getConnectedHalfEdges(cHEdges);

		zItMeshHalfEdge startHe, He;
		for (auto& cHE : cHEdges)
		{
			if (cHE.onBoundary())startHe = cHE;
		}

		He = startHe;

		do
		{
			bIDs.push_back(He.getStartVertex().getId());
			He = He.getNext();

		} while (He != startHe);
	}


}

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars_start, zFloatArray& scalars_end, int currentContourID, int totalContours, zObjGraphArray& o_contourGraphs)
{

	if (currentContourID >= o_contourGraphs.size())
	{
		cout << "Error: currentContourID greater than or eual to size of o_contourGraphs." << endl;
		return;
	}

	zFloatArray scalars;
	scalars.assign(scalars_start.size(), -1);

	// weighted scalars
	float weight = ((float)(currentContourID + 1) / (float)totalContours);

	for (int j = 0; j < scalars.size(); j++)
	{
		scalars[j] = weight * scalars_start[j] - (1 - weight) * scalars_end[j];
	}

	// Generate the isocontour using the threshold value
	zPointArray positions;
	zIntArray edgeConnects;
	zColorArray vColors;
	int pres = 3;
	zFnMesh fnMesh(*o_mesh);
	fnMesh.getIsoContour(scalars, 0.0, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

	// Create graph from the isocontour
	zFnGraph tempFn(o_contourGraphs[currentContourID]);
	tempFn.create(positions, edgeConnects);
	tempFn.setEdgeColor(zColor(255, 255, 255, 1));
	tempFn.setEdgeWeight(2);
	tempFn.setVertexColors(vColors, false);

	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}

void computeContours_A(zObjMesh& o_mesh, zFloatArray& scalars, float contourDistances, zObjGraphArray& o_contourGraphs)
{
	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	printf("\n minScalar : %1.2f | maxScalar %1.2f ", minScalar, maxScalar);

	// weighted scalars
	float dDomain = maxScalar - minScalar;
	int totalContours = ceil(dDomain / contourDistances);
	float distanceIncrements = dDomain / totalContours;

	printf("\n total Contours : %i | distance %1.2f | dDomain %1.2f", totalContours, distanceIncrements, dDomain);

	o_contourGraphs.clear();
	o_contourGraphs.assign(totalContours, zObjGraph());

	// Generate the isocontour using the threshold value
	zFnMesh fnMesh(o_mesh);
	int pres = 3;

	for (int i = 1; i < totalContours; i++)
	{
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vColors;


		fnMesh.getIsoContour(scalars, minScalar + (distanceIncrements * i), positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

		// Create graph from the isocontour
		zFnGraph tempFn(o_contourGraphs[i]);
		tempFn.create(positions, edgeConnects);
		tempFn.setEdgeColor(zColor(255, 255, 255, 1));
		tempFn.setEdgeWeight(2);
		tempFn.setVertexColors(vColors, false);

	}




	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();



	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}

void cleanGraph(zObjGraph& o_graph, zObjGraphArray& out_graphs)
{
	zFnGraph fnGraph(o_graph);

	// check number of corners
	zIntArray corners;
	for (zItGraphVertex v(o_graph); !v.end(); v++)
	{
		if (v.checkValency(1)) corners.push_back(v.getId());
	}

	zBoolArray vertexVisited;
	vertexVisited.assign(fnGraph.numVertices(), false);

	printf("\n corners %i ", corners.size());

	if (corners.size() < 2) return;

	int currentcorner = corners[0];

	out_graphs.assign((int)corners.size() * 0.5, zObjGraph());
	int graph_counter = 0;

	bool exit_1 = false;
	do
	{
		zItGraphVertex v(o_graph, currentcorner);
		vertexVisited[v.getId()] = true;
		zItGraphHalfEdge he = v.getHalfEdge();

		bool exitLoop = false;
		zPointArray positions;
		zIntArray eConnects;

		do
		{
			int vID = he.getVertex().getId();

			for (int i = 0; i < corners.size(); i++)
			{
				if (vID == corners[i])
				{
					exitLoop = true;
					vertexVisited[corners[i]] = true;
				}
			}

			if (positions.size() >= 1)
			{
				eConnects.push_back(positions.size() - 1);
				eConnects.push_back(positions.size());
			}

			positions.push_back(he.getStartVertex().getPosition());

			if (exitLoop)
				positions.push_back(he.getVertex().getPosition());

			if (!exitLoop) he = he.getNext();

		} while (!exitLoop);

		zFnGraph fn(out_graphs[graph_counter]);
		if (positions.size() > 1 && eConnects.size() > 1)
		{
			fn.create(positions, eConnects);
		}

		exit_1 = true;
		for (int i = 0; i < corners.size(); i++)
		{
			if (!vertexVisited[corners[i]] && exit_1)
			{
				exit_1 = false;
				currentcorner = corners[i];
			}
		}

		graph_counter++;
	} while (!exit_1);

	//printf("\n %i %i | %i ", positions.size(), pCounts.size(), fnGraph.numEdges());
}

zPoint getContourPosition(float& threshold, zVector& vertex_lower, zVector& vertex_higher, float& thresholdLow, float& thresholdHigh)
{
	float scaleVal = core.ofMap(threshold, thresholdLow, thresholdHigh, 0.0000f, 1.0000f);

	zVector e = vertex_higher - vertex_lower;
	double edgeLen = e.length();
	e.normalize();

	return (vertex_lower + (e * edgeLen * scaleVal));
}

void computeBoundaryGraph(zObjMesh& o_mesh, zFloatArray& scalars, float lowThreshold, float highThreshold, zObjGraph& o_graph)
{
	int pres = 2;
	zFnMesh fnMesh(o_mesh);

	zPointArray positions;
	zIntArray eConnects;

	zBoolArray vVisited;
	zIntArray vMap;

	vVisited.assign(fnMesh.numVertices(), false);
	vMap.assign(fnMesh.numVertices(), -1);

	zItMeshHalfEdge heStart;

	for (zItMeshHalfEdge he(o_mesh); !he.end(); he++)
	{
		if (he.onBoundary())
		{
			heStart = he;
			break;
		}
	}

	zItMeshHalfEdge he = heStart;
	//printf("\n heStart %i ", heStart.getId());;

	int corners = 0;

	int current = 10;


	do
	{
		int prev = current;

		float s0 = scalars[he.getStartVertex().getId()];
		float s1 = scalars[he.getVertex().getId()];

		int v0 = -1;;
		int v1 = -1;;

		zPoint pStart = he.getStartVertex().getPosition();
		zPoint pEnd = he.getVertex().getPosition();

		if (s0 < lowThreshold && s1 < lowThreshold)
		{
			// do nothing
			current = 0;
		}
		else if (s0 > highThreshold && s1 > highThreshold)
		{
			// do nothing
			current = 0;
		}
		else
		{
			current = 1;

			if (s0 >= lowThreshold && s0 <= highThreshold)
			{
				bool chk = core.checkRepeatElement(pStart, positions, v0, pres);

				if (!chk)
				{
					v0 = positions.size();
					positions.push_back(pStart);
				}
			}

			if (s1 >= lowThreshold && s1 <= highThreshold)
			{
				bool chk = core.checkRepeatElement(pEnd, positions, v1, pres);

				if (!chk)
				{
					v1 = positions.size();
					positions.push_back(pEnd);
				}
			}

			if (s0 > lowThreshold && s0 <= highThreshold && s1 > highThreshold)
			{

				zPoint p = getContourPosition(highThreshold, pStart, pEnd, s0, s1);
				bool chk = core.checkRepeatElement(p, positions, v1, pres);

				if (!chk)
				{
					v1 = positions.size();
					positions.push_back(p);
				}
			}

			if (s0 >= lowThreshold && s0 <= highThreshold && s1 < lowThreshold)
			{

				zPoint p = getContourPosition(lowThreshold, pStart, pEnd, s0, s1);
				bool chk = core.checkRepeatElement(p, positions, v1, pres);

				if (!chk)
				{
					v1 = positions.size();
					positions.push_back(p);
				}
			}

			if (s1 >= lowThreshold && s1 <= highThreshold && s0 > highThreshold)
			{

				zPoint p = getContourPosition(highThreshold, pEnd, pStart, s1, s0);
				bool chk = core.checkRepeatElement(p, positions, v0, pres);

				if (!chk)
				{
					v0 = positions.size();
					positions.push_back(p);
				}
			}

			if (s1 >= lowThreshold && s1 <= highThreshold && s0 < lowThreshold)
			{

				zPoint p = getContourPosition(lowThreshold, pEnd, pStart, s1, s0);
				bool chk = core.checkRepeatElement(p, positions, v0, pres);

				if (!chk)
				{
					v0 = positions.size();
					positions.push_back(p);
				}
			}


			if (s0 < lowThreshold && s1 > highThreshold)
			{
				zPoint p = getContourPosition(lowThreshold, pStart, pEnd, s0, s1);
				bool chk = core.checkRepeatElement(p, positions, v0, pres);

				if (!chk)
				{
					v0 = positions.size();
					positions.push_back(p);
				}

				zPoint p1 = getContourPosition(highThreshold, pStart, pEnd, s0, s1);
				bool chk1 = core.checkRepeatElement(p1, positions, v1, pres);

				if (!chk1)
				{
					v1 = positions.size();
					positions.push_back(p1);
				}

			}

			if (s1 < lowThreshold && s0 > highThreshold)
			{
				zPoint p = getContourPosition(lowThreshold, pEnd, pStart, s1, s0);
				bool chk = core.checkRepeatElement(p, positions, v1, pres);

				if (!chk)
				{
					v1 = positions.size();
					positions.push_back(p);
				}

				zPoint p1 = getContourPosition(highThreshold, pEnd, pStart, s1, s0);
				bool chk1 = core.checkRepeatElement(p1, positions, v0, pres);

				if (!chk1)
				{
					v0 = positions.size();
					positions.push_back(p1);
				}

			}

			if (v0 != -1 && v1 != -1)
			{
				if (v0 != v1)
				{
					eConnects.push_back(v0);
					eConnects.push_back(v1);
				}

			}

		}

		if (abs(current - prev) == 1) corners++;

		he = he.getNext();

	} while (he != heStart);

	printf("\n boundary graph corners | %i ", corners);

	zFnGraph fnGraph(o_graph);
	fnGraph.create(positions, eConnects);

	fnGraph.setEdgeColor(zGREEN);
}


void computeContours_Strips(zObjMesh& o_mesh, zFloatArray& scalars, float& minScalar, float& maxScalar, int& totalContours, float& distanceIncrements, float& contourDistances, float& gap, zObjMeshArray& o_contourMeshes)
{

	o_contourMeshes.clear();
	o_contourMeshes.assign(totalContours, zObjMesh());

	// Generate the isocontour using the threshold value
	zFnMesh fnMesh(o_mesh);
	int pres = 3;

	zDomainFloat contoursDomain(0, totalContours);
	zDomainColor contourColor(zMAGENTA, zBLUE);

	int activeStrips = 0;
	for (int i = 1; i < totalContours; i++)
	{
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vColors;

		float lowThreshold = minScalar + (distanceIncrements * (i - 1)) + (gap * 0.5);
		float highThreshold = minScalar + (distanceIncrements * i) + (gap * -0.5);

		fnMesh.getIsobandMesh(scalars, lowThreshold, highThreshold, o_contourMeshes[i]);

		zFnMesh fnStripMesh(o_contourMeshes[i]);
		//printf("\n strips %i %i %i ", fnStripMesh.numVertices(), fnStripMesh.numEdges(), fnStripMesh.numPolygons());

		//zColor col = core.blendColor((float)i, contoursDomain, contourColor, zHSV);

		if (fnStripMesh.numPolygons() > 0)
		{
			activeStrips++;
			zColor col = (i % 2 == 0) ? contourColor.min : contourColor.max;
			fnStripMesh.setFaceColor(col);
		}

	}

	printf("\n num Active Strips : %i ", activeStrips);

}

void computeContours_Strips_B(zObjMesh& o_mesh, zFloatArray& scalars, zMegaPanel& o_panel, int numStripsPerPanel, float& minScalar, float& maxScalar, int& totalContours, float& distanceIncrements, float& contourDistances, float& gap)
{

	o_panel.o_stripMeshes.clear();
	o_panel.o_stripMeshes.assign(numStripsPerPanel, zObjMesh());

	o_panel.o_railGraphs.clear();
	o_panel.o_railGraphs.assign(numStripsPerPanel * 2, zObjGraph());



	int panelId = o_panel.id;

	// Generate the isocontour using the threshold value
	zFnMesh fnMesh(o_mesh);
	int pres = 3;

	zDomainFloat contoursDomain(0, totalContours);
	zDomainColor contourColor(zMAGENTA, zBLUE);

	int startID = panelId * numStripsPerPanel;
	int endID = (panelId + 1) * numStripsPerPanel;

	if (endID > totalContours) endID = totalContours;

	int activeStrips = 0;
	int counter = 0;
	for (int i = startID + 1; i <= endID; i++)
	{
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vColors;

		float lowThreshold = minScalar + (distanceIncrements * (i - 1)) + (gap * 0.5);
		float highThreshold = minScalar + (distanceIncrements * i) + (gap * -0.5);

		//printf("\n each strips threshold %1.4f %1.4f | diff %1.4f ", lowThreshold, highThreshold, highThreshold - lowThreshold);

		fnMesh.getIsobandMesh(scalars, lowThreshold, highThreshold, o_panel.o_stripMeshes[counter]);


		zFnMesh fnStripMesh(o_panel.o_stripMeshes[counter]);
		//printf("\n strips %i %i %i ", fnStripMesh.numVertices(), fnStripMesh.numEdges(), fnStripMesh.numPolygons());

		//zColor col = core.blendColor((float)i, contoursDomain, contourColor, zHSV);

		if (fnStripMesh.numPolygons() > 0)
		{
			activeStrips++;
			zColor col = (i % 2 == 0) ? contourColor.min : contourColor.max;
			fnStripMesh.setFaceColor(col);
		}

		fnMesh.getIsoContour(scalars, lowThreshold, positions, edgeConnects, vColors);

		zFnGraph fnGraph(o_panel.o_railGraphs[counter * 2]);
		fnGraph.create(positions, edgeConnects);


		fnMesh.getIsoContour(scalars, highThreshold, positions, edgeConnects, vColors);

		zFnGraph fnGraph_1(o_panel.o_railGraphs[counter * 2 + 1]);
		fnGraph_1.create(positions, edgeConnects);



		counter++;

	}



	o_panel.numActiveStrips = activeStrips;

	zFnGraph fnGraph(o_panel.o_railGraphs[0]);
	fnGraph.setEdgeColor(zMAGENTA);


	zFnGraph fnGraph_1(o_panel.o_railGraphs[(counter * 2) - 1]);
	fnGraph_1.setEdgeColor(zMAGENTA);

	// boundary graph
	float lowThreshold = minScalar + (distanceIncrements * (startID)) + (gap * 0.5);
	float highThreshold = minScalar + (distanceIncrements * endID) + (gap * -0.5);
	computeBoundaryGraph(o_mesh, scalars, lowThreshold, highThreshold, o_panel.o_boundaryGraph);


	//printf("\n 4 strips threshold %1.4f %1.4f | distanceIncrements %1.4f gap %1.4f ", lowThreshold, highThreshold, distanceIncrements, gap);

	printf("\n num Active Strips : %i ", activeStrips);

}


void computeMegaPanel(zObjMesh& o_inmesh, string path, zFloatArray& scalars, vector< zMegaPanel>& megaPanels)
{
	json j;
	core.readJSON(path, j);

	zInt2DArray panel_faceIDS;
	bool chk = core.readJSONAttribute(j, "PanelFaces", panel_faceIDS);

	printf("\n megapanels %i ", panel_faceIDS.size());

	zFnMesh fnMesh(o_inmesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zColor* fColors = fnMesh.getRawFaceColors();

	if (chk)
	{
		megaPanels.clear();
		megaPanels.assign(panel_faceIDS.size(), zMegaPanel());

		for (int i = 0; i < panel_faceIDS.size(); i++)
		{
			zIntArray vMap;
			vMap.assign(fnMesh.numVertices(), -1);

			zFnMesh fnMeshPanel(megaPanels[i].o_panelMesh);

			megaPanels[i].scalars.clear();
			megaPanels[i].id = i;

			zPointArray positions;
			zIntArray pCounts, pConnects;
			zColorArray faceColors;

			for (int j = 0; j < panel_faceIDS[i].size(); j++)
			{
				zItMeshFace f(o_inmesh, panel_faceIDS[i][j]);

				zIntArray fVerts;
				f.getVertices(fVerts);

				for (int k = 0; k < fVerts.size(); k++)
				{
					if (vMap[fVerts[k]] == -1)
					{
						vMap[fVerts[k]] = positions.size();
						positions.push_back(vPositions[fVerts[k]]);
						megaPanels[i].scalars.push_back(scalars[fVerts[k]]);

					}

					pConnects.push_back(vMap[fVerts[k]]);
				}

				faceColors.push_back(fColors[f.getId()]);
				pCounts.push_back(fVerts.size());
			}

			fnMeshPanel.create(positions, pCounts, pConnects);
			fnMeshPanel.setFaceColors(faceColors, false);
		}
	}


}

void computeMegaPanel_Strips(vector< zMegaPanel>& megaPanels, zFloatArray& globalScalars, float contourDistances, float contourGaps)
{
	zScalar minScalar = core.zMin(globalScalars);
	zScalar maxScalar = core.zMax(globalScalars);

	printf("\n minScalar : %1.2f | maxScalar %1.2f ", minScalar, maxScalar);

	float dDomain = maxScalar - minScalar;
	int totalContours = ceil(dDomain / contourDistances);
	float distanceIncrements = dDomain / totalContours;

	printf("\n total Contours : %i | distance %1.2f | dDomain %1.2f", totalContours, distanceIncrements, dDomain);

	for (auto& mp : megaPanels)
	{
		printf("\n ---- \n megaPanel %i ", mp.id);

		computeContours_Strips(mp.o_panelMesh, mp.scalars, minScalar, maxScalar, totalContours, distanceIncrements, contourDistances, contourGaps, mp.o_stripMeshes);
	}
}

void computeMegaPanel_StripsB(zObjMesh& o_inmesh, vector< zMegaPanel>& megaPanels, zFloatArray& globalScalars, int numStripsPerPanel, float contourDistances, float contourGaps)
{
	zScalar minScalar = core.zMin(globalScalars);
	zScalar maxScalar = core.zMax(globalScalars);

	printf("\n minScalar : %1.2f | maxScalar %1.2f ", minScalar, maxScalar);

	float dDomain = maxScalar - minScalar;
	int totalContours = ceil(dDomain / contourDistances);
	float distanceIncrements = contourDistances/*dDomain / totalContours*/;

	int numMegaPanels = ceil((float)totalContours / (float)numStripsPerPanel);
	printf("\n numMegaPanels %i  | total Contours : %i | distance %1.2f | dDomain %1.2f", numMegaPanels, totalContours, distanceIncrements, dDomain);

	megaPanels.clear();
	megaPanels.assign(numMegaPanels, zMegaPanel());

	int counter = 0;
	for (auto& mp : megaPanels)
	{
		mp.id = counter;
		counter++;

		//if (mp.id != 38)continue;
		printf("\n ---- \n megaPanel %i / %i ", mp.id, numMegaPanels);

		computeContours_Strips_B(o_inmesh, globalScalars, mp, numStripsPerPanel, minScalar, maxScalar, totalContours, distanceIncrements, contourDistances, contourGaps);
	}

}

void megaPanel_CheckPlanarity(vector< zMegaPanel>& megaPanels, int mID, float tolerance)
{
	auto& mp = megaPanels[mID];
	//for (auto& mp : megaPanels)
	//{
	printf("\n ---- \n megaPanel %i ", mp.id);

	int counter = 0;
	for (auto& strip : mp.o_stripMeshes)
	{
		zFnMesh fnMesh(strip);

		if (fnMesh.numVertices() == 0)
		{
			counter++;
			continue;
		}

		zDoubleArray devs;
		fnMesh.getPlanarityDeviationPerFace(devs, zVolumePlanar, true, tolerance);

		double minDev = core.zMin(devs);
		double maxDev = core.zMax(devs);

		printf("\n s %i | %1.4f %1.4f ", counter, minDev, maxDev);

		counter++;
	}
	//}
}

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

//---- nurbs


void makeCurveFromGraph(zObjGraph& o_graph, zObjNurbsCurve& out_curve)
{
	zFnNurbsCurve fnNurbs(out_curve);
	fnNurbs.create(o_graph, 0.5, 3, false, false, 200);
}

void computePlanes(zObjNurbsCurve& o_curve, double sampleDist, vector<zPlane>& out_planes)
{

	zFnNurbsCurve fnCurve(o_curve);

	//divide curve
	zPointArray positions;
	zDoubleArray tParams;
	int sampleCount = (int)ceil(fnCurve.getLength() / sampleDist);
	fnCurve.divideByCount(sampleCount, positions, tParams);

	//make planes
	out_planes.assign(tParams.size(), zPlane());

	for (int i = 0; i < tParams.size(); i++)
	{
		zVector O;
		zVector X, Y, Z;
		zVector worldZ(0, 0, 1);
		fnCurve.getPointTangentAt(tParams[i], O, Z);

		X = worldZ ^ Z;
		Y = Z ^ X;
		X.normalize();
		Y.normalize();
		Z.normalize();

		Matrix4f plane;
		plane.setIdentity();
		plane(0, 0) = X.x;	plane(0, 1) = X.y;	plane(0, 2) = X.z;
		plane(1, 0) = Y.x;	plane(1, 1) = Y.y;	plane(1, 2) = Y.z;
		plane(2, 0) = Z.x;	plane(2, 1) = Z.y;	plane(2, 2) = Z.z;
		plane(3, 0) = O.x;	plane(3, 1) = O.y;	plane(3, 2) = O.z;

		out_planes[i] = plane;

		//cout << "point:" << endl << O << endl;
		//cout << "tangent:" << endl << Z << endl;
		//cout  << "plane:" << endl << plane << endl;
	}
}

void computeVertical(vector<zPlane>& planes,zObjNurbsCurveArray& refCurves, zObjGraphArray& out_lines,zPointArray& out_pts)
{
	out_pts.clear();
	out_lines.clear();
	out_lines.assign(planes.size(), zObjGraph());
	zFnNurbsCurve fnRef(refCurves[0]);

	////find closest curve
	//zPoint p(planes[0](3, 0), planes[0](3, 1), planes[0](3, 2));

	//double minDist = 100000000;
	//int id = -1;

	//for (int i = 0; i < refCurves.size(); i++)
	//{
	//	fnRef = zFnNurbsCurve(refCurves[i]);
	//	zPoint ref = fnRef.getPointAt(0.5);

	//	const double dist = p.distanceTo(ref);

	//	if (dist < minDist)
	//	{
	//		minDist = dist;
	//		id = i;
	//	}
	//}
	//fnRef = zFnNurbsCurve(refCurves[id]);

	//intersect
	
	for (int i = 0; i < planes.size(); i++)
	{
		zPlane plane = planes[i];
		for (auto& crv : refCurves)
		{
			fnRef = zFnNurbsCurve(crv);
			zPoint origin(plane(3, 0), plane(3, 1), plane(3, 2));

			zPointArray pts;
			zDoubleArray tParams;

			fnRef.intersect(plane, pts, tParams);

			if (pts.size() > 0)
			{
				for (auto& p : pts)
				{
					if (p.distanceTo(origin) < 3)
					{
						//make lines
						zObjGraph intersetion;
						zFnGraph fnGraph(out_lines[i]);
						zPointArray pos;
						zIntArray pConnects;

						pos.push_back(origin);
						pos.push_back(p);
						pConnects.push_back(0);
						pConnects.push_back(1);
						//pos = { origin,p };
						//pConnects = { 0,1 };

						fnGraph.create(pos, pConnects);

						out_pts.push_back(origin);
						out_pts.push_back(p);
						break;
					}
				}
			}
		}



	}


}

////////////////////////////////////////////////////////////////////////// General

bool compute_heat = false;
bool compute_contour = false;
bool exportMeshes = false;
bool exportSplitLines = false;
bool compute_planarity = false;
bool compute_nurbs = false;

bool d_inMesh = true;
bool d_inMeshFace = true;
bool d_MegaPanel = true;
bool d_MegaPanel_strips = true;
bool d_AllPanels = false;
bool d_nurbs = false;

double background = 0.35;


int currentPanelId = 0;
int totalPanels = 60;

int startID0 = 1344;
int startID1 = 320;

int numStripsPerPanel = 4;
float contourDistance = 0.11;
float contourGap = 0.01;
double sampleDist = 4.0;

string filePath = "data/sanya/inMesh_2.json";



////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjMesh o_inMesh;
vector<zMegaPanel> myPanels;

vector<zItMeshHalfEdgeArray> vLoops;
zScalarArray dScalars;

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


	// read
	
	string inPath;
	cout << "Please enter file path: ";
	cin >> inPath;

	zStringArray split = core.splitString(inPath, "\"");
	filePath = split[0];
		
	cout << "Please enter start vertexID 0: ";
	cin >> startID0;

	cout << "Please enter start vertexID 1: ";
	cin >> startID1;

	//cout << "Please enter contour distance: ";
	//cin >> contourDistance;

	//cout << "Please enter contour gap: ";
	//cin >> contourGap;

	//cout << "Please enter mega panel sample distance: ";
	//cin >> sampleDist;

	//cout << "Please enter number of strips per panel: ";
	//cin >> numStripsPerPanel;
	

	zFnMesh fnInmesh(o_inMesh);
	fnInmesh.from(filePath, zJSON);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans	
	o_inMesh.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_heat, "compute_heat");
	B.buttons[0].attachToVariable(&compute_heat);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_inMeshFace, "d_inMeshFace");
	B.buttons[2].attachToVariable(&d_inMeshFace);

	B.addButton(&d_MegaPanel, "d_MegaPanel");
	B.buttons[3].attachToVariable(&d_MegaPanel);

	B.addButton(&d_MegaPanel_strips, "d_MegaPanel_strips");
	B.buttons[4].attachToVariable(&d_MegaPanel_strips);

	B.addButton(&d_nurbs, "d_nurbs");
	B.buttons[5].attachToVariable(&d_nurbs);

	

}

void update(int value)
{
	o_inMesh.setDisplayFaces(d_inMeshFace);

	if (compute_heat)
	{
		computeVLoops(o_inMesh, startID0, startID1, vLoops);		
		computeHeatScalars(o_inMesh, vLoops, dScalars);
		
		computeMegaPanel(o_inMesh, filePath, dScalars, myPanels);

		totalPanels = myPanels.size();

		compute_heat = !compute_heat;
	}

	if (compute_contour)
	{
		//computeContours_Strips(o_inMesh, dScalars, contourDistance, contourGap, o_stripMeshes);
		//computeMegaPanel_Strips(myPanels, dScalars, contourDistance, contourGap);

		computeMegaPanel_StripsB(o_inMesh, myPanels, dScalars, numStripsPerPanel, contourDistance, contourGap);
		
		compute_contour = !compute_contour;
	}

	if (compute_planarity)
	{
		megaPanel_CheckPlanarity(myPanels, currentPanelId, 0.005);

		compute_planarity = !compute_planarity;
	}

	if (compute_nurbs)
	{
		displayCurves.clear();
		displayPts.clear();

		for (int i = 0; i < myPanels.size(); i++)
		{
			//int i = currentPanelId;
			//if (i != 38) continue;

			zObjNurbsCurveArray lowerCurves;
			zObjNurbsCurveArray upperCurves;
			zObjNurbsCurveArray intermediateCurves;
			zObjNurbsCurveArray boundaryCurves;


			for (int j = 0; j < myPanels[i].o_railGraphs.size(); j++)
			{
				zFnGraph fnGraph(myPanels[i].o_railGraphs[j]);
				if (fnGraph.numVertices() > 1)
				{
					zObjGraphArray cleanGraphs;
					if (j == 0 || j == myPanels[i].o_railGraphs.size() - 1)
					{
						cleanGraph(myPanels[i].o_railGraphs[j], cleanGraphs);
					}

					for (int k = 0; k < cleanGraphs.size(); k++)
					{
						zObjNurbsCurve oCurve;
						makeCurveFromGraph(cleanGraphs[k], oCurve);

						if (j == 0)
							lowerCurves.push_back(oCurve);
						else if (j == myPanels[i].o_railGraphs.size() - 1)
							upperCurves.push_back(oCurve);
						//else
						//	intermediateCurves.push_back(oCurve);
					}
				}
			}

			cout << "Computing nurbs " << i << "/ " << myPanels.size() << endl;

			zFnGraph fnGraph(myPanels[i].o_boundaryGraph);
			if (fnGraph.numVertices() > 1)
			{
				zObjGraphArray cleanGraphs;
				cleanGraph(myPanels[i].o_boundaryGraph, cleanGraphs);

				for (int k = 0; k < cleanGraphs.size(); k++)
				{
					zObjNurbsCurve oCurve;
					makeCurveFromGraph(cleanGraphs[k], oCurve);
					boundaryCurves.push_back(oCurve);
				}
			}


			//cout << "numCurves:" << panelCurves.size() << endl;


			zPointArray pts_upper;
			zPointArray pts_bound;

			for (int j = 0; j < lowerCurves.size(); j++)
			{
				zObjNurbsCurve curve = lowerCurves[j];
				vector<zPlane> planes;
				computePlanes(curve, sampleDist, planes);

				zObjGraphArray verticalLines_upper;
				zObjGraphArray verticalLines_bound;

				zPointArray disP_upper;
				zPointArray disP_bound;

				if (planes.size() != 0 && upperCurves.size() != 0)
					computeVertical(planes, upperCurves, verticalLines_upper, disP_upper);
				if (planes.size() != 0 && boundaryCurves.size() != 0)
					computeVertical(planes, boundaryCurves, verticalLines_bound, disP_bound);

				//cout << i << ":verticalLines_upper:" << myPanels[i].verticalLines_upper.size() << endl;
				//cout << i << ":verticalLines_bound:" << myPanels[i].verticalLines_bound.size() << endl;
				// 
				//cout << "Computing nurbs " << i << "/ " << myPanels.size() << endl;
				//cout << "planesSize:" << planes.size() << endl;
				//cout << "lowerCurvesSize:" << lowerCurves.size() << endl;
				//cout << "disPSize:" << disP.size() << endl;

				//displayCurves.push_back(curve);
				for (auto& pl : planes)
				{
					zPoint O(pl(3, 0), pl(3, 1), pl(3, 2));
					zVector xAxis(pl(0, 0), pl(0, 1), pl(0, 2));
					zVector yAxis(pl(1, 0), pl(1, 1), pl(1, 2));
					zPoint X = O + xAxis;
					zPoint Y = O + yAxis;

					//displayPts.push_back(O);
					//displayPts.push_back(X);
					//displayPts.push_back(Y);
				}

				for (auto& p : disP_upper)
				{
					displayPts.push_back(p);
					pts_upper.push_back(p);
				}
				for (auto& p : disP_bound)
				{
					displayPts.push_back(p);
					pts_bound.push_back(p);
				}

			}
			
			myPanels[i].verticalLines_upper.assign(pts_upper.size() / 2, zObjGraph());
			myPanels[i].verticalLines_bound.assign(pts_bound.size() / 2, zObjGraph());

			if (pts_upper.size() > 0)
				for (int j = 0; j < pts_upper.size() -1; j+=2)
				{
					zFnGraph fn(myPanels[i].verticalLines_upper[j/2]);

					zPointArray pos;
					zIntArray pConnects;
					pos.push_back(pts_upper[j]);
					pos.push_back(pts_upper[j+1]);
					pConnects.push_back(0);
					pConnects.push_back(1);

					fn.create(pos, pConnects);
				}

			if (pts_bound.size() > 0)
				for (int j = 0; j < pts_bound.size() - 1; j += 2)
				{
					zFnGraph fn(myPanels[i].verticalLines_bound[j / 2]);

					zPointArray pos;
					zIntArray pConnects;
					pos.push_back(pts_bound[j]);
					pos.push_back(pts_bound[j + 1]);
					pConnects.push_back(0);
					pConnects.push_back(1);

					fn.create(pos, pConnects);
				}

			for (auto& curve : lowerCurves)
			{
				displayCurves.push_back(curve);
			}
			for (auto& curve : upperCurves)
			{
				displayCurves.push_back(curve);
			}
			for (auto& curve : boundaryCurves)
			{
				displayCurves.push_back(curve);
			}



			//cout << "numPlanes:" << planes.size() << endl;



			//cout << "numDisplayPts:" << displayPts.size() << endl;

		}
		compute_nurbs = !compute_nurbs;
	}

	if (exportSplitLines)
	{
		string DIR = "data/sanya/out/panelSplits/";

		bool chk_Dir = dirExists(DIR);
		if (!chk_Dir) _mkdir(DIR.c_str());

		string extension = ".json";
		zFileTpye type = zJSON;

		cout << endl << "Please wait. Alice is cleaning old files from the output folder...";
		filesystem::remove_all(DIR);
		cout << "Exporting new files...";

		for (int i = 0; i < displayVerticals.size(); i++)
		{
			string fileName = DIR + "panelSplit_" + to_string(i) + extension;

			zFnGraph fnGraph(displayVerticals[i]);
			fnGraph.to(fileName, type);
		}

		exportSplitLines = !exportSplitLines;
	}

	if (exportMeshes)
	{
		string DIR = "data/sanya/out/";

		string extension = ".json";
		zFileTpye type = zJSON;

		cout << endl << "Please wait. Alice is cleaning old files from the output folder...";
		filesystem::remove_all(DIR);
		cout << "Exporting new files...";

		bool chkDir = dirExists(DIR);
		if (!chkDir) _mkdir(DIR.c_str());

		for (int i = 0; i < myPanels.size(); i++)
		{
			string subDIR = DIR + "panel_" + to_string(i);

			bool chk_subDir = dirExists(subDIR);
			if (!chk_subDir) _mkdir(subDIR.c_str());
			
			for (const auto& entry : std::filesystem::directory_iterator(subDIR)) std::filesystem::remove_all(entry.path());

			for (int j = 0; j < myPanels[i].o_stripMeshes.size(); j++)
			{
				zFnMesh fnMesh(myPanels[i].o_stripMeshes[j]);

				if (fnMesh.numVertices() == 0) continue;

				string fileName = subDIR + "/strips_" + to_string(j) + extension;
				fnMesh.to(fileName, type);
			}

			for (int j = 0; j < myPanels[i].o_railGraphs.size(); j++)
			{
				zFnGraph fnGraph(myPanels[i].o_railGraphs[j]);

				if (fnGraph.numVertices() == 0) continue;

				string fileName = subDIR + "/rails_" + to_string(j) + extension;
				fnGraph.to(fileName, type);
			}

			for (int j = 0; j < myPanels[i].verticalLines_upper.size(); j++)
			{
				zFnGraph fnGraph(myPanels[i].verticalLines_upper[j]);

				if (fnGraph.numVertices() == 0) continue;

				string fileName = subDIR + "/vertical_upper_" + to_string(j) + extension;
				fnGraph.to(fileName, type);
			}

			for (int j = 0; j < myPanels[i].verticalLines_bound.size(); j++)
			{
				zFnGraph fnGraph(myPanels[i].verticalLines_bound[j]);

				if (fnGraph.numVertices() == 0) continue;

				string fileName = subDIR + "/vertical_bound_" + to_string(j) + extension;
				fnGraph.to(fileName, type);
			}
		}

		exportMeshes = !exportMeshes;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (d_inMesh)
	{
		o_inMesh.draw();		
	}
		
	
	if (d_MegaPanel)
	{

		if (d_AllPanels)
		{
			for (auto& m : myPanels)
			{
				m.o_panelMesh.setDisplayElements(false, false, true);
				m.o_panelMesh.draw();


			}
		}
		else
		{
			int i = currentPanelId;
			if (myPanels.size() > 0 )
			{
				myPanels[i].o_panelMesh.setDisplayElements(false, false, true);
				myPanels[i].o_panelMesh.draw();
			}

		}
	}

	if (d_MegaPanel_strips)
	{
		if (d_AllPanels)
		{
			for (auto& m : myPanels)
			{
				/*for (auto& strip : m.o_stripMeshes)
				{
					strip.setDisplayElements(false, false, true);
					strip.draw();
				}*/

				for (auto& strip : m.o_railGraphs)
				{
					strip.setDisplayElements(false, true);
					strip.draw();
				}

				m.o_boundaryGraph.draw();
			}
		}
		else
		{
			int i = currentPanelId;
			if (myPanels.size() > 0)
			{
				/*for (auto& strip : myPanels[i].o_stripMeshes)
				{
					strip.setDisplayElements(false, true, true);
					strip.draw();
				}*/

				for (auto& strip : myPanels[i].o_railGraphs)
				{
					strip.setDisplayElements(false, true);
					strip.draw();
				}

				myPanels[i].o_boundaryGraph.draw();
			}
		}

	}

	if (d_nurbs)
	{
		//for (auto& pt : displayPts)
		//{
		//	model.displayUtils.drawPoint(pt, zYELLOW, 5);
		//}

		if(displayPts.size()>0)
		for (int i = 0; i < displayPts.size() - 1; i+=2)
		{
			model.displayUtils.drawLine(displayPts[i], displayPts[i + 1], zYELLOW, 2);
		}

		//if (displayPts.size() > 0)
		//	for (int i = 0; i < displayPts.size(); i++)
		//		model.displayUtils.drawPoint(displayPts[i], zYELLOW, 4);


		for (auto& curve : displayCurves)
		{
			zFnNurbsCurve fn(curve);
			fn.setDisplayColor(zCYAN);
			curve.setDisplayElements(false, true);
			curve.draw();
		}


		//for (auto& graphs : verticalLines_upper)
		//{
		//	for (auto& graph : graphs)
		//	{
		//		graph.setDisplayElements(false, true);
		//		zFnGraph fnGraph(graph);
		//		fnGraph.setEdgeColor(zYELLOW);
		//		graph.draw();
		//	}
		//}

		//for (auto& graphs : verticalLines_bound)
		//{
		//	for (auto& graph : graphs)
		//	{
		//		graph.setDisplayElements(false, true);
		//		zFnGraph fnGraph(graph);
		//		fnGraph.setEdgeColor(zYELLOW);
		//		graph.draw();
		//	}
		//}
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Panels #:" + to_string(totalPanels), vec(winW - 350, winH - 800, 0));
	drawString("Current Panel #:" + to_string(currentPanelId), vec(winW - 350, winH - 775, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("p - compute geodesics", vec(winW - 350, winH - 625, 0));
	drawString("o - compute contours", vec(winW - 350, winH - 600, 0));
	drawString("n - compute nurbs", vec(winW - 350, winH - 575, 0));
	drawString("e - exportGeometry", vec(winW - 350, winH - 550, 0));
	

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute_heat = true;;
	if (k == 'o') compute_contour = true;;
	if (k == 'i') compute_planarity = true;;
	if (k == 'e') exportMeshes = true;
	if (k == 'q') exportSplitLines = true;
	if (k == 'n') compute_nurbs = true;

	if (k == 'w')
	{
		if (currentPanelId < myPanels.size() - 1)currentPanelId++;;
	}
	if (k == 's')
	{
		if (currentPanelId > 0)currentPanelId--;;
	}

	if (k == 'd') d_AllPanels = !d_AllPanels;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
