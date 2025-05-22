#include <userSrc/tzSketches/Natpower/include/zSlicer.h>
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>

namespace zSpace
{
	// constructor
	zSlicer::zSlicer()
	{
	}

	zSlicer::~zSlicer()
	{
	}

	// compute
	bool zSlicer::slice(zObjMesh& sliceMesh, zObjMeshArray& oSectionMeshes, zIntArray featuredNumStrides, zIntArray medialIDS, zVector norm, float spacing)
	{
		bool success = false;

		if (featuredNumStrides.size() != 0 && medialIDS.size() != 0 && spacing != 0.0f)
		{
			oSectionMeshes.clear();

			zObjMesh oMesh_top, oMesh_bottom;
			vector<zItMeshHalfEdgeArray> vLoops;

			computeVLoops(sliceMesh, medialIDS, featuredNumStrides, norm, vLoops, oMesh_top, oMesh_bottom);

			zScalarArray scalars;
			computeGeodesicScalars(sliceMesh, vLoops, scalars, true);

			computeGeodesicContours(vLoops, scalars, spacing, oMesh_top, oMesh_bottom, oSectionMeshes);

			if (oSectionMeshes.size() > 0) success = true;
		}

		return success;
	}

	bool zSlicer::intersect_plane_graph(zObjGraph& sliceGraph, zPlane& slicePlane, zPoint& outPt)
	{
		bool success = false;
		zPoint intersectPt;

		zVector planeNormal(slicePlane(2, 0), slicePlane(2, 1), slicePlane(2, 2));
		zVector planeOrigin(slicePlane(3, 0), slicePlane(3, 1), slicePlane(3, 2));

		for (zItGraphEdge e(sliceGraph); !e.end(); e++)
		{
			zPointArray vPos;
			e.getVertexPositions(vPos);

			success = core.line_PlaneIntersection(vPos[0], vPos[1], planeNormal, planeOrigin, intersectPt);

			if (success) break;
		}
		outPt = intersectPt;
		return success;
	}

	bool zSlicer::intersect_mesh_graph(zObjGraph& sliceGraph, zObjMesh& sliceMesh, zPoint& outPt)
	{
			bool success = false;
		zPoint intersectPt;

		for (zItMeshFace f(sliceMesh); !f.end(); f++)
		{
			zPointArray triPos;
			f.getVertexPositions(triPos);
			for (zItGraphEdge e(sliceGraph); !e.end(); e++)
			{
				zPointArray vPos;
				e.getVertexPositions(vPos);

				zPoint pt;
				bool chk1 = core.ray_triangleIntersection(triPos[0], triPos[1], triPos[2], vPos[1] - vPos[0], vPos[0], pt);
				bool chk2 = (vPos[0] - triPos[0]) * (vPos[1] - triPos[0]) < 0 ? true : false;

				if (chk1 && chk2)
				{
					intersectPt = pt;
					success = true;
					break;
				}
			}
			outPt = intersectPt;
		}
		return success;
	}



