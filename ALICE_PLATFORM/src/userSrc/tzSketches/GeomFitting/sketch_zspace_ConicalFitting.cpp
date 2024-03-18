//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#define LOG if (1) cout

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool reset = false;
bool toFile = false;

double background = 0.75;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjMesh oMesh_copy;
double tol_planarity = 0.1;
double tol_sphere = 0.1;
double tol_cylinder = 0.1;
double tol_cone = 0.1;
double dT = 0.1;

zPoint dCenter;
////// --- GUI OBJECTS ----------------------------------------------------

class zPanel
{
public:

	zPanel(zObjMesh& _inMesh, const int _centerVertexId)
		: oMesh(_inMesh), centerVertex(oMesh, _centerVertexId)
	{
		fnDyMesh = zFnMeshDynamics(oMesh);
		fnDyMesh.makeDynamic();

		reoder();
		computeDevs();
	}

	void fit(double dT, double tol_planarity, double tol_sphere, double tol_cylinder, double tol_cone, bool checkPlanar = true, bool checkCylinder = true, bool checkCone = true, bool checkSphere = true)
	{
		computeDevs();
		LOG << endl
			<< "edgeDev_main:" << edgeDev_main << ","
			<< "edgeDev_minor:" << edgeDev_minor << ","
			<< "edgeDev_C1:" << edgeDev_C1 << ","
			<< "edgeDev_C2:" << edgeDev_C2 << ","
			<< "panelDev_planarity:" << panelDev_planarity << ","
			<< "panelDev_cylinder:" << panelDev_cylinder << ","
			<< "panelDev_cone:" << panelDev_cone << endl;

		if (panelDev_planarity < tol_planarity && checkPlanar)
		{
			setPanelColor(GREEN);
			fitPlane();
		}
		else if (panelDev_sphere < tol_sphere && checkSphere)
		{
			setPanelColor(ORANGE);
			fitSphere();
		}

		else if (panelDev_cylinder < tol_cylinder && checkCylinder)
		{
			setPanelColor(CYAN);
			fitCylinder();
		}
		else if (checkCone)
		{
			setPanelColor(PINK);
			fitCone();
		}

		fitUpdate(dT);
	}

	double edgeDev_main;
	double edgeDev_minor;
	double edgeDev_C1;
	double edgeDev_C2;
	double panelDev_planarity;
	double panelDev_cylinder;
	double panelDev_cone;
	double panelDev_sphere;
	zPoint sphereCenter;

private:
	zObjMesh& oMesh;
	zFnMeshDynamics fnDyMesh;
	zItMeshVertex centerVertex;
	pair <zItMeshHalfEdge, zItMeshHalfEdge> main_hes;
	pair <zItMeshHalfEdge, zItMeshHalfEdge> minor_hes;
	pair <zItMeshHalfEdge, zItMeshHalfEdge> c1_hes;
	pair <zItMeshHalfEdge, zItMeshHalfEdge> c2_hes;

	zColor RED = zColor(1, 0, 0, 1);
	zColor BLUE = zColor(0, 0, 1, 1);
	zColor GREEN = zColor(0, 1, 0, 1);
	zColor YELLOW = zColor(1, 1, 0, 1);
	zColor PURPLE = zColor(0.5, 0, 0.5, 1);
	zColor CYAN = zColor(0, 1, 1, 1);
	zColor ORANGE = zColor(1, 0.5, 0, 1);
	zColor PINK = zColor(1, 0, 0.5, 1);

	void setPanelColor(zColor& col)
	{
		zFnMesh(oMesh);
		zItMeshFaceArray faces;
		centerVertex.getConnectedFaces(faces);
		for (auto& face : faces)
			face.setColor(col);
	}

