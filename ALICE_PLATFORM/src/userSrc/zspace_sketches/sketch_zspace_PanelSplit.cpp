//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;
////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;
zColor red(1, 0, 0, 1);
zColor green(0, 1, 0, 1);
zColor blue(0, 0, 1, 1);

zColor white(1, 1, 1, 1);

void getSplitHalfEdges(zItMeshHalfEdge& smooth_he, int nV_lowPoly, zItMeshHalfEdgeArray& smoothEdges)
{
	smoothEdges.clear();

	if (smooth_he.onBoundary())
	{
		// first edge
		smoothEdges.push_back(smooth_he);

		zItMeshVertex v = smooth_he.getVertex();

		while (v.getId() >= nV_lowPoly)
		{
			smooth_he = smooth_he.getNext();
			smoothEdges.push_back(smooth_he);

			v = smooth_he.getVertex();
		}
		

	}
	else
	{
		// first edge
		smoothEdges.push_back(smooth_he);

		zItMeshVertex v = smooth_he.getVertex();

		while (v.getId() >= nV_lowPoly)
		{
			
			smooth_he = smooth_he.getNext().getSym();
			smooth_he = smooth_he.getNext();

			smoothEdges.push_back(smooth_he);

			v = smooth_he.getVertex();
		}
			
		
	}

}

void computeSplitEdges(zObjMesh &_o_lowpolyMesh , zObjMesh &_o_smoothMesh)
{
	zFnMesh fnLowPolyMesh(_o_lowpolyMesh);
	zFnMesh fnSmoothMesh(_o_smoothMesh);

	int nV_lowPoly = fnLowPolyMesh.numVertices();
	printf("\n nV_lowPoly %i  ", nV_lowPoly);

	zBoolArray halfedgeVisited;
	halfedgeVisited.assign(fnLowPolyMesh.numHalfEdges(), false);

	for (zItMeshVertex v(_o_smoothMesh); !v.end(); v++)
	{
		if (v.getId() >= nV_lowPoly) break;


		zItMeshHalfEdgeArray connectedHE;
		v.getConnectedHalfEdges(connectedHE);

		for (auto& he : connectedHE)
		{

			zItMeshHalfEdgeArray hEdges;
			getSplitHalfEdges(he, nV_lowPoly, hEdges);

			// get corresponding low poly half edge
			int v0 = hEdges[0].getStartVertex().getId();
			int v1 = hEdges[hEdges.size() - 1].getVertex().getId();

			

			int he_lowPoly_id = -1;
			bool chk = fnLowPolyMesh.halfEdgeExists(v0, v1, he_lowPoly_id);

			//printf("\n %i | %i %i | %i  ", hEdges.size(), v0, v1, he_lowPoly_id);

			if (chk && !halfedgeVisited[he_lowPoly_id])
			{
				for (auto& splitHE : hEdges)
				{
					splitHE.getEdge().setColor(blue);					
				}
				
				halfedgeVisited[he_lowPoly_id] = true;

				zItMeshHalfEdge he_lowPoly(_o_lowpolyMesh, he_lowPoly_id);
				halfedgeVisited[he_lowPoly.getSym().getId()] = true;

				
			}

		}
	}

	// exclude start point connected edges
	for (zItMeshVertex v(_o_smoothMesh); !v.end(); v++)
	{
		if (v.getId() >= nV_lowPoly) break;

		if (v.onBoundary()) continue;
		if (v.checkValency(4)) continue;

		zItMeshHalfEdgeArray connectedHE;
		v.getConnectedHalfEdges(connectedHE);

		for (auto& cHe : connectedHE)
		{
			bool exit = false;

			zItMeshHalfEdge he = cHe;
			he.getEdge().setColor(green);

			zItMeshVertex v = he.getVertex();	

			do
			{				
				he = he.getNext().getSym().getNext();
				he.getEdge().setColor(green);
				
				v = he.getVertex();

				if (v.onBoundary())exit = true;
				if(!v.checkValency(4)) exit = true;

			} while (!exit);
		}
	}

	for (zItMeshEdge e(_o_smoothMesh); !e.end(); e++)
	{
		if (e.getColor() == blue)
		{
			e.getHalfEdge(0).getStartVertex().setColor(blue);
			e.getHalfEdge(0).getVertex().setColor(blue);

			if (!e.getHalfEdge(0).onBoundary())
			{
				e.getHalfEdge(0).getFace().setColor(red);				
			}
			if (!e.getHalfEdge(1).onBoundary())
			{
				e.getHalfEdge(1).getFace().setColor(red);				
			}
		}
		
	}
	
	

}

