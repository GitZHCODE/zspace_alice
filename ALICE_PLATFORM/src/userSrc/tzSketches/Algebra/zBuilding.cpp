#include "zBuilding.h"
#include "RamerDouglasPeucker.h"
#include "polynomialFitting.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/tokens.h>

using namespace zSpace;
using namespace CurveFitting;
using namespace std;
using namespace pxr;

namespace NS_hangzhou {

    void zFloor::rationaliseSlabEdges(double eps,bool constrainEndpoints)
    {
		rationalizedCurves.clear();
		rationalizedMeshes.clear();
		curveTypes.clear();

		// Divide input point list into multiple
		vector<vector<Eigen::Vector2d>> pointList_segment;
		pointList_segment.reserve(kinkIds.size());
		rationalizedCurves.assign(kinkIds.size(), zObjGraph());
		rationalizedMeshes.assign(kinkIds.size(), zObjMesh());
		curveTypes.assign(kinkIds.size(), CurveType::LINE);

		std::cout << "pointList size: " << points.size() << "\n";
		std::cout << "kinkIds size: " << kinkIds.size() << "\n";
		for (auto& id : kinkIds)
			std::cout << "kinkId: " << id << "\n";


		for (size_t i = 1; i < kinkIds.size(); ++i)
		{
			pointList_segment.emplace_back(
				points.begin() + kinkIds[i - 1],
				points.begin() + kinkIds[i] + 1);

			std::cout << "sub list size: " << pointList_segment.back().size() << "\n";
		}

		// last seg
		std::vector<Eigen::Vector2d> seg;
		size_t left = kinkIds[kinkIds.size() - 1];
		size_t right = kinkIds[0] + 1;
		seg.reserve(points.size() - left + right);

		seg.insert(seg.end(),
			points.begin() + left,
			points.end());

		seg.insert(seg.end(),
			points.begin(),
			points.begin() + right);

		pointList_segment.push_back(std::move(seg));

		std::cout << "pointList_segment size: " << pointList_segment.size() << "\n";


		// Solver
		// Set the method in the curve fitter
		CurveFitter curveFitter;
		CurveFitter::Method currentMethod = CurveFitter::Method::Auto;
		curveFitter.setMethod(currentMethod);
		curveFitter.setConstrainEndpoints(constrainEndpoints);

		for (size_t i = 0; i < pointList_segment.size(); i++)
		{
			curveFitter.setPoints(pointList_segment[i]);

			std::cout << "\n----- Curve Fitting Results -----\n";
			std::cout << "Constrain endpoints: " << (constrainEndpoints ? "Yes" : "No") << "\n";

			vector<Eigen::Vector2d> pointListOut;

			if (curveFitter.solve()) {
				// Get the fitted points
				pointListOut = curveFitter.getPoints(DEFAULT_POINTS);

				// If auto method, print detailed results
				if (currentMethod == CurveFitter::Method::Auto) {
					CurveFitter::Method bestMethod = curveFitter.getBestMethod();
					auto allResults = curveFitter.getAllMethodResults();

					std::cout << "Auto Method Results (Best First):" << std::endl;
					for (size_t i = 0; i < allResults.size(); i++) {
						std::cout << i + 1 << ". " << allResults[i].description
							<< " (MSE: " << allResults[i].variance << ")"
							<< (i == 0 ? " ← SELECTED" : "") << std::endl;
					}

					// Update current method to show the right result info
					currentMethod = bestMethod;
					if(bestMethod == CurveFitter::Method::Arc) curveTypes[i] = CurveType::ARC;
				}

				// Only create the graph if we have points to display
				if (pointListOut.size() > 1) {
					zFnGraph fnGraph(rationalizedCurves[i]);
					zPointArray pos;
					zIntArray pConnects;

					for (size_t j = 0; j < pointListOut.size(); j++)
					{
						pos.emplace_back(pointListOut[j].x(), pointListOut[j].y(), height);

						if (j != 0)
						{
							pConnects.push_back(j - 1);
							pConnects.push_back(j);
						}
					}

					fnGraph.create(pos, pConnects);
					zColor col = zBLACK;
					//col = legendTable[currentMethod].second;
					//std::cout << legendTable[currentMethod].first << "\n";

					fnGraph.setEdgeColor(col);
					fnGraph.setEdgeWeight(5);

					// Only create the mesh if we have points to display
					double meshWidth = 0.15;
					if (pointListOut.size() > 1) {
						zFnMesh fnMesh(rationalizedMeshes[i]);
						zPointArray pos;
						zIntArray pConnects;
						zIntArray pCounts;

						for (size_t j = 1; j < pointListOut.size(); j++)
						{
							zPoint pA(pointListOut[j].x(), pointListOut[j].y(), height);
							zPoint pB(pointListOut[j - 1].x(), pointListOut[j - 1].y(), height);
							zVector vec = pA - pB;
							vec.normalize();
							vec = vec ^ zVector(0, 0, 1);
							vec *= meshWidth;

							if (j == 1)
							{
								pos.push_back(pB - vec);
								pos.push_back(pB + vec);
								pos.push_back(pA - vec);
								pos.push_back(pA + vec);
								pConnects.push_back(0);
								pConnects.push_back(1);
								pConnects.push_back(3);
								pConnects.push_back(2);
							}
							else
							{
								pos.push_back(pA - vec);
								pos.push_back(pA + vec);
								pConnects.push_back((j - 1) * 2 + 0);
								pConnects.push_back((j - 1) * 2 + 1);
								pConnects.push_back(j * 2 + 1);
								pConnects.push_back(j * 2 + 0);
							}
							pCounts.push_back(4);
						}
						fnMesh.create(pos, pCounts, pConnects);

					}
				}
			}

			// write to usd
			string edgeName = "edge_" + to_string(i);
			SdfPath edgePath = floorMesh.GetPath().AppendChild(TfToken(edgeName));
			UsdGeomXform edgeXf = UsdGeomXform::Define(stagePtr, edgePath);

			string edgeMeshName = "edgeMesh";
			SdfPath edgeMeshPath = edgeXf.GetPath().AppendChild(TfToken(edgeMeshName));
			UsdGeomMesh edgeMesh = UsdGeomMesh::Define(stagePtr, edgeMeshPath);


			if (currentMethod == CurveFitter::Method::Arc)
			{
				// Store the computed data as a custom attribute
				UsdAttribute edgeTypeAttr = edgeXf.GetPrim().CreateAttribute(TfToken("edgeType"), SdfValueTypeNames->String);
				edgeTypeAttr.Set("Arc");

				UsdAttribute edgeParams = edgeXf.GetPrim().CreateAttribute(TfToken("edgeParams"), SdfValueTypeNames->DoubleArray);
				vector<double> rawParams = curveFitter.getArcParams().getData();
				edgeParams.Set(VtArray<double>(rawParams.begin(), rawParams.end()));

				// Mesh
				zFnMesh fnMesh(rationalizedMeshes[i]);
				fnMesh.setFaceColor(zMAGENTA);
				fnMesh.to(edgeMesh.GetPrim());

			}
			else if (currentMethod == CurveFitter::Method::Linear)
			{
				UsdAttribute edgeTypeAttr = edgeXf.GetPrim().CreateAttribute(TfToken("edgeType"), SdfValueTypeNames->String);
				edgeTypeAttr.Set("Line");

				UsdAttribute edgeParams = edgeXf.GetPrim().CreateAttribute(TfToken("edgeParams"), SdfValueTypeNames->DoubleArray);
				vector<double> rawParams = curveFitter.getLinearParams().getData();
				edgeParams.Set(VtArray<double>(rawParams.begin(), rawParams.end()));

				UsdAttribute startEndParams = edgeXf.GetPrim().CreateAttribute(TfToken("startEnd"), SdfValueTypeNames->Double3Array);
				VtArray<GfVec3d> u_startEnd = {
					GfVec3d(pointListOut.front().x(), pointListOut.front().y(), height),
					GfVec3d(pointListOut.back().x(), pointListOut.back().y(), height) };
				startEndParams.Set(u_startEnd);

				// Mesh
				zFnMesh fnMesh(rationalizedMeshes[i]);
				fnMesh.setFaceColor(zBLUE);
				fnMesh.to(edgeMesh.GetPrim());
			}

			// z value
			UsdAttribute zValueAttr = edgeXf.GetPrim().CreateAttribute(TfToken("zValue"), SdfValueTypeNames->Double);
			zValueAttr.Set(height);


			// Restore method to auto
			currentMethod = CurveFitter::Method::Auto;
		}
    }

void zFloor::draw() {

    // original geometries
    for (auto& p : points)
    {
		display.drawPoint(zPoint(p.x(), p.y(), height), zGREY, 5);
    }

    // Draw kink points
    for (std::size_t k = 0; k < kinkIds.size(); k++)
    {
        if (kinkIds[k] < points.size()) {
            zPoint p(points[kinkIds[k]].x(), points[kinkIds[k]].y(), height);
            display.drawPoint(p, zRED, 10);
        }
    }

    // rationalised geometries
    for (auto& curve : rationalizedCurves) {
        curve.draw();
    }
    
    //for (auto& mesh : rationalizedMeshes) {
    //    mesh.draw();
    //}
}

void zFloor::read(const UsdGeomMesh& usdMesh) {
    // Read points and kink IDs
    points.clear();
    kinkIds.clear();

	floorMesh = usdMesh;
    
    VtArray<GfVec3d> u_pts;
    VtArray<int> u_ids;
    UsdAttribute rawPtAttr = usdMesh.GetPrim().GetAttribute(TfToken("rawPts"));
    UsdAttribute kinkIdsAttr = usdMesh.GetPrim().GetAttribute(TfToken("kinkIds"));
    
    if (rawPtAttr.Get(&u_pts)) {
        for (int i = 0; i < u_pts.size() * 3; i += 3) {
            points.emplace_back(u_pts.cdata()->GetArray()[i], 
                              u_pts.cdata()->GetArray()[i + 1]);
            height = u_pts.cdata()->GetArray()[i + 2];
        }
    }
    
    if (kinkIdsAttr.Get(&u_ids)) {
        for (int i = 0; i < u_ids.size(); i++) {
            kinkIds.push_back(u_ids.cdata()[i]);
        }
    }
}

void zFloor::save() {
	floorMesh.GetPrim().GetStage()->Save();
}

void zBuilding::rationalise() {
    for (auto& floor : floors)
    {
        floor->rationaliseSlabEdges();
    }
}

void zBuilding::draw() {
    for (auto& floor : floors) {
        floor->draw();
    }
}

void zBuilding::read(const std::string& usdPath) {
    auto stage = UsdStage::Open(usdPath);
    if (!stage) return;
    
    floors.clear();
    
    for (UsdPrim prim : stage->Traverse()) {
        if (!prim.IsA<UsdGeomXform>()) continue;
        
        const std::string name = prim.GetName().GetString();
        if (name.find("Floor") == std::string::npos) continue;
        
        UsdGeomMesh usdMesh;
        for (UsdPrim child : prim.GetChildren()) {
            if (child.IsA<UsdGeomMesh>()) {
                usdMesh = UsdGeomMesh(child);
                break;
            }
        }
        if (!usdMesh) continue;
        
        // Create and import floor
        auto floor = std::make_unique<zFloor>(name, 0.0);
        floor->read(usdMesh);
		floor->stagePtr = stage;
        addFloor(std::move(floor));
    }
}

void zBuilding::save() {

}

} // namespace NS_hangzhou 