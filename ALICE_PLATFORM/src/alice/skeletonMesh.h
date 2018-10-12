
#ifndef _SKELETON_MESH_
#define _SKELETON_MESH_




#include "main.h"
#include "ALICE_ROBOT_DLL.h"
using namespace ROBOTICS;
#include <array>
#include <memory>
#include<time.h>
#include<experimental/generator> 
#include "graph.h"
using namespace std;
using namespace std::experimental;

#define MAX_NUM 200000


class SkeletonMesh
{
public:

	vec P[MAX_NUM];
	qh_vertex_t VERTS[100];
	bool removeFace[100];
	//
	Graph G;
	metaMesh combinedMesh;
	Mesh HULL;
	Mesh Prim;
	Mesh Prim_copy;
	int nSides;
	vec *plPts;
	//

	vec u, v, n, cen;
	float Scale[3];
	Matrix4 T;

	//////////////////////////////////////////////////////////////////////////

	SkeletonMesh() 
	{
		initialise();
	}

	SkeletonMesh( string file  )
	{
		G.constructFromFile(file);
		initialise();
	}

	void initialise()
	{
		nSides = 6;
		MeshFactory fac;
		Prim = fac.createPrism(nSides, 1.0, 0, false);// fac.createFromOBJ("data/cube_tri.obj", 1.0, false);
		Prim_copy = fac.createPrism(nSides, 1.0, 0, false);// fac.createFromOBJ("data/cube_tri.obj", 1.0, false);
		Prim.n_f -= 1 * 2;
		Prim_copy.n_f -= 1 * 2;

		//

		for (int i = 0; i < nSides; i++)Prim.positions[i + nSides].z = 1.0;
		for (int i = 0; i < nSides; i++) Prim_copy.positions[i + nSides].z = 1.0;

		vec min, max;
		Prim.boundingBox(min, max);
		for (int i = 0; i < Prim.n_v; i++)Prim.positions[i].z -= 0.5;// (min + max)*0.5;
		for (int i = 0; i < Prim_copy.n_v; i++) Prim_copy.positions[i].z -= 0.5;//(min + max)*0.5;

																				//

		plPts = new vec[nSides];
		for (int i = 0; i < nSides; i++)plPts[i] = Prim.positions[i];
	}
	
	
	//////////////////////////////////////////////////////////////////////////


	void setTransform(vec &u, vec &v, vec &n, vec &cen, Matrix4 &T)
	{
		T.setColumn(0, u);
		T.setColumn(1, v);
		T.setColumn(2, n);
		T.setColumn(3, cen);


	}

	void getEdgeTransform(int &vertexId, int &relativeEdgeId, double &endOffset, double &wid, Matrix4 &T)
	{
		n = G.getEdgeDir(vertexId, relativeEdgeId);
		double len = n.mag();

		u = n.cross(vec(1, 0, 0));
		v = n.cross(u);
		u.normalise(); v.normalise(); n.normalise();

		u *= wid; v *= wid;
		cen = G.positions[vertexId] + n * ofClamp(endOffset + 0.5, 0, len *0.5);//(MIN(endOffset + 0.5 /*not sure why this is needed*/, len * 0.5) )

		setTransform(u, v, n, cen, T);
	}

	int getEdgeTransform( int edgeId, double &endOffset, double &wid,Matrix4 &T)
	{
		n = G.getEdgeDir(edgeId);
		double len = n.mag() - endOffset * 2.0;
		if (len < 0)return 0;

		u = n.cross(vec(1, 0, 0));
		v = n.cross(u);
		cen = G.getEdgeCenter(edgeId);

		Scale[2] = len; //  n.mag() - (endOffset * 2.0);
		Scale[0] = Scale[1] = wid;
		u.normalise(); v.normalise(); n.normalise();
		u *= Scale[0]; v *= Scale[1]; n *= Scale[2];
		//
		//setTransform(u, v, n, cen, T);
		setTransform(u, v, n, cen, T);

		return 1;
	}

	vec getHullFaceNormal( int faceId , Mesh &HULL)
	{
		int *f_v = HULL.faces[faceId].faceVertices();
		vec fn = (HULL.positions[f_v[1]] - HULL.positions[f_v[0]]).cross((HULL.positions[f_v[2]] - HULL.positions[f_v[0]]));
		fn.normalise();
		return fn;
	}

	vec getHullFaceCenter(int faceId, Mesh &HULL)
	{
		int *f_v = HULL.faces[faceId].faceVertices();
		vec cen = HULL.positions[ f_v[1] ] + HULL.positions[ f_v[0] ] + HULL.positions[ f_v[2] ];
		cen /= 3.0;
		return cen;
	}