	/* FIT METHODS */
	void fitPlane()
	{
		zVector origin, planeZ;
		getBestFitPlane(origin, planeZ);

		zIntArray vIds;
		for (int i = 0; i < fnDyMesh.numVertices(); i++)
		{
			vIds.push_back(i);
		}

		fnDyMesh.addPlaneForce(1.0, vIds,origin, planeZ);

		//zPointArray positions;
		//getVertexPositions(positions);
		//zPlane bestFitPlane = core.getBestFitPlane(positions);
		//zVector origin(bestFitPlane(0, 3), bestFitPlane(1, 3), bestFitPlane(2, 3));
		//zVector planeZ(bestFitPlane(0, 2), bestFitPlane(1, 2), bestFitPlane(2, 2));

		//LOG << "bestFitPlane:" << bestFitPlane << endl;

		//zIntArray vIds;
		//for (int i = 0; i < fnDyMesh.numVertices(); i++)
		//{
		//	vIds.push_back(i);
		//}
		//fnDyMesh.addPlaneForce(1.0, vIds,origin, planeZ);
		//---------------------------------------------------------------------------------------
		//bool exit_planar = false;
		//double mxDev = 0;
		//zVectorArray forceDir;
		//zDoubleArray devs_planarity;
		//double tol_d = 0.01;

		////fnDyMesh.addPlanarityForce(1.0, tol_d, zQuadPlanar, devs_planarity, forceDir, exit_planar);
		//fnDyMesh.addPlanarityForce(1.0, tol_d, zVolumePlanar, devs_planarity, forceDir, exit_planar);

		//for (zItMeshFace f(oMesh); !f.end(); f++)
		//{
		//	if (devs_planarity[f.getId()] < tol_d)
		//		f.setColor(zColor(0, 1, 0, 1));
		//	else f.setColor(zColor(1, 0, 1, 1));
		//}

		//double fPlanar_max = core.zMax(devs_planarity);
		//double fPlanar_min = core.zMin(devs_planarity);
		//printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);
	}
	void fitCylinder()
	{
		float len_c1_a, len_c1_b, len_c2_a, len_c2_b, len_minor_a, len_minor_b;
		float meanLength;
		float restAngle = A_PI;

		//fix c1 & c2 start
		//zIntArray fixedVIds;
		//fixedVIds.push_back(c1_hes.first.getVertex().getId());
		//fixedVIds.push_back(c2_hes.first.getVertex().getId());
		//fnDyMesh.setFixed(fixedVIds);

		//C1 & C2 equal length force
		len_c1_a = c1_hes.first.getLength();
		len_c1_b = c1_hes.second.getLength();
		len_c2_a = c2_hes.first.getLength();
		len_c2_b = c2_hes.second.getLength();
		len_minor_a = minor_hes.first.getLength();
		len_minor_b = minor_hes.second.getLength();
		meanLength = (len_c1_a + len_c1_b + len_c2_a + len_c2_b + len_minor_a + len_minor_b) / 6;
		fnDyMesh.addSpringForce(1.0, c1_hes.first.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, c1_hes.second.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, c2_hes.first.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, c2_hes.second.getEdge().getId(), meanLength);

		//minor equal length force
		fnDyMesh.addSpringForce(1.0, minor_hes.first.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, minor_hes.second.getEdge().getId(), meanLength);

		//main angle force
		//fnDyMesh.addAngleForce(1.0, centerVertex.getId(), main_hes.first.getVertex().getId(), main_hes.second.getVertex().getId(), restAngle);
		//fnDyMesh.addAngleForce(1.0, minor_hes.first.getVertex().getId(), c1_hes.first.getVertex().getId(), c2_hes.first.getVertex().getId(), restAngle);
		//fnDyMesh.addAngleForce(1.0, minor_hes.second.getVertex().getId(), c1_hes.second.getVertex().getId(), c2_hes.second.getVertex().getId(), restAngle);




		zVector c1_vec, c2_vec, minor_vec;
		float average_vecLen;
		c1_vec = c1_hes.first.getVector() + c1_hes.second.getVector();
		c2_vec = c2_hes.first.getVector() + c2_hes.second.getVector();
		minor_vec = minor_hes.first.getVector() + minor_hes.second.getVector();


		zVector average_vec = (c1_vec + c2_vec + minor_vec) / 3;
		//average_vec = zVector(0, 0, 0);

		c1_vec -= average_vec;
		c2_vec -= average_vec;
		minor_vec -= average_vec;

		//main_hes.first.getVertex().setPosition(main_hes.first.getVertex().getPosition() + average_vec);
		//main_hes.first.getVertex().setPosition(main_hes.first.getVertex().getPosition() + average_vec);
		//main_hes.first.getVertex().setPosition(main_hes.first.getVertex().getPosition() + average_vec);

		average_vecLen = (c1_vec + c2_vec + minor_vec).length() / 3;
		fnDyMesh.addLoadForce(0.5 * c1_vec.length(), main_hes.first.getVertex().getId(), c1_vec);
		fnDyMesh.addLoadForce(0.5 * c2_vec.length(), main_hes.second.getVertex().getId(), c2_vec);
		fnDyMesh.addLoadForce(0.5 * minor_vec.length(), centerVertex.getId(), minor_vec);

		cout << endl;
		cout << "c1:" << 0.5 * c1_vec.length() << endl;
		cout << "c2:" << 0.5 * c2_vec.length() << endl;
		cout << "minor:" << 0.5 * minor_vec.length() << endl;


		//plane force
		zIntArray c1_vIds, c2_vIds;
		c1_vIds.push_back(c1_hes.first.getVertex().getId());
		c1_vIds.push_back(c1_hes.second.getVertex().getId());
		c2_vIds.push_back(c2_hes.first.getVertex().getId());
		c2_vIds.push_back(c2_hes.second.getVertex().getId());

		fnDyMesh.addPlaneForce(1.0, c1_vIds, *main_hes.first.getVertex().getRawPosition(), main_hes.first.getVector());
		fnDyMesh.addPlaneForce(1.0, c2_vIds, *main_hes.second.getVertex().getRawPosition(), main_hes.first.getVector());

		//additional forces for equal radius on c1, c2 & minor edge
		float angle_c1 = c1_hes.first.getVector().angle(c1_hes.second.getVector()) * DEG_TO_RAD;
		float angle_c2 = c2_hes.first.getVector().angle(c2_hes.second.getVector()) * DEG_TO_RAD;
		float angle_minor = minor_hes.first.getVector().angle(minor_hes.second.getVector()) * DEG_TO_RAD;
		float angle_mean = (angle_c1 + angle_c2 + angle_minor) / 3;
		
		fnDyMesh.addAngleForce(1.0, main_hes.first.getVertex().getId(), c1_hes.first.getVertex().getId(), c1_hes.second.getVertex().getId(), angle_mean);
		fnDyMesh.addAngleForce(1.0, main_hes.second.getVertex().getId(), c2_hes.first.getVertex().getId(), c2_hes.second.getVertex().getId(), angle_mean);
		fnDyMesh.addAngleForce(1.0, centerVertex.getId(), minor_hes.first.getVertex().getId(), minor_hes.second.getVertex().getId(), angle_mean);

		//zVector biSec_c1 = c1_hes.first.getVector() + c1_hes.second.getVector();
		//zVector biSec_c2 = c2_hes.first.getVector() + c2_hes.second.getVector();
		//zVector biSec_minor = minor_hes.first.getVector() + minor_hes.second.getVector();

		//fnDyMesh.addLoadForce(angle_mean - angle_c1, main_hes.first.getVertex().getId(), biSec_c1);
		//fnDyMesh.addLoadForce(angle_mean - angle_c2, main_hes.second.getVertex().getId(), biSec_c2);
		//fnDyMesh.addLoadForce(angle_mean - angle_minor,centerVertex.getId(), biSec_minor);
	}
	void fitCone()
	{
		float len_first, len_second, meanLength;
		double restAngle = A_PI; //HALF_PI;
		double restAngle_L = HALF_PI;

		//C1 equal length force
		len_first = c1_hes.first.getLength();
		len_second = c1_hes.second.getLength();
		meanLength = 0.5 * (len_first + len_second);
		fnDyMesh.addSpringForce(1.0, c1_hes.first.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, c1_hes.second.getEdge().getId(), meanLength);

		//C2 equal length force
		len_first = c2_hes.first.getLength();
		len_second = c2_hes.second.getLength();
		meanLength = 0.5 * (len_first + len_second);
		fnDyMesh.addSpringForce(1.0, c2_hes.first.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, c2_hes.second.getEdge().getId(), meanLength);

		//minor equal length force
		len_first = minor_hes.first.getLength();
		len_second = minor_hes.second.getLength();
		meanLength = 0.5 * (len_first + len_second);
		fnDyMesh.addSpringForce(1.0, minor_hes.first.getEdge().getId(), meanLength);
		fnDyMesh.addSpringForce(1.0, minor_hes.second.getEdge().getId(), meanLength);

		//main angle force
		fnDyMesh.addAngleForce(1.0, centerVertex.getId(), main_hes.first.getVertex().getId(), main_hes.second.getVertex().getId(), restAngle);
		fnDyMesh.addAngleForce(1.0, minor_hes.first.getVertex().getId(), c1_hes.first.getVertex().getId(), c2_hes.first.getVertex().getId(), restAngle);
		fnDyMesh.addAngleForce(1.0, minor_hes.second.getVertex().getId(), c1_hes.second.getVertex().getId(), c2_hes.second.getVertex().getId(), restAngle);

		//plane force
		zIntArray c1_vIds, c2_vIds;
		c1_vIds.push_back(c1_hes.first.getVertex().getId());
		c1_vIds.push_back(c1_hes.second.getVertex().getId());
		c2_vIds.push_back(c2_hes.first.getVertex().getId());
		c2_vIds.push_back(c2_hes.second.getVertex().getId());

		fnDyMesh.addPlaneForce(1.0, c1_vIds, *main_hes.first.getVertex().getRawPosition(), main_hes.first.getVector());
		fnDyMesh.addPlaneForce(1.0, c2_vIds, *main_hes.second.getVertex().getRawPosition(), main_hes.first.getVector());

		//boundary force
		//zVector v_first = (c1_hes.first.getVertex().getPosition() + c2_hes.first.getVertex().getPosition()) / 2;
		//zVector v_second = (c1_hes.second.getVertex().getPosition() + c2_hes.second.getVertex().getPosition()) / 2;
		//v_first -= minor_hes.first.getVertex().getPosition();
		//v_second -= minor_hes.second.getVertex().getPosition();
		//fnDyMesh.addLoadForce(1.0, minor_hes.first.getVertex().getId(), v_first);
		//fnDyMesh.addLoadForce(1.0, minor_hes.second.getVertex().getId(), v_second);
	}
	void fitSphere()
	{
		//equal distance force
		zIntArray vIds;
		getVertices(vIds, false);

		float meanDist_diagonal = 0;
		for (auto& vId : vIds)
		{
			zItMeshVertex v(oMesh, vId);
			float d = v.getPosition().distanceTo(centerVertex.getPosition());
			meanDist_diagonal += d;
		}
		meanDist_diagonal /= vIds.size();

		for (auto& vId : vIds)
		{
			fnDyMesh.addDistanceForce(1.0, centerVertex.getId(), vId, meanDist_diagonal);
		}

		zIntArray vIds_bot;
		float meanDist_square = 0;

		meanDist_square += c1_hes.first.getVertex().getPosition().distanceTo(c1_hes.second.getVertex().getPosition());
		meanDist_square += c1_hes.second.getVertex().getPosition().distanceTo(c2_hes.second.getVertex().getPosition());
		meanDist_square += c2_hes.second.getVertex().getPosition().distanceTo(c2_hes.first.getVertex().getPosition());
		meanDist_square += c2_hes.first.getVertex().getPosition().distanceTo(c1_hes.first.getVertex().getPosition());

		meanDist_square /= 4;

		fnDyMesh.addDistanceForce(1.0, c1_hes.first.getVertex().getId(),c1_hes.second.getVertex().getId(), meanDist_square);
		fnDyMesh.addDistanceForce(1.0, c1_hes.second.getVertex().getId(), c2_hes.second.getVertex().getId(), meanDist_square);
		fnDyMesh.addDistanceForce(1.0, c2_hes.second.getVertex().getId(), c2_hes.first.getVertex().getId(), meanDist_square);
		fnDyMesh.addDistanceForce(1.0, c2_hes.first.getVertex().getId(), c1_hes.first.getVertex().getId(), meanDist_square);

		//radius
		float radius;
		float angle_main = main_hes.first.getVector().angle(main_hes.second.getVector()) * DEG_TO_RAD;
		float angle_minor = minor_hes.first.getVector().angle(minor_hes.second.getVector()) * DEG_TO_RAD;

		cout << "angle_main:" << angle_main << endl;
		cout << "angle_minor:" << angle_minor << endl;
		float angle_mean = (angle_main + angle_minor) / 2;
		angle_main = angle_main > Z_HALF_PI ? Z_PI - angle_main : angle_main;
		radius = meanDist_square * tanf(0.5 * angle_mean);

		//equal angle force
		fnDyMesh.addAngleForce(1.0, centerVertex.getId(), main_hes.first.getVertex().getId(), main_hes.second.getVertex().getId(), angle_mean, false);
		fnDyMesh.addAngleForce(1.0, centerVertex.getId(), minor_hes.first.getVertex().getId(), minor_hes.second.getVertex().getId(), angle_mean, false);

		//fix angle problem
		zVector check_main = main_hes.first.getVector() + main_hes.second.getVector();
		zVector check_minor = main_hes.first.getVector() + minor_hes.second.getVector();
		zVector normal = centerVertex.getNormal();
		normal.normalize();
		if (check_main * normal > 0 || check_minor * normal > 0)
		{
			float move = meanDist_square * cosf(angle_mean * 0.5);
			centerVertex.setPosition(centerVertex.getPosition() + normal * move * 2);
			//fnDyMesh.addLoadForce(move * 2, centerVertex.getId(), normal);
		}

		//plane force
		zIntArray cluster_bot;

		cluster_bot.push_back(c1_hes.first.getVertex().getId());
		cluster_bot.push_back(c1_hes.second.getVertex().getId());
		cluster_bot.push_back(c2_hes.first.getVertex().getId());
		cluster_bot.push_back(c2_hes.second.getVertex().getId());

		zVector average_bot, average_mid;

		for (auto& id : cluster_bot)
		{
			zItMeshVertex v(oMesh, id);
			average_bot += v.getPosition();
		}
		average_bot /= 4;

		zVector origin, planeZ;
		getBestFitPlane(origin, planeZ,false);
		sphereCenter = origin - planeZ;

		planeZ.normalize();

		fnDyMesh.addPlaneForce(1.0, cluster_bot, average_bot, planeZ);
	}

