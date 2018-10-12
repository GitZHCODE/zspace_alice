#ifndef _LIN_ALG
#define _LIN_ALG

#define _POSITIVE_SEMI_DEFINITE

#include "nvec.h"
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/Sparse>
#include <bench/BenchTimer.h>
using namespace Eigen;
#include <vector>


bool positive_semi_definite = true;
double solver_time = 0.0;

# define EIGEN_ERROR_SPARSE(solver) \
if (solver.info() != Eigen::Success)\
{\
	return 0; \
}\

#define  EIGEN_ERROR_DENSE(relative_error)\
if (fabs(relative_error) > 0.1)\
{\
	cout << "The relative error is:\n" << relative_error << endl; \
	return 0; \
}\
	/*
cout << " solver failed " << endl ;\
if( solver.info()== Eigen::NumericalIssue ) cout << "numerical issue" << endl ;\
if( solver.info()== Eigen::InvalidInput ) cout << "InvalidInput" << endl ;\
if( solver.info()== Eigen::NoConvergence ) cout << "NoConvergence" << endl ;\
return 0 ;\*/
//////////////////////////////// //////////////////////////////////////////////////////////////// MATRIX UTILTIES
typedef SparseMatrix<double> SpMat;
typedef Triplet<double> T;
typedef DiagonalMatrix<double, Eigen::Dynamic, Eigen::Dynamic> Diag;

SpMat subMatrix(SpMat &C, vector<int> &nodes)
{
	SpMat C_sub(C.rows(), nodes.size());
	for (int i = 0; i < nodes.size(); i++)C_sub.col(i) = C.col(nodes[i]);
	return C_sub;
}

MatrixXd subMatrix(MatrixXd &X, vector<int> &nodes)
{
	MatrixXd X_sub(nodes.size(), X.cols());
	for (int i = 0; i < nodes.size(); i++)X_sub.row(i) = X.row(nodes[i]);
	return X_sub;
}

void vecToRow(MatrixXd  &X, int &i, const vec &pt)
{
	X(i, 0) = pt.x;
	X(i, 1) = pt.y;
	X(i, 2) = pt.z;
}



////////////////////////////////////////////////////////////////////////// FDM forward decl.

struct g_Vert
{
	g_Vert(){ valence = 0; };
	vec pos;
	vec orig;
	vec old_id;
	vec force;
	int valence;
	bool boundary;

};

struct g_Edge
{
	g_Edge(){};
	int to;
	int from;

};


double angle(vec a, vec b, vec edgeRef)
{
	vec tmp(a.x, a.y, a.z);

	//return Angle_2D( x,y,b.x,b.y) ;
	a.normalise();
	b.normalise();
	double dot = a*b;
	dot = (dot < -1.0 ? -1.0 : (dot > 1.0 ? 1.0 : dot));
	double  dtheta = atan2(tmp.cross(b).mag(), tmp*b);
	while (dtheta > PI)
		dtheta -= PI * 2;
	while (dtheta < -PI)
		dtheta += PI * 2;
	vec cross = a.cross(b);

	return(dtheta * RAD_TO_DEG * (cross*edgeRef < 0 ? -1 : 1));
}



void meshToInnerGraph(Mesh &m, vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges);
void meshToGraph(Mesh &m, vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges);

int FDM(vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges, nvec densities, double loadMult = -1.0, nvec voronoi_areas = NULL);

