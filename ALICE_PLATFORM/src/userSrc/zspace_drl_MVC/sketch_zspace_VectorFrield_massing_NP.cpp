#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include "zSMin.h"


//#include <headers/include/zCore.h>
//#include <headers/include/zGeometry.h>
//#include <headers/include/zDisplay.h>
//#include <headers/include/zData.h>
//#include <headers/include/zIO.h> 
//
using namespace zSpace;

////////////////////////////////////////////////////////////////////////// Untilities 

Alice::vec zVecToAliceVec(zVector& in)
{
	return Alice::vec(in.x, in.y, in.z);
}

zVector AliceVecToZvec(Alice::vec& in)
{
	return zVector(in.x, in.y, in.z);
}

float distanceBetween(const zVector& a, const zVector& b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

zVector computeQuadNormal(const zVector& a, const zVector& b, const zVector& c)
{
	// Compute u = b - a
	float ux = b.x - a.x;
	float uy = b.y - a.y;
	float uz = b.z - a.z;

	// Compute v = c - a
	float vx = c.x - a.x;
	float vy = c.y - a.y;
	float vz = c.z - a.z;

	// Cross product n = u × v
	float nx = uy * vz - uz * vy;
	float ny = uz * vx - ux * vz;
	float nz = ux * vy - uy * vx;

	zVector n(nx, ny, nz);
	n.normalize();
	return n;
}


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

bool displayField = true;
bool displayStreamlines = true;
bool displaySampledPoints = true;
bool displayRects = true;
bool displayExtrusions = false;




////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zObjMesh oMesh;
zObjGraph oGraph;

class VectorField2D {
public:
	static const int RES = 50;
	zVector grid[RES][RES];
	zVector field[RES][RES];
	float spacing = 0.5f;


	void setupField(float unitSpacing = 5.0f)
	{
		float halfSize = (RES - 1) * 0.5f * unitSpacing;

		for (int i = 0; i < RES; ++i)
		{
			for (int j = 0; j < RES; ++j)
			{
				float x = ofMap(i, 0, RES - 1, -halfSize, halfSize);
				float y = ofMap(j, 0, RES - 1, -halfSize, halfSize);
				grid[i][j] = zVector(x, y, 0);

				float fx = -1.0f - 5 * x * x + 5*y;
				float fy = 1.0f + x - 20*y * y;
				field[i][j] = zVector(fx, fy, 0);
			}
		}
	}


	void drawField() {
		glColor3f(0.2, 0.2, 0.8);
		for (int i = 0; i < RES; ++i) {
			for (int j = 0; j < RES; ++j) {
				zVector p = grid[i][j];
				zVector v = field[i][j];
				v.normalize();
				drawLine(zVecToAliceVec(p), zVecToAliceVec(p + v * spacing));
			}
		}
	}
};

class StreamlineIntegrator2D {
public:
	std::vector<std::vector<zVector>> streamlines;
	VectorField2D* field = nullptr;

	int maxSteps = 200;
	float stepSize = 0.4;
	int numSeeds = 40;
	float gridMin, gridMax;


	void setup(VectorField2D* _field) {
		field = _field;
	}

	zVector sampleFieldAt(zVector p) {
		const int RES = field->RES;
		float halfSize = (RES - 1) * 0.5f;

		float normX = ofMap(p.x, -halfSize, halfSize, 0, RES - 1);
		float normY = ofMap(p.y, -halfSize, halfSize, 0, RES - 1);

		int i = floor(normX);
		int j = floor(normY);

		if (i < 1 || i >= RES - 2 || j < 1 || j >= RES - 2) return zVector();

		return field->field[i][j];
	}


	void generateStreamlines()
	{
		streamlines.clear();

		gridMin = -((field->RES - 1) * 0.5f);
		gridMax = ((field->RES - 1) * 0.5f);

		int seedStep = 5;  // spacing in grid cells (increase for fewer, spaced-out seeds)

		for (int i = 0; i < field->RES; i += seedStep)
		{
			for (int j = 0; j < field->RES; j += seedStep)
			{
				zVector seed = field->grid[i][j];

				std::vector<zVector> pathForward = integrateStreamline(seed, +1);
				std::vector<zVector> pathBackward = integrateStreamline(seed, -1);

				std::reverse(pathBackward.begin(), pathBackward.end());

				if (pathForward.empty() && pathBackward.empty()) continue;

				std::vector<zVector> fullPath = pathBackward;
				if (pathForward.size() > 1)
					fullPath.insert(fullPath.end(), pathForward.begin() + 1, pathForward.end());

				smoothStreamline(fullPath);
				streamlines.push_back(fullPath);

				if (streamlines.size() > 1000) break;
			}
		}
		std::cout << "Generated streamlines: " << streamlines.size() << std::endl;
	}


	std::vector<zVector> integrateStreamline(zVector seed, int direction)
	{
		std::vector<zVector> path;
		zVector p = seed;

		for (int step = 0; step < maxSteps; step++)
		{
			zVector v = sampleFieldAt(p);
			if (v.length() < 1e-4) break;

			v.normalize();
			v *= direction;

			zVector mid = p + v * 0.5f * stepSize;
			zVector v2 = sampleFieldAt(mid);
			v2.normalize();
			v2 *= direction;

			zVector next = p + v2 * stepSize;
			path.push_back(p);
			p = next;

			// Clamp out-of-bounds values
			if (p.x < gridMin || p.x > gridMax || p.y < gridMin || p.y > gridMax) break;
			if (!std::isfinite(p.x) || !std::isfinite(p.y)) break;

		}

		return path;
	}

	void smoothStreamline(std::vector<zVector>& path, int iterations = 3)
	{
		if (path.size() < 3) return;

		for (int k = 0; k < iterations; k++)
		{
			std::vector<zVector> newPath = path;
			for (int i = 1; i < path.size() - 1; i++)
			{
				newPath[i] = path[i - 1] * 0.25 + path[i] * 0.5 + path[i + 1] * 0.25;
			}
			path = newPath;
		}
	}

	void drawStreamlines() {
		for (auto& line : streamlines) {
			glLineWidth(2.0f);
			glBegin(GL_LINE_STRIP);

			for (int i = 0; i < line.size(); i++) {
				zVector center(0, 0, 0);
				float dist = line[i].distanceTo(center);
				float maxDist = 25.0f; // adjust based on field size
				float t = ofClamp(dist / maxDist, 0.0f, 1.0f);
				float r = 1.0f - t;
				float b = t;

				glColor3f(r, 0.2f, b);
				Alice::vec av = zVecToAliceVec(line[i]);
				glVertex3f(av.x, av.y, av.z);
			}
			glEnd();
			glLineWidth(1.0f);
		}
	}
};

class StreamlineSampler {
public:
	std::vector<zVector> sampledPoints;

	// Sampling range control
	float minSpacing = 0.8f;
	float maxSpacing = 4.0f;

	void sampleFromStreamlines(const std::vector<std::vector<zVector>>& streamlines)
	{
		sampledPoints.clear();

		for (const auto& line : streamlines)
		{
			if (line.size() < 2) continue;

			zVector prev = line[0];
			sampledPoints.push_back(prev);

			float accumulated = 0;
			for (int i = 1; i < line.size(); ++i)
			{
				zVector cur = line[i];
				float segmentLength = prev.distanceTo(cur);
				accumulated += segmentLength;

				// Dynamic spacing based on scalar function
				float spacing = getSpacing(cur);

				if (accumulated >= spacing)
				{
					sampledPoints.push_back(cur);
					accumulated = 0;
				}
				prev = cur;
			}
		}
	}

	// Scalar function: spacing is smallest at center, larger away
	float getSpacing(const zVector& pt)
	{
		float dist = distanceBetween(pt, zVector(0, 0, 0));
		
		// normalize based on field radius
		//float t = ofClamp(dist / 25.0f, 0.0f, 1.0f);  

		// Exponential Falloff
		float t = exp(-0.2f * dist);
		return ofLerp(maxSpacing, minSpacing, 1.0f - t);

		// Sinusoidal Field
		/*float t = sin(dist * 0.2f) * 0.5f + 0.5f;
		return ofLerp(minSpacing, maxSpacing, t);*/

		// Directional Gradient (e.g., x-axis emphasis)
		/*float t = ofClamp(fabs(pt.x) / 25.0f, 0.0f, 1.0f);
		return ofLerp(minSpacing, maxSpacing, t);*/


		return ofLerp(minSpacing, maxSpacing, t);     // closer = tighter sampling
	}

	void cullSampledPoints(float radius)
	{
		if (sampledPoints.empty()) return;

		std::vector<zVector> culled;
		culled.push_back(sampledPoints[0]);

		for (int i = 1; i < sampledPoints.size(); i++)
		{
			bool keep = true;
			for (auto& kept : culled)
			{
				//if (abs(sampledPoints[i].x) < 1.0 && abs(sampledPoints[i].y) < 1.0) continue;

				if (distanceBetween(sampledPoints[i], kept) < radius)
				{
					keep = false;
					break;
				}
			}
			if (keep) culled.push_back(sampledPoints[i]);
		}

		sampledPoints = culled;
	}


	void drawSampledPoints()
	{
		glColor3f(1, 1, 1);
		glPointSize(4.0f);
		for (auto& pt : sampledPoints)
		{
			drawPoint(zVecToAliceVec(pt));
		}
		glPointSize(1.0f);
	}
};

class RectangleInstancer {
public:
	std::vector<zObjMesh> rectMeshes;
	zModel* modelRef = nullptr;

	std::vector<zObjMesh> extrudedMeshes;
	bool displayExtrusions = true;


	// For rectnaglel plan instantiate
	float width = 0.75f;
	float lengthScale = 1.2f;
	float maxLength = 2.0f;  // NEW max constraint

	// For extrusion
	float minHeight = 0.2f;
	float maxHeight = 2.0f;
	bool enableExtrusion = true;


	// Optional: central falloff control
	bool enableLengthClamp = true;

	void setup(zModel* model) {
		modelRef = model;
	}

	void setScale(float _width, float _lengthScale)
	{
		width = _width;
		lengthScale = _lengthScale;
	}

	float computeHeight(const zVector& pt)
	{
		float dist = distanceBetween(pt, zVector(0, 0, 0));
		float t = exp(-0.06f * dist);  // More gentle falloff
		return ofLerp(minHeight, maxHeight, t);
	}

	void generateRects(const std::vector<zVector>& points)
	{
		rectMeshes.clear();
		for (int i = 1; i < points.size() - 1; i++)
		{
			zVector pPrev = points[i - 1];
			zVector pCurr = points[i];
			zVector pNext = points[i + 1];			

			float dist = distanceBetween(pCurr, zVector(0, 0, 0));
			float t = exp(-0.05f * dist);
			float widthFactor = ofLerp(0.5f, 1.0f, t);     // small width near center
			float lengthFactor = ofLerp(0.2f, 1.8f, 1.0f - t);  // long rectangles far from center
			float w = width * widthFactor;

			// Skip if points are almost coincident
			if (distanceBetween(pPrev, pNext) < 0.1f) continue;
			zVector forward = pNext - pCurr;
			zVector backward = pCurr - pPrev;
			zVector tangent = (forward + backward) * 0.5f;
			tangent.normalize();

			float length = tangent.length() * lengthScale * lengthFactor;
			length = ofClamp(length, 0.2f, maxLength);


			if (length < 1e-3f) continue;

			if (enableLengthClamp)
			{
				length = MIN(length, maxLength);  // ⛔️ prevent long spikes
			}

			tangent.normalize();
			zVector normal(-tangent.y, tangent.x, 0);  // Perpendicular in XY

			// Rectangle corners
			zVector A = pCurr - tangent * 0.5f * length + normal * 0.5f * w;
			zVector B = pCurr + tangent * 0.5f * length + normal * 0.5f * w;
			zVector C = pCurr + tangent * 0.5f * length - normal * 0.5f * w;
			zVector D = pCurr - tangent * 0.5f * length - normal * 0.5f * w;

			zPointArray verts = { A, B, C, D };
			zIntArray pCounts = { 4 };
			zIntArray pConnects = { 0, 1, 2, 3 };

	
			zObjMesh* mesh = new zObjMesh();
			zFnMesh fnMesh(*mesh);
			fnMesh.create(verts, pCounts, pConnects);
			zVector n = computeQuadNormal(A, B, C);
			zVectorArray faceNormals(1, n);

			fnMesh.setFaceNormals(faceNormals);

			rectMeshes.push_back(*mesh);
			if (modelRef) modelRef->addObject(*mesh);
		}
	}

	void generateExtrudedMeshes()
	{
		extrudedMeshes.clear();

		for (auto& baseMesh : rectMeshes)
		{
			zFnMesh fnBase(baseMesh);
			zPointArray verts;
			fnBase.getVertexPositions(verts);

			zVector center = (verts[0] + verts[1] + verts[2] + verts[3]) * 0.25f;
			float height = computeHeight(center);

			zPointArray newVerts = verts;
			for (auto& v : newVerts)
				v.z += height;

			zPointArray finalVerts = verts;
			finalVerts.insert(finalVerts.end(), newVerts.begin(), newVerts.end());

			zIntArray faceCounts = { 4, 4, 4, 4, 4 };
			zIntArray faceConnects = {
				0,1,2,3,        // base
				0,1,5,4,
				1,2,6,5,
				2,3,7,6,
				3,0,4,7,
				4,5,6,7         // top
			};

			zObjMesh* extruded = new zObjMesh();
			zFnMesh fnExtruded(*extruded);
			fnExtruded.create(finalVerts, faceCounts, faceConnects);

			extrudedMeshes.push_back(*extruded);
			if (modelRef) modelRef->addObject(*extruded);
		}
	}

};

class CustomTransformRectangles {
public:
	std::vector<zObjMesh> transformedMeshes;
	zModel* modelRef = nullptr;

	// Parameters
	float width = 0.5f;
	float length = 1.5f;
	float maxLength = 3.0f;
	float rotationDeg = 30.0f;         // fixed or could be varied per point
	float verticalShift = 0.0f;        // Z offset
	bool alignToDirection = false;

	void setup(zModel* model) {
		modelRef = model;
	}

	void generateTransformedRects(const std::vector<zVector>& points)
	{
		transformedMeshes.clear();

		for (int i = 1; i < points.size() - 1; i++)
		{
			zVector pPrev = points[i - 1];
			zVector pCurr = points[i];
			zVector pNext = points[i + 1];

			zVector tangent = pNext - pPrev;
			tangent.normalize(); 

			if (!alignToDirection) tangent = zVector(1, 0, 0);

			zVector normal(-tangent.y, tangent.x, 0);

			// Apply user-defined rotation in XY
			float theta = degToRad(rotationDeg);
			float cs = cos(theta), sn = sin(theta);
			zVector rotTangent(cs * tangent.x - sn * tangent.y, sn * tangent.x + cs * tangent.y, 0);
			zVector rotNormal(-rotTangent.y, rotTangent.x, 0);

			float clampedLength = ofClamp(length, 0.2f, maxLength);
			float w = width;

			zVector center = pCurr + zVector(0, 0, verticalShift);

			zVector A = center - rotTangent * 0.5f * clampedLength + rotNormal * 0.5f * w;
			zVector B = center + rotTangent * 0.5f * clampedLength + rotNormal * 0.5f * w;
			zVector C = center + rotTangent * 0.5f * clampedLength - rotNormal * 0.5f * w;
			zVector D = center - rotTangent * 0.5f * clampedLength - rotNormal * 0.5f * w;

			zPointArray verts = { A, B, C, D };
			zIntArray pCounts = { 4 };
			zIntArray pConnects = { 0, 1, 2, 3 };

			zObjMesh* mesh = new zObjMesh();
			zFnMesh fnMesh(*mesh);
			fnMesh.create(verts, pCounts, pConnects);

			transformedMeshes.push_back(*mesh);
			if (modelRef) modelRef->addObject(*mesh);
		}
	}

private:
	float degToRad(float deg) { return deg * 0.0174533f; }
};



//class SmoothSDFGenerator {
//public:
//	zObjMeshScalarField scalarField;
//	zObjGraph isoOutline;
//	zSMin smin;
//
//	zModel* modelRef = nullptr;
//
//	int resolution = 100;
//	float isoThreshold = 0.05f;
//	float blendRadius = 3.0f;
//	float falloff = 0.2f;
//
//	void setup(zModel* model)
//	{
//		modelRef = model;
//
//		zFnMeshScalarField fnField(scalarField);
//		fnField.create(zPoint(-25, -25, 0), zPoint(25, 25, 0), resolution, resolution, 1, true, false);
//
//		scalarField.setDisplayElements(false, false, false);
//		model->addObject(scalarField);
//
//		isoOutline.setDisplayElements(false, true);
//		model->addObject(isoOutline);
//	}
//
//	void computeUnionFromPoints(std::vector<zVector>& points)
//	{
//
//		// Generate a circular field per point
//		for (auto& p : points)
//		{
//			zFnMeshScalarField fn(scalarField);
//
//			zScalarArray s;
//			fn.getScalars_Circle(s, p, blendRadius);
//			scalarFields.push_back(s);
//		}
//
//		// Perform smooth min union
//		zScalarArray resultField;
//		smin.smin_multiple(scalarFields, resultField, falloff, zSMin::MODE::exponential);
//
//		// Assign values and get outline
//		zFnMeshScalarField fnResult(scalarField);
//		fnResult.normliseValues(resultField);
//		fnResult.setFieldValues(resultField);
//		fnResult.getIsocontour(isoOutline, isoThreshold);
//	}
//};


////////////////////////////////////////////////////////////////////////// Global Declaration

VectorField2D myField;
StreamlineIntegrator2D myStreamlines;
StreamlineSampler mySampler;
RectangleInstancer myRects;
CustomTransformRectangles customRects;

//SmoothSDFGenerator mySDF;





void setup()
{
	model = zModel(100000);

	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);


	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&displayField, "show field");
	B.addButton(&displayStreamlines, "show streamlines");
	B.addButton(&displaySampledPoints, "show points");
	B.addButton(&displayRects, "show rects");
	B.addButton(&displayExtrusions, "show extrusion");




	B.buttons[0].attachToVariable(&displayField);
	B.buttons[1].attachToVariable(&displayStreamlines);
	B.buttons[2].attachToVariable(&displaySampledPoints);
	B.buttons[3].attachToVariable(&displayRects);
	B.buttons[4].attachToVariable(&displayExtrusions);
	

	////////////////////////////////////////////////////////////////////////// Intialize the variables

	myField.setupField(1.0f); // 1 meter spacing → covers ~25x25 meters if RES = 25
	myStreamlines.setup(&myField);
	myStreamlines.generateStreamlines();
	mySampler.sampleFromStreamlines(myStreamlines.streamlines);
	mySampler.cullSampledPoints(1.2f);  // Adjust as needed
	myRects.setup(&model);
	myRects.generateRects(mySampler.sampledPoints);
	myRects.setScale(0.8f, 1.2f);
	myRects.generateExtrudedMeshes();

	customRects.setup(&model);
	customRects.generateTransformedRects(mySampler.sampledPoints);


	//mySDF.setup(&model);
	//.computeUnionFromPoints(mySampler.sampledPoints);

}

void update(int value)
{

	if (compute)
	{
		
		
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		//model.draw();
		if (displayField) myField.drawField();
		if (displayStreamlines) myStreamlines.drawStreamlines();
		if (displaySampledPoints) mySampler.drawSampledPoints();
		if (displayRects || displayExtrusions)
		{
			model.draw();
		}




	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
