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

double background = 0.85;
double eps = 1.0;

zModel model;
zUtilsCore core;
////////////////////////////////////////////////////////////////////////// zSpace Objects
vector<Eigen::Vector2d> pointList;
vector<Eigen::Vector2d> pointListOut;
vector<size_t> kinkIds;

zObjGraph simplifiedGraph;

// Create curve fitter
CurveFitter curveFitter;

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
	
}

void update(int value)
{
	if (compute)
	{
		const int DEFAULT_POINTS = 100;
		pointList.clear();
		pointListOut.clear();

		// Read data
		std::string path = "data/algebra/2dPoints.txt";
		std::ifstream file(path);

		if (!file.is_open()) {
			std::cerr << "Failed to open file: " << path << std::endl;
			return;
		}

		std::string content((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());

		std::stringstream ss(content);
		std::string pointStr;

		while (std::getline(ss, pointStr, ';')) {
			if (pointStr.empty()) continue; // skip empty segments

			std::stringstream pointStream(pointStr);
			std::string xStr, yStr;

			if (std::getline(pointStream, xStr, ',') && std::getline(pointStream, yStr)) {
				double x = std::stod(xStr);
				double y = std::stod(yStr);
				pointList.emplace_back(x, y);
			}
		}

		file.close();
		std::cout << "\n" + to_string(pointList.size()) << " points read \n";

		// Set initial points in curve fitter
		curveFitter.setPoints(pointList);

		// Find kink points using Ramer-Douglas-Peucker
		std::vector<zPoint2d> points2d;
		for (const auto& p : pointList) {
			points2d.emplace_back(p.x(), p.y());
		}
		
		std::vector<zPoint2d> simplified;
		RamerDouglasPeucker(points2d, eps, simplified, kinkIds);
		
		// Get the current method from keyboard input or UI
		static CurveFitter::Method currentMethod = CurveFitter::Method::Auto;
		
		// Set the method in the curve fitter
		if(!userPressed) curveFitter.setMethod(currentMethod);
		
		// For polynomial method, set the degree
		if (currentMethod == CurveFitter::Method::Polynomial) {
			curveFitter.setPolynomialDegree(static_cast<int>(polynomialDegree));
		}
		
		// Set the endpoint constraint based on UI toggle
		curveFitter.setConstrainEndpoints(constrainEndpoints);
		
		std::cout << "\n----- Curve Fitting Results -----\n";
		std::cout << "Constrain endpoints: " << (constrainEndpoints ? "Yes" : "No") << "\n";
		
		// Solve using the curve fitter
		if (curveFitter.solve()) {
			// Get the fitted points
			pointListOut = curveFitter.getPoints(DEFAULT_POINTS);
			
			// Print the equation
			std::cout << "Equation: " << curveFitter.getEquation() << std::endl;
			
			// If auto method, print detailed results
			if (currentMethod == CurveFitter::Method::Auto) {
				CurveFitter::Method bestMethod = curveFitter.getBestMethod();
				auto allResults = curveFitter.getAllMethodResults();
				
				std::cout << "Auto Method Results (Best First):" << std::endl;
				for (size_t i = 0; i < allResults.size(); i++) {
					std::cout << i+1 << ". " << allResults[i].description 
					          << " (MSE: " << allResults[i].variance << ")" 
							  << (i == 0 ? " ← SELECTED" : "") << std::endl;
				}
				
				// Update current method to show the right result info
				currentMethod = bestMethod;
			}
			
			// Print parameters based on method
			switch (currentMethod) {
				case CurveFitter::Method::Circle: {
					auto params = curveFitter.getCircleParams();
					std::cout << "Circle Parameters:" << std::endl;
					std::cout << "Center: (" << params.h << ", " << params.k << ")" << std::endl;
					std::cout << "Radius: " << params.r << std::endl;
					break;
				}
				case CurveFitter::Method::Arc: {
					auto params = curveFitter.getArcParams();
					std::cout << "Arc Parameters:" << std::endl;
					std::cout << "Center: (" << params.h << ", " << params.k << ")" << std::endl;
					std::cout << "Radius: " << params.r << std::endl;
					std::cout << "Angular span: " << (params.end - params.start) * 180/M_PI << " deg" << std::endl;
					break;
				}
				case CurveFitter::Method::Ellipse: {
					auto params = curveFitter.getEllipseParams();
					std::cout << "Ellipse Parameters:" << std::endl;
					std::cout << "Center: (" << params.h << ", " << params.k << ")" << std::endl;
					std::cout << "Semi-major axis: " << params.a << std::endl;
					std::cout << "Semi-minor axis: " << params.b << std::endl;
					std::cout << "Rotation: " << params.theta * 180/M_PI << " deg" << std::endl;
					break;
				}
				case CurveFitter::Method::Parabola: {
					auto params = curveFitter.getParabolaParams();
					std::cout << "Parabola Parameters:" << std::endl;
					std::cout << "a = " << params.a << ", b = " << params.b << ", c = " << params.c << std::endl;
					std::cout << "d = " << params.d << ", e = " << params.e << ", f = " << params.f << std::endl;
					break;
				}
				case CurveFitter::Method::Polynomial: {
					auto params = curveFitter.getPolynomialParams();
					std::cout << "Polynomial Coefficients (degree " << params.degree << "):" << std::endl;
					for (size_t i = 0; i < params.coefficients.size(); i++) {
						std::cout << "a" << i << " = " << params.coefficients[i];
						if (i < params.coefficients.size() - 1) std::cout << ", ";
					}
					std::cout << std::endl;
					break;
				}
				case CurveFitter::Method::Linear: {
					auto params = curveFitter.getLinearParams();
					std::cout << "Linear Parameters:" << std::endl;
					if (std::isinf(params.m)) {
						std::cout << "Vertical line at x = " << params.b << std::endl;
					} else {
						std::cout << "Slope (m): " << params.m << std::endl;
						std::cout << "Y-intercept (b): " << params.b << std::endl;
						std::cout << "R-squared: " << params.r2 << std::endl;
					}
					break;
				}
			}
		} else {
			std::cout << "Fitting failed: " << curveFitter.getLastError() << std::endl;
		}

		// Only create the graph if we have points to display
		if (!pointListOut.empty()) {
			zFnGraph fnGraph(simplifiedGraph);
			zPointArray pos;
			zIntArray pConnects;

			for (size_t i = 0; i < pointListOut.size(); i++)
			{
				pos.emplace_back(pointListOut[i].x(), pointListOut[i].y(), 0);

				if (i != 0)
				{
					pConnects.push_back(i - 1);
					pConnects.push_back(i);
				}
			}
			fnGraph.create(pos, pConnects);
			fnGraph.setEdgeColor(zBLACK);
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
			model.displayUtils.drawPoint(zPoint(p.x(), p.y(), 0), zGREY, 5);

		// Draw generated curve points
		for (auto& p : pointListOut)
			model.displayUtils.drawPoint(zPoint(p.x(), p.y(), 0),zBLUE,5);

		// Highlight start and end points when constrained
		if (constrainEndpoints && !pointList.empty()) {
			// Draw start point
			model.displayUtils.drawPoint(zPoint(pointList.front().x(), pointList.front().y(), 0), zGREEN, 10);
			
			// Draw end point
			model.displayUtils.drawPoint(zPoint(pointList.back().x(), pointList.back().y(), 0), zGREEN, 10);
		}

		// Draw kink points
		for (std::size_t k = 0; k < kinkIds.size(); k++)
		{
			if (kinkIds[k] < pointList.size()) {
				zPoint p(pointList[kinkIds[k]].x(), pointList[kinkIds[k]].y(), 0);
				model.displayUtils.drawPoint(p, zRED, 10);
			}
		}
		
		// Draw the fitted curve
		simplifiedGraph.draw();
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	// Show current fitting method
	std::string methodName;
	CurveFitter::Method effectiveMethod = curveFitter.getMethod(); // This will return the best method if Auto is selected
	
	switch (effectiveMethod) {
		case CurveFitter::Method::Circle: methodName = "Circle Fit"; break;
		case CurveFitter::Method::Arc: methodName = "Arc Fit"; break;
		case CurveFitter::Method::Ellipse: methodName = "Ellipse Fit"; break;
		case CurveFitter::Method::Parabola: methodName = "Parabola Fit"; break;
		case CurveFitter::Method::Polynomial: 
			methodName = "Polynomial Fit";
			break;
		case CurveFitter::Method::Linear: methodName = "Linear Fit"; break;
		default: methodName = "Unknown"; break;
	}
	
	if (curveFitter.getMethod() == CurveFitter::Method::Auto) {
		methodName = "Auto: " + methodName;
	}
	
	drawString("Active: " + methodName, vec(winW - 1200, winH - 200, 0));
	
	// Show endpoint constraint status
	drawString("Constrain endpoints: " + string(constrainEndpoints ? "Yes" : "No"), vec(winW - 1200, winH - 180, 0));
	
	// Show equation if available
	if (!pointListOut.empty()) {
		drawString("Equation: " + curveFitter.getEquation(), vec(winW - 1200, winH - 220, 0));
	}
	
	// Show auto-fit results if available
	if (curveFitter.getMethod() == CurveFitter::Method::Auto && !pointListOut.empty()) {
		auto results = curveFitter.getAllMethodResults();
		if (!results.empty()) {
			int y = winH - 540;
			drawString("Fit Quality Ranking:", vec(winW - 350, y, 0));
			y -= 20;
			
			// Display top 3 methods only to avoid cluttering
			int count = std::min(3, static_cast<int>(results.size()));
			for (int i = 0; i < count; i++) {
				std::ostringstream oss;
				oss << i+1 << ". " << results[i].description << " (MSE: " 
				    << std::fixed << std::setprecision(6) << results[i].variance << ")";
				drawString(oss.str(), vec(winW - 350, y, 0));
				y -= 20;
			}
		}
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