int moveVertsAbovePlane(vector<g_Vert> &GV,VectorXd &dists, Mesh &imp, float &dAbovePlane, bool measureOnly = false)
{
	int cnt = 0;
	dists.resize(GV.size());

	for (int i = 0; i < GV.size(); i++)
	{
		dists(i) = 0;
		if (GV[i].boundary)continue;

		/*Vertex *verts[3];
		imp.vertices[i].getVertices(verts);
		vec a, b, c, cen, o;
		o = GV[i].pos;
		a = GV[verts[0]->id].pos;
		b = GV[verts[1]->id].pos;
		c = GV[verts[2]->id].pos;
		cen = (a + b + c) / 3.0;
		vec ed1, ed2;
		ed1 = c - a;
		ed2 = b - a;
		vec norm = ed1.cross(ed2).normalise();
		if (norm * vec(0, 0, 1) < 0)norm *= -1;*/
		vec o = GV[i].pos;
		vec cen,norm;
		Vertex *verts[6];
		int n = imp.vertices[i].getVertices(verts);
		for (int j = 0, next = 0; j < n; j++)
		{

			next = (j + 1) % n;
			norm += (GV[verts[j]->id].pos - o).cross(GV[verts[next]->id].pos - o);
			cen += GV[verts[j]->id].pos;
		}

		cen /= n;
		norm.normalise();

		if (norm * vec(0, 0, 1) < 0)norm *= -1; 
		float d = (o - cen)*norm;


		dists(i) = fabs(d);

		float diff = dAbovePlane - d;

		//if (diff <= 0)continue;


		cnt++;
		o += norm * dAbovePlane;

		if (!measureOnly)
		{
			/*imp.positions[i] =*/ GV[i].pos = o;
			//cout << "updatePos" << endl;
		}
	}

	//dAbovePlane = dists.maxCoeff();
	return cnt;
}

void drawGraphStats(vector<g_Vert> &GV, vector<g_Edge> &GE, Mesh &imp,float dAbovePlane, nvec &planeDists)
{


	glLineWidth(1);
	glDisable(GL_LINE_SMOOTH);

	for (int i = 0; i < GV.size(); i++)
	{
		if (GV[i].boundary)continue;

		vec o = GV[i].pos;
		vec cen, norm;
		Vertex *verts[6];
		int n = imp.vertices[i].getVertices(verts);
		for (int j = 0, next = 0; j < n; j++)
		{

			next = (j + 1) % n;
			norm += (GV[verts[j]->id].pos - o).cross(GV[verts[next]->id].pos - o);
			cen += GV[verts[j]->id].pos;
		}

		cen /= n;
		norm.normalise();


		float d = (o - cen)*norm;
		float diff = dAbovePlane - d;

		vec4 clr(0.25, 0.25, 0.25,1);// 
		//vec4 clr = getColour(planeDists.x[i], planeDists.min(), planeDists.max());
		glColor3f(clr.r, clr.g, clr.b);

		vec pOnPlane = o - norm*d;
		

		for (int j = 0, next = 0; j < n; j++)
		{
			next = (j + 1) % n;
		
			vec p_j = GV[verts[j]->id].pos;

			glBegin(GL_TRIANGLES);
				glVertex3f(pOnPlane.x, pOnPlane.y, pOnPlane.z);
				glVertex3f(o.x, o.y, o.z);
				glVertex3f(p_j.x, p_j.y, p_j.z);
			
			glEnd();

		}

		for (int j = 0, next = 0; j < n; j++)
		{
			next = (j + 1) % n;
			vec p_j = GV[verts[j]->id].pos;

			wireFrameOn();

			glColor3f(1, 1, 1);
			glBegin(GL_TRIANGLES);

			glVertex3f(pOnPlane.x, pOnPlane.y, pOnPlane.z);
			glVertex3f(o.x, o.y, o.z);
			glVertex3f(p_j.x, p_j.y, p_j.z);

			glEnd();

			wireFrameOff();

		}

		
		//drawLine(cen, cen + norm * dAbovePlane);
		//drawLine(o, o - norm * d);

	}



	


}
////////////////////////////////////////////////////////////////////////// FDM methods

void meshToGraph(Mesh &m, vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges)
{
	for (int i = 0; i < m.n_v; i++)
	{
		g_Vert v;
		v.pos = v.orig = m.positions[i];
		v.boundary = m.vertices[i].onBoundary();
		graphVerts.push_back(v);
	}

	for (int i = 0; i < m.n_e; i++)
	{
		if (m.edges[i].onBoundary())continue;

		g_Edge e;
		e.from = m.edges[i].vStr->id;
		e.to = m.edges[i].vEnd->id;
		graphEdges.push_back(e);
		graphVerts[e.from].valence += 1;
		graphVerts[e.to].valence += 1;
	}

}

