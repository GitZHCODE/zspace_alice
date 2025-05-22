#include <vector>

#include <userSrc/tzSketches/Natpower/include/zUnroller.h>

namespace zSpace
{
	zUnroller::zUnroller()
	{
	}

	zUnroller::~zUnroller()
	{

	}

	bool zUnroller::unroll(zObjMesh& oMesh, zObjMesh& result)
	{
		zObjMesh oMesh_tri;
		getPokeMesh(oMesh, oMesh_tri);
		zObjGraph oDualGraph;
		zObjMesh oFlattenedMesh;

		zInt2DArray oriVertex_UnrollVertex_map;
		unordered_map<zIntPair, int, zPair_hash> oriFaceVertex_UnrollVertex;
		zItGraphVertexArray bsf_Vertices;
		zIntPairArray bsf_vertexPairs;

		creatUnrollMesh(oMesh_tri, oFlattenedMesh, oDualGraph, oriVertex_UnrollVertex_map, oriFaceVertex_UnrollVertex, bsf_Vertices, bsf_vertexPairs);
		int num_BSF = bsf_Vertices.size();

		unrollMesh(oMesh_tri, oFlattenedMesh, oDualGraph, oriVertex_UnrollVertex_map, oriFaceVertex_UnrollVertex, bsf_vertexPairs);
		mergeMesh(oFlattenedMesh);

		bool success = false;
		zFnMesh fnMesh(oFlattenedMesh);
		if (fnMesh.numVertices() != 0 && fnMesh.numEdges() != 0)
			success = true;

		if (success)
			result = oFlattenedMesh;
		else
			result = zObjMesh();
	}

	bool zUnroller::rollback(zObjGraph& sectionGraph, zObjMesh& refMesh, zObjMesh& targetMesh, bool poke)
	{
		zObjMesh oMesh_tri;

		if (poke)
			getPokeMesh(targetMesh, oMesh_tri);
		else
			oMesh_tri = zObjMesh(targetMesh);

		zFnGraph fnGraph(sectionGraph);
		int oriNumVertices = fnGraph.numVertices();
		int oriNumEdge = fnGraph.numEdges();

		barycentericProjection_triMesh(sectionGraph, refMesh, oMesh_tri);

		if (fnGraph.numVertices() == oriNumVertices && fnGraph.numEdges() == oriNumEdge)
			return true;
		else
			return false;

	}

	bool zUnroller::rollback(zPoint& point, zObjMesh& refMesh, zObjMesh& targetMesh, bool poke)
	{
		zObjMesh oMesh_tri;

		if (poke)
			getPokeMesh(targetMesh, oMesh_tri);
		else
			oMesh_tri = zObjMesh(targetMesh);

		barycentericProjection_triMesh(point, refMesh, oMesh_tri);

		return true;
	}


	// helper function

	// unroller
	void zUnroller::creatUnrollMesh(zObjMesh& o_mesh, zObjMesh& o_mesh_unroll, zObjGraph& o_dualgraph, zInt2DArray& oriVertex_UnrollVertex_map, unordered_map<zIntPair, int, zPair_hash>& oriFaceVertex_UnrollVertex, zItGraphVertexArray& bsf_Vertices, zIntPairArray& bsf_vertexPairs)
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

