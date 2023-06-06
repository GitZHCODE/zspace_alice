//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include<headers/zToolsets/statics/zTsVault.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool readGraph = false;
bool equilibrium = false;
bool equilibriumREACHED = false;

bool display = true;
bool dFormGraph = true;
bool dForceMeshes = true;
bool displayAllMeshes = true;

double background = 0.35;

int currentMeshID = 0;
int totalMeshes = 0;

bool compTargets = true;
double formWeight = 1.0;
double areaScale = 1.0;
double dT = 0.1;
float minMax_formEdge = 0.1;
bool areaForce = false;
int numIterations = 1;
double angleTolerance = 5;
double areaTolerance = 0.1;
bool printInfo = true;

bool exportFiles = false;

double strength = 1;


////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zUtilsCore core;
zIntergrationType type = zRK4;

/*!<Tool sets*/
zObjMesh o_inMesh;
zObjMesh o_mesh;
zObjParticleArray o_Particles;
vector<zFnParticle> fnParticles;

zDoubleArray restLength;

zColor red(1, 0, 0, 1);
zColor blue(0, 0, 1, 1);
zColor green(0, 1, 0, 1);

////// --- CUSTOM METHODS -------------------------------------------------


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

	

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_mesh);
	model.addObject(o_inMesh);

	// set display element booleans
	o_mesh.setDisplayElements(true, true, false);
	o_inMesh.setDisplayElements(false, true, false);
	
	

	////////////////////////////////////////////////////////////////////////// Sliders
	
	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&strength, "strength");
	S.sliders[1].attachToVariable(&strength, 0, 100);


	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&equilibrium, "equilibrium");
	B.buttons[0].attachToVariable(&equilibrium);

	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);


}

void update(int value)
{
	if (readGraph)
	{
		zFnMesh fnMesh_input(o_inMesh);
		fnMesh_input.from("data/suspensionBridge_00.json", zJSON);
		fnMesh_input.setEdgeColor(zColor(0.25, 0.25, 0.25, 1));

		zFnMesh fnMesh(o_mesh);		
		fnMesh.from("data/suspensionBridge_00.json", zJSON);

		fnParticles.clear();
		o_Particles.clear();
		restLength.clear();

		zColor* vColors = fnMesh.getRawVertexColors();

		for (int i = 0; i < fnMesh.numVertices(); i++)
		{
			bool fixed = (vColors[i] == red || vColors[i] == blue) ? true : false;
			
			zObjParticle p;
			p.particle = zParticle(o_mesh.mesh.vertexPositions[i], fixed);
			o_Particles.push_back(p);

		}

		for (int i = 0; i < o_Particles.size(); i++)
		{
			fnParticles.push_back(zFnParticle(o_Particles[i]));
		}

		//fnMesh.getEdgeLengths(restLength);
		restLength.assign(fnMesh.numEdges(), 0.01);
		
		readGraph = !readGraph;
	}

	if (equilibrium)
	{
		
		zVector grav(0, strength * 1, 0);
		zFnMesh fnMesh(o_mesh);

		zPoint* vPositions = fnMesh.getRawVertexPositions();
		zColor* vColors = fnMesh.getRawVertexColors();

		for (int k = 0; k < numIterations; k++)
		{	
			
			for (zItMeshVertex v(o_mesh); !v.end(); v++)
			{
				//if (vColors[v.getId()] == red || vColors[v.getId()] == blue) continue;

				fnParticles[v.getId()].addForce(grav);		

				/*zItMeshHalfEdgeArray cHEdges;
				v.getConnectedHalfEdges(cHEdges);

				zVector resultant;
				int numE = 0;
				for (auto& cHE : cHEdges)
				{
					if (vColors[cHE.getVertex().getId()] == blue) continue;
					
					
					resultant += cHE.getVector();
					numE++;
				}

				resultant /= numE;
				fnParticles[v.getId()].addForce(resultant * strength);*/
			}			

			for (zItMeshEdge e(o_mesh); !e.end(); e++)
			{
				int v1 = e.getHalfEdge(0).getStartVertex().getId();
				int v2 = e.getHalfEdge(0).getVertex().getId();

				//if (vColors[v1] == blue || vColors[v2] == blue)
				//{
					zVector eVec = vPositions[v2] - vPositions[v1];
					float eLen = eVec.length();
					eVec.normalize();

					float restLen = restLength[e.getId()];

					float val = 1 * (eLen - restLen);
					zVector pForce_v1 = eVec * (val * 0.5);

					zVector pForce_v2 = pForce_v1 * -1;

					fnParticles[v1].addForce(pForce_v1);
					fnParticles[v2].addForce(pForce_v2);
				//}

				
			}

			//for (zItMeshVertex v(o_mesh); !v.end(); v++)
			//{
			//	int i = v.getId();


			//	zItMeshHalfEdgeArray cEdges;
			//	v.getConnectedHalfEdges(cEdges);

			//	zVector eForce;

			//	for (auto& he : cEdges)
			//	{
			//		int v1 = he.getVertex().getId();
			//		zVector e = he.getVector();

			//		//double len = e.length();
			//		//e.normalize();				

			//		eForce += (e);
			//	}

			//	eForce /= cEdges.size();


			//	// perturb only in z
			//	eForce.x = eForce.z = 0;


			//	fnParticles[i].addForce(eForce);

			//	fnParticles[i].addForce(grav);

			//	//deviations[i] = eForce.length();

			//}
			

			// update positions
			for (int i = 0; i < fnParticles.size(); i++)
			{
				fnParticles[i].integrateForces(dT, type);
				fnParticles[i].updateParticle(true);
			}

			
		}		


		equilibrium = !equilibrium;	
	}

	if (exportFiles)
	{
		zFnMesh fnMesh(o_mesh);
		fnMesh.to("data/suspensionBridge_00_formfound.obj", zOBJ);

		exportFiles = !exportFiles;
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
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Nodes #:" + to_string(totalMeshes), vec(winW - 350, winH - 800, 0));
	drawString("Current Node #:" + to_string(currentMeshID), vec(winW - 350, winH - 775, 0));

	string equilibriumText = (equilibriumREACHED) ? "true" : "False";
	drawString("equilibrium REACHED:" + equilibriumText, vec(winW - 350, winH - 750, 0));

	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("r - read graph", vec(winW - 350, winH - 600, 0));
	drawString("p - compute equilibrium", vec(winW - 350, winH - 575, 0));
	drawString("c - compute targets", vec(winW - 350, winH - 550, 0));
	drawString("e - export files", vec(winW - 350, winH - 525, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'r')
	{
		readGraph = true;
	}

	if (k == 'p')
	{		
		equilibrium = true;;
	}	

	if (k == 'e')
	{
		exportFiles = true;
	}
	

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