void meshToInnerGraph(Mesh &m, vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges)
{
	int *newIds;
	int *newValence;

	newIds = new int[m.n_v];

	int cnt = 0;
	char s[200];

	for (int i = 0; i < m.n_v; i++)
	{
		newIds[i] = 0;
		if (m.vertices[i].onBoundary())continue;

		newIds[i] = cnt;
		g_Vert v;
		v.pos = v.orig = m.positions[i];
		graphVerts.push_back(v);
		cnt++;
	}

	//////////////////////////////////////////////////////////////////////////

	int ecnt = 0;
	for (int i = 0; i < m.n_e; i++)
	{
		if (m.edges[i].vStr->onBoundary())continue;
		if (m.edges[i].vEnd->onBoundary())continue;


		ecnt++;

		g_Edge e;
		e.from = newIds[m.edges[i].vStr->id];
		e.to = newIds[m.edges[i].vEnd->id];
		graphEdges.push_back(e);
		graphVerts[e.from].valence += 1;
		graphVerts[e.to].valence += 1;
	}

	for (int i = 0; i < graphVerts.size(); i++)graphVerts[i].boundary = (graphVerts[i].valence <= 1) ? true : false;

	delete[]newIds;

}

int FDM(vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges, nvec densities, double loadMult, nvec voronoi_areas  )
{
	BenchTimer timer;
	timer.start();

	int vcnt = graphVerts.size();
	int ecnt = graphEdges.size();

	////////////////////////////////////////////////////////////////////////////  POSITION VECTOR

	MatrixXd X(graphVerts.size(), 3);
	for (int i = 0; i < vcnt; i++)vecToRow(X, i, graphVerts[i].pos);

	////////////////////////////////////////////////////////////////////////////  ADJACENCY MATRIX- SPARSE 

	vector<T> coefs;
	int num_e = 0;
	for (int i = 0; i < ecnt; i++)
	{
		coefs.push_back(T(i, graphEdges[i].from, -1));
		coefs.push_back(T(i, graphEdges[i].to, 1));
	}

	SpMat C(ecnt, vcnt);
	C.setFromTriplets(coefs.begin(), coefs.end());

	////////////////////////////////////////////////////////////////////////////  FORCE DENSITY 
	positive_semi_definite = true;
	VectorXd q(ecnt);
	for (int i = 0; i < ecnt; i++)
	{
		q[i] = densities.x[i];
		if (q[i] < 0)positive_semi_definite = false;
	}



	Diag Q = q.asDiagonal();

	//////////////////////////////////////////////////////////////////////////// LOAD VECTOR

	
	VectorXd p(vcnt);
	MatrixXd P(vcnt, 3);
	if (voronoi_areas.size() == 0 )
	{
		p.setConstant(-loadMult);

		
		P.setConstant(0.0);
		P.col(2) = p.col(0);
	}
	else
	{
		p.setConstant(-loadMult);
		voronoi_areas.normalise()  ;
		for (int i = 0; i < p.rows(); i++)p(i) *= voronoi_areas.x[i] * 3 ;
		P.setConstant(0.0);
		P.col(2) = p.col(0);

		//cout << "v_areas NOT NULL " << endl;
		//cout << P.col(2) << endl;
	}


	//////////////////////////////////////////////////////////////////////////// - SUBMATRICES
	vector<int> free, fixed;
	for (int i = 0; i < vcnt; i++)
		(graphVerts[i].boundary) ? fixed.push_back(i) : free.push_back(i);


	SpMat Ci = subMatrix(C, free);
	SpMat Cf = subMatrix(C, fixed);
	MatrixXd Xfixed = subMatrix(X, fixed);
	MatrixXd Pf = subMatrix(P, free);

	SpMat Cit;
	Cit = Ci.transpose();

	//////////////////////////////////////////////////////////////////////////// SOLVE

	SpMat Dn = Cit * Q * Ci;
	SpMat Df = Cit * Q * Cf;
	MatrixXd B = Pf - Df * Xfixed;

	//---------------------- SPARSE / DENSE SOLVE - PSD or not.


	MatrixXd Xfree;
	if (positive_semi_definite)
	{
		SimplicialLLT< SpMat > solver; // sparse cholesky solver
		solver.compute(Dn); // compute cholesky factors
		EIGEN_ERROR_SPARSE(solver); // check error

		Xfree = solver.solve(B); // solve AX = B ;
		EIGEN_ERROR_SPARSE(solver); // check errors
	}
	else
	{
		MatrixXd denseDn;
		denseDn = MatrixXd(Dn);
		Xfree = denseDn.ldlt().solve(B);

		// convergence error check.
		double relative_error = (denseDn*Xfree - B).norm() / B.norm(); // norm() is L2 norm
		EIGEN_ERROR_DENSE(relative_error);
	}






	int bCnt = 0;
	for (int i = 0, cnt = 0; i < vcnt; i++)
	{
		if (graphVerts[i].boundary) continue;
		graphVerts[i].pos = vec(Xfree(bCnt, 0), Xfree(bCnt, 1), Xfree(bCnt, 2));
		bCnt++;
	}

	timer.stop();
	solver_time = timer.value();
	//printf(" compute time FDM : %1.4f  nodes : %i edges : %i \n" ,timer.value(),X.rows(),C.rows() ) ;

	return 1;
}

