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
bool exportMesh = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjGraph oGraph;

zObjComputeMesh oCompMesh;

zPointArray positions;
zFloatArray positionsZ;

int id = 1;
string fileName = "data/toSTAD/TXT/3DPattern_External_2407_";
string inFilePath = fileName + to_string(id) + ".txt ";
string outFilePath = fileName + to_string(id) + ".json";

zColor BLACK(0, 0, 0, 1);
zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor MAGENTA(1, 0, 1, 1);
zColor CYAN(0, 1, 1, 1);
zColor ORANGE(1, 0.5, 0, 1);
zColor YELLOW(1, 1, 0, 1);

////// --- CUSTOM METODS ----------------------------------------------------

void drawTextAtVec(string s, zVector& pt)
{
	unsigned int i;
	glRasterPos3f(pt.x, pt.y, pt.z);

	for (i = 0; i < s.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
}

bool readTXT(string path, zObjMesh& _o_Mesh, int stride)
{
	ifstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	zPointArray positions;
	zBoolArray onBoundaryArray;
	zIntArray pConnects, pCounts;	

	unordered_map <string, int> positionVertex;

	int vCounter = 0;
	zIntPair eVertexPair;

	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		vector<string> perlineData = core.splitString(str, " ");

		if (perlineData.size() > 0)
		{
			// vertex
			if (perlineData[0] == "v")
			{
				if (perlineData.size() == 4)
				{
					zVector pos;
					pos.x = atof(perlineData[1].c_str());
					pos.y = atof(perlineData[2].c_str());
					pos.z = atof(perlineData[3].c_str());

					
					positions.push_back(pos);				
					
				}
				
			}

		
		}
	}

	myfile.close();

	for (int i = 0; i < positions.size() - stride; i+= stride)
	{
		int nextLoopStartID = (i + stride);

		for (int j = 0; j < stride; j++)
		{
			int nextID = (j + 1) % stride;

			pConnects.push_back(i + j);
			pConnects.push_back(nextLoopStartID + j);
			pConnects.push_back(nextLoopStartID + nextID);
			pConnects.push_back(i + nextID);
			pCounts.push_back(4);
		}

		

	}

	zFnMesh fnMesh(_o_Mesh);	
	fnMesh.create(positions,pCounts,pConnects);		

	printf("\n v %i  e %i f %i", fnMesh.numVertices(), fnMesh.numEdges(), fnMesh.numPolygons());
}



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

	// read Graph	
	readTXT(inFilePath, oMesh, 3);
	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oGraph);
	model.addObject(oMesh);

	// set display element booleans
	oGraph.setDisplayElements(true, true);	
	oMesh.setDisplayElements(true, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

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

		compute = !compute;	
	}

	if (exportMesh)
	{
		zFnMesh fnMesh(oMesh);
		fnMesh.to(outFilePath, zJSON);

		exportMesh = !exportMesh;
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

		/*zFnGraph fnGraph(oGraph);
		zPoint* positions = fnGraph.getRawVertexPositions();

		for (int i = 0; i < fnGraph.numVertices(); i++)
		{
			drawTextAtVec(to_string(i), positions[i]);
		}*/
		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'e') exportMesh = true;;
	

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
