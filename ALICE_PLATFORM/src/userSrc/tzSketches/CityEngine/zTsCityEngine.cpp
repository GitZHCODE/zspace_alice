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


#include "zTsCityEngine.h"

namespace zSpace
{
	//generic methods

	Vectors zPointArrayToVectors(zPointArray& vertices)
	{
		Vectors vectors;
		for (auto& v : vertices)
			vectors.push_back(Vector(v.x, v.y, v.z));

		return vectors;
	}

	zPointArray VectorsToPointArray(Vectors& vertices)
	{
		zPointArray pts;
		for (auto& v : vertices)
			pts.push_back(zPoint(v.x, v.y, v.z));

		return pts;
	}

	void zPolygonsToMesh(vector<zPolygon>& polygons, zObjMesh& mesh)
	{
		zFnMesh fn(mesh);
		zPointArray positions;
		zIntArray pCounts;
		zIntArray pConnects;
		int offset = 0;

		for (auto& polygon : polygons)
		{
			Vectors pos = polygon.getVectors();
			zPointArray polygonPoints = VectorsToPointArray(pos);
			positions.insert(positions.end(), polygonPoints.begin(), polygonPoints.end());

			for (int i = 0; i < polygonPoints.size(); i++)
				pConnects.push_back(i + offset);

			pCounts.push_back(polygonPoints.size());
			offset+= polygonPoints.size();
		}

		fn.create(positions, pCounts, pConnects);
	}

	void meshToPolygons(zObjMesh& mesh, vector<zPolygon>& polys)
	{
		zFnMesh fn(mesh);
		int numPolys = fn.numPolygons();

		polys.clear();
		polys.assign(numPolys, zPolygon());

		for (int i = 0; i < numPolys; i++)
		{
			zItMeshFace f(mesh,i);
			zPointArray positions;
			f.getVertexPositions(positions);

			polys[i] = zPolygon(zPointArrayToVectors(positions));
		}
	}

	void getPolygon(zObjMesh& mesh,int id, zPolygon& poly)
	{
		zItMeshFace f(mesh, id);

		zPointArray pos;
		f.getVertexPositions(pos);

		poly = zPolygon(zPointArrayToVectors(pos));
	}

	zVector closestOrthogonalVector(zVector vec, zVector guideVec)
	{
		vec.normalize();
		guideVec.normalize();

		zVector norm(0, 0, 1);
		zVectorArray orthogonalVecs;
		orthogonalVecs.assign(4, zVector());
		orthogonalVecs[0] = guideVec;
		orthogonalVecs[1] = norm ^ guideVec;
		orthogonalVecs[2] = norm ^ orthogonalVecs[1];
		orthogonalVecs[3] = norm ^ orthogonalVecs[2];

		float maxDot = -10000.0f;
		zVector closestVec;

		for (const auto& oVec : orthogonalVecs)
		{
			float dot = vec.x * oVec.x + vec.y * oVec.y + vec.z * oVec.z;
			if (dot > maxDot)
			{
				maxDot = dot;
				closestVec = oVec;
			}
		}
		return closestVec;
	}

	void getBestFitBoundingBox(zPointArray& pos, zPointArray& bbox, float offsetX, float offsetY)
	{
		zUtilsCore core;
		zPlane BFP = core.getBestFitPlane(pos);

		zPoint minBB, maxBB, minBB_local, maxBB_local;
		core.boundingboxPCA(pos, minBB, maxBB, minBB_local, maxBB_local);

		minBB_local.x -= offsetX;
		maxBB_local.x += offsetX;
		minBB_local.y -= offsetY;
		maxBB_local.y += offsetY;

		zPointArray corners =
		{
		   zPoint(minBB_local.x, minBB_local.y, minBB_local.z),
		   zPoint(maxBB_local.x, minBB_local.y, minBB_local.z),
		   zPoint(maxBB_local.x, maxBB_local.y, minBB_local.z),
		   zPoint(minBB_local.x, maxBB_local.y, minBB_local.z),
		};
		zObjPointCloud cloud;
		zFnPointCloud fnCloud(cloud);
		fnCloud.create(corners);
		fnCloud.setTransform(BFP);

		fnCloud.getVertexPositions(bbox);
	}