nvec voronoi_areas;
void iterativeFDM(vector<g_Vert> &GV, vector<g_Edge> &GE, nvec &densities,double loadMult, Mesh &topology)
{
	voronoi_areas.resize(GV.size());

	for (int l = 0; l < 3; l++)
	{

		Face *vfaces[8];
		vec faceCenters[8];

		voronoi_areas = 0;

		for (int i = 0; i < GV.size(); i++)
		{
			float ar_f;
			topology.vertices[i].vertexNormal(topology.positions, &ar_f);
			voronoi_areas.x[i] = ar_f;
		}


		FDM(GV, GE, densities, loadMult , voronoi_areas);
		for (int i = 0; i < GV.size(); i++)topology.positions[i] = GV[i].pos;
	}

}

VectorXd r;

nvec calculateResidualsAndGradient(vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges, nvec densities, nvec &scores, double threshold, VectorXd &surf_a)
{

	int vcnt = graphVerts.size();
	int ecnt = graphEdges.size();

	////////////////////////////////////////////////////////////////////////////  POSITION VECTOR

	MatrixXd X(graphVerts.size(), 3);
	for (int i = 0; i < vcnt; i++)vecToRow(X, i, graphVerts[i].pos);

	MatrixXd X_orig(graphVerts.size(), 3);
	for (int i = 0; i < vcnt; i++)vecToRow(X_orig, i, graphVerts[i].orig);

	////////////////////////////////////////////////////////////////////////////  ADJACENCY MATRIX- SPARSE 

	vector<T> coefs;
	int num_e = 0;
	for (int i = 0; i < ecnt; i++)
	{
		coefs.push_back(T(i, graphEdges[i].from, -1));
		coefs.push_back(T(i, graphEdges[i].to, 1));
	}

	SpMat C(ecnt, vcnt);
	C.setFromTriplets(coefs.begin(), coefs.end());

	////////////////////////////////////////////////////////////////////////////  FORCE DENSITY 
	positive_semi_definite = true;
	VectorXd q(ecnt);
	for (int i = 0; i < ecnt; i++)
	{
		q[i] = densities.x[i];
		if (q[i] < 0)positive_semi_definite = false;
	}



	Diag Q = q.asDiagonal();


	//////////////////////////////////////////////////////////////////////////// - SUBMATRICES
	vector<int> free, fixed;
	for (int i = 0; i < vcnt; i++)
		(graphVerts[i].boundary) ? fixed.push_back(i) : free.push_back(i);


	SpMat Ci = subMatrix(C, free);
	SpMat Cf = subMatrix(C, fixed);

	SpMat Cit;
	Cit = Ci.transpose();
	//MatrixXd Xfree = subMatrix(X, free);

	VectorXd u, v, w;
	u = C * X.col(0);
	v = C * X.col(1);
	w = C * X.col(2);
	Diag U = u.asDiagonal();
	Diag V = v.asDiagonal();
	Diag W = w.asDiagonal();

	MatrixXd A(3 * free.size(), ecnt);
	A << MatrixXd(Cit*U), MatrixXd(Cit*V), MatrixXd(Cit*W);

	VectorXd P(free.size() * 3);
	P << VectorXd::Zero(free.size()), VectorXd::Zero(free.size()), VectorXd::Ones(free.size()) * -1;
	//P << Xfree.col(0), Xfree.col(1), Xfree.col(2);
	
	r = A * q - P;
	float d = r.norm();

	surf_a.resize(r.rows());
	surf_a.setConstant(0);
	MatrixXd diff = X_orig - X ;
	MatrixXd diff_free = subMatrix(diff, free); //
	surf_a << diff_free.col(0), diff_free.col(1), diff_free.col(2);



	//int cnt = 0;
	//for (int i = 0; i < graphVerts.size(); i++)
	//	if (!graphVerts[i].boundary /*&& scores.x[i] < threshold*/)
	//	{
	//		surf_a(cnt) *= 2;
	//		surf_a(cnt + free.size()) *= 2;
	//		
	//		surf_a(cnt + 2 * free.size()) = 0;
	//		cnt++;
	//	}
	d = surf_a.norm();
	surf_a.normalize();
	surf_a *= 0.15 * d;
	//r += surf_a;


	MatrixXd LHS = A.transpose() * A;
	MatrixXd RHS = A.transpose() * r;
	VectorXd del_q = LHS.ldlt().solve(RHS);

	nvec nv_del_q(del_q.rows());
	for (int i = 0; i < del_q.rows(); i++)nv_del_q.x[i] = del_q(i);

	//nv_del_q.print();

	//////////////////////////////////////////////////////////////////////////// SOLVE

	return nv_del_q;
}


