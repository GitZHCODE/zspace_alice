#//define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

//#include <headers/zApp/include/zTsGeometry.h>

//#include <igl/avg_edge_length.h>
//#include <igl/cotmatrix.h>
//#include <igl/invert_diag.h>
//#include <igl/massmatrix.h>
//#include <igl/parula.h>
//#include <igl/per_corner_normals.h>
//#include <igl/per_face_normals.h>
//#include <igl/per_vertex_normals.h>
//#include <igl/principal_curvature.h>
//#include <igl/gaussian_curvature.h>
//#include <igl/read_triangle_mesh.h>

using namespace zSpace;
using namespace std;
////////////////////////////////////////////

zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);

zColor YELLOW(1, 1, 0, 1);

zUtilsCore core;

struct zPair_hash {
	template <class T1, class T2>
	size_t operator()(const pair<T1, T2>& p) const
	{
		auto hash1 = hash<T1>{}(p.first);
		auto hash2 = hash<T2>{}(p.second);

		if (hash1 != hash2) {
			return hash1 ^ hash2;
		}

		// If hash1 == hash2, their XOR is zero.
		return hash1;
	}
};

void computeGridPlanes(zObjMesh& o_mesh, int  startVID, vector<zIntArray> &outVIDs, zPointArray& outOrigins, zVectorArray& outNormals, int loopSteps)
{
	zFnMesh fnMesh(o_mesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zColor* vColors = fnMesh.getRawVertexColors();


	outVIDs.clear();
	outOrigins.clear();
	outNormals.clear();

	zBoolArray vVisited;
	vVisited.assign(fnMesh.numVertices(), false);


	

	zItMeshVertex v_start(o_mesh, startVID);
	zItMeshVertex v = v_start;
	
	zItMeshHalfEdge he_U, he_V;

	zItMeshHalfEdgeArray cHEdges;
	v.getConnectedHalfEdges(cHEdges);

	for (auto& cHE : cHEdges)
	{
		if (cHE.onBoundary()) he_U = cHE;
	}

	zColor col = zMAGENTA;

	bool exit_U = false;
	int U_Counter = 0;
	do
	{
		if (he_U.getVertex().checkValency(2)) exit_U = true;

		he_V = he_U.getSym().getNext();

		if (U_Counter % loopSteps == 0)
		{

			zPoint p0 = vPositions[he_V.getStartVertex().getId()];
			p0.z = 0;

			zPoint p1 = vPositions[he_V.getVertex().getId()];
			p1.z = 0;

			zVector X = p1 - p0;
			X.normalize();

			zVector Y(0, 0, 1);

			zVector Z = X ^ Y;
			Z.normalize();

			outOrigins.push_back(p0);
			outNormals.push_back(Z);

			zIntArray vIDs;

			bool exit_V = false;
			do
			{
				vIDs.push_back(he_V.getStartVertex().getId());
				if (he_V.getVertex().onBoundary())
				{
					exit_V = true;
					vIDs.push_back(he_V.getVertex().getId());
				}

				he_V.getEdge().setColor(col);
				if (!exit_V) he_V = he_V.getNext().getSym().getNext();

			} while (!exit_V);

			outVIDs.push_back(vIDs);

			/*cout << "\n \n  X " << X;
			cout << "\n Y " << Y;
			cout << "\n Z " << Z;
			cout << "\n O " << p0;
			cout << "\n vIDs " << vIDs.size();*/
		}
		

		//he_U = he_U.getNext();
		U_Counter++;
		if (he_U.getVertex() == v_start) exit_U = true;
		if (!exit_U) he_U = he_U.getNext();

	} while (!exit_U);

	
	printf("\n Plane constraints %i %i ", outOrigins.size(), outNormals.size());

}

void computeDistancePairs(zObjMesh& o_mesh, int  startVID, zIntPairArray& outVPairs, zDoubleArray &outDistances, int loopSteps)
{
	zFnMesh fnMesh(o_mesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zColor* vColors = fnMesh.getRawVertexColors();


	outVPairs.clear();
	outDistances.clear();

	zBoolArray vVisited;
	vVisited.assign(fnMesh.numVertices(), false);




	zItMeshVertex v_start(o_mesh, startVID);
	zItMeshVertex v = v_start;

	zItMeshHalfEdge he_U, he_V;

	zItMeshHalfEdgeArray cHEdges;
	v.getConnectedHalfEdges(cHEdges);

	for (auto& cHE : cHEdges)
	{
		if (cHE.onBoundary()) he_U = cHE;
	}

	zColor col = zYELLOW;

	int U_Counter = 0;
	bool exit_U = false;
	do
	{
		if (he_U.getVertex().checkValency(2)) exit_U = true;
		
		if (U_Counter % loopSteps == 0)
		{
			he_V = he_U.getSym().getNext();



			int V_Counter = 0;
			bool exit_V = false;
			do
			{
				if (he_V.getVertex().onBoundary())
				{
					exit_V = true;
				}

				if (V_Counter % 2 == 1)
				{
					if (he_V.getLength() < 0.8)
					{
						he_V.getEdge().setColor(col);

						outVPairs.push_back(zIntPair(he_V.getStartVertex().getId(), he_V.getVertex().getId()));
						
						
						if(he_V.getLength() > 0.5) outDistances.push_back(0.5);
						else outDistances.push_back(he_V.getLength());

						//fixed distance
						//outDistances.push_back(0.25);

						//printf("\n pair %i %i ", he_V.getStartVertex().getId(), he_V.getVertex().getId());
					}

				
				}

				V_Counter++;
				if (!exit_V) he_V = he_V.getNext().getSym().getNext();

			} while (!exit_V);

		}
		
		U_Counter++;
		//he_U = he_U.getNext();

		if (he_U.getVertex() == v_start) exit_U = true;
		if (!exit_U) he_U = he_U.getNext();

	} while (!exit_U);


	printf("\n Distance constraints %i %i  ", outVPairs.size(), outDistances.size());

}

void computeFixed(zObjMesh& o_mesh, int  startVID, zIntArray& fixedVerts, int loopSteps)
{
	zFnMesh fnMesh(o_mesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zColor* vColors = fnMesh.getRawVertexColors();


	
	fixedVerts.clear();

	zBoolArray vVisited;
	vVisited.assign(fnMesh.numVertices(), false);




	zItMeshVertex v_start(o_mesh, startVID);
	zItMeshVertex v = v_start;

	zItMeshHalfEdge he_U, he_V;

	zItMeshHalfEdgeArray cHEdges;
	v.getConnectedHalfEdges(cHEdges);

	for (auto& cHE : cHEdges)
	{
		if (cHE.onBoundary()) he_U = cHE;
	}

	zColor col = zRED;

	int U_Counter = 0;
	bool exit_U = false;
	do
	{
		if (he_U.getVertex().checkValency(2)) exit_U = true;

		if (U_Counter % loopSteps == 0)
		{
			he_V = he_U.getSym().getNext();



			int V_Counter = 0;
			bool exit_V = false;
			do
			{
				if (he_V.getVertex().onBoundary())
				{
					exit_V = true;
				}

				if (V_Counter % 2 == 1)
				{
					he_V.getEdge().setColor(col);

					fixedVerts.push_back(he_V.getStartVertex().getId());
					fixedVerts.push_back(he_V.getVertex().getId());
				}

				V_Counter++;
				if (!exit_V) he_V = he_V.getNext().getSym().getNext();

			} while (!exit_V);

		}

		U_Counter++;
		//he_U = he_U.getNext();

		if (he_U.getVertex() == v_start) exit_U = true;
		if (!exit_U) he_U = he_U.getNext();

	} while (!exit_U);


	printf("\n fixedVerts constraints %i  ", fixedVerts.size());

}

void computeAlignPairs(zObjMesh& o_mesh, zIntPairArray& alignPairs)
{
	alignPairs.clear();

	zFnMesh fnMesh(o_mesh);
	zColor* vColors = fnMesh.getRawVertexColors();

	zPoint* vPositions = fnMesh.getRawVertexPositions();

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{

		if (vColors[i] == YELLOW)
		{
			float distMin = 100000;
			int v0 = i;
			int v1 = -1;

			for (int j = 0; j < fnMesh.numVertices(); j++)
			{
				if (i != j)
				{
					float dist = vPositions[i].squareDistanceTo(vPositions[j]);
					if (dist < distMin)
					{
						distMin = dist;
						v1 = j;
					}
				}
			}

			if (v1 != -1)
			{
				alignPairs.push_back(zIntPair(v0, v1));
				//printf("\n %i %i ", v0, v1);
			}
		}
	}

	printf("\n Alignment constraint pairs %i  ", alignPairs.size());

}

void checkPlanarity(zObjMesh& oMesh, zDoubleArray &diaDistDevs, double tolerance = EPS)
{
	zFnMesh fnMesh(oMesh);

	diaDistDevs.clear();
	diaDistDevs.assign(fnMesh.numPolygons(), -1);

	float planarArea = 0;
	float totalArea = 0;
	int planarCounter = 0;

	for (zItMeshFace f(oMesh); !f.end(); f++)
	{
		int i = f.getId();		

		double uA, uB;
		zPoint pA, pB;

		zPointArray fVerts;
		f.getVertexPositions(fVerts);

		core.line_lineClosestPoints(fVerts[0], fVerts[2], fVerts[1], fVerts[3], uA, uB, pA, pB);

		float dist = pA.distanceTo(pB);

		diaDistDevs[f.getId()] = dist;

		float area = f.getPlanarFaceArea();
		totalArea += area;

		if (dist < tolerance)
		{
			f.setColor(zColor(0, 0.95, 0.270, 1));
			planarArea+= area;
			planarCounter++;
		}
		else f.setColor(zColor(0.95, 0, 0.55, 1));
	}

	float perc =  (planarArea / totalArea) * 100.0;
	printf("\n planar %i | planarity percentage : %1.2f ", planarCounter, perc);
}


////////////////////////////////////////////////////////////////////////// General

bool computePlanar = false;
bool computeHeights = false;
bool exportMESH = false;

bool d_inMesh = true;
bool d_outMesh = true;
bool d_outMesh_faces = true;
bool d_forces = true;

double background = 0.35;

int numIterations = 1;

////// --- zSpace Objects --------------------------------------------------


/*!<model*/
zModel model;

/*!<Objects*/

zObjMesh oMesh;
zObjMesh oMesh_in;

zObjMesh oMesh_unroll;

zObjMesh oMesh_inplanes;



zIntPairArray vertexAlignPairs;

int bsfcurrentID = 0;
int num_BSF = 0;

zFnMeshDynamics fnDyMesh;

zVectorArray fNorms;
zPointArray fCens;

double tol_distPairs = 0.005;
double tol_planar = 0.000001;
double tol_gridPlanar = 0.001;
double tol_unroll = 0.0001;

double strength_planar = 1.0;
double strength_gridPlanar = 0.1;
double strength_distPairs = 0.1;

float fPlanar_max, fPlanar_min;
float distPair_max, distPair_min;
float gPlanar_max, gPlanar_min;


float dT = 0.8;

zVectorArray forceDir_planar;
zVectorArray forceDir_gridPlanar;
zVectorArray forceDir_distPairs;

vector<zIntArray> gridPlanar_vGroups;
zPointArray gridPlanar_Origins;
zVectorArray gridPlanar_Normals;

zIntPairArray vPairs;
zDoubleArray distance_vPairs;
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

	//string fileName = "ccf_corner";
	//cout << "Please enter geometry file name: ";
	//cin >> fileName;

	// read mesh

	//string path = "data/pSolver/" + fileName + ".json";
	string path = "data/nansha/upper.json";

	string path1 = "data/nansha/upper.json";


	zFnMesh fnMesh_in(oMesh_in);
	fnMesh_in.from(path, zJSON);
	fnMesh_in.setEdgeColor(zColor(0.5, 0.5, 0.5, 1));

	zFnMesh fnMesh(oMesh);
	fnMesh.from(path1, zJSON);
		

	fnDyMesh.create(oMesh, false);

	//computeGridPlanes(oMesh_in, 0,  gridPlanar_vGroups, gridPlanar_Origins, gridPlanar_Normals,4);	
	//computeDistancePairs(oMesh_in, 0, vPairs, distance_vPairs, 4);

	zIntArray fixedVIds = {5,71,11,64, 4,10,0,2 };
	//computeFixed(oMesh_in, 2, fixedVIds, 4);

	/*for (zItMeshVertex v(oMesh); !v.end(); v++)
	{
		if (v.onBoundary())fixedVIds.push_back(v.getId());
	}*/

	/*zDoubleArray vertexCurvatures;
	fnMesh.getGaussianCurvature(vertexCurvatures);

	double gc_max = core.zMax(vertexCurvatures);
	double gc_min = core.zMin(vertexCurvatures);

	zDomainFloat gcDomain(gc_max, gc_min);
	zDomainColor colDomain(zMAGENTA, zBLUE);

	for (zItMeshVertex v(oMesh); !v.end(); v++)
	{
		zColor blendCol = core.blendColor(vertexCurvatures[v.getId()], gcDomain, colDomain, zHSV);
		v.setColor(blendCol);
	}

	fnMesh.computeFaceColorfromVertexColor()*/;
	

	fnDyMesh.setFixed(fixedVIds);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_in);
	model.addObject(oMesh_unroll);
	//model.addObject(o_dualGraph);

	// set display element booleans
	oMesh.setDisplayElements(true, true, false);
	oMesh_in.setDisplayElements(false, true, false);
	oMesh_unroll.setDisplayElements(false, true, false);
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&strength_planar, "str_planar");
	S.sliders[1].attachToVariable(&strength_planar, 0, 1);

	S.addSlider(&strength_distPairs, "str_dev");
	S.sliders[2].attachToVariable(&strength_distPairs, 0, 1);

	//S.addSlider(&strength_panelBoundaryplanar, "str_bplanar");
	//S.sliders[3].attachToVariable(&strength_panelBoundaryplanar, 0, 1);

	S.addSlider(&tol_unroll, "tol_unroll");
	S.sliders[3].attachToVariable(&tol_unroll, 0.000001, 0.01);

	S.addSlider(&tol_planar, "tol_planar");
	S.sliders[4].attachToVariable(&tol_planar, 0.0001, 0.01);

	S.addSlider(&tol_distPairs, "tol_distPairs");
	S.sliders[5].attachToVariable(&tol_distPairs, 0.0001, 0.01);



	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	
	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[0].attachToVariable(&d_inMesh);

	B.addButton(&d_outMesh, "d_outMesh");
	B.buttons[1].attachToVariable(&d_outMesh);

	B.addButton(&d_outMesh_faces, "d_outMesh_faces");
	B.buttons[2].attachToVariable(&d_outMesh_faces);	

	B.addButton(&d_forces, "d_forces");
	B.buttons[3].attachToVariable(&d_forces);


}

