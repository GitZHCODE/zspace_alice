#include "zProceduralBuilding.h"

namespace zSpace
{
	zProceduralBuilding::zProceduralBuilding()
	{
	}


	zProceduralBuilding::~zProceduralBuilding()
	{
	}

	void zProceduralBuilding::setPlotOutline(zMesh & _plot)
	{
		plot = _plot;

	}

	void zProceduralBuilding::setBuildingOutline(double plotOffset)
	{
		buildingOutline = getOffsetMesh(plot, plotOffset);

		building_PlotOffset = plotOffset;
	}


	void zProceduralBuilding::generateEdgeDistanceField(zGraph& inGraph, double & width, vector<vector<double>>& scalars)
	{
		vector<double> out;

		assignScalarsAsEdgeDistance(fieldMesh, inGraph, 0.01, width, out);

		scalars.push_back(out);

	}

	void zProceduralBuilding::generateFloors_GFA(bool floor_fieldBooleans[4], vector<vector<double>>& scalarValues, double& requiredGFA, double& maxHeight, bool dBuffer)
	{
		floors.clear();
		building_Height = 0;
		building_GFA = 0;

		bool exit = false;

		zMesh slab = isobandMesh(fieldMesh, 0.0, 0.5);

		double floorArea = getGFA(slab);

		do
		{
			if (building_Height >= maxHeight) exit = true;
			if (building_GFA >= requiredGFA) exit = true;

			if (!exit)
			{	
				addFloor(floor_fieldBooleans, scalarValues, dBuffer);;

				building_GFA += floorArea;

				building_Height += floorHeight;
			}
			
		} while(!exit);

	}

	void zProceduralBuilding::addFloor(bool floor_fieldBooleans[4], vector<vector<double>>& scalars, bool triangulate)
	{
		building_Height = getTotalHeight();

		if (building_Height + floorHeight > maxHeight)
		{
			printf("\n Can't add floor outside of permissible height");
			return;
		}

		scalars.clear();

		if (floor_fieldBooleans[0]) generateEdgeDistanceField(slabGraph, floorWidth, scalars);
		if (floor_fieldBooleans[1]) generateEdgeDistanceField(balconyGraph, balconyWidth, scalars);
		if (floor_fieldBooleans[2]) generateEdgeDistanceField(terraceGraph, terraceWidth, scalars);
		if (floor_fieldBooleans[3]) generateEdgeDistanceField(bridgeGraph, bridgeWidth, scalars);

		vector<double> floorField;
		vector<double> terraceField;
		if (floor_fieldBooleans[0] && floor_fieldBooleans[3]) union_fields(scalars[0], scalars[scalars.size() - 1], floorField);
		else if (floor_fieldBooleans[0] && !floor_fieldBooleans[3])floorField = scalars[0];

		if (floor_fieldBooleans[1] && floor_fieldBooleans[2])  union_fields(scalars[1], scalars[2], terraceField);
		else if (floor_fieldBooleans[1] && !floor_fieldBooleans[2])terraceField = scalars[1];
		else if (!floor_fieldBooleans[1] && floor_fieldBooleans[2])terraceField = scalars[1];

		if (floor_fieldBooleans[1] || floor_fieldBooleans[2]) difference_fields(floorField, terraceField, floorField);

		zProceduralFloor tempfloor;
		updateColors(fieldMesh, floorField);

		// floor slab Mesh
		tempfloor.floor_slab = isobandMesh(fieldMesh, 0.0, 0.5);
		if (triangulate) tempfloor.floor_slab.triangulate(false);
		slabs2D.push_back(tempfloor.floor_slab);

		tempfloor.floor_slab = extrudeMeshUp(slabs2D[slabs2D.size() - 1], slabDepth, triangulate);
		setVertexColor(tempfloor.floor_slab, zColor(0.5, 0.5, 0.5, 1), true);

		// soffit
		tempfloor.floor_soffit = tempfloor.floor_slab;
		zVector soffitTrans(0, 0, floorHeight - slabDepth);
		tempfloor.translatePositions(tempfloor.floor_soffit, soffitTrans);
		setVertexColor(tempfloor.floor_soffit, zColor(0.5, 0.5, 0.5, 1), true);

		// glass
		double glassMinThreshold = ofMap(floorWidth - 0.1, 0, floorWidth, 0.0, 0.5);
		zMesh tempGlass = isobandMesh(fieldMesh, glassMinThreshold, 0.5);
		if (triangulate) tempGlass.triangulate(false);

		tempfloor.glass = extrudeMeshUp(tempGlass, floorHeight - (2 * slabDepth), triangulate);
		zVector glassTrans(0, 0, slabDepth);
		tempfloor.translatePositions(tempfloor.glass, glassTrans);
		setVertexColor(tempfloor.glass, zColor(0, 0.140, 0.140, 1), true);

		// balcony 

		if (floor_fieldBooleans[1] || floor_fieldBooleans[2])
		{
			updateColors(fieldMesh, terraceField);

			zMesh tempBalcony = isobandMesh(fieldMesh, 0.0, 0.5);
			if (triangulate) tempBalcony.triangulate(false);

			tempfloor.floor_balcony = extrudeMeshUp(tempBalcony, slabDepth, triangulate);
			setVertexColor(tempfloor.floor_balcony, zColor(0.2, 0.2, 0.2, 1), true);
		}


		// transform 
		tempfloor.floorTransform.setIdentity();
		tempfloor.floorTransform(2, 3) = building_Height;

		floors.push_back(tempfloor);

		floors[floors.size() - 1].floorWidth = floorWidth;
		floors[floors.size() - 1].floorHeight = floorHeight;
		floors[floors.size() - 1].terraceWidth = terraceWidth;
		floors[floors.size() - 1].balconyWidth = balconyWidth;
		floors[floors.size() - 1].slabDepth = slabDepth;

		floors[floors.size() - 1].floorTransform = tempfloor.floorTransform;
		floors[floors.size() - 1].transformFloor(floors[floors.size() - 1].floorTransform);

		if (triangulate)
		{
			floors[floors.size() - 1].floor_slab.computeTriMeshNormals();
			floors[floors.size() - 1].floor_soffit.computeTriMeshNormals();
			floors[floors.size() - 1].glass.computeTriMeshNormals();
			floors[floors.size() - 1].floor_balcony.computeTriMeshNormals();
		}


		printf("\n floor added! ");

	}