	void zUnroller::unrollMesh(zObjMesh& o_mesh, zObjMesh& o_mesh_unroll, zObjGraph& o_dualgraph, zInt2DArray& oriVertex_UnrollVertex_map, unordered_map<zIntPair, int, zPair_hash>& oriFaceVertex_UnrollVertex, zIntPairArray& bsf_vertexPairs)
	{
		zFnMesh fnMesh(o_mesh);
		zPoint* vPositions = fnMesh.getRawVertexPositions();

		zFnMesh fnMesh_unroll(o_mesh_unroll);
		zPoint* vPositions_unroll = fnMesh_unroll.getRawVertexPositions();

		/*for (int i = 0; i < oriVertex_UnrollVertex_map.size(); i++)
		{
			zPoint p = vPositions[i];

			for (auto vID : oriVertex_UnrollVertex_map[i])
			{
				vPositions_unroll[vID] = p;
			}
		}*/

		// unroll 
		// https://computergraphics.stackexchange.com/questions/8774/unfold-a-3d-mesh-to-a-2d-plane

		for (int i = 0; i < bsf_vertexPairs.size(); i++)
		{
			zItMeshFace f1(o_mesh, bsf_vertexPairs[i].first);
			zItMeshFace f2(o_mesh, bsf_vertexPairs[i].second);

			zItMeshFace f1_unroll(o_mesh_unroll, bsf_vertexPairs[i].first);
			zItMeshFace f2_unroll(o_mesh_unroll, bsf_vertexPairs[i].second);

			zIntPair hePair = getCommonEdge(f1, f2);

			zItMeshHalfEdge he_1(o_mesh, hePair.first);
			zItMeshHalfEdge he_2(o_mesh, hePair.second);

			zPoint A = vPositions[he_2.getStartVertex().getId()];
			zPoint B = vPositions[he_2.getVertex().getId()];

			// unroll first face
			if (i == 0)
			{
				zItMeshHalfEdge he_walker_1 = he_1;
				int f1_numV = f1.getNumVertices();

				float l_ab = he_1.getLength();

				zPoint a(2, 0, 0);
				zPoint b(2, l_ab, 0);

				// update  postions of corresponding a & b in unroll mesh
				zIntPair hashKey_a(f1.getId(), he_1.getStartVertex().getId());
				std::unordered_map<zIntPair, int>::const_iterator got_a = oriFaceVertex_UnrollVertex.find(hashKey_a);
				if (got_a != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_a->second] = a;

				zIntPair hashKey_b(f1.getId(), he_1.getVertex().getId());
				std::unordered_map<zIntPair, int>::const_iterator got_b = oriFaceVertex_UnrollVertex.find(hashKey_b);
				if (got_b != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_b->second] = b;

				for (int j = 0; j < f1_numV; j++)
				{
					he_walker_1 = he_walker_1.getNext();
					zPoint C = vPositions[he_walker_1.getVertex().getId()];

					zVector ca = C - A;
					zVector ba = B - A;;

					float s = ((ba ^ ca).length()) / (l_ab * l_ab);
					float c = (ba * ca) / (l_ab * l_ab);

					// alternate point
					/*zPoint c1;
					c1.x = a.x + c * (b.x - a.x) - s * (b.y - a.y);
					c1.y = a.y + c * (b.y - a.y) + s * (b.x - a.x);
					c1.z = 0;*/

					zPoint c1;
					c1.x = a.x + c * (b.x - a.x) + s * (b.y - a.y);
					c1.y = a.y + c * (b.y - a.y) - s * (b.x - a.x);
					c1.z = 0;

					// update  postions of corresponding a & b in unroll mesh
					zIntPair hashKey_c(f1.getId(), he_walker_1.getVertex().getId());
					std::unordered_map<zIntPair, int>::const_iterator got_c = oriFaceVertex_UnrollVertex.find(hashKey_c);
					if (got_c != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_c->second] = c1;
				}

			}





			zItMeshHalfEdge he_walker_2 = he_2;
			int f2_numV = f2.getNumVertices();

			// get positions of the prev edge unrolled.
			zIntPair hashKey_a_prev(f1.getId(), he_2.getStartVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_a_prev = oriFaceVertex_UnrollVertex.find(hashKey_a_prev);
			zPoint a = vPositions_unroll[got_a_prev->second];

			zIntPair hashKey_b_prev(f1.getId(), he_2.getVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_b_prev = oriFaceVertex_UnrollVertex.find(hashKey_b_prev);
			zPoint b = vPositions_unroll[got_b_prev->second];

			// update  postions of corresponding a & b in unroll mesh
			zIntPair hashKey_a(f2.getId(), he_2.getStartVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_a = oriFaceVertex_UnrollVertex.find(hashKey_a);
			if (got_a != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_a->second] = a;

			zIntPair hashKey_b(f2.getId(), he_2.getVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_b = oriFaceVertex_UnrollVertex.find(hashKey_b);
			if (got_b != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_b->second] = b;


			for (int j = 0; j < f2_numV; j++)
			{
				he_walker_2 = he_walker_2.getNext();
				zPoint C = vPositions[he_walker_2.getVertex().getId()];

				float l_ab = he_2.getLength();

				zVector ca = C - A;
				zVector ba = B - A;;

				float s = ((ba ^ ca).length()) / (l_ab * l_ab);
				float c = (ba * ca) / (l_ab * l_ab);

				zPoint c1;
				c1.x = a.x + c * (b.x - a.x) - s * (b.y - a.y);
				c1.y = a.y + c * (b.y - a.y) + s * (b.x - a.x);
				c1.z = 0;

				// update  postions of corresponding a & b in unroll mesh
				zIntPair hashKey_c(f2.getId(), he_walker_2.getVertex().getId());
				std::unordered_map<zIntPair, int>::const_iterator got_c = oriFaceVertex_UnrollVertex.find(hashKey_c);
				if (got_c != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_c->second] = c1;

			}


		}

	}

	void zUnroller::mergeMesh(zObjMesh& o_mesh)
	{
		zObjMesh oTmpMesh = o_mesh;

		zPointArray positions;
		zIntArray pCounts, pConnects;

		for (zItMeshFace f(o_mesh); !f.end(); f++)
		{
			zPointArray fVPositions;
			f.getVertexPositions(fVPositions);

			for (auto& p : fVPositions)
			{

				int id = -1;
				if (!core.checkRepeatVector(p, positions, id))
				{
					id = positions.size();
					positions.push_back(p);
				}

				pConnects.push_back(id);
			}

			pCounts.push_back(fVPositions.size());

		}

		zFnMesh fnMesh(o_mesh);
		fnMesh.clear();
		fnMesh.create(positions, pCounts, pConnects);

	}

	void zUnroller::getPokeMesh(zObjMesh& o_mesh, zObjMesh& o_TriMesh)
	{
		zFnMesh fnMesh(o_mesh);


		zPointArray vPositions;
		fnMesh.getVertexPositions(vPositions);

		zPointArray fCens;
		fnMesh.getCenters(zHEData::zFaceData, fCens);

		zPointArray positions;
		zIntArray pCounts, pConnects;

		positions.insert(positions.end(), vPositions.begin(), vPositions.end());
		positions.insert(positions.end(), fCens.begin(), fCens.end());

		int numOriginalVerts = vPositions.size();

		for (zItMeshFace f(o_mesh); !f.end(); f++)
		{
			zIntArray fVerts;
			f.getVertices(fVerts);

			int fID = f.getId();

			for (int i = 0; i < fVerts.size(); i++)
			{
				int nextID = (i + 1) % fVerts.size();

				pConnects.push_back(fVerts[i]);
				pConnects.push_back(fVerts[nextID]);
				pConnects.push_back(numOriginalVerts + fID);

				pCounts.push_back(3);
			}
		}

		zFnMesh fnPokeMesh(o_TriMesh);
		fnPokeMesh.create(positions, pCounts, pConnects);
	}

	zIntPair zUnroller::getCommonEdge(zItMeshFace& f1, zItMeshFace& f2)
	{
		zIntPair out;

		zItMeshHalfEdgeArray f1_HEdges;
		f1.getHalfEdges(f1_HEdges);

		zItMeshHalfEdgeArray f2_HEdges;
		f2.getHalfEdges(f2_HEdges);


		for (auto& f1HE : f1_HEdges)
		{
			for (auto& f2HE : f2_HEdges)
			{
				if (f1HE.getEdge().getId() == f2HE.getEdge().getId())
				{
					out = zIntPair(f1HE.getId(), f2HE.getId());
					break;
				}
			}
		}

		return out;
	}

	void zUnroller::computeDualGraph_BST(zObjMesh& o_mesh, zObjGraph& o_graph, zItGraphVertexArray& bsf_Vertices, zIntPairArray& bsf_vertexPairs)
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

	// roll back

	void zUnroller::barycentericProjection_triMesh(zObjGraph& o_graph, zObjMesh& o_inMesh, zObjMesh& o_projectionMesh)
	{
		zFnGraph fnGraph(o_graph);

		zPointArray positions;
		fnGraph.getVertexPositions(positions);

		int vcount = 0;
		for (auto& p : positions)
		{
			for (zItMeshFace f(o_inMesh); !f.end(); f++)
			{
				zPointArray fVerts;
				f.getVertexPositions(fVerts);

				if (core.pointInTriangle(p, fVerts[0], fVerts[1], fVerts[2]))
				{
					zPoint baryCoordinates;
					getBaryCentricCoordinates_triangle(p, fVerts[0], fVerts[1], fVerts[2], baryCoordinates);

					//cout << "\n current v & f id: " << vcount << "," << f.getId();
					zItMeshFace fProjection(o_projectionMesh, f.getId());

					zPointArray fVerts_projection;
					fProjection.getVertexPositions(fVerts_projection);

					zPoint projectionPt;
					getProjectionPoint_triangle(baryCoordinates, fVerts_projection[0], fVerts_projection[1], fVerts_projection[2], projectionPt);

					p = projectionPt;

					break;
				}
			}
			vcount++;
		}

		fnGraph.setVertexPositions(positions);
	}

	void zUnroller::barycentericProjection_triMesh(zPoint& point, zObjMesh& o_inMesh, zObjMesh& o_projectionMesh)
	{
		for (zItMeshFace f(o_inMesh); !f.end(); f++)
		{
			zPointArray fVerts;
			f.getVertexPositions(fVerts);

			if (core.pointInTriangle(point, fVerts[0], fVerts[1], fVerts[2]))
			{
				zPoint baryCoordinates;
				getBaryCentricCoordinates_triangle(point, fVerts[0], fVerts[1], fVerts[2], baryCoordinates);

				//cout << "\n current v & f id: " << vcount << "," << f.getId();
				zItMeshFace fProjection(o_projectionMesh, f.getId());

				zPointArray fVerts_projection;
				fProjection.getVertexPositions(fVerts_projection);

				zPoint projectionPt;
				getProjectionPoint_triangle(baryCoordinates, fVerts_projection[0], fVerts_projection[1], fVerts_projection[2], projectionPt);

				point = projectionPt;

				break;
			}
		}
	}

	void zUnroller::getBaryCentricCoordinates_triangle(zPoint& pt, zPoint& t0, zPoint& t1, zPoint& t2, zPoint& baryCoordinates)
	{
		zVector v0 = t1 - t0;
		zVector v1 = t2 - t0;
		zVector v2 = pt - t0;

		float d00 = v0 * (v0);
		float d01 = v0 * (v1);
		float d11 = v1 * (v1);
		float d20 = v2 * (v0);
		float d21 = v2 * (v1);

		float denom = d00 * d11 - d01 * d01;

		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0 - v - w;

		baryCoordinates = zPoint(u, v, w);

	}

	void zUnroller::getProjectionPoint_triangle(zPoint& baryCoordinates, zPoint& t0, zPoint& t1, zPoint& t2, zPoint& projectionPt)
	{
		projectionPt = t0 * baryCoordinates.x + t1 * baryCoordinates.y + t2 * baryCoordinates.z;
	}

}