	void fitSphere_legacy()
	{
		//equal length force
		zDoubleArray lens_edges;
		double meanLength = 0;
		zFloatArray restLengths;
		fnDyMesh.getEdgeLengths(lens_edges);
		for (auto& len : lens_edges)
		{
			meanLength += len;
		}
		meanLength /= fnDyMesh.numEdges();
		restLengths.assign(fnDyMesh.numEdges(), meanLength);
		fnDyMesh.addSpringForce(0.2, restLengths);


		//radius
		float radius;
		float angle_main = main_hes.first.getVector().angle360(main_hes.second.getVector(),minor_hes.first.getVector()) * DEG_TO_RAD;
		float angle_minor = minor_hes.first.getVector().angle360(minor_hes.second.getVector(),main_hes.second.getVector()) * DEG_TO_RAD;
		//float angle_main = main_hes.first.getVector().angle360(main_hes.second.getVector(),minor_hes.first.getVector()) * DEG_TO_RAD;
		//float angle_minor = minor_hes.first.getVector().angle360(minor_hes.second.getVector(), main_hes.second.getVector()) * DEG_TO_RAD;

		cout << "angle_main:" << angle_main << endl;
		cout << "angle_minor:" << angle_minor << endl;
		float angle_mean = (angle_main + angle_minor) / 2;
		angle_main = angle_main > Z_HALF_PI ? Z_PI - angle_main : angle_main;
		angle_main = 3.1;
		radius = meanLength * tanf(0.5 * angle_mean);

		//equal angle force
		fnDyMesh.addAngleForce(1.0, centerVertex.getId(), main_hes.first.getVertex().getId(), main_hes.second.getVertex().getId(), angle_mean, false);
		fnDyMesh.addAngleForce(1.0, centerVertex.getId(), minor_hes.first.getVertex().getId(), minor_hes.second.getVertex().getId(), angle_mean, false);

		//radius force
		zVector origin, planeZ;
		getBestFitPlane(origin, planeZ);
		planeZ.normalize();
		planeZ *= radius;
		origin = centerVertex.getPosition();
		origin -= planeZ;

		sphereCenter = origin;

		zItMeshVertex v(oMesh);
		for (v.begin(); !v.end(); v++)
		{
			zVector vPos = v.getPosition();
			float dist = origin.distanceTo(vPos);
			float moveDist = radius - dist;
			zVector vec = vPos - origin;
			fnDyMesh.addLoadForce(0.1*moveDist, v.getId(), vec);

			cout << "dist:" << dist << endl;
		}
	}

