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


#include <userSrc/tzSketches/Configurator3d/zTsConfigurator3d.h>

namespace zSpace
{
	// zGameObj
	//
	// Public

	zGameObj::zGameObj() {};

	zGameObj::zGameObj(zObjMesh* _voxel, string _assetTypeName)
		:voxel(_voxel), assetTypeName(_assetTypeName), vertex(nullptr), graph(nullptr),
		neighbour_up(-1), neighbour_down(-1), orientation(zOrientation::bottom)
	{
		transformation.setIdentity();
	}

	zGameObj::~zGameObj() {};

	void zGameObj::computeOrientation()
	{
		zVector xAxis(1, 0, 0);
		zVector zAxis(0, 0, 1);
		orientation = zOrientation::single;

		if (vertex!=nullptr)
		{
			if (!vertex->checkValency(0))
			{
				// check up and down
				zIntArray cHes;
				vertex->getConnectedHalfEdges(cHes);
				for (int id : cHes)
				{
					zItGraphHalfEdge he(*graph, id);
					int neighbourId = he.getVertex().getId();
					float dot = he.getVector() * zAxis;
					if (dot > 1 - EPS)
					{
						neighbour_up = neighbourId;
						quadrants[4] = true;
					}
					else if (dot < EPS - 1)
					{
						neighbour_down = neighbourId;
						quadrants[5] = true;
					}
					else
						neighbour_level.push_back(neighbourId);
				}

				// check horizontal
				vector<pair<float, zVector>> angleVecPairs;
				vector<pair<float, int>> angleIdPairs;

				for (auto& id : neighbour_level)
				{
					zItGraphVertex vNeighbour(*graph, id);
					zVector vec = vNeighbour.getPosition() - vertex->getPosition();
					float angle = vec.angle360(xAxis, zAxis);
					angle = angle == 0 ? 0 : 360 - angle;

					angleVecPairs.emplace_back(angle, vec);
					angleIdPairs.emplace_back(angle, id);
				}

				// update quadrant
				if (quadrants[4] && quadrants[5]) orientation = zOrientation::middle;
				else if (quadrants[4] && !quadrants[5]) orientation = zOrientation::bottom;
				else if (!quadrants[4] && quadrants[5]) orientation = zOrientation::top;

				// Sort the angle-vec pairs based on angles
				std::sort(angleVecPairs.begin(), angleVecPairs.end(),
					[](const std::pair<float, zVector>& a, const std::pair<float, zVector>& b) {
						return a.first < b.first;
					});

				// Sort the angle-id pairs based on angles
				std::sort(angleIdPairs.begin(), angleIdPairs.end(),
					[](const std::pair<float, int>& a, const std::pair<float, int>& b) {
						return a.first < b.first;
					});

				neighbour_level.clear();
				for (auto& item : angleIdPairs) neighbour_level.push_back(item.second);
					


				LOG_DEV << "--------" << endl;
				LOG_DEV << "vId: " << vertex->getId() << endl;
				for (auto& item : angleVecPairs)
					LOG_DEV << item.first << ", ";

				LOG_DEV << endl;
				LOG_DEV << "neighbour Id: ";
				for (auto& id : neighbour_level)
					LOG_DEV << id << ", ";

				computeLocalTransformation(angleVecPairs);

			}
		}
	}

