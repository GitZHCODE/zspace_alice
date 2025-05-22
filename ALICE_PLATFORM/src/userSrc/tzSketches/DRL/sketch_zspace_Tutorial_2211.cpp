//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include "VoxelSystem.h"

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

//Voxel systems
VoxelSystem vs;

zObjGraph oGraph;
zObjMesh oBbox;

//create a container for the unit mesh
zObjMeshArray inMesh;

//create a container for the populated mesh
zObjMeshArray fieldMeshArray;

//parameter controls the field offset
double fieldOffset = 0.0;

//declare field values
zFloatArray fieldValues;

double meshScale = 0;
double threshold = 0;

bool showMesh = true;

////// --- GUI OBJECTS ----------------------------------------------------


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	// read the boundingbox
	zFnMesh fnBbox(oBbox);
	fnBbox.from("data/drl/bbox.obj", zOBJ);

	//get the minBB & maxBB from the boundingbox
	zPoint minBB, maxBB;
	fnBbox.getBounds(minBB, maxBB);

	//set the boundingbox of the field
	vs.setFieldBounds(minBB, maxBB);

	//declare the dimensions XYZ
	int nX, nY, nZ;
	nX = nY = nZ = 10;
	//set the dimension of the field;
	vs.setFieldDim(nX, nY, nZ);

	//set the graph of the field (.obj)
	string path1 = "data/drl/graph.obj";
	vs.setGraphFromFile(oGraph, path1, zOBJ);

	//set the container of the mesh population and the input mesh directory (folder)
	string path2 = "data/drl/voxelMesh";
	vs.setMeshFromDirectory(inMesh, path2, zOBJ);

	//set display options of the field
	//particles, graph vertex, graph edges, mesh vertex, mesh edges, mesh faces
	vs.setDisplayElements(true, false, true, false, false, false);

	//create the field
	vs.create();


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oBbox);
	oBbox.setDisplayElements(false, true, false);
	// set display element booleans

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&fieldOffset, "fieldOffset");
	S.sliders[1].attachToVariable(&fieldOffset, -10, 10);
	S.addSlider(&meshScale, "meshScale");
	S.sliders[2].attachToVariable(&meshScale, -5, 5);
	S.addSlider(&threshold, "threshold");
	S.sliders[3].attachToVariable(&threshold, -5, 5);

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
		//set the field offset
		vs.setFieldOffset(fieldOffset);

		//update the field values from the distance to the graph
		vs.update();

		//copy the field values from class to sketch
		fieldValues = vs.getFieldValues(false);

		//copy the mesh from class to sketch
		fieldMeshArray = vs.getFieldMesh();

		//get the centers of the field
		zPointArray centers = vs.getFieldPosition();


		cout << endl;
		//ouput values to the console
		for (int i = 0; i < fieldMeshArray.size(); i++)
		{
			if (fieldValues[i] < threshold)
			{
				fieldMeshArray[i] = zObjMesh();
			}
			else
			{
				//fnMesh refering to current obj mesh in the array
				zFnMesh fnMesh(fieldMeshArray[i]);

				zPointArray vertices;
				fnMesh.getVertexPositions(vertices);

				//an iterator of the vertices in a mesh
				zItMeshVertex v(fieldMeshArray[i]);

				//get each vertex position and update it by iterators
				for (v.begin(); !v.end(); v++)
				{
					if (!v.onBoundary())
					{

						zPoint pos = v.getPosition();
						zVector vec = pos - centers[i];
						vec.normalize();

						vec = vec * meshScale * fieldValues[i];
						pos += vec; //same as v = v + vec;

						//update the vertex positions
						v.setPosition(pos);
					}
				}
			}

			//cout << fieldValues[i] << ",";

			//cout << vertices[i] << endl;

			//cout
			//	<< "V: " << fnMesh.numVertices()
			//	<< "E: " << fnMesh.numEdges()
			//	<< "F: " << fnMesh.numPolygons() << endl;
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

	if (display)
	{
		// zspace model draw
		model.draw();

		//draw the voxels
		//particles, graph, mesh
		vs.draw(true, true, false);

		if(showMesh)
		for (int i = 0; i < fieldMeshArray.size(); i++)
			fieldMeshArray[i].draw();
	}




	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

	if (k == 'm') showMesh = !showMesh;

	if (k == 'o')
	{
		string path = "data/drl/output.obj";
		vs.exportToObj(path, fieldMeshArray);
	}
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