nvec calculateBestFitQ(vector<g_Vert> &graphVerts, vector<g_Edge> &graphEdges, double loadMult )
{

	int vcnt = graphVerts.size();
	int ecnt = graphEdges.size();

	////////////////////////////////////////////////////////////////////////////  POSITION VECTOR

	MatrixXd X(graphVerts.size(), 3);
	for (int i = 0; i < vcnt; i++)vecToRow(X, i, graphVerts[i].orig);

	////////////////////////////////////////////////////////////////////////////  ADJACENCY MATRIX- SPARSE 

	vector<T> coefs;
	int num_e = 0;
	for (int i = 0; i < ecnt; i++)
	{
		coefs.push_back(T(i, graphEdges[i].from, -1));
		coefs.push_back(T(i, graphEdges[i].to, 1));
	}

	SpMat C(ecnt, vcnt);
	C.setFromTriplets(coefs.begin(), coefs.end());

	////////////////////////////////////////////////////////////////////////////  FORCE DENSITY 
	positive_semi_definite = true;
	


	//////////////////////////////////////////////////////////////////////////// - SUBMATRICES
	vector<int> free, fixed;
	for (int i = 0; i < vcnt; i++)
		(graphVerts[i].boundary) ? fixed.push_back(i) : free.push_back(i);


	SpMat Ci = subMatrix(C, free);
	SpMat Cf = subMatrix(C, fixed);

	SpMat Cit;
	Cit = Ci.transpose();
	MatrixXd Xfree = subMatrix(X, free);

	VectorXd u, v, w;
	u = C * X.col(0);
	v = C * X.col(1);
	w = C * X.col(2);
	Diag U = u.asDiagonal();
	Diag V = v.asDiagonal();
	Diag W = w.asDiagonal();

	MatrixXd A(3 * free.size(), ecnt);
	A << MatrixXd(Cit*U), MatrixXd(Cit*V), MatrixXd(Cit*W);

	VectorXd P(free.size() * 3);
	P << VectorXd::Zero(free.size()), VectorXd::Zero(free.size()), VectorXd::Ones(free.size()) * -loadMult;


	MatrixXd LHS = A.transpose() * A;
	MatrixXd RHS = A.transpose() * P ;
	VectorXd del_q = LHS.ldlt().solve(RHS);

	nvec nv_del_q(del_q.rows());
	for (int i = 0; i < del_q.rows(); i++)nv_del_q.x[i] = del_q(i);

	//nv_del_q.print();

	//////////////////////////////////////////////////////////////////////////// SOLVE

	return nv_del_q;
}





#endif