	void zGameObj::computeLocalTransformation(vector<pair<float, zVector>>& angleVecPairs)
	{
		zVector xAxis(1, 0, 0);
		zVector zAxis(0, 0, 1);

		int valency = neighbour_level.size();
		int firstAngle = angleVecPairs[0].first;
		int rotation = 90;
		for (auto& item : angleVecPairs)
		{
			int m = (int)(item.first - firstAngle) / rotation;
			quadrants[m] = true;
		}

		string levelName = orientToString(orientation);
		string typeName = "A";
		// compute axis
		int alignVecId = 0;
		switch (valency)
		{
		case 1:
			break;
		case 2:
			if (quadrants[0] && quadrants[3])
			{
				alignVecId = 1;
				typeName = "B";
			}
			else if (quadrants[0] && quadrants[1])
			{
				alignVecId = 0;
				typeName = "B";
			}
			else
			{
			}

			switch (orientation)
			{
			case zSpace::zOrientation::bottom:
				break;
			case zSpace::zOrientation::middle:
				break;
			case zSpace::zOrientation::top:
				break;
			case zSpace::zOrientation::single:
				break;
			default:
				break;
			}

			break;
		case 3:
			if (quadrants[0] && quadrants[2] && quadrants[3]) alignVecId = 1;
			if (quadrants[0] && quadrants[1] && quadrants[3]) alignVecId = 2;
			break;
		case 4:
			break;
		default:
			break;
		}
		assetName = levelName + "_" + typeName + "_" + to_string(valency);

		LOG_DEV << endl;
		for (auto& q : quadrants)
			LOG_DEV << q << ",";
		LOG_DEV << endl << assetName << endl;
		LOG_DEV << endl << "--------" << endl;



		// make transform
		zVector frameX = angleVecPairs[alignVecId].second;
		frameX.normalize();
		zVector frameY = zAxis ^ frameX;
		zVector frameZ = zAxis;

		// update transform
		transformation.setIdentity();
		transformation(0, 0) = frameX.x;	transformation(0, 1) = frameY.x;	transformation(0, 2) = frameZ.x;
		transformation(1, 0) = frameX.y;	transformation(1, 1) = frameY.y;	transformation(1, 2) = frameZ.y;
		transformation(2, 0) = frameX.z;	transformation(2, 1) = frameY.z;	transformation(2, 2) = frameZ.z;

		// Set the translation part (3x1 vector, last column)
		zVector translation = vertex->getPosition();
		transformation(0, 3) = translation.x;
		transformation(1, 3) = translation.y;
		transformation(2, 3) = translation.z;
	}

	string zGameObj::orientToString(zOrientation orientation)
	{
		switch (orientation)
		{
		case zOrientation::bottom:
			return "BOT";
		case zOrientation::middle:
			return "MID";
		case zOrientation::top:
			return "TOP";
		case zOrientation::single:
			return "SIN";
		default:
			return "UNKNOWN";
		}
	}


	zFloorMap::zFloorMap() {};
	zFloorMap::zFloorMap(float _unitX, float _unitY, int _numX, int _numY, zPoint _minBB)
	{
		unitX = _unitX;
		unitY = _unitY;

		zFnMeshScalarField fnField(floorField);
		fnField.create(unitX, unitY, _numX, _numY, _minBB);
		floorValues.assign(unitX * unitY, false);
	}

	zFloorMap::~zFloorMap() {};


	// zConfigurator3d
	// 
	// Public

	zConfigurator3d::zConfigurator3d() {};
	zConfigurator3d::~zConfigurator3d() {};

	void zConfigurator3d::initialise()
	{
		gameObjs.clear();

		// initialise file locations based on user configuration
		init_userConfig();
		//usd_readFile(path_assets);

		// read assets and construct voxels
		vector<string> filePaths;
		usd_readFolder(path_assets_folder, "Voxels", filePaths);

		for (auto& file : filePaths)
		{
			LOG_DEV << file << endl;
			usd_readFile(file);
		}

		// initialise graph and game objs
		zObjMeshPointerArray voxels;
		for (auto& obj : gameObjs)
			voxels.push_back(obj->voxel);
		computeDualGraph(voxels, connectionGraph);
		computeGameObjs();

		// initialise floor maps
		init_floorMaps();
	}

	void zConfigurator3d::compute()
	{
		populateAssets(path_assets, path_agg);
	}