	// compute
	void zSlicer::computeVLoops(zObjMesh& oMesh, zIntArray& medialIDS, zIntArray& featuredNumStrides, zVector& norm, vector<zItMeshHalfEdgeArray>& v_Loops, zObjMesh& oMesh_top, zObjMesh& oMesh_bottom)
	{

		int stride = featuredNumStrides[0];
		int startVID = medialIDS[0];
		int endVID = medialIDS[1];

		zItMeshVertex vStart(oMesh, startVID);
		zItMeshVertex vEnd(oMesh, endVID);

		zVector dir = vEnd.getPosition() - vStart.getPosition();

		// numFrames = ceil((vEnd.getPosition().z - vStart.getPosition().z) / spacing) + 1;
		// printf("\n numFrames %i", numFrames);

		zItMeshHalfEdgeArray hEdges_Start;
		vStart.getConnectedHalfEdges(hEdges_Start);

		float ang = 10000;
		zItMeshHalfEdge heStart;

		for (auto& he : hEdges_Start)
		{
			if (he.getVector().angle(dir) < ang)
			{
				ang = he.getVector().angle(dir);
				heStart = he;
			}
		}

		zItMeshHalfEdge he = heStart;
		norm.normalize();

		zItMeshHalfEdge he_Bottom, he_Top;
		int VCounter = 0;
		int tempCounter = 0;
		do
		{
			zVector fNorm = he.getFace().getNormal();
			fNorm.normalize();

			if (norm * fNorm > 0.98)
			{
				VCounter = tempCounter;
				he_Top = he;
			}

			if (norm * fNorm < -0.98)
			{
				he_Bottom = he;
			}

			he = he.getNext().getSym().getNext();
			tempCounter++;

		} while (he != heStart);

		printf("\n VCounter %i ", VCounter);

		zPointArray positions_top, positions_bottom;
		zIntArray pCounts_top, pCounts_bottom;
		zIntArray pConnects_top, pConnects_bottom;

		zIntArray pMap_bottom, pMap_top;

		zFnMesh fnMesh_in(oMesh);

		pMap_bottom.assign(fnMesh_in.numVertices(), -1);
		pMap_top.assign(fnMesh_in.numVertices(), -1);

		bool corner = true;

		for (int i = 0; i < stride; i++)
		{
			he_Top = he_Top.getNext().getNext();
			he_Bottom = he_Bottom.getNext().getNext();

			if ((i + 1) % stride != 0)
			{
				he_Bottom = he_Bottom.getSym();
				he_Top = he_Top.getSym();
			}
		}

		zItMeshHalfEdge heWalk_Bottom = he_Bottom;
		int walkCounter = 0;
		int loopCounter;
		do
		{
			if (corner)
			{
				loopCounter = v_Loops.size();
				getLoop(heWalk_Bottom, true, corner, VCounter, v_Loops);
				corner = false;

				pMap_bottom[v_Loops[loopCounter][0].getVertex().getId()] = positions_bottom.size();
				positions_bottom.push_back(v_Loops[loopCounter][0].getVertex().getPosition());

				pMap_top[v_Loops[loopCounter][v_Loops[loopCounter].size() - 1].getStartVertex().getId()] = positions_top.size();
				positions_top.push_back(v_Loops[loopCounter][v_Loops[loopCounter].size() - 1].getStartVertex().getPosition());
			}

			loopCounter = v_Loops.size();

			// zItMeshHalfEdge he = heWalk.getNext();
			getLoop(heWalk_Bottom, true, corner, VCounter, v_Loops);
			// he.getEdge().setColor(zBLUE);

			pMap_bottom[v_Loops[loopCounter][0].getVertex().getId()] = positions_bottom.size();
			positions_bottom.push_back(v_Loops[loopCounter][0].getVertex().getPosition());

			pMap_top[v_Loops[loopCounter][v_Loops[loopCounter].size() - 1].getStartVertex().getId()] = positions_top.size();
			positions_top.push_back(v_Loops[loopCounter][v_Loops[loopCounter].size() - 1].getStartVertex().getPosition());

			heWalk_Bottom = heWalk_Bottom.getNext().getNext();

			if ((walkCounter + 1) % (2 * stride) != 0)
				heWalk_Bottom = heWalk_Bottom.getSym();
			else
				corner = true;
			walkCounter++;

		} while (heWalk_Bottom != he_Bottom);

		// create meshes

		for (int i = 0; i < stride * 2; i++)
		{

			zIntArray fVerts;
			getFaceVerticesFromHalfedge(he_Bottom, true, fVerts);
			for (auto& id : fVerts)
				pConnects_bottom.push_back(pMap_bottom[id]);
			pCounts_bottom.push_back(fVerts.size());

			fVerts.clear();
			getFaceVerticesFromHalfedge(he_Top, true, fVerts);
			for (auto& id : fVerts)
				pConnects_top.push_back(pMap_top[id]);
			pCounts_top.push_back(fVerts.size());

			he_Bottom = he_Bottom.getNext().getNext().getSym();
			he_Top = he_Top.getNext().getNext().getSym();
		}

		zFnMesh fnTop(oMesh_top);
		fnTop.create(positions_top, pCounts_top, pConnects_top);

		zFnMesh fnBottom(oMesh_bottom);
		fnBottom.create(positions_bottom, pCounts_bottom, pConnects_bottom);
	}