	void getBBoxVectors(bool longSide, zPointArray& bbox, zPointArray& edgeCentres, zVectorArray& edgeVecs)
	{
		// edge centers
		edgeCentres.clear();
		edgeCentres.push_back((bbox[0] + bbox[1]) * 0.5f);
		edgeCentres.push_back((bbox[1] + bbox[2]) * 0.5f);
		edgeCentres.push_back((bbox[2] + bbox[3]) * 0.5f);
		edgeCentres.push_back((bbox[3] + bbox[0]) * 0.5f);

		// edge vectors
		edgeVecs.clear();
		edgeVecs.push_back(bbox[1] - bbox[0]);
		edgeVecs.push_back(bbox[2] - bbox[1]);
		edgeVecs.push_back(bbox[3] - bbox[2]);
		edgeVecs.push_back(bbox[0] - bbox[3]);

		int startId;
		 startId = edgeVecs[0].length() > edgeVecs[1].length() ? 0 : 1;
		 if (!longSide) startId++;

		 zIntArray ids;

		 ids.push_back((startId) % 4);
		 ids.push_back((startId + 2) % 4);
		 ids.push_back((startId + 1) % 4);
		 ids.push_back((startId + 3) % 4);

		 zVectorArray temp;

		 for (int i = 0; i < 4; i++)
			 temp.push_back(edgeCentres[ids[i]]);

		 edgeCentres = temp;
		 temp.clear();

		 for (int i = 0; i < 4; i++)
			 temp.push_back(edgeVecs[ids[i]]);

		 edgeVecs = temp;
	}

	void makeRectangle(zPoint anchorPoint, zVector alignVec, double dimX, double dimY, zPointArray& rectangle)
	{
		zPointArray rect;
		zVector cross = zVector(0, 0, 1) ^ alignVec;

		rect.push_back(anchorPoint);
		rect.push_back(anchorPoint + alignVec * dimX);
		rect.push_back((anchorPoint + alignVec * dimX) + cross * dimY);
		rect.push_back(anchorPoint + cross * dimY);

		rectangle = rect;
	}

	//constructor & deconstructor

	zPlot::zPlot() {}
	zPlot::~zPlot() {}

	zBlock::zBlock() {}
	zBlock::~zBlock() {}

	//plot methods

	void zPlot::create(zObjMesh& oMesh)
	{

	}

	void zPlot::create(zPointArray& positions, zVector alignVector)
	{
		plot = zPolygon(zPointArrayToVectors(positions));
		plotVector = alignVector;
		plotVector.normalize();
	}

	void zPlot::create(zPolygon& polygon, zVector alignVector)
	{
		plot = polygon;
		plotVector = alignVector;
		plotVector.normalize();
	}

	Vectors zPlot::getVectors()
	{
		return plot.getVectors();
	}

	Vector zPlot::getCenter()
	{
		return plot.countCenter();
	}

	double zPlot::getArea()
	{
		return plot.countSquare();
	}

	//block methods

	void zBlock::create(zObjMesh& oMesh)
	{

	}

	void zBlock::create(zPointArray& positions, zIntArray& pCounts)
	{
		int currentIndex = 0;
		zPointArray pos;
		blocks.clear(); 
		anchorPlot = zPlot();

		for (int i = 0; i < pCounts.size(); i++)
		{
			pos.clear();

			for (int j = currentIndex; j < currentIndex + pCounts[i]; j++)
				pos.push_back(positions[j]);

			blocks.push_back(zPolygon(zPointArrayToVectors(pos)));
			currentIndex += pCounts[i];
		}

		blocks_original = blocks;

		zPolygonsToMesh(blocks, blockMesh);

		zItMeshEdge e(blockMesh);
		for (; !e.end(); e++)
		{
			map_eId_age[e.getId()] = 0;
		}
	}

	void zBlock::create(vector<zPolygon>& polygons)
	{
		blocks = polygons;
		blocks_original = polygons;

		anchorPlot = zPlot();

		zPolygonsToMesh(blocks, blockMesh);

		zItMeshEdge e(blockMesh);
		for (; !e.end(); e++)
			map_eId_age[e.getId()] = 0;
	}