	// Private
	void zConfigurator3d::populateAssets(string path_assets, string path_agg)
	{
		//UsdStageRefPtr stage = UsdStage::Open(path_assets);

		UsdStageRefPtr stage_out;
		usd_createStage(path_agg, "Type", stage_out);

		if (stage_out)
		{
			//reference geometry
			int counter = 0;
			for (auto gameObj : gameObjs)
			{
				string path_stage = path_assets_folder;
				path_stage += "/Type/Assets/" + gameObj->assetTypeName + "/" + gameObj->assetName + ".usd";
				//string assetPrimPath("/Type/Assets/" + gameObj->assetTypeName + "/" + gameObj->assetName);
				//UsdPrim assetPrim = stage->GetPrimAtPath(SdfPath(assetPrimPath));

				string refGeoPrimPath("/Type/" + gameObj->assetTypeName + "/References/Ref" + to_string(counter));

				//LOG_DEV << "----" << endl;
				//LOG_DEV << assetPrim.GetPrimPath().GetString() << endl;

				//UsdPrim newGeoPrim = stage_out->OverridePrim(SdfPath(assetPrimPath + to_string(counter)));
				UsdPrim refGeoPrim = stage_out->DefinePrim(SdfPath(refGeoPrimPath));
				//refGeoPrim.GetReferences().AddReference(stage->GetRootLayer()->GetIdentifier(), assetPrim.GetPrimPath());

				// clear transformation
				UsdStageRefPtr stage = UsdStage::Open(path_stage);
				string transformPath = "/Type/Assets/" + gameObj->assetTypeName + "/" + gameObj->assetName;
				UsdGeomXformable tempPrim(stage->GetPrimAtPath(SdfPath(transformPath)));
				tempPrim.ClearXformOpOrder();
				stage->GetRootLayer()->Save();

				// add reference
				refGeoPrim.GetReferences().AddReference(path_stage);
				refGeoPrim.SetInstanceable(true);

				// new material binding
				//UsdShadeMaterialBindingAPI materialBindingAPI = UsdShadeMaterialBindingAPI::Apply(refGeoPrim.GetPrim());
				//materialBindingAPI.Bind(UsdShadeMaterial::Get(stage, SdfPath("/Materials/SlabVoxels_1_GREYSG")));


				// add transformation to newGeoPrim
				UsdGeomXformable xformable(refGeoPrim);

				// Apply transformation
				auto& t = gameObj->transformation;
				GfMatrix4d transform;
				transform.Set(t(0, 0), t(1, 0), t(2, 0), t(3, 0),
					t(0, 1), t(1, 1), t(2, 1), t(3, 1),
					t(0, 2), t(1, 2), t(2, 2), t(3, 2),
					t(0, 3), t(1, 3), t(2, 3), t(3, 3));

				xformable.ClearXformOpOrder();
				xformable.AddTransformOp(UsdGeomXformOp::PrecisionDouble).Set(transform);

				counter++;
			}
		}
		stage_out->Save();
	}

	void zConfigurator3d::computeGameObjs()
	{
		zFnGraph fnGraph(connectionGraph);

		for (int i = 0; i < fnGraph.numVertices(); i++)
		{
			// register voxel and vertex
			gameObjs[i]->vertex = new zItGraphVertex(connectionGraph, i);
			gameObjs[i]->graph = &connectionGraph;

			// initialise quadrants
			gameObjs[i]->quadrants.assign(6, false);

			// check orientation
			gameObjs[i]->computeOrientation();
		}
	}

	bool zConfigurator3d::checkNeighbourOrientation_level(int objId, zOrientation checkOrientation, int num)
	{
		int found = 0;
		for (int i = 0; i < gameObjs[objId]->neighbour_level.size(); i++)
		{
			if (gameObjs[i]->orientation == checkOrientation)
				found++;
		}
		return found == num;
	}

	bool zConfigurator3d::checkNeighbourOrientation_vertical(int objId, zOrientation checkOrientation, int num)
	{
		int found = 0;
		if (gameObjs[gameObjs[objId]->neighbour_up]->orientation == checkOrientation)found++;
		if (gameObjs[gameObjs[objId]->neighbour_down]->orientation == checkOrientation)found++;
		return found == num;
	}

