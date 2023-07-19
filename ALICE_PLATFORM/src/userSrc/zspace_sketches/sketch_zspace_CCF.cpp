//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include <igl/avg_edge_length.h>
#include <igl/cotmatrix.h>
#include <igl/invert_diag.h>
#include <igl/massmatrix.h>
#include <igl/parula.h>
#include <igl/per_corner_normals.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>
#include <igl/principal_curvature.h>
#include <igl/gaussian_curvature.h>
#include <igl/read_triangle_mesh.h>

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

void computeConnectionPlanes(zObjMesh& o_mesh,zObjMesh oMesh_inplanes,vector<zIntArray> &outVIDs, zPointArray& outOrigins, zPointArray& outNormals)
{
	zFnMesh fnMesh(o_mesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();

	outVIDs.clear();
	outOrigins.clear();
	outNormals.clear();

	zBoolArray vVisited;
	vVisited.assign(fnMesh.numVertices(), false);


	for (zItMeshFace fPlanes(oMesh_inplanes); !fPlanes.end(); fPlanes++)
	{
		int checkCounter = 0;

		zIntArray vIDs;

		zPointArray fVPositions;
		fPlanes.getVertexPositions(fVPositions);

		for (int j = 0; j < fVPositions.size(); j++)
		{
			for (int i = 0; i < fnMesh.numVertices(); i++)
			{
				if (vPositions[i].distanceTo(fVPositions[j]) < 0.001)
				{
					checkCounter++;
					vIDs.push_back(i);
				}
			}
		}

		if (checkCounter == fVPositions.size())
		{
			outOrigins.push_back(fPlanes.getCenter());
			outNormals.push_back(fPlanes.getNormal());

			outVIDs.push_back(vIDs);
		}
		
	}

	//for (zItMeshVertex v(o_mesh); !v.end(); v++)
	//{
	//	zColor vCOl = v.getColor();
	//	printf("\n %s | %1.2f %1.2f %1.2f ", (v.onBoundary()) ? "T" : "F", vCOl.r, vCOl.g, vCOl.b);

	//	if (v.onBoundary() && v.getColor() == GREEN && !vVisited[v.getId()])
	//	{
	//		zItMeshHalfEdgeArray cHEdges;
	//		v.getConnectedHalfEdges(cHEdges);

	//		zItMeshHalfEdge he;
	//		for (int i = 0; i < cHEdges.size(); i++)
	//		{
	//			if (!cHEdges[i].getEdge().onBoundary()) he = cHEdges[i];				
	//		}

	//		bool exit = false;

	//		zPointArray greenPoints;
	//		zIntArray vIDs;

	//		do
	//		{
	//			exit = (he.getVertex().onBoundary()) ? true : false;

	//			vIDs.push_back(he.getStartVertex().getId());
	//			vVisited[he.getStartVertex().getId()] = true;
	//			greenPoints.push_back(vPositions[he.getStartVertex().getId()]);

	//			if (exit)
	//			{
	//				vIDs.push_back(he.getVertex().getId());
	//				vVisited[he.getVertex().getId()] = true;
	//				greenPoints.push_back(vPositions[he.getVertex().getId()]);
	//			}
	//			else he = he.getNext().getSym().getNext();

	//		} while (!exit);

	//		
	//		zPlane p = core.getBestFitPlane(greenPoints);
	//		
	//		
	//	
	//		zPoint O(p(0, 3), p(1, 3), p(2, 3));
	//		zPoint Z(p(0, 2), p(1, 2), p(2, 2));

	//		printf("\n vIDs %i ", vIDs.size());
	//		cout << "\n origin " << O;
	//		cout << "\n normal " << Z;

	//		/*outOrigins.push_back(O);
	//		outNormals.push_back(Z);

	//		outVIDs.push_back(vIDs);*/

	//		
	//	}
	//}

	printf("\n Plane constraints %i %i ", outOrigins.size(), outNormals.size());

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

void computeDualGraph_BST(zObjMesh& o_mesh, zObjGraph& o_graph, zItGraphVertexArray &bsf_Vertices, zIntPairArray& bsf_vertexPairs)
{
	zFnMesh fnMesh(o_mesh);

	zIntArray inEdge_dualEdge;
	zIntArray dualEdge_inEdge;
	fnMesh.getDualGraph(o_graph, inEdge_dualEdge, dualEdge_inEdge, true, false, false);

	zFnGraph fnGraph(o_graph);
	fnGraph.setEdgeColor(zColor(1, 1, 0, 1));

	zItGraphVertex v_MaxValence;
	int maxValence = 0;;

	for (zItGraphVertex v(o_graph); !v.end(); v++)
	{
		if (v.getValence() > maxValence)
		{
			v_MaxValence = v;
			maxValence = v.getValence();
		}
	}

	maxValence += 1;

	// breadth search first sorting
	
	v_MaxValence.getBSF(bsf_Vertices, bsf_vertexPairs);


}

void creatUnrollMesh(zObjMesh& o_mesh, zObjMesh& o_mesh_unroll, zObjGraph& o_dualgraph, zInt2DArray& oriVertex_UnrollVertex_map, unordered_map<zIntPair, int, zPair_hash>& oriFaceVertex_UnrollVertex, zItGraphVertexArray& bsf_Vertices, zIntPairArray& bsf_vertexPairs)
{
	zFnMesh fnMesh(o_mesh);	

	computeDualGraph_BST(o_mesh, o_dualgraph, bsf_Vertices, bsf_vertexPairs);

	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zPointArray positions;
	zIntArray pConnects, pCounts;

	oriVertex_UnrollVertex_map.clear();
	oriVertex_UnrollVertex_map.assign(fnMesh.numVertices(), zIntArray());

	for (zItMeshFace f(o_mesh); !f.end(); f++)
	{
		zIntArray fVerts;
		f.getVertices(fVerts);

		for (auto fV : fVerts)
		{
			int numVerts = positions.size();
			
			pConnects.push_back(numVerts);
			oriVertex_UnrollVertex_map[fV].push_back(numVerts);

			zIntPair hashKey(f.getId(),fV);
			oriFaceVertex_UnrollVertex[hashKey] = numVerts;

			positions.push_back(vPositions[fV]);
		}

		pCounts.push_back(fVerts.size());
	}


	zFnMesh fnMesh_unroll(o_mesh_unroll);
	fnMesh_unroll.create(positions, pCounts, pConnects);

}

zIntPair getCommonEdge(zItMeshFace& f1, zItMeshFace& f2)
{
	zIntPair out;
	
	zItMeshHalfEdgeArray f1_HEdges;
	f1.getHalfEdges(f1_HEdges);

	zItMeshHalfEdgeArray f2_HEdges;
	f2.getHalfEdges(f2_HEdges);
	

	for (auto& f1HE : f1_HEdges)
	{
		for (auto& f2HE : f2_HEdges)
		{
			if (f1HE.getEdge().getId() == f2HE.getEdge().getId())
			{
				out = zIntPair(f1HE.getId(), f2HE.getId());
				break;
			}
		}
	}

	return out;
}

void unroll_weightEdge(float tolerance, zObjMesh& o_mesh_unroll, zInt2DArray& oriVertex_UnrollVertex_map)
{
	zFnMesh fnMesh_unroll(o_mesh_unroll);
	zPoint* vPositions_unroll = fnMesh_unroll.getRawVertexPositions();
	zColor* vColors_unroll = fnMesh_unroll.getRawVertexColors();

	for (int i = 0; i < oriVertex_UnrollVertex_map.size(); i++)
	{
		zPoint p = vPositions_unroll[oriVertex_UnrollVertex_map[i][0]];

		zPoint avg = p;

		float maxDist = 0;
		for (int j = 1; j < oriVertex_UnrollVertex_map[i].size(); j++)
		{
			float tempDist = p.distanceTo(vPositions_unroll[oriVertex_UnrollVertex_map[i][j]]);
			if (tempDist > maxDist) maxDist = tempDist;
			
			avg += vPositions_unroll[oriVertex_UnrollVertex_map[i][j]];
		}

		avg /= oriVertex_UnrollVertex_map[i].size();

		for (int j = 0; j < oriVertex_UnrollVertex_map[i].size(); j++)
		{

			if (maxDist < tolerance)
			{
				vPositions_unroll[oriVertex_UnrollVertex_map[i][j]] = (avg);
				vColors_unroll[oriVertex_UnrollVertex_map[i][j]] = (BLUE);
			}
			else vColors_unroll[oriVertex_UnrollVertex_map[i][j]] = (RED);

		}			
		
	}

	for (zItMeshEdge e(o_mesh_unroll); !e.end(); e++)
	{
		zIntArray eVerts;
		e.getVertices(eVerts);

		if (vColors_unroll[eVerts[0]] == vColors_unroll[eVerts[1]]) e.setWeight(1.0);
		else e.setWeight(3.0);
	}

}

void unrollMesh(zObjMesh& o_mesh, zObjMesh& o_mesh_unroll, zObjGraph& o_dualgraph, zInt2DArray& oriVertex_UnrollVertex_map, unordered_map<zIntPair, int, zPair_hash>& oriFaceVertex_UnrollVertex, zIntPairArray& bsf_vertexPairs)
{
	zFnMesh fnMesh(o_mesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();

	zFnMesh fnMesh_unroll(o_mesh_unroll);
	zPoint* vPositions_unroll = fnMesh_unroll.getRawVertexPositions();

	/*for (int i = 0; i < oriVertex_UnrollVertex_map.size(); i++)
	{
		zPoint p = vPositions[i];

		for (auto vID : oriVertex_UnrollVertex_map[i])
		{
			vPositions_unroll[vID] = p;
		}
	}*/

	// unroll 
	// https://computergraphics.stackexchange.com/questions/8774/unfold-a-3d-mesh-to-a-2d-plane

	for (int i = 0; i < bsf_vertexPairs.size(); i++)
	{
		zItMeshFace f1(o_mesh, bsf_vertexPairs[i].first);
		zItMeshFace f2(o_mesh, bsf_vertexPairs[i].second);

		zItMeshFace f1_unroll(o_mesh_unroll, bsf_vertexPairs[i].first);
		zItMeshFace f2_unroll(o_mesh_unroll, bsf_vertexPairs[i].second);

		zIntPair hePair = getCommonEdge(f1, f2);

		zItMeshHalfEdge he_1(o_mesh, hePair.first);
		zItMeshHalfEdge he_2(o_mesh, hePair.second);

		zPoint A = vPositions[he_2.getStartVertex().getId()];
		zPoint B = vPositions[he_2.getVertex().getId()];

		// unroll first face
		if (i == 0)
		{
			zItMeshHalfEdge he_walker_1 = he_1;
			int f1_numV = f1.getNumVertices();

			float l_ab = he_1.getLength();

			zPoint a(2,0,0);
			zPoint b(2, l_ab, 0);

			// update  postions of corresponding a & b in unroll mesh
			zIntPair hashKey_a(f1.getId(), he_1.getStartVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_a = oriFaceVertex_UnrollVertex.find(hashKey_a);
			if (got_a != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_a->second] = a;
			
			zIntPair hashKey_b(f1.getId(), he_1.getVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_b = oriFaceVertex_UnrollVertex.find(hashKey_b);
			if (got_b != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_b->second] = b;

			for (int j = 0; j < f1_numV; j++)
			{
				he_walker_1 = he_walker_1.getNext();
				zPoint C = vPositions[he_walker_1.getVertex().getId()];

				zVector ca = C - A;
				zVector ba = B - A;;

				float s = ((ba ^ ca).length()) / (l_ab * l_ab);
				float c = (ba * ca) / (l_ab * l_ab);

				// alternate point
				/*zPoint c1;
				c1.x = a.x + c * (b.x - a.x) - s * (b.y - a.y);
				c1.y = a.y + c * (b.y - a.y) + s * (b.x - a.x);
				c1.z = 0;*/

				zPoint c1;
				c1.x = a.x + c * (b.x - a.x) + s * (b.y - a.y);
				c1.y = a.y + c * (b.y - a.y) - s * (b.x - a.x);
				c1.z = 0;

				// update  postions of corresponding a & b in unroll mesh
				zIntPair hashKey_c(f1.getId(), he_walker_1.getVertex().getId());
				std::unordered_map<zIntPair, int>::const_iterator got_c = oriFaceVertex_UnrollVertex.find(hashKey_c);
				if (got_c != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_c->second] = c1;
			}

		}

		
		
		

		zItMeshHalfEdge he_walker_2 = he_2;
		int f2_numV = f2.getNumVertices();

		// get positions of the prev edge unrolled.
		zIntPair hashKey_a_prev(f1.getId(), he_2.getStartVertex().getId());
		std::unordered_map<zIntPair, int>::const_iterator got_a_prev = oriFaceVertex_UnrollVertex.find(hashKey_a_prev);
		zPoint a = vPositions_unroll[got_a_prev->second];

		zIntPair hashKey_b_prev(f1.getId(), he_2.getVertex().getId());
		std::unordered_map<zIntPair, int>::const_iterator got_b_prev = oriFaceVertex_UnrollVertex.find(hashKey_b_prev);
		zPoint b = vPositions_unroll[got_b_prev->second];
		
		// update  postions of corresponding a & b in unroll mesh
		zIntPair hashKey_a(f2.getId(), he_2.getStartVertex().getId());
		std::unordered_map<zIntPair, int>::const_iterator got_a = oriFaceVertex_UnrollVertex.find(hashKey_a);
		if (got_a != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_a->second] = a;

		zIntPair hashKey_b(f2.getId(), he_2.getVertex().getId());
		std::unordered_map<zIntPair, int>::const_iterator got_b = oriFaceVertex_UnrollVertex.find(hashKey_b);
		if (got_b != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_b->second] = b;


		for (int j = 0; j < f2_numV; j++)
		{
			he_walker_2 = he_walker_2.getNext();
			zPoint C = vPositions[he_walker_2.getVertex().getId()];

			float l_ab = he_2.getLength();

			zVector ca = C - A;
			zVector ba = B - A;;

			float s = ((ba ^ ca).length()) / (l_ab * l_ab);
			float c = (ba * ca) / (l_ab * l_ab);

			zPoint c1;
			c1.x = a.x + c * (b.x - a.x) - s * (b.y - a.y);
			c1.y = a.y + c * (b.y - a.y) + s * (b.x - a.x);
			c1.z = 0;

			// update  postions of corresponding a & b in unroll mesh
			zIntPair hashKey_c(f2.getId(), he_walker_2.getVertex().getId());
			std::unordered_map<zIntPair, int>::const_iterator got_c = oriFaceVertex_UnrollVertex.find(hashKey_c);
			if (got_c != oriFaceVertex_UnrollVertex.end()) vPositions_unroll[got_c->second] = c1;

		}

		
	}

}

////////////////////////////////////////////////////////////////////////// General

bool computePlanar = false;
bool computeDev = false;
bool computeCCF = false;
bool exportMESH = false;
bool unroll = false;

bool d_inMesh = true;
bool d_ccfMesh = true;
bool d_unrollMesh = true;
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

zObjGraph o_dualGraph;

zInt2DArray oriVertex_UnrollVertex_map;
unordered_map<zIntPair, int, zPair_hash> oriFaceVertex_UnrollVertex;
zItGraphVertexArray bsf_Vertices;
zIntPairArray bsf_vertexPairs;

zIntPairArray vertexAlignPairs;

int bsfcurrentID = 0;
int num_BSF = 0;

zFnMeshDynamics fnDyMesh;

zVectorArray fNorms;
zPointArray fCens;

double tol_dev = 0.0001;
double tol_planar = 0.0001;
double tol_panelBoundaryplanar = 0.001;
double tol_unroll = 0.0001;

double strength_planar = 0.5;
double strength_panelBoundaryplanar = 0.3;
double strength_dev = 0.1;

float fPlanar_max, fPlanar_min;
float vGauss_max, vGauss_min;
float bPlanar_max, bPlanar_min;

zDoubleArray creaseData;
string creaseAttrib = "EdgeCreaseData";

float dT = 0.1;

zVectorArray forceDir_dev;
zVectorArray forceDir_planar;
zVectorArray forceDir_panelBoundaryplanar;

vector<zIntArray> panelBoundaryplanar_vGroups;
zPointArray panelBoundaryplanar_Origins;
zVectorArray panelBoundaryplanar_Normals;

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

	string fileName = "ccf_corner";
	cout << "Please enter geometry file name: ";
	cin >> fileName;

	// read mesh

	string path = "data/pSolver/" + fileName + ".json";
	//string path = "data/pSolver/DF_Y_shape.obj";

	zFnMesh fnMesh_inPlanes(oMesh_inplanes);
	fnMesh_inPlanes.from("data/pSolver/panelBoundary_planes.json", zJSON);

	zFnMesh fnMesh_in(oMesh_in);
	fnMesh_in.from(path, zJSON);
	fnMesh_in.setEdgeColor(zColor(0.5, 0.5, 0.5, 1));

	zFnMesh fnMesh(oMesh);
	fnMesh.from(path, zJSON);
	
	json j;
	bool chk = core.readJSON(path, j);

	
	core.readJSONAttribute(j, creaseAttrib, creaseData);

	fnDyMesh.create(oMesh, false);

	fnDyMesh.getCenters(zHEData::zFaceData, fCens);
	fnDyMesh.getFaceNormals(fNorms);

	zIntArray fixedVerts;
	for (zItMeshVertex v(oMesh_in); !v.end(); v++)
	{
		if (v.getColor() == RED) fixedVerts.push_back(v.getId());
	}
	fnDyMesh.setFixed(fixedVerts);

	printf("\n fixedVerts %i ", fixedVerts.size());


	computeConnectionPlanes(oMesh_in, oMesh_inplanes, panelBoundaryplanar_vGroups,panelBoundaryplanar_Origins, panelBoundaryplanar_Normals);

	computeAlignPairs(oMesh_in, vertexAlignPairs);

	creatUnrollMesh(oMesh, oMesh_unroll, o_dualGraph, oriVertex_UnrollVertex_map, oriFaceVertex_UnrollVertex, bsf_Vertices, bsf_vertexPairs);
	num_BSF = bsf_Vertices.size();

	unrollMesh(oMesh, oMesh_unroll, o_dualGraph, oriVertex_UnrollVertex_map, oriFaceVertex_UnrollVertex, bsf_vertexPairs);
	unroll_weightEdge(tol_unroll, oMesh_unroll, oriVertex_UnrollVertex_map);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	model.addObject(oMesh_in);
	model.addObject(oMesh_unroll);
	//model.addObject(o_dualGraph);

	// set display element booleans
	//zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	//o_paramMesh->setDisplayElements(false, true, false);

	oMesh.setDisplayElements(true, true, true);
	oMesh_in.setDisplayElements(false, true, false);
	oMesh_unroll.setDisplayElements(false, true, false);

	//o_dualGraph.setDisplayElements(false, true);
	//o_dualGraph.setDisplayElementIds(true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&strength_planar, "str_planar");
	S.sliders[1].attachToVariable(&strength_planar, 0, 1);

	S.addSlider(&strength_dev, "str_dev");
	S.sliders[2].attachToVariable(&strength_dev, 0, 1);

	//S.addSlider(&strength_panelBoundaryplanar, "str_bplanar");
	//S.sliders[3].attachToVariable(&strength_panelBoundaryplanar, 0, 1);

	S.addSlider(&tol_unroll, "tol_unroll");
	S.sliders[3].attachToVariable(&tol_unroll, 0.000001, 0.01);

	S.addSlider(&tol_planar, "tol_planar");
	S.sliders[4].attachToVariable(&tol_planar, 0.0001, 0.01);

	S.addSlider(&tol_dev, "tol_dev");
	S.sliders[5].attachToVariable(&tol_dev, 0.0001, 0.01);



	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	
	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[0].attachToVariable(&d_inMesh);

	B.addButton(&d_ccfMesh, "d_ccfMesh");
	B.buttons[1].attachToVariable(&d_ccfMesh);

	B.addButton(&d_forces, "d_forces");
	B.buttons[2].attachToVariable(&d_forces);
	
	B.addButton(&d_unrollMesh, "d_unrollMesh");
	B.buttons[3].attachToVariable(&d_unrollMesh);

}

void update(int value)
{
	oMesh_in.setDisplayObject(d_inMesh);
	oMesh.setDisplayObject(d_ccfMesh);
	oMesh_unroll.setDisplayObject(d_unrollMesh);

	if (computePlanar)
	{
				
		zDoubleArray devs_planarity;
		bool exit_planar;
			
		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addPlanarityForce(strength_planar, tol_planar, zVolumePlanar, devs_planarity, forceDir_planar, exit_planar);

			//fnDyMesh.addPlanarityForce_targetPlane(1.0,fCens, fNorms, tol_planar, devs_planarity, forceDir_planar, exit_planar);

			fnDyMesh.update(dT, zRK4, true, true, true);
			
		}
		
		// Planar deviations
		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_planar) f.setColor(zColor(0, 0.95, 0.27, 1));
			else f.setColor(zColor(0.95, 0, 0.55, 1));
		}

		fPlanar_max = core.zMax(devs_planarity);
		fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);
		
		unroll = true;
		//computePlanar = !computePlanar;
	}

	if (computeDev)
	{

		bool exit_gaussian;
		zDoubleArray devs_gaussian;		

		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addDevelopabilityForce(strength_dev, tol_dev, devs_gaussian, forceDir_dev, exit_gaussian);
			fnDyMesh.update(dT, zRK4, true, true, true);
			
		}
		
		// Gaussian deviations
		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			if (devs_gaussian[v.getId()] < tol_dev) v.setColor(zColor(0, 0, 1, 1));
			else v.setColor(zColor(1, 0, 0, 1));

			//if (!v.onBoundary())printf("\n %i %1.6f ", v.getId(), devs_gaussian[v.getId()]);
		}

		vGauss_max = core.zMax(devs_gaussian);
		vGauss_min = core.zMin(devs_gaussian);
		printf("\n gauss devs : %1.6f %1.6f \n", vGauss_max, vGauss_min);
	
		unroll = true;
		//computeDev = !computeDev;
	}

	if (computeCCF)
	{
		bool exit_planar;
		zDoubleArray devs_planarity;

		bool exit_gaussian;
		zDoubleArray devs_gaussian;

		bool exit_panelBoundaryplanar;
		zDoubleArray devs_panelBoundaryplanar;

		bool exit_align;
		zDoubleArray devs_align;
		zVectorArray forceDir_align;

		for (int i = 0; i < numIterations; i++)
		{
			fnDyMesh.addPlanarityForce(strength_planar, tol_planar, zVolumePlanar, devs_planarity, forceDir_planar, exit_planar);
			fnDyMesh.addDevelopabilityForce(strength_dev, tol_dev, devs_gaussian, forceDir_dev, exit_gaussian);

			//fnDyMesh.addPlanarityForce_vertexgroups(strength_panelBoundaryplanar, tol_panelBoundaryplanar, panelBoundaryplanar_vGroups, panelBoundaryplanar_Origins, panelBoundaryplanar_Normals, devs_panelBoundaryplanar, forceDir_panelBoundaryplanar, exit_panelBoundaryplanar);

			if (vertexAlignPairs.size() > 0)
			{

				fnDyMesh.addRigidLineForce(0.5, 0.001, vertexAlignPairs, devs_align, forceDir_align, exit_align);
			}

			fnDyMesh.update(dT, zRK4, true, true, true);
		}

		// Planar deviations
		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_planar) f.setColor(zColor(0, 0.95, 0.27, 1));
			else f.setColor(zColor(0.95, 0, 0.55, 1));
		}

		
		fPlanar_max = core.zMax(devs_planarity);
		fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f", fPlanar_max, fPlanar_min);

		// Gaussian deviations
		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			if (devs_gaussian[v.getId()] < tol_dev) v.setColor(zColor(0, 0, 1, 1));
			else v.setColor(zColor(1, 0, 0, 1));
		}

		vGauss_max = core.zMax(devs_gaussian);
		vGauss_min = core.zMin(devs_gaussian);
		printf("\n gauss devs : %1.6f %1.6f", vGauss_max, vGauss_min);

		// vertex group planar deviations
		
		/*bPlanar_max = core.zMax(devs_panelBoundaryplanar);
		bPlanar_min = core.zMin(devs_panelBoundaryplanar);
		printf("\n pBoundary devs : %1.6f %1.6f \n", bPlanar_max, bPlanar_max);*/

		if(exit_planar && exit_gaussian) printf("\n CCF ACHIEVED!! ");

		unroll = true;
		//computeCCF = !computeCCF;
	}

	if (unroll)
	{
		unrollMesh(oMesh, oMesh_unroll, o_dualGraph, oriVertex_UnrollVertex_map, oriFaceVertex_UnrollVertex, bsf_vertexPairs);
		unroll_weightEdge(tol_unroll, oMesh_unroll, oriVertex_UnrollVertex_map);
	}

	if (exportMESH)
	{			
		fnDyMesh.setVertexColor(BLUE);

		for (int i = 0; i < creaseData.size(); i++)
		{
			zItMeshEdge e(oMesh, i);
			if (creaseData[i] > 0)
			{
				if (e.onBoundary())
				{
					zItMeshVertexArray eVerts;
					e.getVertices(eVerts);

					eVerts[0].setColor(RED);
					eVerts[1].setColor(RED);
				}
				e.setColor(RED);
			}
			else e.setColor(BLUE);
		}

		string fPath_unroll = "data/pSolver/ccf_out_unroll.json";
		zFnMesh fnMesh_unroll(oMesh_unroll);
		fnMesh_unroll.to(fPath_unroll, zJSON);

		string fPath = "data/pSolver/ccf_out.json";
		fnDyMesh.to(fPath, zJSON);

		json j;
		bool chk = core.readJSON(fPath, j);

		j[creaseAttrib] = creaseData;

		// EXPORT	
		ofstream myfile;
		myfile.open(fPath.c_str());

		if (myfile.fail())
		{
			cout << " error in opening file  " << fPath.c_str() << endl;
			return;
		}

		//myfile.precision(16);
		myfile << j.dump();
		myfile.close();


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

		for (int i = 0; i < forceDir_dev.size(); i++)
		{
			zPoint v2 = vPositions[i] + (forceDir_dev[i] * 0.1);
			model.displayUtils.drawLine(vPositions[i], v2, RED);
		}

		for (int i = 0; i < forceDir_panelBoundaryplanar.size(); i++)
		{
			zPoint v2 = vPositions[i] + (forceDir_panelBoundaryplanar[i] * 0.1);
			model.displayUtils.drawLine(vPositions[i], v2, GREEN);
		}
	}
	
	/*zFnGraph fnGraph(o_dualGraph);
	zPoint* graph_vPositions = fnGraph.getRawVertexPositions();
	model.displayUtils.drawPoint(graph_vPositions[bsf_sequence[0].getId()], zColor(1, 0, 1, 1), 4);;
	model.displayUtils.drawPoint(graph_vPositions[bsf_sequence[bsfcurrentID].getId()], zColor(1, 1, 0, 1), 4);;*/
	
	
	model.draw();


	//////////////////////////////////////////////////////////

	setup2d();
	glColor3f(0, 0, 0);
	
	drawString("planar deviation max:" + to_string(fPlanar_max), vec(winW - 350, winH - 900, 0));
	drawString("planar deviation min:" + to_string(fPlanar_min), vec(winW - 350, winH - 875, 0));

	drawString("gaussian deviation max:" + to_string(vGauss_max), vec(winW - 350, winH - 825, 0));
	drawString("gaussian deviation min:" + to_string(vGauss_min), vec(winW - 350, winH - 800, 0));


	drawString("b planar deviation max:" + to_string(bPlanar_max), vec(winW - 350, winH - 750, 0));
	drawString("b planar deviation min:" + to_string(bPlanar_min), vec(winW - 350, winH - 725, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("p - compute planar", vec(winW - 350, winH - 625, 0));
	drawString("d - compute developability", vec(winW - 350, winH - 600, 0));
	drawString("c - compute ccf", vec(winW - 350, winH - 575, 0));


	drawString("e - export geometry", vec(winW - 350, winH - 425, 0));


	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computePlanar = !computePlanar;;

	if (k == 'd') computeDev = !computeDev;;

	if (k == 'c') computeCCF = !computeCCF;

	if (k == 'e') exportMESH = true;

	if (k == 'w')
	{
		if (bsfcurrentID < num_BSF) bsfcurrentID++;	
	}

	if (k == 's')
	{
		if (bsfcurrentID > 0) bsfcurrentID--;
	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
