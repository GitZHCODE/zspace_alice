#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include "RamerDouglasPeucker.h"
#include "polynomialFitting.h" // Combined fitting header
#include <limits>
#include <iostream>
#include <algorithm>

using namespace zSpace;
using namespace std;
using namespace CurveFitting;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;
double polynomialDegree = 3; // Default degree for polynomial fitting
bool userPressed = false;
bool constrainEndpoints = false; // New toggle for endpoint constraint
bool userKink = false;

double background = 0.85;
double eps = 1.0;

zModel model;
zUtilsCore core;
////////////////////////////////////////////////////////////////////////// zSpace Objects
vector<Eigen::Vector2d> pointList;
vector<vector<Eigen::Vector2d>> pointList_segment;
vector<Eigen::Vector2d> pointListOut;
vector<size_t> kinkIds;

zObjGraph simplifiedGraph;
vector<zObjGraph> outputGraphs;
vector<zObjMesh> outputMeshes;

// Create curve fitter
CurveFitter curveFitter;
map<CurveFitter::Method, std::pair<string, zColor>> legendTable;

string filePath = "data/algebra/test_slabs.usda";
UsdStageRefPtr stage;

void setup()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	model = zModel(100000);



	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&eps, "eps");
	S.sliders[1].attachToVariable(&eps, 0, 1);
	S.addSlider(&polynomialDegree, "poly degree");
	S.sliders[2].attachToVariable(&polynomialDegree, 1, 10);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&constrainEndpoints, "constrain endpoints");
	B.buttons[2].attachToVariable(&constrainEndpoints);
	B.addButton(&userKink, "userKink");
	B.buttons[3].attachToVariable(&userKink);

	//legend
	legendTable[CurveFitter::Method::Arc] = std::make_pair("Arc", zBLUE);
	legendTable[CurveFitter::Method::Linear] = std::make_pair("Linear", zRED);
	legendTable[CurveFitter::Method::Polynomial] = std::make_pair("Polynomial", zMAGENTA);
	legendTable[CurveFitter::Method::Parabola] = std::make_pair("Parabola", zGREEN);

	// load slabs
	stage = UsdStage::Open(filePath);


}