	void zProceduralBuilding::editFloor(int floorNum, double& maxHeight, double& floorWidth, double& floorHeight, double& slabDepth, double& terraceWidth, double& balconyWidth, bool triangulate)
	{
		if (floorNum > floors.size())
		{
			printf("\n No input floor number!");
			return;
		}


		floors[floorNum].floorWidth = floorWidth;
		floors[floorNum].floorHeight = floorHeight;
		floors[floorNum].terraceWidth = terraceWidth;
		floors[floorNum].balconyWidth = balconyWidth;
		floors[floorNum].slabDepth = slabDepth;

		// floor slab Mesh
		slabs2D[floorNum] = isobandMesh(fieldMesh, 0.0, 0.5);
		if (triangulate) slabs2D[floorNum].triangulate(false);
		floors[floorNum].floor_slab = extrudeMeshUp(slabs2D[floorNum], slabDepth, triangulate);
		setVertexColor(floors[floorNum].floor_slab, zColor(0.5, 0.5, 0.5, 1), true);

		// soffit
		floors[floorNum].floor_soffit = floors[floorNum].floor_slab;
		zVector soffitTrans(0, 0, floorHeight - slabDepth);
		floors[floorNum].translatePositions(floors[floorNum].floor_soffit, soffitTrans);
		setVertexColor(floors[floorNum].floor_soffit, zColor(0.5, 0.5, 0.5, 1), true);

		// glass
		double glassMinThreshold = ofMap(floorWidth - 0.1, 0, floorWidth, 0.0, 0.5);
		zMesh tempGlass = isobandMesh(fieldMesh, glassMinThreshold, 0.5);
		if (triangulate) tempGlass.triangulate(false);

		floors[floorNum].glass = extrudeMeshUp(tempGlass, floorHeight - (2 * slabDepth), triangulate);
		zVector glassTrans(0, 0, slabDepth);
		floors[floorNum].translatePositions(floors[floorNum].glass, glassTrans);
		setVertexColor(floors[floorNum].glass, zColor(0, 0.140, 0.140, 1), true);

		// transform 

		floors[floorNum].transformFloor(floors[floorNum].floorTransform);

		// get height till current floor 
		double dheight = 0;


		for (int i = 0; i < floorNum; i++)
		{
			dheight += floors[i].floorHeight;
		}

		for (int i = floorNum; i < floors.size(); i++)
		{
			Matrix4f prevT = floors[i].floorTransform;

			floors[i].floorTransform(2, 3) = dheight;

			Matrix4f transform = PlanetoPlane(prevT, floors[i].floorTransform);;

			floors[i].transformFloor(transform);

			dheight += floors[i].floorHeight;
		}


	}

	double zProceduralBuilding::getTotalHeight()
	{
		double out = 0;

		for (int i = 0; i < floors.size(); i++)
		{
			out += floors[i].floorHeight;
		}

		return out;
	}