void computeDetachMesh(zObjMesh& _o_lowpolyMesh, zObjMesh& _o_smoothMesh, zObjMesh& _o_detachMesh)
{

	zFnMesh fnLowPolyMesh(_o_lowpolyMesh);
	int nV_lowPoly = fnLowPolyMesh.numVertices();

	zFnMesh fnSmoothMesh(_o_smoothMesh);
	zFnMesh fnDetachMesh(_o_detachMesh);

	zPointArray positions;
	zIntArray pCounts, pConnects;

	fnSmoothMesh.getVertexPositions(positions);
	zBoolArray vertexVisited;
	vertexVisited.assign(fnSmoothMesh.numVertices(), false);

	// panel corner vertices
	zBoolArray pCornerVertices_boolean;
	pCornerVertices_boolean.assign(fnSmoothMesh.numVertices(), false);

	zItMeshVertexArray pCornerVertices;

	zBoolArray pCornerVertices_cHEdges_boolean;
	pCornerVertices_cHEdges_boolean.assign(fnSmoothMesh.numHalfEdges(), false);

	for (zItMeshVertex v(_o_smoothMesh); !v.end(); v++)
	{
		if (v.getId() >= nV_lowPoly) break;

		zItMeshEdgeArray cEdges;
		v.getConnectedEdges(cEdges);

		int numBlueEdges = 0;
		for (auto& e : cEdges)
		{
			if (e.getColor() == blue) numBlueEdges++;
		}

		if (numBlueEdges == v.getValence())
		{
			pCornerVertices_boolean[v.getId()] = true;
			pCornerVertices.push_back(v);
			//printf("\n %i ", v.getId());
		}
		
	}

	for (zItMeshFace f(_o_smoothMesh); !f.end(); f++)
	{
		if (f.getColor() == red) continue;
		
		zIntArray fVerts;
		f.getVertices(fVerts);

		for (auto& fV : fVerts)
		{
			pConnects.push_back(fV);
			vertexVisited[fV] = true;
		}
		pCounts.push_back(fVerts.size());
	}

	int numPanels = 0;
	int current_numVertices = positions.size();

	unordered_map<string, int> positionVertex;

	for (auto& v : pCornerVertices)
	{
		zItMeshHalfEdgeArray cHEdges;
		v.getConnectedHalfEdges(cHEdges);

		for (auto& cHE : cHEdges)
		{
			if (!pCornerVertices_cHEdges_boolean[cHE.getId()] && !cHE.onBoundary())
			{
				zItMeshHalfEdge he = cHE;
				int numHE = 0;
			
				do
				{
					// make polygon
					if (pCornerVertices_cHEdges_boolean[he.getSym().getId()] || pCornerVertices_cHEdges_boolean[he.getPrev().getSym().getId()] )
					{
						zItMeshVertexArray fVerts;
						he.getFace().getVertices(fVerts);

						for (auto& fV : fVerts)
						{
							if (fV.getColor() == blue)
							{
								if (vertexVisited[fV.getId()])
								{
									int new_fV = -1;

									// check repeat in current cycle
									bool chkRepeat = false;
									for (int k = current_numVertices; k < positions.size(); k++)
									{
										if (numPanels == 8)
										{
											printf("\n %1.3f %1.3f %1.3f | %1.4f ", positions[fV.getId()].x, positions[fV.getId()].y, positions[fV.getId()].z, positions[k].distanceTo(positions[fV.getId()]));
											printf("\n %i | %i ", current_numVertices, positions.size());

										}

										if (positions[k].distanceTo(positions[fV.getId()]) < 0.001)
										{
											new_fV = k;
											chkRepeat = true;											
											break;
										}
									}

									if (!chkRepeat)
									{
										new_fV = positions.size();
										positions.push_back(positions[fV.getId()]);
									}
									
									pConnects.push_back(new_fV);

								}
								else
								{
									pConnects.push_back(fV.getId());
									vertexVisited[fV.getId()] = true;
								}
							}
							else pConnects.push_back(fV.getId());

						}
						pCounts.push_back(fVerts.size());
					}
					else
					{
						zItMeshVertexArray fVerts;
						he.getFace().getVertices(fVerts);

						for (auto& fV : fVerts)
						{
							pConnects.push_back(fV.getId());
							vertexVisited[fV.getId()] = true;
						}

						pCounts.push_back(fVerts.size());
					}
					

					// walk
					pCornerVertices_cHEdges_boolean[he.getId()] = true;
					he = he.getNext().getSym().getNext();
					numHE++;

					// move to next at corner
					if (pCornerVertices_boolean[he.getVertex().getId()])
					{
						pCornerVertices_cHEdges_boolean[he.getId()] = true;
						he = he.getNext();
						numHE++;
					}


				} while (he != cHE);

				

				printf("\n p %i : %i ", numPanels, numHE);
				numPanels++;
				current_numVertices = positions.size();

			}
		}
	}

	//for (zItMeshEdge e(_o_smoothMesh); !e.end(); e++)
	//{
	//	if (e.getColor() == blue)
	//	{
	//		zItMeshFaceArray eFaces;
	//		e.getFaces(eFaces);

	//		for (auto& f : eFaces)
	//		{
	//			zItMeshVertexArray fVerts;
	//			f.getVertices(fVerts);

	//			for (auto& fV : fVerts)
	//			{
	//				if (fV.getColor() == blue)
	//				{
	//					if (vertexVisited[fV.getId()])
	//					{
	//						int new_fV = positions.size();
	//						positions.push_back(positions[fV.getId()]);
	//						pConnects.push_back(new_fV);

	//					}
	//					else
	//					{
	//						pConnects.push_back(fV.getId());
	//						vertexVisited[fV.getId()] = true;
	//					}
	//				}
	//				else pConnects.push_back(fV.getId());
	//				
	//			}
	//			pCounts.push_back(fVerts.size());
	//		}					
	//		
	//	}

	//}

	fnDetachMesh.create(positions, pCounts, pConnects);
	fnDetachMesh.to("data/Panelling/test_split.obj", zOBJ);

	for (zItMeshEdge e(_o_detachMesh); !e.end(); e++)
	{
		if (e.onBoundary())
		{
			e.setWeight(4);
		}
	}
	//printf("\n %i %i %i ", fnDetachMesh.numVertices(), fnDetachMesh.numEdges(), fnDetachMesh.numPolygons());
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;

bool d_lowpoly = true;
bool d_smooth = true;
bool d_detach = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh o_lowpolyMesh;
zObjMesh o_smoothMesh;
zObjMesh o_detachMesh;


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
	zFnMesh fnLowPolyMesh(o_lowpolyMesh);
	fnLowPolyMesh.from("data/Panelling/test.obj", zOBJ);

	zFnMesh fnSmoothMesh(o_smoothMesh);
	fnSmoothMesh.from("data/Panelling/test.obj", zOBJ);
	fnSmoothMesh.smoothMesh(2);
	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_lowpolyMesh);
	model.addObject(o_smoothMesh);

	// set display element booleans
	o_lowpolyMesh.setDisplayElements(false, true, false);
	o_smoothMesh.setDisplayElements(true, true, false);
	o_detachMesh.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&d_lowpoly, "d_lowpoly");
	B.buttons[1].attachToVariable(&d_lowpoly);

	B.addButton(&d_smooth, "d_smooth");
	B.buttons[2].attachToVariable(&d_smooth);
		
	B.addButton(&d_detach, "d_detach");
	B.buttons[3].attachToVariable(&d_detach);

}

void update(int value)
{
	if (compute)
	{
		computeSplitEdges(o_lowpolyMesh, o_smoothMesh);

		computeDetachMesh(o_lowpolyMesh, o_smoothMesh, o_detachMesh);
		
		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (d_lowpoly)
	{
		o_lowpolyMesh.draw();
	}

	if (d_smooth)
	{		
		o_smoothMesh.draw();		
	}

	
	if (d_detach)
	{
		o_detachMesh.draw();
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
