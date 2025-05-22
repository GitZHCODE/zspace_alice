//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/local_basis.h>
//#include <igl/readOFF.h>
#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/PI.h>

# define M_PI           3.14159265358979323846

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool d_inMesh = true;
bool d_paramMesh = true;
bool populate = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zObjMesh oMesh;
zFnMeshDynamics fnDyMesh;

zObjMeshArray circles;

zIntArray fixedArr;
zIntArray unSolvedArr;

double dT = 0.1;
float tol = 0.01;

////// --- GUI OBJECTS ----------------------------------------------------

////// --- igl Objects --------------------------------------------------
// Mesh

void fixBoundFaces(zObjMesh& oMesh, zIntArray& fixed)
{
	zItMeshFace f(oMesh);
	for (; !f.end(); f++)
	{
		zIntArray vIds;
		f.getVertices(vIds);
		for (auto& vId : vIds)
		{
			zItMeshVertex v(oMesh, vId);
			if (v.onBoundary())
			{
				fixed.insert(fixed.end(), vIds.begin(),vIds.end());
				break;
			}
		}
	}
}

zPointArray generateCirclePoints(zVector& center, zVector& normal, float radius, int numPoints) {
	normal.normalize();
	
	//zVector arbitraryVec1 = (normal.x != 0 || normal.y != 0) ? zVector(normal.y, -normal.x, 0) : zVector(0, normal.z, -normal.y);

	zVector arbitraryVec1;
	if (normal.z != 0)
		arbitraryVec1 = zVector(normal.z, 0, -normal.x);
	else
		arbitraryVec1 = (normal.x != 0 || normal.y != 0) ? zVector(normal.y, -normal.x, 0) : zVector(1, 0, 0);

	zVector arbitraryVec2 = { normal.y * arbitraryVec1.z - normal.z * arbitraryVec1.y,
							 normal.z * arbitraryVec1.x - normal.x * arbitraryVec1.z,
							 normal.x * arbitraryVec1.y - normal.y * arbitraryVec1.x };

	zPointArray points;
	points.reserve(numPoints);

	double angleIncrement = 2 * M_PI / numPoints;

	for (int i = 0; i < numPoints; ++i) {
		double angle = i * angleIncrement;
		// Compute the coordinates of the point on the circle
		double x = center.x + radius * (arbitraryVec1.x * cos(angle) + arbitraryVec2.x * sin(angle));
		double y = center.y + radius * (arbitraryVec1.y * cos(angle) + arbitraryVec2.y * sin(angle));
		double z = center.z + radius * (arbitraryVec1.z * cos(angle) + arbitraryVec2.z * sin(angle));
		points.emplace_back(x, y, z);
	}

	return points;
}

zObjMesh createCircle(zItMeshVertex& v, int numDivisions, float scale)
{
	zIntArray eIds;
	v.getConnectedEdges(eIds);
	float minLength = 100000;
	for (auto& eId : eIds)
	{
		zItMeshEdge e(oMesh, eId);
		zIntArray eVs;
		e.getVertices(eVs);
		zItMeshVertex v0(oMesh, eVs[0]);
		zItMeshVertex v1(oMesh, eVs[1]);

		if(!v0.onBoundary()&&!v1.onBoundary())
			minLength = e.getLength() < minLength ? e.getLength() : minLength;
	}
	//cout << "id:" << v.getId() << "minLength:" << minLength << endl;
	minLength = minLength * 0.5 * scale;
	zPointArray positions = generateCirclePoints(v.getPosition(), v.getNormal(), minLength, numDivisions);
	zIntArray pConnects, pCounts;

	for (int i = 0; i < numDivisions; i++)
		pConnects.push_back(i);
	
	pCounts.push_back(numDivisions);

	zObjMesh m;
	zFnMesh fn(m);
	fn.create(positions, pCounts, pConnects);
	fn.setFaceColor(zWHITE, true);
	return m;
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

	// read mesh
	zFnMesh fn(oMesh);
	fn.from("data/Parameterization/guangdu/new/chamfered_test.obj", zOBJ);

	fnDyMesh.create(oMesh, false);
	fixBoundFaces(oMesh, fixedArr);
	fnDyMesh.setFixed(fixedArr);

	for (int i = 0; i < fnDyMesh.numVertices(); i++)
	{
		unSolvedArr.push_back(1);
	}

	circles.assign(fnDyMesh.numVertices(), zObjMesh());

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);

	for (auto& c : circles)
	{
		model.addObject(c);
		c.setDisplayElements(0, 0, 1);
	}

	// set display element booleans
	oMesh.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[2].attachToVariable(&d_paramMesh);

	B.addButton(&populate, "populate");
	B.buttons[3].attachToVariable(&populate);

}

void update(int value)
{
	if (compute)
	{
		zItMeshVertex v(oMesh);
		for (; !v.end(); v++)
		{
			zIntArray eIds;
			v.getConnectedEdges(eIds);
			float aveLength = 0;
			for (auto& eId : eIds)
			{
				zItMeshEdge e(oMesh, eId);
				aveLength += e.getLength();
			}
			aveLength /= eIds.size();

			//check
			bool b_solve = true;
			for (auto& eId : eIds)
			{
				zItMeshEdge e(oMesh, eId);
				if (abs(e.getLength() - aveLength) > tol)
				{
					b_solve = false;
					fnDyMesh.addSpringForce(1.0, eId, 0);
					unSolvedArr[v.getId()] = 0;
				}
			}
		}

		fnDyMesh.update(dT, zRK4, true, true, true);

		//compute = !compute; 
	}

	if (populate)
	{
		zItMeshVertex v(oMesh);
		for (; !v.end(); v++)
		{
			if (!v.onBoundary())
			{
				circles[v.getId()] = createCircle(v, 16, 1.0f);
			}
		}
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();


	if (d_inMesh)
	{
		model.draw();

		//zItMeshVertex v(oMesh);
		//for (; !v.end(); v++)
		//{
		//	if (unSolvedArr[v.getId()])
		//		model.displayUtils.drawPoint(v.getPosition(), zGREEN, 2);
		//	else
		//		model.displayUtils.drawPoint(v.getPosition(), zMAGENTA, 2);
		//}

		for (auto& vId : fixedArr)
		{
			zItMeshVertex fixed(oMesh,vId);
			model.displayUtils.drawPoint(fixed.getPosition(), zRED, 2);
		}
	}

	if (d_paramMesh)
	{

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