	int getRemoveFacesFromHull( int vertexId , bool *removeFace , Mesh &HULL )
	{
		int valence = G.vertices[vertexId].n_e;
		vec fn, edgeDir;
		
		int cnt = 0;
		//

		for (int f = 0; f < HULL.n_f; f += 1)
		{
			bool delFace = false;

			fn = getHullFaceNormal(f, HULL);

			for (int i = 0; i < valence; i += 1)
			{
				edgeDir = G.getEdgeDir(vertexId, i).normalise();


				//if ((1.0 - (edgeDir * fn)) < EPS * 10.0 || (1.0 - (edgeDir * (fn * -1))) < EPS * 10.0)
				if( fabs(edgeDir.angle(fn)) < 5 || fabs(edgeDir.angle(fn * -1)) < 5 )
				{
					delFace = true;
					break;
				}
			}

			removeFace[f] = delFace;
			if (delFace)cnt++;
		}

		return cnt;
	}

	void combineMeshes(Mesh &sub, Mesh &parent, bool *removeFaces = NULL )
	{
		int P_nv = parent.n_v;
		for (int i = 0; i < sub.n_v; i++)parent.createVertex(sub.positions[i]);

		Vertex *FV[6];
		for (int i = 0; i < sub.n_f; i++)
		{
			if(removeFaces != NULL )
				if (removeFaces[i]) continue;

			int *f_v = sub.faces[i].faceVertices();

			for (int j = 0; j < sub.faces[i].n_e; j++)
			{
				int id = sub.vertices[f_v[j]].id;
				id += P_nv;
				FV[j] = &parent.vertices[id];
			}

			parent.createFace(FV, sub.faces[i].n_e);
		}

		for (int i = 0; i < parent.n_f; i++)parent.faces[i].faceVertices();
	}

	void meshFromGraph(double endOffset = 0.2, double wid = 0.2, bool edges = true, bool nodes = true, bool remove = true)
	{

		combinedMesh.reset();

		////////////////////////////////////////////////////////////////////////// iterate through vertices

		int num = 0;

		if (nodes)
			for (int vv = 0; vv < G.n_v; vv++)
			{
				//if (vv != 47)continue;
				// prelims
				int valence = G.vertices[vv].n_e;

					if (valence < 2)continue;

				num = nSides * valence; // number of input points to construct a convexHull from;

				// iterate through vertices and construct input points for convex hull by
				// transforming the points of the profile curve/poly along each edge
				int cnt = 0;
				for (int i = 0; i < valence; i += 1)
				{

					getEdgeTransform(vv, i, endOffset, wid, T);

					for (int j = 0; j < nSides; j++)
						if (cnt < MAX_NUM)P[cnt++] = T * (plPts[j]);
				}


				// construct hull from profile points placed on edge of the edge, 
				// at a distance = endOffset from current vertex, alogn the edge;

				HULL.n_v = HULL.n_f = HULL.n_v = 0; // reset mesh;
				quickHull(P, num, VERTS, HULL);

				// remove faces from the hull that are aligned with any of the edges emanating from the current vertex

				int r_n = getRemoveFacesFromHull(vv, removeFace, HULL);
				cout << r_n << " " << vv << endl;


				////////////////////////////////////////////////////////////////////////// combine meshes

				int nv = combinedMesh.n_v;
				remove ? combineMeshes(HULL, combinedMesh, removeFace) : combineMeshes(HULL, combinedMesh);

				////////////////////////////////////////////////////////////////////////// assign scalars for contouring

				for (int o = nv; o < combinedMesh.n_v; o++)
				{
					double val = 1e10;
					vec p = combinedMesh.positions[o];

					// get distance to nearest plane, for each of the newly added vertices..
					vec edgeDir = G.getEdgeDir(vv, 0).normalise();
					cen = G.positions[vv] + (edgeDir * 0.1);
					val = MIN((p - cen) * n, val); // scalar = distance of the current point from the plane centered at current vertex
													  // and with a normal parallel to edge 0 (arbitrary choice) emanating from vertex

					combinedMesh.scalars[o] = val;
				}
			}

		////////////////////////////////////////////////////////////////////////// iterate through edges

		if (edges)
			for (int i = 0; i < G.n_e; i++)
			{
				
				////////////////////////////////////////////////////////////////////////// scale and position primitive along edge

				// this functions returns zero if length of edge is too small w.r.t endOffset ;
				int success = getEdgeTransform(i, endOffset, wid, T);

				//transform
				if(success)
					for (int n = 0; n < Prim.n_v; n++)Prim.positions[n] = T * Prim_copy.positions[n];

				////////////////////////////////////////////////////////////////////////// combine meshes

				int nv = combinedMesh.n_v;
				combineMeshes(Prim, combinedMesh);

				////////////////////////////////////////////////////////////////////////// assign scalars for contouring

				vec edgeDir = G.getEdgeDir(i).normalise();

				for (int o = nv; o < combinedMesh.n_v; o++)
				{
					double val;
					vec p = combinedMesh.positions[o];
					val = (p - cen) * edgeDir;
					combinedMesh.scalars[o] = val;
				}

			}
	}


	//////////////////////////////////////////////////////////////////////////

	void draw()
	{
		combinedMesh.draw(true);
		G.draw();

		/*for (int i = 0; i < HULL.n_f; i++)
		{
			vec fn = getHullFaceNormal(i, HULL);
			vec cen = getHullFaceCenter(i, HULL);

			drawLine(cen, cen + fn);

		}*/
	}
};


#endif // !_SKELETON_MESH_