	void zConfigurator3d::computeDualGraph(zObjMeshPointerArray& meshes, zObjGraph& dualGraph)
	{
		zPointArray positions;
		zIntArray pConnects;
		zColorArray vCols;

		// Check neighbours
		unordered_map <int, vector<int>> map_id_neighbourIds;
		float distTol = 0.01;

		for (int i = 0; i < meshes.size(); i++)
		{
			zColor vCol;
			bool isIsolated = true;
			for (int j = i + 1; j < meshes.size(); j++)
			{
				zItMeshFace f(*meshes[i]);
				for (; !f.end(); f++)
				{
					zItMeshFace fNext(*meshes[j]);
					for (; !fNext.end(); fNext++)
					{
						if (f.getCenter().distanceTo(fNext.getCenter()) < distTol /*&&
							f.getColor() == fNext.getColor()*/)
						{
							map_id_neighbourIds[i].push_back(j);
							vCol = f.getColor();
							break;
						}
					}
				}
			}
				zFnMesh fn(*meshes[i]);
				positions.push_back(fn.getCenter());
				vCols.push_back(vCol);
		}

		for (auto& entry : map_id_neighbourIds)
		{
			for (auto& id : entry.second)
			{
				//left
				pConnects.push_back(entry.first);
				//right
				pConnects.push_back(id);

				//cout << entry.first << "," << id << endl;
			}
		}

		zFnGraph fnGraph(dualGraph);
		fnGraph.create(positions, pConnects);
		fnGraph.setVertexColors(vCols, true);

		LOG_BUILD << "V:" << fnGraph.numVertices() << " E:" << fnGraph.numEdges() << endl;
	}

	void zConfigurator3d::init_floorMaps()
	{
		float unitX, unitY, unitZ;
		for (zGameObj* obj : gameObjs)
		{
			if (obj->vertex->getValence() > 2 && obj->quadrants[4])
			{
				unitX = obj->vertex->getPosition().distanceTo(gameObjs[obj->neighbour_level[0]]->vertex->getPosition());
				unitY = obj->vertex->getPosition().distanceTo(gameObjs[obj->neighbour_level[1]]->vertex->getPosition());
				unitZ = obj->vertex->getPosition().distanceTo(gameObjs[obj->neighbour_up]->vertex->getPosition());
				break;
			}
		}

		cout << "unitX: " << unitX << endl;
		cout << "unitY: " << unitY << endl;
		cout << "unitZ: " << unitZ << endl;

		zPoint minBB, maxBB;
		zFnGraph fnGraph(connectionGraph);
		fnGraph.getBounds(minBB, maxBB);

		int numX = (maxBB.x - minBB.x) / unitX;
		int numY = (maxBB.y - minBB.y) / unitY;
		int numZ = (maxBB.z - minBB.z) / unitZ;
		numZ += 1;


		for (int i = 0; i < numZ; i++)
		{
			zPoint localMinBB(minBB.x, minBB.y, minBB.z + unitZ * i);
			floorMaps.push_back(new zFloorMap(unitX, unitY, numX, numY, localMinBB));
		}
	}

	// Generic - USD
	bool zConfigurator3d::init_userConfig()
	{
		json j;
		if (json_read(path_userConfig, j))
		{
			path_assets = j["path_assets"].get<string>();
			path_agg = j["path_aggregation"].get<string>();

			path_assets_folder = j["path_assets_folder"].get<string>();

			LOG_DEV << endl << path_assets << endl;
			LOG_DEV << endl << path_agg << endl;

			return true;
		}
		else
		{
			LOG_BUILD << endl << "cannot find userConfig.json, using default file locations" << endl;
			return false;
		}
	}

	void zConfigurator3d::usd_readFolder(string folder, string pattern, vector<string>& filesFound)
	{
		filesFound.clear();
		for (const auto& entry : fs::recursive_directory_iterator(folder))
		{
			// Check if the current entry is a file and matches the given file name
			//if (entry.is_regular_file() && entry.path().filename() == fileName)
			//filesFound.push_back(entry.path().string());

			if (entry.is_regular_file())
			{
				string fullPath = entry.path().string();
				if (fullPath.find(pattern) != std::string::npos)
				{
					std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
					filesFound.push_back(fullPath);
				}
			}
		}
	}

