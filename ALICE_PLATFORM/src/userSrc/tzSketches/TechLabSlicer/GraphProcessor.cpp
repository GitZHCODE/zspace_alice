
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <stack>

using namespace std;

namespace zSpace
{
	class GraphProcessor
	{
	public:
		GraphProcessor() {};
		~GraphProcessor() {};

		vector<zObjGraph> separateGraph(zObjGraph& inputGraph)
		{
			vector<zObjGraph> separated_graphs;
			zFnGraph fnGraph(inputGraph);

			// If graph is empty, return empty vector
			if (fnGraph.numVertices() == 0) return separated_graphs;

			// Track visited vertices
			vector<bool> visited(fnGraph.numVertices(), false);

			// Process each unvisited vertex to find components
			for (int startVertex = 0; startVertex < fnGraph.numVertices(); startVertex++)
			{
				if (visited[startVertex]) continue;

				// Initialize new component
				vector<zVector> componentVertices;
				vector<int> componentEdges;
				map<int, int> oldToNewIndex;

				// Use stack for DFS
				stack<int> dfsStack;
				dfsStack.push(startVertex);

				// First pass: collect vertices
				while (!dfsStack.empty())
				{
					int currentVertex = dfsStack.top();
					dfsStack.pop();

					if (!visited[currentVertex])
					{
						visited[currentVertex] = true;

						// Add vertex to component
						//cout << "\n currentVertex: " << currentVertex;
						//cout << "\n numVertex: " << fnGraph.numVertices();

						zItGraphVertex v(inputGraph, currentVertex);
						if (!v.checkValency(0))
						{
							componentVertices.push_back(v.getPosition());
							oldToNewIndex[currentVertex] = componentVertices.size() - 1;

							// Get connected vertices

							vector<int> connectedVerts;
							v.getConnectedVertices(connectedVerts);

							// Add unvisited neighbors to stack
							for (int neighbor : connectedVerts)
							{
								if (!visited[neighbor])
								{
									dfsStack.push(neighbor);
								}
							}
						}
					}
				}

				// Second pass: collect edges
				for (auto& pair : oldToNewIndex)
				{
					int oldIndex = pair.first;
					zItGraphVertex v(inputGraph, oldIndex);

					vector<int> connectedVerts;
					v.getConnectedVertices(connectedVerts);

					for (int neighbor : connectedVerts)
					{
						// Add edge if both vertices are in this component
						if (oldToNewIndex.find(neighbor) != oldToNewIndex.end())
						{
							// For each vertex pair, only add the edge once
							if (oldIndex < neighbor ||
								// Special case: add closing edge for the last vertex in a loop
								(oldIndex == componentVertices.size() - 1 && neighbor == 0))
							{
								componentEdges.push_back(oldToNewIndex[oldIndex]);
								componentEdges.push_back(oldToNewIndex[neighbor]);
							}
						}
					}
				}

				// Create new graph component if valid
				if (componentVertices.size() >= 2 && !componentEdges.empty())
				{
					zObjGraph component;
					zFnGraph fnComponent(component);
					fnComponent.create(componentVertices, componentEdges);
					separated_graphs.push_back(component);
				}
			}

			return separated_graphs;
		}