void update(int value)
{
	if (compute)
	{
		const int DEFAULT_POINTS = 20;

		for (UsdPrim prim : stage->Traverse())
		{
			if (!prim.IsA<UsdGeomXform>()) continue;

			const std::string name = prim.GetName().GetString();
			if (name.find("Floor") == std::string::npos) continue;

			UsdGeomMesh usdMesh;
			for (UsdPrim child : prim.GetChildren())
			{
				if (child.IsA<UsdGeomMesh>())
				{
					usdMesh = UsdGeomMesh(child);
					break;
				}
			}
			if (!usdMesh) continue;

			// clean up arrays
			pointList.clear();
			pointListOut.clear();
			outputGraphs.clear();
			pointList_segment.clear();
			kinkIds.clear();

			// read from usd
			double height = 0.0;

			VtArray<GfVec3d> u_pts;
			VtArray<int> u_ids;
			UsdAttribute rawPtAttr = usdMesh.GetPrim().GetAttribute(pxr::TfToken("rawPts"));
			UsdAttribute kinkIdsAttr = usdMesh.GetPrim().GetAttribute(pxr::TfToken("kinkIds"));

			if (rawPtAttr.Get(&u_pts))
				for (int i = 0; i < u_pts.size() * 3; i += 3)
				{
					pointList.emplace_back(u_pts.cdata()->GetArray()[i], u_pts.cdata()->GetArray()[i + 1]);
					height = u_pts.cdata()->GetArray()[i + 2];
				}

			if (kinkIdsAttr.Get(&u_ids))
				for (int i = 0; i < u_ids.size(); i++)
				{
					kinkIds.push_back(u_ids.cdata()[i]);
				}

			// run

	/*
	// Read data
	//std::string path = "data/algebra/2dPoints.txt";
	//std::ifstream file(path);

	//if (!file.is_open()) {
	//	std::cerr << "Failed to open file: " << path << std::endl;
	//	return;
	//}

	//std::string content((std::istreambuf_iterator<char>(file)),
	//	std::istreambuf_iterator<char>());

	//std::stringstream ss(content);
	//std::string pointStr;

	//while (std::getline(ss, pointStr, ';')) {
	//	if (pointStr.empty()) continue; // skip empty segments

	//	std::stringstream pointStream(pointStr);
	//	std::string xStr, yStr;

	//	if (std::getline(pointStream, xStr, ',') && std::getline(pointStream, yStr)) {
	//		double x = std::stod(xStr);
	//		double y = std::stod(yStr);
	//		pointList.emplace_back(x, y);
	//	}
	//}

	//file.close();
	//std::cout << "\n" + to_string(pointList.size()) << " points read \n";

	//if (userKink)
	//{
	//	// Read data
	//	std::ifstream file_k("data/algebra/kinks.txt");

	//	if (!file_k) {
	//		std::cerr << "Failed to open data/algebra/kinks.txt\n";
	//		return;
	//	}

	//	for (std::size_t id; file_k >> id; )
	//	{
	//		kinkIds.push_back(id);
	//		file_k.ignore(std::numeric_limits<std::streamsize>::max(), ','); // discard comma
	//	}
	//}
	//else
	//{
	//	// Find kink points using Ramer-Douglas-Peucker
	//	std::vector<zPoint2d> points2d;
	//	for (const auto& p : pointList) {
	//		points2d.emplace_back(p.x(), p.y());
	//	}

	//	std::vector<zPoint2d> simplified;
	//	RamerDouglasPeucker(points2d, eps, simplified, kinkIds);
	//}
	*/

	// Divide input point list into multiple
			pointList_segment.reserve(kinkIds.size());
			outputGraphs.assign(kinkIds.size(), zObjGraph());
			outputMeshes.assign(kinkIds.size(), zObjMesh());

			std::cout << "pointList size: " << pointList.size() << "\n";
			std::cout << "kinkIds size: " << kinkIds.size() << "\n";
			for (auto& id : kinkIds)
				std::cout << "kinkId: " << id << "\n";


			for (size_t i = 1; i < kinkIds.size(); ++i)
			{
				pointList_segment.emplace_back(
					pointList.begin() + kinkIds[i - 1],
					pointList.begin() + kinkIds[i] + 1);

				std::cout << "sub list size: " << pointList_segment.back().size() << "\n";
			}

			// last seg
			std::vector<Eigen::Vector2d> seg;
			size_t left = kinkIds[kinkIds.size() - 1];
			size_t right = kinkIds[0] + 1;
			seg.reserve(pointList.size() - left + right);

			seg.insert(seg.end(),
				pointList.begin() + left,
				pointList.end());

			seg.insert(seg.end(),
				pointList.begin(),
				pointList.begin() + right);

			pointList_segment.push_back(std::move(seg));

			std::cout << "pointList_segment size: " << pointList_segment.size() << "\n";


			// Solver
			// Set the method in the curve fitter
			static CurveFitter::Method currentMethod = CurveFitter::Method::Auto;
			curveFitter.setMethod(currentMethod);
			curveFitter.setConstrainEndpoints(constrainEndpoints);

			for (size_t i = 0; i < pointList_segment.size(); i++)
			{
				curveFitter.setPoints(pointList_segment[i]);

				std::cout << "\n----- Curve Fitting Results -----\n";
				std::cout << "Constrain endpoints: " << (constrainEndpoints ? "Yes" : "No") << "\n";

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
					}

					// Only create the graph if we have points to display
					if (pointListOut.size() > 1) {
						zFnGraph fnGraph(outputGraphs[i]);
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
						col = legendTable[currentMethod].second;
						std::cout << legendTable[currentMethod].first << "\n";

						fnGraph.setEdgeColor(col);
						fnGraph.setEdgeWeight(5);

						// Only create the mesh if we have points to display
						double meshWidth = 0.15;
						if (pointListOut.size() > 1) {
							zFnMesh fnMesh(outputMeshes[i]);
							zPointArray pos;
							zIntArray pConnects;
							zIntArray pCounts;

							for (size_t j = 1; j < pointListOut.size(); j++)
							{
								zPoint pA(pointListOut[j].x(), pointListOut[j].y(), height);
								zPoint pB(pointListOut[j-1].x(), pointListOut[j-1].y(), height);
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


						// write to usd
						string edgeName = "edge_" + to_string(i);
						SdfPath edgePath = prim.GetPath().AppendChild(TfToken(edgeName));
						UsdGeomXform edgeXf = UsdGeomXform::Define(stage, edgePath);

						string edgeMeshName = "edgeMesh";
						SdfPath edgeMeshPath = edgeXf.GetPath().AppendChild(TfToken(edgeMeshName));
						UsdGeomMesh edgeMesh = UsdGeomMesh::Define(stage, edgeMeshPath);


						if (currentMethod == CurveFitter::Method::Arc)
						{
							// Store the computed data as a custom attribute
							UsdAttribute edgeTypeAttr = edgeXf.GetPrim().CreateAttribute(TfToken("edgeType"), SdfValueTypeNames->String);
							edgeTypeAttr.Set("Arc");

							UsdAttribute edgeParams = edgeXf.GetPrim().CreateAttribute(TfToken("edgeParams"), SdfValueTypeNames->DoubleArray);
							vector<double> rawParams = curveFitter.getArcParams().getData();
							edgeParams.Set(VtArray<double>(rawParams.begin(), rawParams.end()));

							// Mesh
							zFnMesh fnMesh(outputMeshes[i]);
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
								GfVec3d(pointListOut.back().x(), pointListOut.back().y(), height)};
							startEndParams.Set(u_startEnd);

							// Mesh
							zFnMesh fnMesh(outputMeshes[i]);
							fnMesh.setFaceColor(zBLUE);
							fnMesh.to(edgeMesh.GetPrim());

							//UsdAttribute startEndParams = edgeXf.GetPrim().CreateAttribute(TfToken("startEnd"), SdfValueTypeNames->DoubleArray);
							//vector<double>  startEnd = {
							//	pointListOut.front().x(), pointListOut.front().y(), height,
							//	pointListOut.back().x(), pointListOut.back().y(), height };
							//startEndParams.Set(VtArray<double>(startEnd.begin(),startEnd.end()));
						}

						// z value
						UsdAttribute zValueAttr = edgeXf.GetPrim().CreateAttribute(TfToken("zValue"), SdfValueTypeNames->Double);
						zValueAttr.Set(height);


						// Restore method to auto
						currentMethod = CurveFitter::Method::Auto;
					}
				}
				else
				{
					std::cout << "fit failed" << "\n";
				}
			}
			stage->Save();
		}
		compute = !compute;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	model.draw();

	S.draw();
	B.draw();

	if (display)
	{
		// Draw original data points
		for (auto& p : pointList)
			model.displayUtils.drawPoint(zPoint(p.x(), p.y(), 0), zGREY, 2);

		//// Draw generated curve points
		//for (auto& p : pointListOut)
		//	model.displayUtils.drawPoint(zPoint(p.x(), p.y(), 0), zBLUE, 5);

		//// Highlight start and end points when constrained
		//if (constrainEndpoints && !pointList.empty()) {
		//	// Draw start point
		//	model.displayUtils.drawPoint(zPoint(pointList.front().x(), pointList.front().y(), 0), zGREEN, 10);

		//	// Draw end point
		//	model.displayUtils.drawPoint(zPoint(pointList.back().x(), pointList.back().y(), 0), zGREEN, 10);
		//}

		// Draw kink points
		for (std::size_t k = 0; k < kinkIds.size(); k++)
		{
			if (kinkIds[k] < pointList.size()) {
				zPoint p(pointList[kinkIds[k]].x(), pointList[kinkIds[k]].y(), 0);
				model.displayUtils.drawPoint(p, zRED, 10);
			}
		}

		// Draw the fitted curve
		for (size_t i = 0; i < outputGraphs.size(); i++)
		{
			outputGraphs[i].draw();
		}

		//simplifiedGraph.draw();
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	// draw legend
	vec pos_icon(winW - 350, winH - 100, 0);
	vec pos_legend(pos_icon.x + 20, pos_icon.y, pos_icon.z);
	vec move(0, -15, 0);
	for (auto& legend : legendTable)
	{
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		drawString(legend.second.first, pos_legend);
		glColor4f(legend.second.second.r, legend.second.second.g, legend.second.second.b, legend.second.second.a);
		drawRectangle(vec(pos_icon.x - 5, pos_icon.y - 5, 0), vec(pos_icon.x + 5, pos_icon.y + 5, 0));
		pos_legend += move;
		pos_icon += move;
	}

	restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
	// Add hotkeys
	switch (k) {
	case '0':
		curveFitter.setMethod(CurveFitter::Method::Auto);
		goto END;
	case '1':
		curveFitter.setMethod(CurveFitter::Method::Circle);
		goto END;
	case '2':
		curveFitter.setMethod(CurveFitter::Method::Arc);
		goto END;
	case '3':
		curveFitter.setMethod(CurveFitter::Method::Ellipse);
		goto END;
	case '4':
		curveFitter.setMethod(CurveFitter::Method::Parabola);
		goto END;
	case '5':
		curveFitter.setMethod(CurveFitter::Method::Polynomial);
		goto END;
	case '6':
		curveFitter.setMethod(CurveFitter::Method::Linear);
		goto END;
	case 'c':
	case 'C':
		constrainEndpoints = !constrainEndpoints;
		goto END;
	default:
		goto END;

	END:
		userPressed = true;
		compute = true;
	}
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}

#endif // _MAIN_