	double zProceduralBuilding::getGFA(zMesh & slab)
	{
		double out = 0;

		vector<zVector> fCenters;
		slab.getCenters(zFaceData, fCenters);

		vector<zVector> eCenters;
		slab.getCenters(zEdgeData, eCenters);

		vector<double> vAreas;
		slab.getVertexArea(fCenters, eCenters, vAreas);


		out = accumulate(vAreas.begin(), vAreas.end(), 0.0);

		return out;
	}

	zMesh zProceduralBuilding::getOffsetMesh(zMesh& inMesh, double offset)
	{
		zMesh out;

		vector<zVector>positions;
		vector<int>polyConnects;
		vector<int>polyCounts;


		for (int i = 0; i < inMesh.numPolygons(); i++)
		{



			vector<zVector> offsetVertPositions;
			offsetMeshFace(inMesh, i, offset, offsetVertPositions);


			for (int j = 0; j < offsetVertPositions.size(); j++)
			{
				positions.push_back(offsetVertPositions[j]);

				polyConnects.push_back(positions.size() - 1);;
			}

			polyCounts.push_back(offsetVertPositions.size());
		}


		out = zMesh(positions, polyCounts, polyConnects);

		return out;
	}

	zGraph zProceduralBuilding::getSlabGraph(zMesh & inMesh, double & offsetweight, int splitSteps)
	{
		zGraph out;

		vector<zVector> positions;
		vector<int> edgeConnects;

		zMesh tempOffset = getOffsetMesh(inMesh, floorWidth * offsetweight);

		for (int i = 0; i < tempOffset.numPolygons(); i++)
		{
			zGraph tempGraph = getMeshFaceEdgesasGraph(tempOffset, i);

			for (int k = 0; k < splitSteps; k++)
			{
				vector<int> edgeList;
				vector<double> edgeFactor;
				vector<int> splitVertexId;

				for (int j = 0; j < tempGraph.numEdges(); j += 2)
				{
					edgeList.push_back(j);
					edgeFactor.push_back(0.5);
				}

				tempGraph.splitEdges(edgeList, edgeFactor, splitVertexId);

				//printf("\n numEdges : %i ", tempGraph.numEdges());

				//tempGraph.printGraphInfo();

			}

			int numVerts = positions.size();

			for (int i = 0; i < tempGraph.numEdges(); i += 2)
			{
				edgeConnects.push_back(numVerts + tempGraph.edges[i].getSym()->getVertex()->getVertexId());

				edgeConnects.push_back(numVerts + tempGraph.edges[i].getVertex()->getVertexId());
			}

			for (int i = 0; i < tempGraph.vertexPositions.size(); i++)
			{
				positions.push_back(tempGraph.vertexPositions[i]);
			}


		}

		out = zGraph(positions, edgeConnects);



		for (int i = 0; i < out.numEdges(); i += 2)
		{
			out.edgeColors[i] = zColor(0, 1, 0, 1);
		}


		return out;
	}