		void mergeVertices(zObjGraph& oGraph, double tol)
		{
			zIntArray new_Connects, eConnects;
			zPointArray new_Positions, vPositions;

			std::unordered_map<int, int> vertex_map;

			zFnGraph fnGraph(oGraph);
			fnGraph.getVertexPositions(vPositions);
			fnGraph.getEdgeData(eConnects);

			// Graph Edge, Number of v1 vertices
			std::vector<std::tuple<zItGraphEdge, int>> v1_edges;

			zItGraphVertexArray f_edge_verts, s_edge_verts;

			for (zItGraphEdge e(oGraph); !e.end(); e++)
			{
				f_edge_verts.clear();
				e.getVertices(f_edge_verts);

				int v1_num = 0;
				for (auto& v : f_edge_verts)
				{
					if (v.checkValency(1))
						++v1_num;
				}

				// Has v1 vertex
				if (v1_num)
				{
					v1_edges.push_back(std::make_tuple(e, v1_num));
				}
				else
				{
					// Add existing edge
					for (auto& v : f_edge_verts)
					{
						int id = get_corrected_id(vertex_map, v.getId(), new_Positions.size());

						if (id == -1)
						{
							new_Positions.push_back(v.getPosition());
							new_Connects.push_back(new_Positions.size() - 1);
						}
						else
						{
							new_Connects.push_back(id);
						}
					}
				}
			}

			zPoint f_pos, s_pos, n_pos;

			// Generate the new vertex positions and fill in the vertex_map;
			for (auto& [f_edge, f_valen] : v1_edges)
			{
				// Still has v1 vertices
				if (f_valen)
				{
					f_edge_verts.clear();
					f_edge.getVertices(f_edge_verts);
					// Find another v1 edge within tolerance
					for (auto& [s_edge, s_valen] : v1_edges)
					{
						if (s_valen && (f_edge != s_edge))
						{
							s_edge_verts.clear();
							s_edge.getVertices(s_edge_verts);

							for (auto& f_vert : f_edge_verts)
							{
								if (f_vert.checkValency(1))
								{
									f_pos = f_vert.getPosition();

									for (auto& s_vert : s_edge_verts)
									{
										s_pos = s_vert.getPosition();

										if (f_pos.distanceTo(s_pos) < tol)
										{
											n_pos = (f_pos + s_pos) * 0.5f;
											new_Positions.push_back(n_pos);

											// Both of them now point to the same id;
											get_corrected_id(vertex_map, f_vert.getId(), new_Positions.size() - 1);
											get_corrected_id(vertex_map, s_vert.getId(), new_Positions.size() - 1);

											--f_valen;
											--s_valen;

											break;
										}
									}
								}
							}
						}
					}
				}
			}

			// Fill in the connectivity based on the vertex_map
			for (auto& [edge, _] : v1_edges)
			{
				f_edge_verts.clear();
				edge.getVertices(f_edge_verts);

				for (auto& v : f_edge_verts)
				{
					int id = get_corrected_id(vertex_map, v.getId(), new_Positions.size());

					if (id == -1)
					{
						new_Positions.push_back(v.getPosition());
						new_Connects.push_back(new_Positions.size() - 1);
					}
					else
					{
						new_Connects.push_back(id);
					}
				}
			}

			int num_merged = vPositions.size() - new_Positions.size();
			if (num_merged)
			{
				printf("\033[0;32m");
				printf("\n Merged %d vert%s\n", num_merged, num_merged == 1 ? "ex" : "ices");
				printf("\033[0m");
			}

			fnGraph.create(new_Positions, new_Connects);
		};

		void resampleGraph(zObjGraph& oGraph, double sampleDist)
		{
			zItGraphHalfEdge he(oGraph, 0);

			for (zItGraphHalfEdge he_check(oGraph); !he_check.end(); he_check++)
			{
				if (he_check.getStartVertex().checkValency(1))
				{
					he = he_check;
					break;
				}
			}

			zPointArray pos;
			zIntArray pConnects;

			int startId = he.getId();
			pos.push_back(he.getStartVertex().getPosition());

			int counter = 0;
			zPoint pt = pos[0];
			double acc_dist = 0;
			do {
				pt = he.getStartVertex().getPosition();
				acc_dist += he.getPrev().getLength();
				if (acc_dist > sampleDist)
				{
					pos.push_back(pt);
					pConnects.push_back(counter);
					pConnects.push_back(counter + 1);
					counter++;
					acc_dist = 0;
				}
				he = he.getNext();

			} while (!he.getVertex().checkValency(1) && he.getId() != startId);

			pos.push_back(he.getVertex().getPosition());
			pConnects.push_back(counter);
			pConnects.push_back(counter + 1);

			zFnGraph fnGraph(oGraph);
			fnGraph.create(pos, pConnects);
		}

		bool makeClosed(zObjGraph& oGraph)
		{
			zFnGraph fnGraph(oGraph);
			zItGraphVertex v0(oGraph, 0);
			zItGraphVertex v1(oGraph, fnGraph.numVertices() - 1);

			if (v0.checkValency(1) && v1.checkValency(1))
			{
				zPointArray pos;
				zIntArray edgeConnects;
				fnGraph.getVertexPositions(pos);
				fnGraph.getEdgeData(edgeConnects);

				edgeConnects.push_back(fnGraph.numVertices() - 1);
				edgeConnects.push_back(0);
				pos.push_back(pos[0]);

				fnGraph.create(pos, edgeConnects);
				return true;
			}
			else return false;
		}

		private:

			int get_corrected_id(std::unordered_map<int, int>& map, int id_to_check, int id_to_set)
			{
				if (map.count(id_to_check))
					return map[id_to_check];
				else
				{
					map[id_to_check] = id_to_set;
					return -1;
				}
			};

	};
}