	void zBlock::split_block_rest(bool longside,bool b_field, bool closest)
	{
		bool split;

		do
		{
			split = false;

			//split
			vector<zPolygon> inPolys;
			vector<zPolygon> outPolys;
			inPolys = blocks;

			int size = blocks.size();

			for (int i = 0; i < size; i++)
			{
				bool foundAnchorPlot = find(anchorPlotIds.begin(), anchorPlotIds.end(), i) != anchorPlotIds.end();
				if (!foundAnchorPlot)
				{
					zPolygon poly;
					getPolygon(blockMesh, i, poly);

					zScalar val = 1;
					if (b_field)
					{
						zFnMeshScalarField fnField(blockField);
						zPoint centre(poly.countCenter().x, poly.countCenter().y, poly.countCenter().z);
						fnField.getFieldValue(centre, zFieldNeighbourWeighted, val);
						//fnField.getFieldValue(centre, zFieldIndex, val);
					}

					double targetArea_local = map_fId_targetArea[i];
					val = sinf(val) * targetArea_local;

					zVector alignVec_local = map_fId_alignVector[i];

					bool useDynamicMethod = false;
					if (alignVec_local.z > 0)useDynamicMethod = true;

					if (poly.countSquare() > val * 2)
					{
						split = true;
						zPoint temp;
						zPolygon poly1, poly2;
						zLine cutLine;

						if (!useDynamicMethod)
						{
							//align method
							split_pos_vec(true, closest, poly, temp, alignVec_local, poly1, poly2, cutLine);

							if (data.f0 < 0 || data.f0>1 || data.f1 < 0 || data.f1>1)
							{
								double area = 0.5 * poly.countSquare();
								split_area(area, poly, poly1, poly2, cutLine);
							}
						}
						else
						{
							//dynmamic method
							double area = 0.5 * poly.countSquare();
							split_area(area, poly, poly1, poly2, cutLine);
						}

						LOG_DEV << "id: " << i << endl;
						LOG_DEV << "virtual split success" << endl;

						//actural split
						zItMeshFace f(blockMesh, i);
						zIntArray heIds;
						f.getHalfEdges(heIds);

						zIntArray eIds;
						for (auto& id : heIds)
						{
							zItMeshHalfEdge he(blockMesh, id);
							eIds.push_back(he.getEdge().getId());
						}

						int maxEdgeValue = -1;
						for (auto& id : eIds)
						{
							if (maxEdgeValue < map_eId_age[id])
							{
								maxEdgeValue = map_eId_age[id];
							}
						}
						maxEdgeValue++;
						numIterations = maxEdgeValue;

						LOG_DEV << "fid: " << i << "\n";
						//LOG_BUILD << "e0: " << eIds[data.e0] << "," << "e1: " << eIds[data.e1] << "\n";
						LOG_DEV << "e0: " << data.e0 << "," << "e1: " << data.e1 << "\n";
						LOG_DEV << "f0: " << data.f0 << "," << "f1: " << data.f1 << "\n";

						zUtilsCore core;
						data.f0 = core.factorise(data.f0, 3);
						data.f1 = core.factorise(data.f1, 3);

						//snap start and end
						//snapCutLine(0.1);

						zFnMesh fn(blockMesh);
						fn.splitFace(i, eIds[data.e0], eIds[data.e1], data.f0, data.f1);

						LOG_DEV << "id: " << i << endl;
						LOG_DEV << "actual split success" << endl;
						LOG_DEV << "--------------------------" << endl;

						//update face map
						map_fId_targetArea[fn.numPolygons() - 1] = targetArea_local;
						map_fId_alignVector[fn.numPolygons() - 1] = alignVec_local;

						//update edge data after face split
						int counter = -1;
						map_eId_age[fn.numEdges() + counter] = maxEdgeValue;
						map_eId_level[fn.numEdges() + counter] = internalRoadLevel;

						counter--;
						if (data.f1 != 0 && data.f1 != 1)
						{
							map_eId_age[fn.numEdges() + counter] = map_eId_age[eIds[data.e1]];
							map_eId_level[fn.numEdges() + counter] = map_eId_level[eIds[data.e1]];
							counter--;
						}
						if (data.f0 != 0 && data.f0 != 1)
						{
							map_eId_age[fn.numEdges() + counter] = map_eId_age[eIds[data.e0]];
							map_eId_level[fn.numEdges() + counter] = map_eId_level[eIds[data.e0]];
						}

						if (data.f0 < 0.05 || data.f1 < 0.05 || data.f0>0.95 || data.f1>0.95)
						{
							LOG_DEV << "f0: " << data.f0 << "," << "f1: " << data.f1 << "\n";
						}
					}
				}
			}

			meshToPolygons(blockMesh, blocks);
		} while (split);
	}

