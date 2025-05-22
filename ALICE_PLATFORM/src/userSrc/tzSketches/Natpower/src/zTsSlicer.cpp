#include <userSrc/tzSketches/Natpower/include/zTsSlicer.h>
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>

#include <chrono>
using namespace std::chrono;


namespace zSpace
{
	zTsSlicer::zTsSlicer()
	{
	}

	zTsSlicer::~zTsSlicer()
	{
	}

	void zTsSlicer::clear()
	{
		blockMesh = zObjMesh();
		sectionMeshes.clear();;
		sectionGraphs.clear();

		unrolledMeshes.clear();
		unrolledGraphs.clear();
	}

	void zTsSlicer::compute_slice()
	{
#ifdef CLOCK_PERFORMANCE
		auto clockStart = high_resolution_clock::now();
#endif

		// slice
		slicer.slice(blockMesh,
			sectionMeshes,
			params.featuredNumStrides,
			params.medialIds,
			params.norm,
			params.spacing);

		numLayers = sectionMeshes.size();

		// unroll
		unrolledMeshes.assign(numLayers, zObjMesh());
		for (int i = 0; i < numLayers; i++)
		{
			unroller.unroll(sectionMeshes[i], unrolledMeshes[i]);
		}


		// make layers
		layers.assign(numLayers, zLayer());
		for (int i = 0; i < numLayers; i++)
		{
			layers[i].id = i;
			layers[i].sectionMesh = &sectionMeshes[i];
			layers[i].unrolledMesh = &unrolledMeshes[i];

		}

		// cable
		if (cableGraph != nullptr)
		{
			for (int i = 0; i < numLayers; i++)
			{
				//zPointArray vPos;
				//zFnMesh fnMesh(*layers[i].sectionMesh);
				//fnMesh.getVertexPositions(vPos);

				//zPlane bestFitPlane = core.getBestFitPlane(vPos);

				//slicer.cut(*cableGraph, bestFitPlane, *layers[i].cablePos);

				slicer.intersect_mesh_graph(*cableGraph, *layers[i].sectionMesh, *layers[i].sectionCablePos);
				//unroller.rollback(*layers[i].sectionCablePos, *layers[i].sectionMesh, unrolledMeshes[i], false);
				layers[i].setDisplay();
			}
		}

#ifdef CLOCK_PERFORMANCE
		auto clockEnd = high_resolution_clock::now();
		auto clockDuration = duration_cast<milliseconds>(clockEnd - clockStart);
		cout << "\n compute slice: " << clockDuration.count() << " milliseconds" << endl;
#endif

	}

	void zTsSlicer::compute_section(zObjGraph& sectionGraph)
	{
#ifdef CLOCK_PERFORMANCE
		auto clockStart = high_resolution_clock::now();
#endif

		sectionGraphs.assign(numLayers, zObjGraph(sectionGraph));
		unrolledGraphs.assign(numLayers, zObjGraph(sectionGraph));

		//int i = 0;
		zObjMesh normalisedBaseMesh(unrolledMeshes[0]);
		normaliseMesh(normalisedBaseMesh, 0, 4, zVector(1, 0, 0));

		for (int i = 0; i < numLayers; i++)
		{
			unroller.rollback(sectionGraphs[i], normalisedBaseMesh, sectionMeshes[i], true);

			unroller.rollback(unrolledGraphs[i], normalisedBaseMesh, unrolledMeshes[i], false);
		}

		for (int i = 0; i < numLayers; i++)
		{
			layers[i].sectionGraph = &sectionGraphs[i];
			layers[i].unrolledGraph = &unrolledGraphs[i];

			layers[i].setDisplay();

		}

#ifdef CLOCK_PERFORMANCE
		auto clockEnd = high_resolution_clock::now();
		auto clockDuration = duration_cast<milliseconds>(clockEnd - clockStart);
		cout << "\n compute section: " << clockDuration.count() << " milliseconds" << endl;
#endif

	}


	void zTsSlicer::exportTo(string folder)
	{
		for (size_t i = 0; i < unrolledMeshes.size(); i++)
		{
			string file = folder + "/slicer_result_" + to_string(i) + ".usda";
			zFnMesh fnMesh(unrolledMeshes[i]);

			fnMesh.to(file,zUSD);
		}

	}

	void zTsSlicer::initialise(zObjMesh& _blockMesh, zTsSlicerParams _params, zObjGraph* _cableGraph)
	{
		blockMesh = _blockMesh;
		params = _params;
		cableGraph = _cableGraph;

		blockMesh.setDisplayElements(false, true, false);
	}

	void zTsSlicer::initialise(string blockMeshPath, zTsSlicerParams _params, zObjGraph* _cableGraph)
	{
		zFnMesh fnMesh(blockMesh);
		fnMesh.from(blockMeshPath, zJSON);
		params = _params;
		cableGraph = _cableGraph;

		blockMesh.setDisplayElements(false, true, false);
	}

	void zTsSlicer::draw(bool d_block, bool d_section, bool d_unroll)
	{
		if (d_block)
		{
			blockMesh.draw();
		}

		if (d_section)
		{
			for (auto& layer : layers)
			{
				layer.drawSectionMesh();
				layer.drawSectionGraph();
			}
		}

		if (d_unroll)
		{
			for (auto& layer : layers)
			{
				layer.drawUnrolledMesh();
			}
		}
	}

	// helper

	bool zTsSlicer::normaliseMesh(zObjMesh& mesh, int vId0, int vId1, zVector alignVec)
	{
		alignVec.normalize();
		zVector dirX = alignVec;
		zVector dirY = zVector(0, 0, 1) ^ alignVec;

		zItMeshHalfEdge heX;
		zFnMesh fn(mesh);
		bool found = fn.halfEdgeExists(vId0, vId1, heX);

		if (found && heX.onBoundary())
		{
			zPointArray pos;
			fn.getVertexPositions(pos);

			zItMeshHalfEdge heY1 = heX.getPrev();
			zItMeshHalfEdge heY2 = heX.getNext();
			zIntArray row1;
			zIntArray row2;

			row1.push_back(vId0);
			do
			{
				row1.push_back(heY1.getStartVertex().getId());
				heY1 = heY1.getPrev();
			} while (!heY1.getStartVertex().checkValency(3));
			row1.push_back(heY1.getStartVertex().getId());

			row2.push_back(vId1);
			do
			{
				row2.push_back(heY2.getVertex().getId());
				heY2 = heY2.getNext();
			} while (!heY2.getVertex().checkValency(3));
			row2.push_back(heY2.getVertex().getId());


			for (int i = 0; i < row1.size(); i++)
			{
				pos[row1[i]] = zVector(0, 0, 0) + dirY * ((float)i / ((float)row1.size() - 1));
				pos[row2[i]] = pos[row1[i]] + dirX;
			}

			for (zItMeshVertex v(mesh); !v.end(); v++)
			{
				if (!v.onBoundary())
				{
					zIntArray cvs;
					v.getConnectedVertices(cvs);

					pos[v.getId()] = zVector(0, 0, 0);
					for (int i = 0; i < cvs.size(); i++)
					{
						pos[v.getId()] += pos[cvs[i]];
					}
					pos[v.getId()] /= cvs.size();
				}
			}

			zIntArray pConnects;
			zIntArray pCounts;
			fn.getPolygonData(pConnects,pCounts);

			fn.create(pos, pCounts, pConnects);

			return true;
		}
		else return false;
	}

}