	void zSlicer::computeGeodesicScalars(zObjMesh& oMesh, vector<zItMeshHalfEdgeArray>& v_Loops, zScalarArray& scalars, bool normalise)
	{
		zFnMesh fnMesh(oMesh);

		scalars.clear();
		scalars.assign(fnMesh.numVertices(), -1);

		float minMaxDist = 10000;
		vector<zDomainFloat> loopDomains;
		loopDomains.assign(v_Loops.size(), zDomainFloat(10000, -10000));

		for (int l = 0; l < v_Loops.size(); l++)
		{
			float length = 0;
			for (int j = 0; j < v_Loops[l].size(); j++)
			{
				if (j == 0)
				{
					scalars[v_Loops[l][j].getVertex().getId()] = length;
					loopDomains[l].min = length;
				}

				length += v_Loops[l][j].getLength();
				scalars[v_Loops[l][j].getStartVertex().getId()] = length;

				if (j == v_Loops[l].size() - 1 && length < minMaxDist)
					minMaxDist = length;

				if (length > loopDomains[l].max)
					loopDomains[l].max = length;
			}
		}

		if (normalise)
		{
			zDomainFloat outDomain(0, minMaxDist);
			for (int l = 0; l < v_Loops.size(); l++)
			{
				for (int j = 0; j < v_Loops[l].size(); j++)
				{
					scalars[v_Loops[l][j].getStartVertex().getId()] = core.ofMap(scalars[v_Loops[l][j].getStartVertex().getId()], loopDomains[l], outDomain);
				}
			}
		}

		colorMesh(oMesh, scalars);

		zScalar minScalar = core.zMin(scalars);
		zScalar maxScalar = core.zMax(scalars);

		zColor* mesh_vColors = fnMesh.getRawVertexColors();

		zDomainFloat distanceDomain(minScalar, maxScalar);
		printf("\n scalar domain %1.2f %1.2f | minMaxDist %1.2f ", minScalar, maxScalar, minMaxDist);

		zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
		}