	void zBlock::split_anchor_edge_aligned(int blockId, float dimX, float dimY, float offset, int eId1, int eId2)
	{
		zVector alignVector = map_fId_alignVector[blockId];

		if (dimX * dimY < getArea(blockId))
		{
			zPointArray rect;
			bool flip = false;

			if (eId1 != -1 && eId2 == -1)
			{
				archorOnEdge(dimX, dimY, blockId, eId1, alignVector, true, rect, flip);
			}
			else if (eId1 == -1 && eId2 != -1)
			{
				archorOnEdge(dimX, dimY, blockId, eId2, alignVector, true, rect, flip);
			}
			else if (eId1 != -1 && eId2 != -1)
			{
				anchorOnCorner(dimX, dimY, blockId, eId1, eId2, alignVector, true, rect, flip);
			}
			else if (eId1 == -1 && eId2 == -1)
			{
				archorOnCenter(dimX, dimY, blockId, alignVector, rect, flip);
			}

			anchorPlot.create(rect, alignVector);

			//LOG_DEV << endl;
			//for (auto& p : rect)
			//	LOG_DEV << p << endl;

			split_around_anchor(flip, offset, offset);
		}
		else
		{
			anchorPlot.create(blocks[blockId], alignVector);
			anchorPlotIds.push_back(blockId);
		}

		meshToPolygons(blockMesh, blocks);
	}

	//utilities

	int zBlock::snapCutLine(double eps)
	{
		data.f0 = data.f0 < eps ? 0 : data.f0;
		data.f0 = data.f0 > (1 - eps) ? 1 : data.f0;
		data.f1 = data.f1 < eps ? 0 : data.f1;
		data.f1 = data.f1 > (1 - eps) ? 1 : data.f1;

		return 1;
	}

	//field

	void zBlock::createField(int numX, int numY)
	{
		zPointArray positions;
		for (int i = 0; i < blocks.size(); i++)
		{
			Vectors vecs = blocks[i].getVectors();
			zPointArray temp = VectorsToPointArray(vecs);
			positions.insert(positions.end(), temp.begin(), temp.end());
		}
		zUtilsCore core;
		zPoint minBB, maxBB;
		core.getBounds(positions, minBB, maxBB);
		zFnMeshScalarField fnField(blockField);
		fnField.create(minBB, maxBB, numX, numY, 1, 1, 0);
	}

	void zBlock::createField(zObjMesh& mesh, int numX, int numY)
	{
		zFnMesh fn(mesh);

		zPoint minBB, maxBB;
		fn.getBounds(minBB, maxBB);

		zFnMeshScalarField fnField(blockField);
		fnField.create(minBB, maxBB, numX, numY, 1, true, false);

		zFloatArray fVals;
		fVals.assign(fnField.numFieldValues(), 0);

		zItMeshVertex v(mesh);
		for (; !v.end(); v++)
		{
			zIntArray neighbours;

			zPoint vPos(v.getPosition().x, v.getPosition().y, 0);
			fnField.getNeighbour_Contained(vPos, neighbours);

			for (auto& id : neighbours)
			{
				zItMeshScalarField it(blockField, id);
				fVals[id] += v.getColor().r;
			}
		}

		fnField.setFieldValues(fVals);
		fnField.updateColors();
	}

	void zBlock::createField(string path, zFileTpye type)
	{
		if (fs::exists(path))
		{
			json j;
			zUtilsCore core;
			core.json_read(path, j);

			zFnMeshScalarField fnField(blockField);
			fnField.from(path, zJSON, true, false);

			zObjMesh temp;
			zFnMesh fn(temp);
			fn.from(path, zJSON);

			int numX = j["FieldX"].get<int>();
			int numY = j["FieldY"].get<int>();

			createField(temp, numX, numY);

			vector<float> vals = j["FieldValues"].get<vector<float>>();
			fnField.setFieldValues(vals);
			fnField.updateColors();
		}
	}

	void zBlock::setFieldScalar(double& _min, double& _max)
	{
		zUtilsCore core;
		zFnMeshScalarField fnField(blockField);
		zFloatArray fVals;
		fnField.getFieldValues(fVals);

		float min = core.zMin(fVals);
		float max = core.zMax(fVals);
		zDomainFloat inDomain(min, max);
		zDomainFloat outDomain(_min, _max);
		for (auto& val : fVals)
		{
			val = core.ofMap(val, inDomain, outDomain);
		}

		fnField.setFieldValues(fVals);
		fnField.updateColors();
	}

