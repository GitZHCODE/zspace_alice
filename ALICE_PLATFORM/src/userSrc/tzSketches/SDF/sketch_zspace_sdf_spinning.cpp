#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

//#include <headers/zCore/zExtMesh.h>
//#include <headers/zCore/zExtPoint.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool d_vals = false;
bool b_distort = false;

double background = 0.85;

// Add attractor variables
zPoint attractorPoint(0, 0, 0);
zPoint attractorPoint1(17, -11, 0);
zPoint attractorPoint2(60, -11, 0);
double attStrength = 1.0;
double attRadius = 1.0;
double attScale = 1.0;
double threshold = 1.0;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMeshVectorField o_field;
zObjMesh o_mesh;
zScalarArray m_scalars;

zFileTpye type = zFileTpye::zUSD;
string file_in = "data/sdf/swirl.usda";
string file_tri = "data/sdf/swirl_tri.usda";

////// --- GUI OBJECTS ----------------------------------------------------

// Add distortion function
void addDistort(zPoint attractorPoint, bool CCW)
{
	zFnMeshVectorField fnField(o_field);
	vector<zVector> positions;
	fnField.getPositions(positions);

	// up vector for swirl "plane"
	zVector up(0, 0, 1);
	
	for (zItMeshVectorField v(o_field); !v.end(); v++)
	{
		zVector pos = v.getPosition();
		zVector vec = pos - attractorPoint;     // from attractor to vertex
		vec.x *= 0.5;

		// Cross with "up" to get a perpendicular swirl direction
		zVector swirlDir = CCW ? vec ^ up : up ^ vec;
		zVector scaleDir = vec;

		float dist = vec.length();

		// Normalize direction, then apply some falloff
		swirlDir.normalize();
		scaleDir.normalize();

		scaleDir *= attScale;

		// linear clamp
		//float magnitude = (dist == 0.0f) ? 0.0f : (1.0f / dist);
		//if (magnitude > attRadius) magnitude = attRadius;

		// 1/x
		float magnitude = 1.0f / (dist+ attRadius);

		// guassian
		//float magnitude = dist * exp(-(dist - attRadius) * (dist - attRadius));

		// smooth
		//float x = min(dist / (float)attRadius, 1.0f);
		//float smooth = x * x * (3.0f - 2.0f * x);
		//float magnitude = smooth;

		// scale the swirl direction
		swirlDir *= magnitude;
		scaleDir *= magnitude;

		v.setValue(swirlDir + scaleDir, true);
		
	}
}

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	zPoint minBB, maxBB;

	zFnMesh fnMesh(o_mesh);
	zFileTpye type;
	fnMesh.from(file_in, zUSD);
	fnMesh.getBounds(minBB, maxBB);
	minBB.x -= 2;
	minBB.y -= 2;
	maxBB.x += 2;
	maxBB.y += 2;

	// setup field
	int numX;
	int numY;
	numX =numY = 40;
	zFnMeshVectorField fnField(o_field);
	fnField.create(minBB,maxBB, numX, numY, 1, true, false);
	
	zDomainColor colDomain(zRED, zGREEN);
	fnField.setFieldColorDomain(colDomain);

	// After creating the field, initialize with zero values
	zVector* fieldVals = fnField.getRawFieldValues();
	for (int i = 0; i < fnField.numFieldValues(); i++)
	{
		fieldVals[i] = zVector(0,0,0);
	}

	addDistort(attractorPoint1, true);
	//addDistort(attractorPoint2, false);

	// geodesic
	zTsMeshParam meshParam;
	meshParam.setFromFile(file_tri, zUSD);

	//zFloatArray geodesics_start;
	//zFloatArray geodesics_end;

	//vector<int> start_ids;
	//float minDist = 100000;
	//int minId = -1;
	//for (zItMeshFace f(o_mesh); !f.end(); f++)
	//{
	//	float dist = f.getCenter().squareDistanceTo(attractorPoint);
	//	if (dist < minDist) 
	//	{
	//		minDist = dist;
	//		minId = f.getId();
	//	}
	//}

	//zItMeshFace f_min(o_mesh, minId);
	//f_min.getVertices(start_ids);

	vector<int> end_ids;
	for (zItMeshVertex v(*meshParam.getRawInMesh()); !v.end(); v++)
		if (v.onBoundary())
			end_ids.push_back(v.getId());
	meshParam.computeGeodesics_Exact(end_ids, m_scalars);

	//meshParam.computeGeodesics_Exact(start_ids, geodesics_start);
	//meshParam.computeGeodesics_Exact(end_ids, geodesics_end);

	//zFloatArray scalars;
	//scalars.assign(geodesics_start.size(), -1);

	//// weighted scalars
	//float weight = 0.5f;

	//for (int j = 0; j < scalars.size(); j++)
	//	scalars[j] = weight * geodesics_start[j] - (1 - weight) * geodesics_end[j];

	//m_scalars = scalars;

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_field);
	model.addObject(o_mesh);

	// set display element booleans
	o_field.setDisplayElements(false, true, false);
	o_mesh.setDisplayElements(false, true, false);
	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&attStrength, "attStrength");
	S.sliders[1].attachToVariable(&attStrength, -1, 1);
	S.addSlider(&attRadius, "attRadius");
	S.sliders[2].attachToVariable(&attRadius, 0.01, 1);
	S.addSlider(&attScale, "attScale");
	S.sliders[3].attachToVariable(&attScale, -1, 1);
	S.addSlider(&threshold, "threshold");
	S.sliders[4].attachToVariable(&threshold, -5, 5);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{
		// geodesic
		
		//
		zFnMeshVectorField fnField(o_field);

		for (zItMeshVertex v(o_mesh); !v.end(); v++)
		{
			if (v.onBoundary()) continue;

			zVector val;
			fnField.getFieldValue(v.getPosition(), zFieldValueType::zFieldNeighbourWeighted, val);

			float magnitude = m_scalars[v.getId()];
			magnitude *= attStrength;
			if (magnitude > threshold) magnitude = threshold;
			val *= magnitude;

			zVector* pos = v.getRawPosition();
			pos->x += val.x;
			pos->y += val.y;
		}

		compute = !compute;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	model.draw();

	if (display)
	{
		// zspace model draw
		o_field.setDisplayObject(true);
	}
	if (!display)
	{
		o_field.setDisplayObject(false);
	}
	if (d_vals)
	{
		// Draw attractor point
		model.displayUtils.drawPoint(attractorPoint1, zRED, 10);
		//model.displayUtils.drawPoint(attractorPoint2, zBLUE, 10);

		for (zItMeshVectorField v(o_field); !v.end(); v++)
		{
			model.displayUtils.drawLine(v.getPosition(),v.getPosition() + v.getValue(), zBLUE);

		}
	}



	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'c') compute = true;
	if (k == 'v') d_vals = !d_vals;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}

#endif // _MAIN_
