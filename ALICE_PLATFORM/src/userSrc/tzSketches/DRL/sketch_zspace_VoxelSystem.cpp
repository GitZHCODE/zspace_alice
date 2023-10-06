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
bool computeMap = false;
bool exportToCSV = false;
bool display = true;
bool ptMove = false;

double background = 1.0;
double voxelSize = 0.35;
int numVoxels = -1;
////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjGraph oGraph;

zObjPointCloud oCloud;
zPoint minBB, maxBB;

zPoint target(10, 10, 0);
zVector force(0, 0, 0.5);


////// --- GUI OBJECTS ----------------------------------------------------

void createPointCloud(zFnPointCloud& fnCloud, double unit_X, double unit_Y, double unit_Z, int n_X, int n_Y, int n_Z, zPoint& _minBB, zPoint& _maxBB, zPoint startPt = zPoint(0, 0, 0))
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
	fnCloud.create(positions);
	fnCloud.getBounds(_minBB, _maxBB);
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
	model.addObject(oCloud);
	oCloud.setDisplayElements(false);



		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans
	//oMesh.setDisplayElements(true, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&voxelSize, "size");
	S.sliders[1].attachToVariable(&voxelSize, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&ptMove, "ptMove");
	B.buttons[2].attachToVariable(&ptMove);
	B.addButton(&exportToCSV, "exportToCSV");
	B.buttons[3].attachToVariable(&exportToCSV);

}

void update(int value)
{
	if (compute)
	{
		zFnPointCloud fnCloud(oCloud);
		float unitX, unitY, unitZ;
		int numX, numY, numZ;

		unitX = unitY = unitZ = 1;
		numX = numY = numZ = 20;

		createPointCloud(fnCloud, unitX, unitY, unitZ, numX, numY, numZ, minBB, maxBB);
		//cout << maxBB << "," << minBB << endl;

		numVoxels = fnCloud.numVertices();
		zDomainColor colDomain(zColor(0, 0, 0, 0), zColor(1, 0, 0, 1));

		float maxDist = (maxBB - minBB).length();
		for (zItPointCloudVertex v(oCloud); !v.end(); v++)
		{
			zVector vec = v.getPosition() - target;
			float dist = vec.length();
			zColor col = core.blendColor(dist, zDomainFloat(0,maxDist), colDomain, zRGB);
			v.setColor(col);
		}

		//compute = !compute;	
	}

	if (ptMove)
	{
		if (target.z + force.z > 20 || target.z + force.z < 0)
			force.z = -force.z;

		//cout << force << endl;
		target += force;
	}

	if (exportToCSV)
	{
		ofstream file;
		file.open("data/points.csv");

		float maxDist = (maxBB - minBB).length();
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
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		
		model.displayUtils.drawPoint(target, zColor(0, 0, 1, 1), 10);

		for (zItPointCloudVertex v(oCloud); !v.end(); v++)
		{
			//draw pt
			model.displayUtils.drawPoint(v.getPosition(), v.getColor(), 10* voxelSize);

			////draw cube
			//zPoint min(v.getPosition().x - voxelSize, v.getPosition().y - voxelSize, v.getPosition().z - voxelSize);
			//zPoint max(v.getPosition().x + voxelSize, v.getPosition().y + voxelSize, v.getPosition().z + voxelSize);
			//model.displayUtils.drawCube(min,max, v.getColor());

		}
		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();
	glColor3f(0, 0, 0);

	drawString("AADRL WS2 2022", vec(50, 200, 0));

	drawString("Total num of voxels #:" + to_string(numVoxels), vec(50, 250, 0));
	drawString("p - compute voxels", vec(50, 275, 0));
	drawString("m - move target", vec(50, 300, 0));
	drawString("o - export to csv", vec(50, 325, 0));

	restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

	if (k == 'o') exportToCSV = true;

	if (k == 'm') ptMove = true;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