	void fitUpdate(const double dT)
	{
		fnDyMesh.update(dT, zRK4, true, true, true);
		//fnDyMesh.update(dT, zEuler, true, true, true);
	}
	void computeDevs()
	{
		computeEdgeDev_main();
		computeEdgeDev_minor();
		computeEdgeDev_C1();
		computeEdgeDev_C2();

		computeDev_planarity();
		computeDev_cylinder();
		computeDev_cone();
	}

	/* PREPARATION */
	void reoder()
	{
		zIntArray cHes;
		centerVertex.getConnectedVertices(cHes);
		zItMeshHalfEdge heFirst = zItMeshHalfEdge(oMesh, cHes[0]);
		int startId = heFirst.getId();

		cHes.clear();
		do
		{
			cHes.push_back(heFirst.getId());
			heFirst = heFirst.getPrev().getSym();
		} while (heFirst.getId() != startId);

		double dev_u = zItMeshHalfEdge(oMesh, cHes[0]).getVector() * zItMeshHalfEdge(oMesh, cHes[2]).getVector();
		double dev_v = zItMeshHalfEdge(oMesh, cHes[1]).getVector() * zItMeshHalfEdge(oMesh, cHes[3]).getVector();


		// Reorder cHes based on dev_u and dev_v
		if (dev_u >= dev_v)
		{
			std::swap(cHes[0], cHes[1]);
			std::swap(cHes[2], cHes[3]);
		}

		// Update member variables based on reordered cHes
		main_hes.first   = zItMeshHalfEdge(oMesh, cHes[0]);
		main_hes.second  = zItMeshHalfEdge(oMesh, cHes[2]);
		minor_hes.first	 = zItMeshHalfEdge(oMesh, cHes[3]);
		minor_hes.second = zItMeshHalfEdge(oMesh, cHes[1]);

		c1_hes.first  = zItMeshHalfEdge(oMesh, cHes[0]).getNext();
		c1_hes.second = zItMeshHalfEdge(oMesh, cHes[0]).getSym().getPrev().getSym();
		c2_hes.first  = zItMeshHalfEdge(oMesh, cHes[2]).getSym().getPrev().getSym();
		c2_hes.second = zItMeshHalfEdge(oMesh, cHes[2]).getNext();
	}