		fnMesh.computeFaceColorfromVertexColor();
	}

	void zSlicer::computeGeodesicContours(vector<zItMeshHalfEdgeArray>& v_Loops, zScalarArray& scalars, float spacing, zObjMesh& oMesh_top, zObjMesh& oMesh_bottom, zObjMeshArray& oMeshes)
	{

		zScalar minScalar = core.zMin(scalars);
		zScalar maxScalar = core.zMax(scalars);

		int totalContours = ceil((maxScalar - minScalar) / spacing);
		float increments = (maxScalar - minScalar) / totalContours;

		oMeshes.clear();
		oMeshes.assign(totalContours, oMesh_bottom);

		for (int l = 0; l < totalContours; l++)
		{
			float threshold = l * increments;

			zFnMesh fnMesh(oMeshes[l]);
			zPoint* points = fnMesh.getRawVertexPositions();

			for (int i = 0; i < v_Loops.size(); i++)
			{
				for (int j = 0; j < v_Loops[i].size(); j++)
				{
					float s0 = scalars[v_Loops[i][j].getStartVertex().getId()];
					float s1 = scalars[v_Loops[i][j].getVertex().getId()];

					bool contour = false;
					if (s0 <= threshold && s1 >= threshold)
						contour = true;
					if (s0 >= threshold && s1 <= threshold)
						contour = true;

					if (!contour)
						continue;

					zPoint v0 = v_Loops[i][j].getStartVertex().getPosition();
					zPoint v1 = v_Loops[i][j].getVertex().getPosition();

					zPoint pos1 = getContourPosition(threshold, v1, v0, s1, s0);
					points[i] = (pos1);
				}
			}
		}

		oMeshes.push_back(oMesh_top);

		/*for (int l = 0; l < oMeshes.size(); l++ )
		{
			zFnMesh fnMesh(oMeshes[l]);
			fnMesh.to("data/Natpower/test/temp_" + to_string(l) + ".json", zJSON);
		}*/
	}

	void zSlicer::createSectionGraphs(zObjMeshArray& oMeshes, zObjGraphArray& o_sectionsGraphs)
	{
		o_sectionsGraphs.clear();
		o_sectionsGraphs.assign(oMeshes.size(), zObjGraph());

		int counter = 0;
		for (auto& oMesh : oMeshes)
		{
			createBoundaryEdgeGraph(oMesh, true, o_sectionsGraphs[counter]);

			zFnGraph fnGraph(o_sectionsGraphs[counter]);
			fnGraph.setEdgeColor(zGREEN);
			fnGraph.setEdgeWeight(3);

			counter++;
		}
	}

	// helper function
	void zSlicer::getLoop(zItMeshHalfEdge& heStart, bool forward, bool corner, int vCounter, vector<zItMeshHalfEdgeArray>& v_Loops)
	{
		zItMeshHalfEdge he_U = (forward) ? heStart.getNext() : heStart.getPrev();
		if (corner)
			he_U = heStart;

		bool exit_1 = false;

		zItMeshHalfEdge he_V = (forward) ? he_U.getSym().getNext() : he_U.getSym().getPrev();

		zItMeshHalfEdgeArray tempV;

		bool exit_2 = false;

		for (int i = 0; i < vCounter; i++)
		{
			if (forward)
				tempV.push_back(he_V.getSym());
			else
				tempV.push_back(he_V);

			he_V.getEdge().setColor(zBLUE);

			if (!exit_2)
				he_V = (forward) ? he_V.getNext().getSym().getNext() : he_V.getPrev().getSym().getPrev();
		}

		v_Loops.push_back(tempV);

		he_U.getEdge().setColor(zMAGENTA);
	}

	void zSlicer::getFaceVerticesFromHalfedge(zItMeshHalfEdge& heStart, bool forward, zPointArray& fVerts)
	{
		fVerts.clear();

		zItMeshHalfEdge he = heStart;

		do
		{
			if (forward)
			{
				fVerts.push_back(he.getVertex().getPosition());
				he = he.getNext();
			}
			else
			{
				fVerts.push_back(he.getStartVertex().getPosition());
				he = he.getPrev();
			}

		} while (he != heStart);
	}

	void zSlicer::getFaceVerticesFromHalfedge(zItMeshHalfEdge& heStart, bool forward, zIntArray& fVerts)
	{
		fVerts.clear();

		zItMeshHalfEdge he = heStart;

		do
		{
			if (forward)
			{
				fVerts.push_back(he.getVertex().getId());
				he = he.getNext();
			}
			else
			{
				fVerts.push_back(he.getStartVertex().getId());
				he = he.getPrev();
			}

		} while (he != heStart);
	}

	zPoint zSlicer::getContourPosition(float& threshold, zVector& vertex_lower, zVector& vertex_higher, float& thresholdLow, float& thresholdHigh)
	{
		float scaleVal = core.ofMap(threshold, thresholdLow, thresholdHigh, 0.0000f, 1.0000f);

		zVector e = vertex_higher - vertex_lower;
		double edgeLen = e.length();
		e.normalize();

		return (vertex_lower + (e * edgeLen * scaleVal));
	}

	void zSlicer::colorMesh(zObjMesh& o_mesh, zFloatArray& scalars)
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

	void zSlicer::createBoundaryEdgeGraph(zObjMesh& o_mesh, bool closeGraph, zObjGraph& o_Graph)
	{
		zPointArray positions;
		zIntArray eConnects;
		zColorArray vColors;

		zItMeshHalfEdge he;

		for (zItMeshHalfEdge tmpHE(o_mesh); !tmpHE.end(); tmpHE++)
		{
			if (tmpHE.onBoundary())
			{
				he = tmpHE;
				break;
			}
		}

		zItMeshHalfEdge startHE = he;
		positions.push_back(he.getStartVertex().getPosition());
		vColors.push_back(he.getStartVertex().getColor());

		do
		{
			positions.push_back(he.getVertex().getPosition());
			vColors.push_back(he.getVertex().getColor());

			eConnects.push_back(positions.size() - 2);
			eConnects.push_back(positions.size() - 1);

			he = he.getNext();

		} while (he != startHE);

		if (closeGraph)
		{
			eConnects.push_back(positions.size() - 1);
			eConnects.push_back(0);
		}

		zFnGraph fnGraph(o_Graph);
		fnGraph.create(positions, eConnects);

		fnGraph.setVertexColors(vColors);
	}
}