	void zConfigurator3d::usd_readFile(string path)
	{
		int numMeshes = 0;
		string primName_voxel = "Voxels";
		string primName_asset = "Assets";

		UsdStageRefPtr stage = UsdStage::Open(path);
		if (stage)
		{
			for (UsdPrim prim : stage->Traverse())
			{
				if (prim.GetName() == primName_voxel)
				{
					for(auto& childPrim : prim.GetAllChildren())
					{
						UsdGeomImageable geom = UsdGeomImageable(childPrim);
						TfToken visibility;
						geom.GetVisibilityAttr().Get(&visibility);
						if (childPrim.IsA<UsdGeomMesh>() && visibility != UsdGeomTokens->invisible)
						{
							numMeshes++;
							gameObjs.push_back(new zGameObj(usd_readMesh(childPrim, true),(prim.GetParent().GetName().GetString())));
						}
					}

					//if (prim.IsA<UsdGeomMesh>() /*&& prim.GetParent().GetName() == primName_voxel*/)
					//{
					//	//cout << "primName: " << prim.GetParent().GetName() << endl;
					//	numMeshes++;
					//	gameObjs.push_back(new zGameObj(readUsdMesh(prim, true), &prim));
					//}
				}
			}
		}

		LOG_BUILD << to_string(numMeshes) + " meshes loaded from " + path << endl;
	}