	void computeEdgeDev_main() 
	{
		edgeDev_main = (main_hes.first.getVector() + main_hes.second.getVector()).length();
	}
	void computeEdgeDev_minor()
	{
		edgeDev_minor = (minor_hes.first.getVector() + minor_hes.second.getVector()).length();
	}
	void computeEdgeDev_C1()
	{
		//simple solution, need to update to check radius dev
		edgeDev_C1 = (c1_hes.first.getVector() + c1_hes.second.getVector()).length();
	}
	void computeEdgeDev_C2()
	{
		//simple solution, need to update to check radius dev
		edgeDev_C2 = (c2_hes.first.getVector() + c2_hes.second.getVector()).length();
	}

	void computeDev_planarity()
	{
		panelDev_planarity = edgeDev_main + edgeDev_minor;
	}
	void computeDev_cylinder()
	{
		panelDev_cylinder = abs(edgeDev_C1 - edgeDev_C2);
	}
	void computeDev_cone()
	{
		panelDev_cone = abs(edgeDev_C1 - edgeDev_C2);
	}
	void computeDev_sphere()
	{
		panelDev_sphere = abs(edgeDev_main - edgeDev_minor);
	}

	void getVertices(zIntArray& vertices, bool includeCenter = true)
	{
		vertices.clear();

		if (includeCenter)
			vertices.push_back(centerVertex.getId());

		vertices.push_back(main_hes.first.getVertex().getId());
		vertices.push_back(main_hes.second.getVertex().getId());
		
		vertices.push_back(minor_hes.first.getVertex().getId());
		vertices.push_back(minor_hes.second.getVertex().getId());
		
		vertices.push_back(c1_hes.first.getVertex().getId());
		vertices.push_back(c1_hes.second.getVertex().getId());
		
		vertices.push_back(c2_hes.first.getVertex().getId());
		vertices.push_back(c2_hes.second.getVertex().getId());
	}

