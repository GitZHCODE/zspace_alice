// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Taizhong Chen <taizhong.chen@zaha-hadid.com>
//


#include <userSrc/tzSketches/TJU/zTsTowerMaker.h>
#include <userSrc/tzSketches/TechLabSlicer/GraphProcessor.cpp>


using namespace std;
using namespace std::chrono;

namespace zSpace
{
	// UTILS

	zTsTowerMaker::zTsTowerMaker() {};
	zTsTowerMaker::~zTsTowerMaker() {};

	void zTsTowerMaker::from(string path)
	{
		floors.clear();

		PXR_NS::UsdStageRefPtr stage = UsdStage::Open(path);

		if (stage)
		{
			int counter = 0;
			for (PXR_NS::UsdPrim prim : stage->Traverse())
			{
				if (prim.IsA<UsdGeomMesh>())
				{
					floors.push_back(new zFloor());
					floors[counter]->from(prim);
					floors[counter]->id = counter;

					counter++;
				}
			}
		}
		else cout << " error in opening file  " << path.c_str() << endl;

		floors.shrink_to_fit();

		//re order
		std::sort(floors.begin(), floors.end(),
			[](zFloor* a, zFloor* b)
			{
				return a->level < b->level;
			}
		);

		for (size_t i = 0; i < floors.size(); i++)
		{
			floors[i]->id = static_cast<int>(i);
		}

	}

	void zTsTowerMaker::initialise(int dimX, int dimY, zColor col0, zColor col1)
	{
		layers.clear();  // Clear any existing layers
		minBB = zPoint();
		maxBB = zPoint();

		for (auto& floor : floors)
		{
			floor->computeFloorBounds();

			zPoint temp_min, temp_max;
			zFnMesh fn(floor->floorMesh);
			fn.getBounds(temp_min, temp_max);

			if (minBB.x > temp_min.x) minBB.x = temp_min.x;
			if (minBB.y > temp_min.y) minBB.y = temp_min.y;
			if (minBB.z > temp_min.z) minBB.z = temp_min.z;
			if (maxBB.x < temp_max.x) maxBB.x = temp_max.x;
			if (maxBB.y < temp_max.y) maxBB.y = temp_max.y;
			if (maxBB.z < temp_max.z) maxBB.z = temp_max.z;
		}

		float buffer = 2.0f;
		minBB.x -= buffer;
		minBB.y -= buffer;
		maxBB.x += buffer;
		maxBB.y += buffer;

		// field
		for (auto& floor : floors)
		{
			floor->computeFloorBounds();
			zFnMeshScalarField fn(field);
			zPoint minBB_local(minBB.x, minBB.y, floor->level);
			zPoint maxBB_local(maxBB.x, maxBB.y, floor->level);
			fn.create(minBB_local, maxBB_local, dimX, dimY, 1, true, false);
			zDomainColor colDomain(zRED, zGREEN);
			fn.setFieldColorDomain(colDomain);
		}

		col_a = col0;
		col_b = col1;
		// display
		//setDisplay(col0, col1);
	}


	void zTsTowerMaker::compute(zSMin::MODE mode_horizontal, zSMin::MODE mode_vertical, double k1, double k2, double contourHeight, double sampleDist)
	{
		// horizontal key plans
		computeHorizontal(mode_horizontal, k1);

		// vertical blend
		cout << "\n start vertical \n";
		computeVertical(contourHeight, mode_vertical, k2);


		//post processing
		GraphProcessor processor;
		for (auto& layer : layers)
		{
			// Get separated graphs
			vector<zObjGraph> separated = processor.separateGraph(layer->graphs[0]);
			
			// Process each separated graph
			for (auto& g : separated)
			{
				// Merge vertices
				processor.mergeVertices(g, sampleDist);
				processor.makeClosed(g);
				processor.resampleGraph(g, sampleDist);
			}
			
			// Clear and assign new graphs
			layer->graphs = std::move(separated);  // Use move semantics for efficiency
		}


		//
		setDisplay(col_a, col_b);

		cout << "\nComputed " << layers.size() << " contours.\n";
	}

	void zTsTowerMaker::exportTo(string path)
	{

	}

	void zTsTowerMaker::draw()
	{
		// Draw floors
		for (auto& floor : floors)
			floor->draw();

		// Draw layers
		for (auto& layer : layers)
		{
			for (auto& graph : layer->graphs)
			{
				graph.draw();
				zFnGraph fnGraph(graph);
				if (fnGraph.numVertices() > 0)
				{
					zPoint* pts = fnGraph.getRawVertexPositions();
					display.drawPoint(pts[0], zBLACK, 5);  // First point
					
					// Rest of points
					for (int i = 1; i < fnGraph.numVertices(); i++)
						display.drawPoint(pts[i], zRED, 3);
				}
			}
		}
	}

	// PRIVATE METHODS

	void zTsTowerMaker::setDisplay(zColor col0, zColor col1)
	{
		zDomainFloat domain_in(0, layers.size());
		zDomainColor domain_out(col0, col1);

		for (auto& layer : layers)
			for (auto& graph : layer->graphs)
				graph.setDisplayElements(true, true);


		for (size_t i = 0; i < layers.size(); i++)
		{
			zColor col = core.blendColor(i, domain_in, domain_out, zColorType::zHSV);
			layers[i]->setDisplay(col, 1.0);  // Use the layer's setDisplay method
		}
	}