	void zBlock::addField(zIntArray eIds, zDoubleArray eVals, int wt, zFieldType type)
	{
		switch (type)
		{
		case zSpace::RoadField:
		{
			zPointArray positions;
			getVertexPositions(positions);
			zFnMeshScalarField fnField(blockField);

			vector<zScalarArray> scalars_temp(eIds.size(), zScalarArray());
			zScalarArray scalars;
			scalars_temp.assign(eIds.size(), zScalarArray());

			auto& values = map_field_type_values[type];
			auto& weight = map_field_type_weight[type];

			for (int i = 0; i < eIds.size(); i++)
			{

				zPoint v0 = positions[eIds[i]];
				zPoint v1 = positions[(eIds[i] + 1) % positions.size()];
				fnField.getScalars_Line(scalars_temp[i], v0, v1, eVals[i], false);

				if (values.size() > 0)
				{
					fnField.boolean_union(scalars_temp[i], values, scalars, false);
					values = scalars;
					weight = wt;
				}
				else
				{
					values = scalars_temp[i];
					weight = wt;
				}
			}

			fnField.setFieldValues(values, zFieldColorType::zFieldRegular);

			//scalars_temp[0]

			break;
		}

		default:
			break;
		}
	}

	void zBlock::computeField()
	{

	}

	//set

	void zBlock::setEdgeLevels(zIntArray& edgeLevels)
	{
		map_eId_level.clear();

		zFnMesh fn(blockMesh);

		internalRoadLevel = 0;
		for (int i = 0; i < fn.numEdges(); i++)
		{
			map_eId_level[i] = edgeLevels[i];

			internalRoadLevel = (internalRoadLevel < edgeLevels[i]) ? edgeLevels[i] : internalRoadLevel;
		}
		internalRoadLevel++;
	}

	void zBlock::setTargetAreas(vector<double>& targetAreas)
	{
		map_fId_targetArea.clear();

		zFnMesh fn(blockMesh);

		for (int i = 0; i < fn.numPolygons(); i++)
			map_fId_targetArea[i] = targetAreas[i];
	}

	void zBlock::setAlignVectors(vector<zVector>& alignVectors)
	{

		map_fId_alignVector.clear();

		zFnMesh fn(blockMesh);

		for (int i = 0; i < fn.numPolygons(); i++)
			map_fId_alignVector[i] = alignVectors[i];
	}

	//get

	int zBlock::getNumBlocks()
	{
		return blocks.size();
	}

	int zBlock::getNumIterations()
	{
		return numIterations;
	}

	//export

	void zBlock::dumpJSON(string path)
	{
		json j;
		zFnMesh fn(blockMesh);

		//zVectorArray vNormalTmp;
		//fn.getVertexNormals(vNormalTmp);
		//printf("\n  v %i, vn %i ", fn.numVertices(), vNormalTmp.size());

		//zVectorArray fNormalTmp;
		//fn.getFaceNormals(fNormalTmp);
		//printf("\n  v %i, vn %i ", fn.numPolygons(), fNormalTmp.size());

		//set normals
		zVectorArray fNormals;
		fNormals.assign(fn.numPolygons(), zVector(0, 0, 1));
		fn.setFaceNormals(fNormals, false);

		zVectorArray vNormals;
		vNormals.assign(fn.numVertices(), zVector(0, 0, 1));
		fn.setVertexNormals(vNormals);

		fn.to(j);

		j["AnchorIds"] = anchorPlotIds;

		vector<double> j_blockVector;
		j_blockVector.push_back(blockVector.x);
		j_blockVector.push_back(blockVector.y);
		j_blockVector.push_back(blockVector.z);

		//block vectors
		vector<vector<float>> j_blockVectors;
		j_blockVectors.assign(fn.numEdges(), vector<float>());
		for (auto& it : map_fId_alignVector)
		{
			vector<float> vec = { it.second[0],it.second[1],it.second[2] };
			j_blockVectors[it.first] = vec;
		}

		vector<float> j_vector_tmp;
		zVector tmp = map_fId_alignVector[0];
		j_vector_tmp = { tmp[0],tmp[1],tmp[2] };

		//edge ages
		vector<int> j_edgeAges;
		j_edgeAges.assign(fn.numEdges(), 0);
		for (auto& it : map_eId_age)
			j_edgeAges[it.first] = it.second;

		//edge levels
		vector<int> j_edgeLevels;
		j_edgeLevels.assign(fn.numEdges(), 0);
		for (auto& it : map_eId_level)
			j_edgeLevels[it.first] = it.second;

		j["InternalRoadLevel"] = internalRoadLevel;
		j["EdgeAges"] = j_edgeAges;
		j["EdgeLevels"] = j_edgeLevels;
		//j["BlockVector"] = j_blockVectors;
		j["BlockVector"] = j_vector_tmp;

		//write to json
		std::ofstream outFile(path);
		
		outFile << std::setw(4) << j << std::endl;
		outFile.close();
	}