	void getVertexPositions(zPointArray& positions, bool includeCenter = true)
	{
		positions.clear();

		if(includeCenter)
			positions.push_back(centerVertex.getPosition());

		positions.push_back(main_hes.first.getVertex().getPosition());
		positions.push_back(main_hes.second.getVertex().getPosition());

		positions.push_back(minor_hes.first.getVertex().getPosition());
		positions.push_back(minor_hes.second.getVertex().getPosition());

		positions.push_back(c1_hes.first.getVertex().getPosition());
		positions.push_back(c1_hes.second.getVertex().getPosition());

		positions.push_back(c2_hes.first.getVertex().getPosition());
		positions.push_back(c2_hes.second.getVertex().getPosition());
	}

	void getVertexNormals(zVectorArray& normals, bool includeCenter = true)
	{
		normals.clear();

		if(includeCenter)
			normals.push_back(centerVertex.getNormal());

		normals.push_back(main_hes.first.getVertex().getNormal());
		normals.push_back(main_hes.second.getVertex().getNormal());

		normals.push_back(minor_hes.first.getVertex().getNormal());
		normals.push_back(minor_hes.second.getVertex().getNormal());

		normals.push_back(c1_hes.first.getVertex().getNormal());
		normals.push_back(c1_hes.second.getVertex().getNormal());

		normals.push_back(c2_hes.first.getVertex().getNormal());
		normals.push_back(c2_hes.second.getVertex().getNormal());
	}

