//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool movePt = false;
bool exportToCSV = false;

double background = 0.75;

zPoint minBB;
zPoint maxBB;

int numVoxels;
double voxelSize = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zUtilsCore core;

//define target and force
zPoint target(0, 0, 0);
zVector force(0,0,0);

//define the point cloud container
zObjPointCloud oCloud;

////// --- GUI OBJECTS ----------------------------------------------------

// a custom method that creates point cloud
void createPointCloud(zObjPointCloud& oCloud, double unit_X, double unit_Y, double unit_Z, int n_X, int n_Y, int n_Z, zPoint& _minBB, zPoint& _maxBB, int& _numV, zPoint startPt = zPoint(0, 0, 0))
{
	vector<zVector>positions;

	zVector unitVec = zVector(unit_X, unit_Y, unit_Z);

	for (int i = 0; i < n_X; i++)
	{
		for (int j = 0; j < n_Y; j++)
		{
			for (int k = 0; k < n_Z; k++)
			{

				zVector pos;
				pos.x = startPt.x + i * unitVec.x;
				pos.y = startPt.y + j * unitVec.y;
				pos.z = startPt.z + k * unitVec.z;

				positions.push_back(pos);
			}
		}
	}

	zFnPointCloud fnCloud(oCloud);
	fnCloud.create(positions);
	fnCloud.getBounds(_minBB, _maxBB);
	_numV = fnCloud.numVertices();
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

	/*
	// read mesh
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/cube.obj", zOBJ);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	 //append to model for displaying the object
	model.addObject(oMesh);

	 //set display element booleans
	oMesh.setDisplayElements(true, true, true);
	*/
	////////////////////////////////////////////////////////////////////////// This is my sketch



	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&voxelSize, "voxelSize");
	S.sliders[1].attachToVariable(&voxelSize, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&movePt, "movePt");
	B.buttons[2].attachToVariable(&movePt);
	B.addButton(&exportToCSV, "exportToCSV");
	B.buttons[3].attachToVariable(&exportToCSV);

}

void update(int value)
{

	if (compute)
	{
		//define unit distance on X,Y,Z and number of points
		float unitX, unitY, unitZ;
		int numX, numY, numZ;

		//initialise the value of unit distance and number of voxels
		unitX = unitY = unitZ = 1;
		numX = numY = numZ = 10;

		//create the point point
		createPointCloud(oCloud, unitX, unitY, unitZ, numX, numY, numZ, minBB, maxBB, numVoxels);

		//update the target force
		force = maxBB - minBB;
		force.normalize();
		force *= 0.2;

		compute = !compute;	
	}

	if (movePt)
	{
		if (target.z > maxBB.z || target.z < minBB.z)
			force = force * -1;

		target += force;

		//define the color domain
		zDomainColor colDomain(zColor(1, 0, 0, 0), zColor(0, 1, 0, 1));

		//check the max distance from two corners
		float maxDist = (maxBB - minBB).length();

		//travers all points in point cloud, compute the distance to the target and set a rgb value
		for (zItPointCloudVertex v(oCloud); !v.end(); v++)
		{
			zVector vec = v.getPosition() - target;
			float dist = vec.length();
			zColor col = core.blendColor(dist, zDomainFloat(0, maxDist), colDomain, zHSV);
			v.setColor(col);
		}
	}

	if (exportToCSV)
	{
		ofstream file;
		file.open("data/points.csv");

		for (zItPointCloudVertex v(oCloud); !v.end(); v++)
		{
			file << v.getId() << "," << v.getPosition().x << "," << v.getPosition().y << "," << v.getPosition().z << ",";
			file << v.getColor().r << "," << v.getColor().g << "," << v.getColor().b << "\n";
		}
		file.close();

		exportToCSV = !exportToCSV;
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
		// zspace model draw
		model.draw();

		//draw the attractor
		model.displayUtils.drawPoint(target, zColor(0, 0, 1, 1), 10);

		//traverse all points in the point cloud, get the position, color and draw
		for (zItPointCloudVertex v(oCloud); !v.end(); v++)
		{
			//draw pt
			model.displayUtils.drawPoint(v.getPosition(), v.getColor(), voxelSize * 10);
		}
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	//add strings to screen
	drawString("AADRL WS2 2022", vec(50, 250, 0));
	drawString("Total num of voxels #:" + to_string(numVoxels), vec(50, 275, 0));
	drawString("p - compute the point cloud", vec(50, 300, 0));
	drawString("m - move the target", vec(50, 325, 0));
	drawString("o - export to csv", vec(50, 350, 0));

	restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;
	if (k == 'm') movePt = true;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
