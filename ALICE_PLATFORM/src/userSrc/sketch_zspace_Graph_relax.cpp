//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsStatics.h>


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

zTsGraphVault myVault;
double vLoad = 1.0;
double forceDensity = 0.10;

zObjGraph oinGraph;
zObjGraph oGraph;

vector<zFnParticle> fnParticles;
zObjParticleArray o_particles;

float dT = 0.5;
zIntergrationType type = zRK4;
float tolerance = 0.001;
int numIterations = 1;
bool printInfo = true;

int id = 1;
string inFilePath = "data/toSTAD/TXT/3DPattern_0808_" + to_string(id) + ".txt ";
string outFilePath = "data/toSTAD/TXT/3DPattern_0208_" + to_string(id) + ".obj";

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

bool readTXT(string path, zObjGraph& _o_Graph)
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
	zIntArray eConnects;
	

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
				if (perlineData.size() == 5)
				{
					zVector pos;
					pos.x = atof(perlineData[1].c_str());
					pos.y = atof(perlineData[2].c_str());
					pos.z = atof(perlineData[3].c_str());

					bool onBoundary = (perlineData[4] == "True") ? true : false;

					zPoint pos_factorised = core.factoriseVector(pos, 2);

					int vId = -1;
					bool chk = core.checkRepeatVector(pos_factorised, positions, vId, 2);

					if (chk)
					{
						onBoundaryArray[vId] = onBoundary;
					}
					else
					{
						vId = positions.size();
						positions.push_back(pos);
						onBoundaryArray.push_back(onBoundary);						
					}

					

					vCounter++;
					
					(vCounter % 2 == 0) ? eVertexPair.first = vId : eVertexPair.second = vId;

					if (vCounter % 2 == 0)
					{
						eConnects.push_back(eVertexPair.first);
						eConnects.push_back(eVertexPair.second);
					}

					
				}
				
			}

		
		}
	}

	myfile.close();

	zFnGraph fnGraph(_o_Graph);
	zVector up(0, 0, -1);
	fnGraph.create(positions, eConnects, up);

	zColor* vColors = fnGraph.getRawVertexColors();
	for (int i = 0; i < onBoundaryArray.size(); i++)
	{
		if (!onBoundaryArray[i]) vColors[i] = GREEN;
	}
	

	printf("\n v %i  e %i ", fnGraph.numVertices(), fnGraph.numEdges());
}

void relaxGraph(zObjGraph& _o_Graph, vector<zFnParticle>& _fnParticles, zObjParticleArray& _o_particles, float dT, zIntergrationType type, float tolerance, int numIterations, bool printInfo)
{
	zFnGraph fnGraph(_o_Graph);


	// create particles if it doesnt exist
	if (_fnParticles.size() != fnGraph.numVertices())
	{
		_fnParticles.clear();
		_o_particles.clear();

		zColor* vColor = fnGraph.getRawVertexColors();

		for (int i = 0; i < fnGraph.numVertices(); i++)
		{
			bool fixed = (vColor[i] == BLACK) ? true : false;
			
			zObjParticle p;
			p.particle = zParticle(_o_Graph.graph.vertexPositions[i], fixed);
			_o_particles.push_back(p);

		}

		for (int i = 0; i < _o_particles.size(); i++)
		{
			_fnParticles.push_back(zFnParticle(_o_particles[i]));
		}

		fnGraph.setEdgeColor(GREEN);
	}

	// update

	zFloatArray deviations;
	deviations.assign(fnGraph.numVertices(), 100);

	zVector grav(0, 0, 0.05);

	for (int k = 0; k < numIterations; k++)
	{
		//compute forces
		
		for (zItGraphVertex v(_o_Graph); !v.end(); v++)
		{
			int i = v.getId();

			
				if (_fnParticles[i].getFixed()) continue;

				zItGraphHalfEdgeArray cEdges;
				v.getConnectedHalfEdges(cEdges);

				zVector eForce;

				for (auto& he : cEdges)
				{
					int v1 = he.getVertex().getId();
					zVector e = he.getVector();

					//double len = e.length();
					//e.normalize();				

					eForce += (e);
				}

				eForce /= cEdges.size();
				

				// perturb only in z
				eForce.x = eForce.y = 0;
							

				_fnParticles[i].addForce(eForce);

				_fnParticles[i].addForce(grav);
			
				deviations[i] = eForce.length();

		}

		
		// update positions
		for (int i = 0; i < _fnParticles.size(); i++)
		{
			_fnParticles[i].integrateForces(dT, type);
			_fnParticles[i].updateParticle(true);
		}			

	}

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
	readTXT(inFilePath, oGraph);

	readTXT(inFilePath, oinGraph);

	myVault = zTsGraphVault(oGraph);


	// default set constraints - boundary vertices

	zIntArray fixedVertices;
	for (zItGraphVertex v(oGraph); !v.end(); v++)
	{
		if (v.getColor() == RED) fixedVertices.push_back(v.getId());
	}

	myVault.setConstraints(zResultDiagram, fixedVertices);

	myVault.setVertexWeightsfromConstraints(zResultDiagram);

	// default set loads
	myVault.setVertexMass(vLoad);

	myVault.setVertexThickness(0.1);

	

	
	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oGraph);
	model.addObject(oinGraph);

	// set display element booleans
	oGraph.setDisplayElements(true, true);	
	oinGraph.setDisplayElements(false, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&forceDensity, "forceDensity");
	S.sliders[1].attachToVariable(&forceDensity, -5.0, 5.0);

	S.addSlider(&vLoad, "vLoad");
	S.sliders[2].attachToVariable(&vLoad, 0.0, 20.0);

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
		
		relaxGraph(oGraph, fnParticles, o_particles, dT, type, tolerance, numIterations, printInfo);

		// set forcedensities
		//myVault.setForceDensity(forceDensity);
		
		// formfind
		//myVault.forceDensityMethod();


		compute = !compute;	
	}

	if (exportMesh)
	{
		
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