	void getBestFitPlane(zPoint& origin, zVector& normal, bool includeCenter = true)
	{
		zPointArray positions;
		zVectorArray normals;
		getVertexPositions(positions, includeCenter);
		getVertexNormals(normals, includeCenter);

		for (int i = 0; i < positions.size(); i++)
		{
			origin += positions[i];
			normal += normals[i];
		}
		origin /= positions.size();
		normal /= normals.size();
		normal.normalize();
	}

	void getBestFitPlane(zItMeshVertexArray& vertices, zPoint& origin, zVector& normal)
	{
		zPointArray positions;
		zVectorArray normals;

		for (auto& v : vertices)
		{
			positions.push_back(v.getPosition());
			normals.push_back(v.getNormal());
		}

		for (int i = 0; i < positions.size(); i++)
		{
			origin += positions[i];
			normal += normals[i];
		}
		origin /= positions.size();
		normal /= normals.size();
		normal.normalize();
	}
};

vector<zPanel> panels;


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
	zFnMesh fnMesh(oMesh);
	//fnMesh.from("data/geomFitting/inMesh.json", zJSON);
	fnMesh.from("data/geomFitting/geomFitting_2.json", zJSON);
	oMesh_copy = oMesh;

	model.addObject(oMesh);
	oMesh.setDisplayElements(true, true, true);

	zItMeshVertex v(oMesh);
	for (v.begin(); !v.end(); v++)
	{
		if (v.checkValency(4))
			panels.push_back(zPanel(oMesh, v.getId()));
	}

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object


	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&tol_planarity, "tol_planarity");
	S.sliders[1].attachToVariable(&tol_planarity, 0, 1);
	S.addSlider(&tol_sphere, "tol_sphere");
	S.sliders[2].attachToVariable(&tol_sphere, 0, 1);
	S.addSlider(&tol_cylinder, "tol_cylinder");
	S.sliders[3].attachToVariable(&tol_cylinder, 0, 1);
	S.addSlider(&tol_cone, "tol_cone");
	S.sliders[4].attachToVariable(&tol_cone, 0, 1);
	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&reset, "reset");
	B.buttons[2].attachToVariable(&reset);

}

void update(int value)
{
	if (compute)
	{
		for (auto& panel : panels)
		{
			panel.fit(dT, tol_planarity, tol_sphere, tol_cylinder, tol_cone, true, true, true, true);
			dCenter = panel.sphereCenter;
		}

	}

	if (reset)
	{
		oMesh = oMesh_copy;
		reset = false;
	}

	if (toFile)
	{

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
		
		zFnMesh fn(oMesh);
		zPoint* vPositions = fn.getRawVertexPositions();

		for (int i = 0; i < fn.numVertices(); i++) {

				model.displayUtils.drawLine(vPositions[i], dCenter, zColor(1, 0, 0, 1), 2);
		}
		model.displayUtils.drawPoint(dCenter, zColor(1, 0, 0, 1), 5);

	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;
	if (k == '1')
	{
		//float z, rx, ry, tx, ty;
		//getCamera(z, rx, ry, tx, ty);
		//cout << endl;
		//cout << z << endl;
		//cout << rx << endl;
		//cout << ry << endl;
		//cout << tx << endl;
		//cout << ty << endl;

		setCamera(1.5, -38.7, -45.899, 0.2, 0.4);
	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