	zObjMesh* zConfigurator3d::usd_readMesh(UsdPrim& usd, bool staticGeom)
	{
		// Declare arrays to store mesh data
		VtArray<GfVec3f> u_points;
		VtArray<GfVec3f> u_normals;
		VtArray<GfVec3f> u_Colors;
		VtArray<int>     faceVertexCounts;
		VtArray<int>     faceVertexIndices;
		VtArray<int>	 u_ColorIndices;

		GfMatrix4d transform;
		bool tmp = true;

		UsdGeomMesh usdMesh(usd);

		// Retrieve attributes from `usdMesh`
		UsdAttribute pointsAttr = usdMesh.GetPointsAttr();
		UsdAttribute normalsAttr = usdMesh.GetNormalsAttr();
		UsdAttribute faceVertexCountsAttr = usdMesh.GetFaceVertexCountsAttr();
		UsdAttribute faceVertexIndicesAttr = usdMesh.GetFaceVertexIndicesAttr();
		UsdAttribute colorAttr = usdMesh.GetDisplayColorAttr();
		UsdAttribute colorIndicesAttr = usdMesh.GetPrim().GetAttribute(pxr::TfToken("primvars:displayColor:indices"));


		// Prepare data structures for the zObjMesh
		zPointArray positions;
		zIntArray polyCounts;
		zIntArray polyConnects;
		zTransform myTransform;
		zColorArray palette;
		zColorArray colors;

		if (pointsAttr.Get(&u_points))
			for (int i = 0; i < u_points.size() * 3; i += 3)
			{
				zPoint pos = zPoint(u_points.cdata()->GetArray()[i], u_points.cdata()->GetArray()[i + 1], u_points.cdata()->GetArray()[i + 2]);
				positions.push_back(pos);
			}

		if (faceVertexCountsAttr.Get(&faceVertexCounts))
			for (int i = 0; i < faceVertexCounts.size(); i++)
			{
				polyCounts.push_back(faceVertexCounts.cdata()[i]);
			}

		if (faceVertexIndicesAttr.Get(&faceVertexIndices))
			for (int i = 0; i < faceVertexIndices.size(); i++)
			{
				polyConnects.push_back(faceVertexIndices.cdata()[i]);
			}

		if (colorAttr.Get(&u_Colors))
			for (int i = 0; i < u_Colors.size() * 3; i += 3)
			{
				zColor col = zColor(u_Colors.cdata()->GetArray()[i], u_Colors.cdata()->GetArray()[i + 1], u_Colors.cdata()->GetArray()[i + 2], 1);
				palette.push_back(col);
			}

		if (colorIndicesAttr.Get(&u_ColorIndices))
		{
			if (u_ColorIndices.size() == 1)
			{
				for (size_t i = 0; i < positions.size(); i++)
				{
					colors.push_back(palette[0]);
				}
			}
			else
			{
				for (size_t i = 0; i < u_ColorIndices.size(); i++)
				{
					int id = u_ColorIndices[i];
					colors.push_back(palette[id]);
				}
			}
		}

		// Convert the GfMatrix4d to a `zTransform` matrix
		if (usdMesh.GetLocalTransformation(&transform, &tmp))
		{
			double* data = transform.GetArray();
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					myTransform(i, j) = data[i * 4 + j];
				}
			}
		}

		// Create the zObjMesh
		zObjMesh* oMesh = new zObjMesh();
		zFnMesh fnMesh(*oMesh);
		fnMesh.create(positions, polyCounts, polyConnects);
		fnMesh.computeMeshNormals();

		// Set vertex colors
		if (colors.size() == fnMesh.numVertices())
		{
			fnMesh.setVertexColors(colors, true);
			fnMesh.computeFaceColorfromVertexColor();
		}

		if (colors.size() == fnMesh.numPolygons())
		{
			fnMesh.setFaceColors(colors, true);
			fnMesh.computeVertexColorfromFaceColor();
		}

		// Set the transformation matrix
		fnMesh.setTransform(myTransform);

		//if (staticGeom) fnMesh.setStaticContainers();
		//LOG_DEV << "V:" + to_string(fnMesh.numVertices()) + " E:" + to_string(fnMesh.numEdges()) + " F:" + to_string(fnMesh.numPolygons()) << endl;

		return oMesh;
	}

	bool zConfigurator3d::usd_openStage(std::string path, UsdStageRefPtr& uStage)
	{
		uStage = UsdStage::Open(path);
		if (!uStage) std::cout << "\n Failure to open stage." << std::endl;
		else cout << "\n opened USD stage of file:  " << path.c_str() << endl;

		return (uStage) ? true : false;
	}

	bool zConfigurator3d::usd_createStage(std::string path, string defaultPrimName, UsdStageRefPtr& uStage)
	{
		uStage = UsdStage::CreateNew(path);

		if (!uStage) cout << "\n error creating USD file  " << path.c_str() << endl;
		else
		{
			uStage->SetMetadata(TfToken("defaultPrim"), VtValue(defaultPrimName));
			uStage->SetMetadata(TfToken("upAxis"), VtValue("Z"));
			uStage->SetMetadata(TfToken("metersPerUnit"), VtValue(0.01));

			UsdGeomXform root = UsdGeomXform::Define(uStage, SdfPath("/" + defaultPrimName));
			//UsdGeomXform layer = UsdGeomXform::Define(uStage, SdfPath("/World/Geometry"));

			cout << "\n creating USD file: " << path.c_str() << endl;
		}

		return (uStage) ? true : false;
	}

	void zConfigurator3d::usd_export(string path, string defaultPrim)
	{
		//bool checkStage = usd_openStage(path, aggStagePtr);
		//if (!checkStage) checkStage = usd_createStage(path, aggStagePtr);

		//if (checkStage)
		//{
		//	for (auto gameObj : gameObjs)
		//	{
		//	}
		//		//usd_addToStage(gameObj, aggStagePtr);
		//}
	}

	bool zConfigurator3d::json_read(string path, json& j)
	{
		j.clear();
		ifstream in_myfile;
		in_myfile.open(path.c_str());

		int lineCnt = 0;

		if (in_myfile.fail())
		{
			cout << " error in opening file  " << path.c_str() << endl;
			return false;
		}

		in_myfile >> j;
		in_myfile.close();

		return true;
	}
}