void update(int value)
{
	oMesh_in.setDisplayObject(d_inMesh);
	oMesh.setDisplayObject(d_outMesh);
	oMesh.setDisplayElements(true, true, d_outMesh_faces);

	if (computePlanar)
	{
				
		zDoubleArray devs_planarity;
		bool exit_planar;

		zDoubleArray devs_gridPlanar;
		bool exit_gridPlanar;
			
		zDoubleArray devs_distPairs;
		bool exit_distPairs;

		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addPlanarityForce(strength_planar, tol_planar, zVolumePlanar, devs_planarity, forceDir_planar, exit_planar, zSolverForceConstraints::zConstraintFree);

			//fnDyMesh.addPlanarityForce_vertexgroups(strength_gridPlanar, tol_gridPlanar, gridPlanar_vGroups, gridPlanar_Origins, gridPlanar_Normals, devs_gridPlanar, forceDir_gridPlanar, exit_gridPlanar);

			//fnDyMesh.addRigidLineForce(strength_distPairs, tol_distPairs, vPairs, distance_vPairs, devs_distPairs, forceDir_distPairs, exit_distPairs);

			fnDyMesh.update(dT, zRK4, true, true, true);
			
		}
		
		// Planar deviations
		/*for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_planar) f.setColor(zColor(0, 0.95, 0.27, 1));
			else f.setColor(zColor(0.95, 0, 0.55, 1));
		}*/

		//checkPlanarity(oMesh, devs_planarity, 0.0075);

		fnDyMesh.getPlanarityDeviationPerFace(devs_planarity, zQuadPlanar, true, 0.0075);

		fPlanar_max = core.zMax(devs_planarity);
		fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);


		/*gPlanar_max = core.zMax(devs_gridPlanar);
		gPlanar_min = core.zMin(devs_gridPlanar);
		printf("\n gridPlanar devs : %1.6f %1.6f \n", gPlanar_max, gPlanar_min);

		distPair_max = core.zMax(devs_distPairs);
		distPair_min = core.zMin(devs_distPairs);
		printf("\n distPairs devs : %1.6f %1.6f \n", distPair_max, distPair_min);*/

		/*for (int i = 0; i < vPairs.size(); i++)
		{
			zFnMesh fnMesh(oMesh);
			zItMeshHalfEdge he;
			bool chk = fnMesh.halfEdgeExists(vPairs[i].first, vPairs[i].second, he);

			if (chk)
			{
				if (devs_distPairs[i] > tol_distPairs) he.getEdge().setColor(RED);
				else he.getEdge().setColor(BLUE);				
			}
		}*/

		/*for (int i = 0; i < gridPlanar_vGroups.size(); i++)
		{
			zFnMesh fnMesh(oMesh);

			for (int j = 0; j < gridPlanar_vGroups[i].size() - 1; j++)
			{
				zItMeshHalfEdge he;
				bool chk = fnMesh.halfEdgeExists(gridPlanar_vGroups[i][j], gridPlanar_vGroups[i][j + 1], he);

				if (chk)
				{
					if (devs_gridPlanar[i] > tol_gridPlanar) he.getEdge().setColor(RED);
					else he.getEdge().setColor(BLUE);
				}
			}
			
		}*/
	
		//computePlanar = !computePlanar;
	}

	if (computeHeights)
	{
		zDoubleArray devs_gridPlanar;
		bool exit_gridPlanar;

		zDoubleArray devs_distPairs;
		bool exit_distPairs;

		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addPlanarityForce_vertexgroups(strength_gridPlanar, tol_gridPlanar, gridPlanar_vGroups, gridPlanar_Origins, gridPlanar_Normals, devs_gridPlanar, forceDir_gridPlanar, exit_gridPlanar);

			fnDyMesh.addRigidLineForce(strength_distPairs, tol_distPairs, vPairs, distance_vPairs, devs_distPairs, forceDir_distPairs, exit_distPairs);

			fnDyMesh.update(dT, zRK4, true, true, true);

		}

		gPlanar_max = core.zMax(devs_gridPlanar);
		gPlanar_min = core.zMin(devs_gridPlanar);
		printf("\n gridPlanar devs : %1.6f %1.6f \n", gPlanar_max, gPlanar_max);

		distPair_max = core.zMax(devs_distPairs);
		distPair_min = core.zMin(devs_distPairs);
		printf("\n distPairs devs : %1.6f %1.6f \n", distPair_max, distPair_min);

		//computeHeights = !computeHeights;
	}


	if (exportMESH)
	{			
		fnDyMesh.setVertexColor(BLUE);
		
		string fPath = "data/nansha/out_Roof.json";
		fnDyMesh.to(fPath, zJSON);

		exportMESH = !exportMESH;
	}

}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	
	zPoint* vPositions = fnDyMesh.getRawVertexPositions();
	
	if (d_forces)
	{
		for (int i = 0; i < forceDir_planar.size(); i++)
		{
			zPoint v2 = vPositions[i] + (forceDir_planar[i] * 0.1);
			model.displayUtils.drawLine(vPositions[i], v2, BLUE);
		}

		for (int i = 0; i < forceDir_gridPlanar.size(); i++)
		{
			zPoint v2 = vPositions[i] + (forceDir_gridPlanar[i] * 0.1);
			model.displayUtils.drawLine(vPositions[i], v2, RED);
		}

		for (int i = 0; i < forceDir_distPairs.size(); i++)
		{
			zPoint v2 = vPositions[i] + (forceDir_distPairs[i] * 0.1);
			model.displayUtils.drawLine(vPositions[i], v2, YELLOW);
		}
	}
	
	
	
	model.draw();


	//////////////////////////////////////////////////////////

	setup2d();
	glColor3f(0, 0, 0);
	
	drawString("planar deviation max:" + to_string(fPlanar_max), vec(winW - 350, winH - 900, 0));
	drawString("planar deviation min:" + to_string(fPlanar_min), vec(winW - 350, winH - 875, 0));

	drawString("b planar deviation max:" + to_string(gPlanar_max), vec(winW - 350, winH - 750, 0));
	drawString("b planar deviation min:" + to_string(gPlanar_min), vec(winW - 350, winH - 725, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("p - compute planar", vec(winW - 350, winH - 625, 0));

	drawString("e - export geometry", vec(winW - 350, winH - 425, 0));


	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computePlanar = !computePlanar;;

	if (k == 'o') computeHeights = !computeHeights;;

	if (k == 'e') exportMESH = true;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