	zGraph zProceduralBuilding::getBalconyGraph(zMesh & inMesh, double& offsetweight, int splitSteps, int numVerts_Terrace, int numVerts_Gap, int startId)
	{
		zGraph out;


		zMesh tempOffset = getOffsetMesh(inMesh, -balconyWidth * offsetweight);

		vector<zVector> positions;
		vector<int> edgeConnects;



		for (int i = 0; i < tempOffset.numPolygons(); i++)
		{

			zGraph tempGraph = getMeshFaceEdgesasGraph(tempOffset, i);

			for (int k = 0; k < splitSteps; k++)
			{
				vector<int> edgeList;
				vector<double> edgeFactor;
				vector<int> splitVertexId;


				for (int j = 0; j < tempGraph.numEdges(); j += 2)
				{
					edgeList.push_back(j);
					edgeFactor.push_back(0.5);
				}

				tempGraph.splitEdges(edgeList, edgeFactor, splitVertexId);

				//printf("\n numEdges : %i ", tempGraph.numEdges());

				//tempGraph.printGraphInfo();

			}


			bool terraceVert = true;
			int terraceVertCounter = 0;
			int gapVertCounter = 0;

			zEdge *start = &tempGraph.edges[0];
			startId = startId % tempGraph.numVertices();
			for (int k = 0; k < startId; k++) start = start->getNext();

			zEdge *e = start;

			bool exit = false;

			do
			{

				int vertId = e->getSym()->getVertex()->getVertexId();
				positions.push_back(tempGraph.vertexPositions[vertId]);
				terraceVertCounter++;

				if (terraceVertCounter == numVerts_Terrace)
				{
					terraceVert = false;

					for (int i = terraceVertCounter - 1; i > 0; i--)
					{
						edgeConnects.push_back(positions.size() - i - 1);
						edgeConnects.push_back(positions.size() - i);
					}

					/*positions.push_back(positions[positions.size() - terraceVertCounter] + zVector(0, 0, 6));
					positions.push_back(positions[positions.size() - 2] + zVector(0, 0, 6));

					edgeConnects.push_back(positions.size() - terraceVertCounter - 2);
					edgeConnects.push_back(positions.size() - 2);

					edgeConnects.push_back(positions.size() - 3);
					edgeConnects.push_back(positions.size() - 1);*/

					for (int l = 0; l < numVerts_Gap; l++)
					{
						e = e->getNext();

						if (e == start)
						{
							exit = true;
							break;
						}
					}

					terraceVertCounter = 0;

				}
				else
				{
					e = e->getNext();
				}

				if (e == start)
				{
					exit = true;

					if (terraceVertCounter == 1)
					{
						// dont add sigle vertex in the end, hence remove
						positions.erase(positions.begin() + positions.size() - 1);

					}
					else if (terraceVertCounter > 1)
					{
						for (int i = terraceVertCounter - 1; i > 0; i--)
						{
							edgeConnects.push_back(positions.size() - i - 1);
							edgeConnects.push_back(positions.size() - i);
						}


						/*positions.push_back(positions[positions.size() - terraceVertCounter] + zVector(0, 0, 6));
						positions.push_back(positions[positions.size() - 2] + zVector(0, 0, 6));

						edgeConnects.push_back(positions.size() - terraceVertCounter - 2);
						edgeConnects.push_back(positions.size() - 2);

						edgeConnects.push_back(positions.size() - 3);
						edgeConnects.push_back(positions.size() - 1);*/
					}
				}

			} while (!exit);

			if (numVerts_Terrace >= tempGraph.numVertices())
			{
				// close loop 

				edgeConnects.push_back(positions.size() - 1);
				edgeConnects.push_back(positions.size() - numVerts_Terrace);
			}


		}

		out = zGraph(positions, edgeConnects);



		for (int i = 0; i < out.numEdges(); i += 2)
		{
			out.edgeColors[i] = zColor(0, 0, 1, 1);
		}

		return out;
	}

	zGraph zProceduralBuilding::getTerraceGraph(zMesh & inMesh, double & offsetweight, int splitSteps, int numVerts_Terrace, int numVerts_Gap, int startId)
	{
		zGraph out;


		zMesh tempOffset = getOffsetMesh(inMesh, terraceWidth * offsetweight);

		vector<zVector> positions;
		vector<int> edgeConnects;



		for (int i = 0; i < tempOffset.numPolygons(); i++)
		{

			zGraph tempGraph = getMeshFaceEdgesasGraph(tempOffset, i);

			for (int k = 0; k < splitSteps; k++)
			{
				vector<int> edgeList;
				vector<double> edgeFactor;
				vector<int> splitVertexId;


				for (int j = 0; j < tempGraph.numEdges(); j += 2)
				{
					edgeList.push_back(j);
					edgeFactor.push_back(0.5);
				}

				tempGraph.splitEdges(edgeList, edgeFactor, splitVertexId);

				//printf("\n numEdges : %i ", tempGraph.numEdges());

				//tempGraph.printGraphInfo();

			}


			bool terraceVert = true;
			int terraceVertCounter = 0;
			int gapVertCounter = 0;

			zEdge *start = &tempGraph.edges[0];

			startId = startId % tempGraph.numVertices();
			for (int k = 0; k < startId; k++) start = start->getNext();

			zEdge *e = start;

			bool exit = false;

			do
			{

				int vertId = e->getSym()->getVertex()->getVertexId();
				positions.push_back(tempGraph.vertexPositions[vertId]);
				terraceVertCounter++;

				if (terraceVertCounter >= numVerts_Terrace)
				{
					terraceVert = false;

					for (int i = terraceVertCounter - 1; i > 0; i--)
					{
						edgeConnects.push_back(positions.size() - i - 1);
						edgeConnects.push_back(positions.size() - i);
					}


					for (int l = 0; l < numVerts_Gap; l++)
					{
						e = e->getNext();

						if (e == start)
						{
							exit = true;
							break;
						}
					}

					terraceVertCounter = 0;

				}
				else
				{
					e = e->getNext();
				}

				if (e == start)
				{
					exit = true;

					if (terraceVertCounter == 1)
					{
						// dont add sigle vertex in the end, hence remove
						positions.erase(positions.begin() + positions.size() - 1);

					}
					else if (terraceVertCounter > 1)
					{
						for (int i = terraceVertCounter - 1; i > 0; i--)
						{
							edgeConnects.push_back(positions.size() - i - 1);
							edgeConnects.push_back(positions.size() - i);
						}
					}
				}

			} while (!exit);

			if (numVerts_Terrace == tempGraph.numVertices())
			{
				// close loop 

				edgeConnects.push_back(positions.size() - 1);
				edgeConnects.push_back(positions.size() - numVerts_Terrace);
			}


		}

		out = zGraph(positions, edgeConnects);

		for (int i = 0; i < out.numEdges(); i += 2)
		{
			out.edgeColors[i] = zColor(1, 0, 0, 1);
		}

		return out;
	}