	//---- PROTECTED

	void zBlock::split_area(double area, zPolygon& block, zPolygon& poly1, zPolygon& poly2, zLine& cutLine)
	{
		block.split(area, poly1, poly2, cutLine);
		block.getData(data.e0, data.e1, data.f0, data.f1);
	}

	int zBlock::split_pos_vec(bool b_equalise, bool closest, zPolygon& block, zPoint position, zVector alignVec, zPolygon& poly1, zPolygon& poly2, zLine& cutLine)
	{
		Vector centre(position.x, position.y, position.z);
		Vector move;

		if (b_equalise)
		{
			//get default cut
			double area = 0.5 * block.countSquare();
			block.split(area, poly1, poly2, cutLine);

			//get centre
			centre = cutLine.getStart() + cutLine.getEnd();
			centre *= 0.5;
		}

		if (closest)
		{
			Vector temp = cutLine.getEnd() - cutLine.getStart();
			zVector vec(temp.x, temp.y, temp.z);
			alignVec = closestOrthogonalVector(vec, alignVec);
		}

		move = Vector(alignVec.x, alignVec.y, alignVec.z);
		cutLine = zLine(centre, centre + move);

		//make new cut aligned to alignVec
		for (int i = 0; i < block.size() - 1; i++)
		{
			for (int j = i + 1; j < block.size(); j++)
			{
				zLine l1 = zLine(block[i], block[i + 1]);
				zLine l2 = zLine(block[j], block[(j + 1) < block.size() ? (j + 1) : 0]);

				Vector start, end;
				int cut1 = cutLine.crossLineSegment(l1, start);
				int cut2 = cutLine.crossLineSegment(l2, end);

				if (cut1 && cut2)
				{
					cutLine = zLine(start, end);

					data.e0 = i;
					data.e1 = j;
					data.f0 = (start - l1.getStart()).length() / l1.length();
					data.f1 = (end - l2.getStart()).length() / l2.length();

					//LOG_DEV << endl;
					//LOG_DEV << "start:" << start.x << "," << start.y << "," << start.z << endl;
					//LOG_DEV << "end:" << end.x << "," << end.y << "," << end.z << endl;

					//LOG_DEV << "e0:" << data.e0 << " " << "f0: " << data.f0 << " " << "e1:" << data.e1 << " " << "f1:" << data.f1 << endl;

					return 1;
				}
			}
		}
		return 0;
	}

