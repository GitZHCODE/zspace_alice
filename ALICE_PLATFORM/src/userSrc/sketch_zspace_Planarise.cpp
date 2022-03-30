//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;
/*!<Objects*/

zUtilsCore core;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 
 bool planariseMesh(zObjMesh &o_inMesh, zObjParticleArray &o_inparticles, vector<zFnParticle> &fn_inParticles, zDomainFloat& deviation, float dT, zIntergrationType type, float tolerance, int numIterations, bool printInfo, bool minEdgeConstraint, float minEdgeLen)
{
	 zFnMesh fnMesh(o_inMesh);

	 zPoint* meshPts = fnMesh.getRawVertexPositions();

	 // create particles if it doesnt exist
	 if (fn_inParticles.size() != fnMesh.numVertices())
	 {
		 fn_inParticles.clear();
		 o_inparticles.clear(); 


		 for (int i = 0; i < fnMesh.numVertices(); i++)
		 {
			 bool fixed = false;			 

			 zObjParticle p;
			 p.particle = zParticle(meshPts[i], fixed);
			 o_inparticles.push_back(p);

		 }

		 for (int i = 0; i < o_inparticles.size(); i++)
		 {
			 fn_inParticles.push_back(zFnParticle(o_inparticles[i]));
		 }

		 // Fix some vertices if needed
		 //for (zItMeshVertex v(o_inMesh); !v.end(); v++)
		 //{
			// //if (v.onBoundary()) fn_inParticles[v.getId()].setFixed(true);
			// if (v.checkValency(2)) fn_inParticles[v.getId()].setFixed(true);
		 //}
	 }

	 // update

	 vector<zIntArray> fTris;
	 zPointArray fCenters;
	 zDoubleArray fVolumes;
	 zVectorArray fNormals;

	 fnMesh.getMeshTriangles(fTris);

	 zVectorArray v_residual;
	 v_residual.assign(fnMesh.numVertices(), zVector());

	 for (int k = 0; k < numIterations; k++)
	 {
		 fCenters.clear();
		 fnMesh.getMeshFaceVolumes(fTris, fCenters, fVolumes, true);

		 fNormals.clear();
		 fnMesh.getFaceNormals(fNormals);
		
		 // Compute planarity forces
		 for (zItMeshFace f(o_inMesh); !f.end(); f++)
		 {
			 int i = f.getId();

			 if (fVolumes[i] > tolerance)
			 {
				 vector<int> fVerts;
				 f.getVertices(fVerts);

				 for (int j = 0; j < fVerts.size(); j++)
				 {
					 double dist = core.minDist_Point_Plane(meshPts[fVerts[j]], fCenters[i], fNormals[i]);
					 zVector pForce = fNormals[i] * dist * -1.0;
					 fn_inParticles[fVerts[j]].addForce(pForce);				 

				 }
			 }

		 }

		
		 // edge length constraint
		 if (minEdgeConstraint)
		 {

			 for (zItMeshEdge e(o_inMesh); !e.end(); e++)
			 {
				 int i = e.getId();

				 if (e.getLength() < minEdgeLen)
				 {

					 zItMeshHalfEdge he0 = e.getHalfEdge(0);
					 zItMeshHalfEdge he1 = e.getHalfEdge(1);

					 zVector  he0_vec = he0.getVector();
					 zVector  he1_vec = he1.getVector();
					 he0_vec.normalize();
					 he1_vec.normalize();

					 zVector pForce1 = (he1.getStartVertex().getPosition() + he1_vec * minEdgeLen) - he1.getVertex().getPosition();
					 zVector pForce0 = (he0.getStartVertex().getPosition() + he0_vec * minEdgeLen) - he0.getVertex().getPosition();

					 fn_inParticles[he1.getVertex().getId()].addForce(pForce1);
					 fn_inParticles[he0.getVertex().getId()].addForce(pForce0);
					

				 }
			 }
		 }
		


		 // update positions
		 for (int i = 0; i < fn_inParticles.size(); i++)
		 {
			 fn_inParticles[i].integrateForces(dT, type);
			 fn_inParticles[i].updateParticle(true);
		 }

		 fnMesh.computeMeshNormals();
	 }

	 bool out = true;

	 zFloatArray deviations;

	 fCenters.clear();
	 fnMesh.getMeshFaceVolumes(fTris, fCenters, fVolumes, true);

	 deviation.min = core.zMin(fVolumes);
	 deviation.max = core.zMax(fVolumes);

	 if (deviation.max > tolerance) out = false;

	 zDomainColor colDomain(zColor(0, 0.55, 0.95, 1), zColor(0.95, 0, 0.55, 1));

	 for (zItMeshFace f(o_inMesh); !f.end(); f++)
	 {
		 int i = f.getId();

		 /*zColor col = core.blendColor(fVolumes[i], deviation, colDomain, zHSV);
		 if (fVolumes[i] < tolerance) col = zColor(120, 1, 1);
		 f.setColor(col);*/

		 if (fVolumes[i] < tolerance) f.setColor(colDomain.min);
		 else f.setColor(colDomain.max);

	 }

	 if (printInfo) printf("\n planarity tolerance : %1.7f minDeviation : %1.7f , maxDeviation: %1.7f ", tolerance, deviation.min, deviation.max);

	 if (out)
	 {
		 printf("\n planarity tolerance : %1.7f minDeviation : %1.7f , maxDeviation: %1.7f ", tolerance, deviation.min, deviation.max);
	
	 }

	 return out;

}


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

float planarityTol = 0.001;

double dT = 1.0;
zIntergrationType intType = zRK4;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

/*!< input mesh object  */
zObjMesh o_Mesh;

/*!< container of  particle objects  */
zObjParticleArray o_Particles;

/*!< container of particle function set  */
vector<zFnParticle> fnParticles;


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

	// read mesh
	zFnMesh fnMesh(o_Mesh);
	fnMesh.from("data/testPlanarise.obj", zOBJ);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_Mesh);

	// set display element booleans
	o_Mesh.setDisplayElements(true, true, true);

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
		zDomainFloat deviation;
		bool out = planariseMesh(o_Mesh, o_Particles, fnParticles, deviation, dT, intType, planarityTol, 1, true, false,3.0);
		
		compute = !out;
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
		
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p')compute = true;;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