	zGraph zProceduralBuilding::getBridgeGraph(zMesh & inMesh, double& offsetweight, int splitSteps, int numBridges, int numVerts_Gap, int startId)
	{

		zGraph out;

		vector<zVector> positions;
		vector<int> edgeConnects;

		zMesh tempOffset = getOffsetMesh(inMesh, floorWidth * offsetweight);

		for (int i = 0; i < tempOffset.numPolygons(); i++)
		{
			zGraph tempGraph = getMeshFaceEdgesasGraph(tempOffset, i);

			for (int k = 0; k < splitSteps; k++)
			{
				vector<int> edgeList;
				vector<double> edgeFactor;
				vector<int> splitVertexId;


				for (int j = 0; j < tempGraph.numEdges(); j += 2)
				{
					edgeList.push_back(j);
					edgeFactor.push_back(0.5);
				}

				tempGraph.splitEdges(edgeList, edgeFactor, splitVertexId);

				//printf("\n numEdges : %i ", tempGraph.numEdges());

				//tempGraph.printGraphInfo();

			}

			zEdge *start = &tempGraph.edges[0];

			startId = startId % tempGraph.numVertices();

			for (int k = 0; k < startId; k++) start = start->getNext();
			zEdge *e = start;

			for (int i = 0; i < numBridges; i++)
			{
				positions.push_back(tempGraph.vertexPositions[e->getSym()->getVertex()->getVertexId()]);

				for (int j = 0; j < numVerts_Gap; j++)
				{
					e = e->getNext();
				}

				positions.push_back(tempGraph.vertexPositions[e->getSym()->getVertex()->getVertexId()]);

				edgeConnects.push_back(positions.size() - 2);
				edgeConnects.push_back(positions.size() - 1);

			}

		}





		out = zGraph(positions, edgeConnects);



		for (int i = 0; i < out.numEdges(); i += 2)
		{
			out.edgeColors[i] = zColor(0, 1, 1, 1);
		}

		return out;
	}

	zGraph zProceduralBuilding::getMeshFaceEdgesasGraph(zMesh & inMesh, int faceIndex)
	{
		zGraph out;

		vector<zVector> positions;
		vector<int> edgeConnects;


		vector<int> fVerts;
		inMesh.getVertices(faceIndex, zFaceData, fVerts);

		for (int i = 0; i < fVerts.size(); i++)
		{
			positions.push_back(inMesh.vertexPositions[fVerts[i]]);

			edgeConnects.push_back(i);
			edgeConnects.push_back((i + 1) % fVerts.size());
		}


		out = zGraph(positions, edgeConnects);

		return out;
	}

	zGraph zProceduralBuilding::splitGraph(zMesh & inMesh, double & offsetWeight)
	{
		zGraph out;

		zMesh tempOffset = getOffsetMesh(inMesh, floorWidth * offsetWeight);

		vector<zVector> positions_Graph1;
		vector<int> edgeConnects_Graph1;

		for (int i = 0; i < tempOffset.numVertices() - 1; i++)
			positions_Graph1.push_back(tempOffset.vertexPositions[i]);

		edgeConnects_Graph1.push_back((0,1));
		edgeConnects_Graph1.push_back((1,2));
		
		out = zGraph(positions_Graph1, edgeConnects_Graph1);

		return out;
	}


	void zProceduralFloor::translatePositions(zMesh & m, zVector & trans)
	{

		for (int i = 0; i < m.vertexPositions.size(); i++)
		{
			m.vertexPositions[i] += trans;
		}

	}

	void zProceduralFloor::transformFloor(Matrix4f& trans)
	{

		transformMesh(floor_slab, trans);
		transformMesh(floor_soffit, trans);
		transformMesh(floor_balcony, trans);

		transformMesh(glass, trans);
		transformMesh(balustrade, trans);
	}



}