	void zBlock::split_around_anchor(bool longside, float offsetX, float offsetY)
	{
		//get bbox info
		zPointArray pos = VectorsToPointArray(anchorPlot.getVectors());

		zPointArray bbox;
		getBestFitBoundingBox(pos, bbox, offsetX, offsetY);

		zPointArray edgeCentres;
		zVectorArray edgeVectors;
		getBBoxVectors(longside, bbox, edgeCentres, edgeVectors);

		//split
		vector<zPolygon> inPolys;
		vector<zPolygon> outPolys;
		inPolys = blocks;

		zUtilsCore core;

		int numSplit = 0;

		for (int i = 0; i < edgeCentres.size(); i++)
		{
			LOG_DEV << endl;
			LOG_DEV << "inPolys.size():" << inPolys.size() << endl;

			for (int j = 0; j < inPolys.size(); j++)
			{
				Vectors vecs = inPolys[j].getVectors();
				bool isInPoly = core.pointInPlanarPolygon(edgeCentres[i], VectorsToPointArray(vecs), zVector(0, 0, 1));

				if (isInPoly)
				{
					//LOG_DEV << "face:" << j << endl;

					zPolygon poly1, poly2;
					zLine cutLine;

					int check = split_pos_vec(false, false, inPolys[j], edgeCentres[i], edgeVectors[i], poly1, poly2, cutLine);

					if (check)
					{

						//actural split
						zItMeshFace f(blockMesh, j);
						zIntArray heIds;
						f.getHalfEdges(heIds);

						zIntArray eIds;
						for (auto& id : heIds)
						{
							zItMeshHalfEdge he(blockMesh, id);
							eIds.push_back(he.getEdge().getId());
						}

						//LOG_DEV << "face: " << j << "e0: " << eIds[data.e0] << "e1 : " << eIds[data.e1] << endl;

						int maxEdgeValue = -1;
						for (auto& id : eIds)
						{
							if (maxEdgeValue < map_eId_age[id])
							{
								maxEdgeValue = map_eId_age[id];
							}
						}

						if (numSplit != 1) maxEdgeValue++;
						numSplit++;

						zFnMesh fn(blockMesh);
						fn.splitFace(j, eIds[data.e0], eIds[data.e1], data.f0, data.f1);

						//update face map
						map_fId_targetArea[fn.numPolygons() - 1] = map_fId_targetArea[j];
						map_fId_alignVector[fn.numPolygons() - 1] = map_fId_alignVector[j];

						//update edge data after face split
						int counter = -1;
						map_eId_age[fn.numEdges() + counter] = maxEdgeValue;
						map_eId_level[fn.numEdges() + counter] = internalRoadLevel;

						counter--;
						if (data.f1 != 0 && data.f1 != 1)
						{
							map_eId_age[fn.numEdges() + counter] = map_eId_age[eIds[data.e1]];
							map_eId_level[fn.numEdges() + counter] = map_eId_level[eIds[data.e1]];
							counter--;
						}
						if (data.f0 != 0 && data.f0 != 1)
						{
							map_eId_age[fn.numEdges() + counter] = map_eId_age[eIds[data.e0]];
							map_eId_level[fn.numEdges() + counter] = map_eId_level[eIds[data.e0]];
						}

						if (data.f0 < 0.05 || data.f1 < 0.05 || data.f0>0.95 || data.f1>0.95)
						{
							LOG_DEV << "f0: " << data.f0 << "," << "f1: " << data.f1 << "\n";
						}

					}

					//update polygons
					meshToPolygons(blockMesh, inPolys);
					break;
				}
			}
		}

		meshToPolygons(blockMesh, blocks);

		for (int j = 0; j < blocks.size(); j++)
		{
			Vector centre;
			centre = anchorPlot.getCenter();
			zPoint pt(centre.x, centre.y, centre.z);
			Vectors vecs = blocks[j].getVectors();

			bool isInPoly = core.pointInPlanarPolygon(pt, VectorsToPointArray(vecs), zVector(0, 0, 1));

			if (isInPoly)
			{
				anchorPlotIds.push_back(j);
				break;
			}
		}
	}

	void zBlock::getPolygonData(zPointArray& positions, zIntArray pCounts, zIntArray pConnects)
	{
		zFnMesh fn(blockMesh);

		fn.getVertexPositions(positions);
		fn.getPolygonData(pConnects, pCounts);
	}

	void zBlock::getVertexPositions(zPointArray& positions)
	{
		positions.clear();
		for (auto& b : blocks_original)
		{
			zPointArray pos;
			Vectors vecs = b.getVectors();
			pos = VectorsToPointArray(vecs);
			positions.insert(positions.end(), pos.begin(), pos.end());
		}
	}

	double zBlock::getArea(int id)
	{
		return blocks[id].countSquare();
	}


	void zBlock::archorOnEdge(double dimX, double dimY, int blockId, int eId1, zVector northVec, bool alignToEdge, zPointArray& rectangle, bool& flip)
	{
		northVec = northVec ^ zVector(0, 0, 1);
		flip = false;

		zVector alignVec;
		zVector anchorPoint;
		int alignTo = eId1;

		Vectors vectors = blocks[blockId].getVectors();
		zPointArray vertices = VectorsToPointArray(vectors);
		int size = vertices.size();
		anchorPoint = (vertices[alignTo % size] + vertices[(alignTo + 1) % size]) * 0.5;

		if (alignToEdge)
		{
			alignVec = vertices[(alignTo + 1) % size] - vertices[alignTo % size];

			//check edge length
			if (dimX > alignVec.length())
			{
				double area = dimX * dimY;
				dimX = alignVec.length();
				dimY = area / dimX;
				flip = true;
			}
		}
		else
			alignVec = northVec;


		alignVec.normalize();

		anchorPoint += alignVec * dimX * -0.5;

		makeRectangle(anchorPoint, alignVec, dimX, dimY, rectangle);
	}