	void zTsTowerMaker::computeHorizontal(zSMin::MODE mode, double k1)
	{
		for (auto& floor : floors)
		{
			//if (floor->id == 0)
			{
				vector<zScalarArray> inputs;
				zScalarArray scalars_result;
				inputs.assign(floor->floorBounds.size(), zScalarArray());

				zFnMeshScalarField fnField(field);
				scalars_result.assign(fnField.numFieldValues(), -1);

				for (size_t i = 0; i < floor->floorBounds.size(); i++)
				{
					zObjGraph temp;
					temp.graph = floor->floorBounds[i].graph;
					zFnGraph fnTemp(temp);
					fnTemp.setTranslation(zVector(0, 0, floor->level * -1), true);
					fnField.getScalars_Polygon(inputs[i], temp);
				}

				zSMin smin;
				smin.smin_multiple(inputs, scalars_result, k1, mode);


				fnField.normliseValues(scalars_result);
				fnField.setFieldValues(scalars_result, zFieldColorType::zFieldSDF, 0.01f);
				fnField.getIsocontour(floor->contour, 0.01f);

				floor->scalars = scalars_result;

				//cout << "\n";
				//cout << "floor_" << floor->id << ": " << floor->contour.graph.n_v << "\n";
				if (floor->contour.graph.n_v > 0)
				{
					floor->contour.setDisplayElements(false, true);
					zFnGraph fnGraph(floor->contour);
					fnGraph.setTranslation(zVector(0, 0, floor->level), true);
					fnGraph.setEdgeColor(floor->contourColor, false);
					fnGraph.setEdgeWeight(floor->contourWeight);
				}

				//for (auto& v : scalars_result)
				//	cout << "floor_" << floor->id << ": " << v << "\n";
			}
		}
	}

	void zTsTowerMaker::computeVertical(double contourHeight, zSMin::MODE mode, double k2)
	{
		layers.clear();

		for (size_t i = 1; i < floors.size(); i++)
		{
			zFloor* f_prev = floors[i - 1];
			zFloor* f_now = floors[i];

			double delta = f_now->level - f_prev->level;
			delta /= contourHeight;

			int numContours = (int)ceil(delta);
			double localLevel = f_prev->level;
			int layerCounter = 0;

			for (size_t j = 0; j < numContours; j++)
			{
				// Create new layer
				zPrintLayer* newLayer = new zPrintLayer();
				newLayer->id = layerCounter++;
				
				// Initialize with one graph
				newLayer->graphs.emplace_back();  // Create empty graph directly in vector
				
				// Calculate blend weight
				double wt = (double)j / (double)numContours;

				// Compute scalar field
				zScalarArray scalars_result;
				zSMin smin;
				smin.smin_exponential_weighted(f_prev->scalars, f_now->scalars, scalars_result, k2, wt);

				zFnMeshScalarField fnField(field);
				fnField.normliseValues(scalars_result);
				fnField.setFieldValues(scalars_result, zFieldColorType::zFieldSDF, 0.01f);
				
				// Generate contour directly into the layer's graph
				fnField.getIsocontour(newLayer->graphs[0], 0.01f);

				if (newLayer->graphs[0].graph.n_v > 0 && newLayer->graphs[0].graph.n_e > 0)
				{
					newLayer->graphs[0].setDisplayElements(false, true);
					zFnGraph fnGraph(newLayer->graphs[0]);
					fnGraph.setTranslation(zVector(0, 0, localLevel), false);
				}
				else
				{
					cout << "\n empty graph \n";
				}

				layers.push_back(newLayer);
				localLevel += contourHeight;
			}
		}
	}

	// zFLOOR

	zFloor::zFloor() {};
	zFloor::~zFloor() {};

	void zFloor::from(UsdPrim prim)
	{
		zFnMesh fn(floorMesh);
		fn.from(prim);

		zPoint* pos = fn.getRawVertexPositions();
		level = pos[0].z;
	}

	void zFloor::computeFloorBounds()
	{
		floorBounds.clear();

		zFnMesh fn(floorMesh);
		zBoolArray isVisited;
		isVisited.assign(fn.numHalfEdges(), false);

		for (zItMeshHalfEdge he(floorMesh); !he.end(); he++)
		{
			if (!isVisited[he.getId()] && he.onBoundary())
			{
				zObjGraph graph;
				zPointArray positions;
				zIntArray pConnects;

				zItMeshHalfEdge he_start = he;
				int id_start = he_start.getId();
				int counter = 0;
				do
				{
					positions.push_back(he_start.getVertex().getPosition());

					if (counter == 0) pConnects.push_back(0);
					else
					{
						pConnects.push_back(counter);
						pConnects.push_back(counter);
					}
					isVisited[he_start.getId()] = true;

					counter++;
					he_start = he_start.getPrev();
				} while (he_start.getId() != id_start);

				// fix the last vertex id
				pConnects.push_back(pConnects[0]);

				zFnGraph fn(graph);
				fn.create(positions, pConnects);
				floorBounds.emplace_back(graph);

				// compute floor level
				level = positions[0].z;
			}
		}
	}

	void zFloor::setDisplay(zColor col, double wt)
	{
		floorMesh.setDisplayElements(false, false, false);

		contourColor = col;
		contourWeight = wt;

		for (auto& bounds : floorBounds)
		{
			bounds.setDisplayElements(false, true);
			zFnGraph fn(bounds);
			fn.setEdgeWeight(1.0);
		}
	}

	void zFloor::draw()
	{
		floorMesh.draw();

		for (auto& bounds : floorBounds) bounds.draw();
	}

	// zPRINTLAYER

	zPrintLayer::zPrintLayer() {};
	zPrintLayer::~zPrintLayer() {};

	void zPrintLayer::setDisplay(zColor col, double wt)
	{
		for (auto& graph : graphs)
		{
			graph.setDisplayElements(true, true);
			zFnGraph fn(graph);
			fn.setEdgeColor(col);
			fn.setEdgeWeight(wt);
		}
	}

	void zPrintLayer::draw()
	{
		for (auto& graph : graphs)
			graph.draw();
	}
}