	void zBlock::anchorOnCorner(double dimX, double dimY, int blockId, int eId1, int eId2, zVector northVec, bool alignToEdge, zPointArray& rectangle, bool& flip)
	{
		northVec = northVec ^ zVector(0, 0, 1);
		flip = false;

		zVector alignVec, checkVec;
		zVector anchorPoint;
		int alignTo = eId1;

		Vectors vectors = blocks[blockId].getVectors();
		zPointArray vertices = VectorsToPointArray(vectors);
		int size = vertices.size();
		anchorPoint = vertices[(alignTo + 1) % size];
		alignVec = vertices[(alignTo + 1) % size] - vertices[alignTo % size];
		
		if (alignToEdge)
		{
			alignVec = vertices[(alignTo + 1) % size] - vertices[alignTo % size];

			//check edge length
			if (dimX > alignVec.length())
			{
				double area = dimX * dimY;
				dimX = alignVec.length();
				dimY = area / dimX;
				flip = true;
			}
		}
		else
			alignVec = northVec;

		alignVec.normalize();

		anchorPoint += alignVec * dimX * -1;

		//set back if sharp corner
		checkVec = vertices[(alignTo + 2) % size] - vertices[(alignTo + 1) % size];
		checkVec.normalize();

		double dot = alignVec.x * checkVec.x + alignVec.y * checkVec.y;
		double cross = alignVec.x * checkVec.y + alignVec.y * checkVec.x;

		//LOG_DEV << endl;
		//LOG_DEV << "dot:" << dot << endl;
		//LOG_DEV << "cross:" << cross << endl;

		if (dot < 0)
			anchorPoint += alignVec * (dimY / (cross / dot)) * -1;

		makeRectangle(anchorPoint, alignVec, dimX, dimY, rectangle);
	}

	void zBlock::archorOnCenter(double dimX, double dimY, int blockId, zVector northVec, zPointArray& rectangle, bool& flip)
	{
		if(flip) northVec = northVec ^ zVector(0, 0, 1);

		zVector alignVec;
		zVector anchorPoint;

		Vectors vectors = blocks[blockId].getVectors();
		zPointArray vertices = VectorsToPointArray(vectors);
		int size = vertices.size();

		Vector blockCenter = blocks[blockId].countCenter();
		anchorPoint = zPoint(blockCenter.x, blockCenter.y, blockCenter.z);

		alignVec = northVec;
		alignVec.normalize();
		anchorPoint += alignVec * dimY * -0.5;
		alignVec = northVec ^ zVector(0, 0, 1);
		anchorPoint += alignVec * dimX * -0.5;

		makeRectangle(anchorPoint, alignVec, dimX, dimY, rectangle);
	}

	//---- CONSTRUCTOR

	zTsCityEngine::zTsCityEngine() {}

	//---- DESTRUCTOR

	zTsCityEngine::~zTsCityEngine() {}

	void zTsCityEngine::updateBlocks()
	{

	}

	void zTsCityEngine::readBlockData(string path, vector<zBlock>blocks)
	{
		zUtilsCore coreUtil;

		zPointArray blockPos;
		zIntArray blockCount;
		vector<zAnchorData> anchorData;

		json j;
		coreUtil.json_read(path, j);

		numBlocks_initial = j["NumBlocks"].get<int>();
		vector<vector<double>> pos = j["BlockPositions"].get<vector<vector<double>>>();
		blockCount = j["BlockCount"].get<vector<int>>();
		for (const auto& ad : j["AnchorData"]) {
			zAnchorData data;
			data.blockId = ad["BlockId"];
			data.dimX = ad["DimX"];
			data.dimY = ad["DimY"];
			data.eId1 = ad["EId1"];
			data.eId2 = ad["EId2"];
			anchorData.push_back(data);
		}

		//assign blocks
		blocks.assign(numBlocks_initial, zBlock());

		//for (int i = 0; i < numBlocks_initial; i++)
		//{
		//	blocks[i].create()
		//}







		blockPos.assign(pos.size(), zPoint());
		for (int i = 0; i < pos.size(); i++)
		{
			zPoint p(pos[i][0], pos[i][1], pos[i][2]);
			blockPos[i] = p;
		}




		//edgeLevels = j["EdgeLevels"].get<vector<int>>();

		//block.targetArea = j["TargetArea"].get<double>();
		//vector<double> vec = j["NorthVector"].get<vector<double>>();
		//block.blockVector = zVector(vec[0], vec[1], vec[2]);

		//field
		//eVals = j["BlockEdgeProperties"].get<vector<double>>();
		//eIds.clear();
		//for (int i = 0; i < eVals.size(); i++)
		//	eIds.push_back(i);

	}
}