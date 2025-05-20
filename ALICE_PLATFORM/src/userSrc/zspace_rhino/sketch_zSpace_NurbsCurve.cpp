//#define _MAIN_
#define _HAS_STD_BYTE 0

#ifdef _MAIN_

#include "main.h"

//////  zSpace Library
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>
#include <headers/zApp/include/zViewer.h>

//#include <headers/api/functionsets/zFnMesh.h>
//#include <headers/api/functionsets/zFnGraph.h>



using namespace zSpace;
using namespace std;


////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

/*!<model*/
zModel model;

zObjNurbsCurve o_curve;
zObjNurbsCurve o_curve_interpolate;
zObjNurbsCurve o_curve_circle;
zObjNurbsCurve o_curve_line;
zObjNurbsCurve o_curve_self;
zObjNurbsCurve o_curve_sub;

string path = "C:/Users/vishu.b/desktop/cube.obj";

string file = "data/nurbsCurve.json";

//string path = "C:/Users/vishu.b/desktop/zSpace_graph_fromPentahedron_100x200.txt";

bool c_Dual = false;

bool d_dualgraph = true;
bool d_inputmesh = true;


int currentId = 0;
int rCnt = 0;

string what;
////// --- GUI OBJECTS ----------------------------------------------------


char s[200], text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------

zPointArray pts;
zPointArray pts_1;

bool d_cvs = false;


void makeCircle(zObjNurbsCurve& o_curve)
{
	zFnNurbsCurve fnNurbs(o_curve);

	zPointArray controlPts;
	controlPts.push_back(zPoint(0, 0, 0));
	controlPts.push_back(zPoint(1, 0, 0));
	controlPts.push_back(zPoint(1, -1, 0));
	controlPts.push_back(zPoint(0, -1, 0));

	fnNurbs.create(controlPts, 3, true,false, 200);
	o_curve.setDisplayColor(zColor(1, 0, 0, 1));
	o_curve.setDisplayWeight(4);
	fnNurbs.setTranslation(zVector(0, -0.5, 0));
}

void makeLine(zObjNurbsCurve& o_curve)
{
	zFnNurbsCurve fnNurbs(o_curve);

	zPointArray controlPts;
	controlPts.push_back(zPoint(-3, -3, 0));
	controlPts.push_back(zPoint(0, 0, 0));
	controlPts.push_back(zPoint(3, 3, 0));

	fnNurbs.create(controlPts, 2, false,false, 200);
	o_curve.setDisplayColor(zColor(1, 0, 0, 1));
	o_curve.setDisplayWeight(4);
}

void makeSelf(zObjNurbsCurve& o_curve)
{
	zFnNurbsCurve fnNurbs(o_curve);

	zPointArray controlPts;
	controlPts.push_back(zPoint(0, 0, 0));
	controlPts.push_back(zPoint(1, 0, 0));
	controlPts.push_back(zPoint(1, -1, 0));
	controlPts.push_back(zPoint(0, -1, 0));
	controlPts.push_back(zPoint(0, 0, 0));
	controlPts.push_back(zPoint(1, 0, 0));
	controlPts.push_back(zPoint(1, 0, 0));

	fnNurbs.create(controlPts, 3, false,false, 200);
	o_curve.setDisplayColor(zColor(1, 0, 0, 1));
	o_curve.setDisplayWeight(4);
}

void makeCurve(zObjNurbsCurve& o_curve)
{
	zFnNurbsCurve fnNurbs(o_curve);

	zPointArray controlPts;
	controlPts.push_back(zPoint(-0.5, 0, 0.3));
	controlPts.push_back(zPoint(0.2, 0, -0.2));
	controlPts.push_back(zPoint(1, -2, 0));
	controlPts.push_back(zPoint(2, 1, 0.5));
	controlPts.push_back(zPoint(1, -0.5, -0.5));

	fnNurbs.create(controlPts, 3, false, false, 200);
	o_curve.setDisplayColor(zColor(1, 0, 0, 1));
	o_curve.setDisplayWeight(4);
	fnNurbs.setTranslation(zVector(0, 1, 0));

	//double len = fnNurbs.getLength();
	//printf("\n cvs %i | length %1.2f ", fnNurbs.numControlVertices(), fnNurbs.getLength());
}

void makeInterpolate(zObjNurbsCurve& o_curve)
{
	zFnNurbsCurve fnNurbs(o_curve);

	zPointArray controlPts;
	controlPts.push_back(zPoint(-0.5, 0, 0.3));
	controlPts.push_back(zPoint(0.2, 0, -0.2));
	controlPts.push_back(zPoint(1, -2, 0));
	controlPts.push_back(zPoint(2, 1, 0.5));
	controlPts.push_back(zPoint(1, -0.5, -0.5));

	fnNurbs.create(controlPts, 3, false, true, 200);
	o_curve.setDisplayColor(zColor(0, 0, 1, 1));
	o_curve.setDisplayWeight(4);
	fnNurbs.setTranslation(zVector(0, 1, 0));
}

////// ---------------------------------------------------- MODEL  ----------------------------------------------------

void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////

	// initialise model
	model = zModel(100000);
	
	//fnNurbs.from("data/out_nurbsCurve.json", zJSON);



	for (auto& pt : pts)
		cout << "pts:" << pt << endl;

	//fnNurbs.to("data/out_nurbsCurve.json", zJSON);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_curve);
	model.addObject(o_curve_circle);
	model.addObject(o_curve_line);
	model.addObject(o_curve_self);
	model.addObject(o_curve_interpolate); 
	model.addObject(o_curve_sub);

	// set display element booleans
	
	makeCurve(o_curve);
	makeSelf(o_curve_self);
	makeCircle(o_curve_circle);
	makeLine(o_curve_line);
	makeInterpolate(o_curve_interpolate);

	o_curve.setDisplayElements(true, true);

	//////////////////////////////////////////////////////////

	S = *new SliderGroup();
	S.addSlider(&background, "background");

	//S.addSlider(&formWeight, "formWeight");
	//S.sliders[1].attachToVariable(&formWeight, 0.00, 1.00);

	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));



	B.addButton(&d_inputmesh, "d_Input");

	B.addButton(&d_dualgraph, "d_Dual");

	B.addButton(&c_Dual, "c_Dual");

	


	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{


}


////// ---------------------------------------------------- VIEW  ----------------------------------------------------

void draw()
{

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	backGround(background);
	drawGrid(1);

	glColor3f(1, 0, 0);


	S.draw();
	B.draw();
	// ------------------------ draw the path points / Tool orientations 
	
	model.draw();	

	for (auto& pt : pts)
		model.displayUtils.drawPoint(pt, zBLUE, 10);

	for (auto& pt : pts_1)
		model.displayUtils.drawPoint(pt, zGREEN, 10);

	if (d_cvs)
	{
		zFnNurbsCurve fn(o_curve);
		zPoint* cv = fn.getRawControlPoints();
		int ncv = fn.numControlVertices();
		for (int i = 0; i < ncv; i++)
		{
			model.displayUtils.drawPoint(cv[i], zBLACK, 10);
		}

	}

	//////////////////////////////////////////////////////////



	glColor3f(0, 0, 0);
	setup2d();

	drawString(what, vec(winW - 350, winH - 475, 0));

	restore3d();
	//drawVector(camPt, vec(wid, hts + 25, 0), "cam");

}

////// ---------------------------------------------------- CONTROLLER  ----------------------------------------------------

void keyPress(unsigned char k, int xm, int ym)
{

	///// GRAPH GENERTOR PROGRAM 
	if (k == 'i')setCamera(15, -40, 60, -2, 4);

	if (k == 'f')updateCamera();

	if (k == '1')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(true, true);
		o_curve_circle.setDisplayElements(false, false);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_interpolate.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zFnNurbsCurve fnNurbs(o_curve);

		Matrix4f plane;
		plane.setIdentity();
		plane(0, 0) = 1; plane(0, 1) = 0; plane(0, 2) = 0;
		plane(1, 0) = 0; plane(1, 1) = 1; plane(1, 2) = 0;
		plane(2, 0) = 0; plane(2, 1) = 0; plane(2, 2) = 1;
		plane(3, 0) = 0; plane(3, 1) = 0; plane(3, 2) = 0;

		zDoubleArray tParams;
		fnNurbs.intersect(plane, pts, tParams);

		what = string("curve/plane intersect");
	}

	if (k == '2')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(false, false);
		o_curve_circle.setDisplayElements(false, false);
		o_curve_self.setDisplayElements(true, true);
		o_curve_line.setDisplayElements(false, false);
		o_curve_interpolate.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zFnNurbsCurve fnNurbs(o_curve_self);

		zDoubleArray tParams;
		fnNurbs.intersectSelf(pts, tParams);

		what = string("curve self intersect");
	}

	if (k == '3')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(false, false);
		o_curve_circle.setDisplayElements(true, true);
		o_curve_self.setDisplayElements(true, true);
		o_curve_line.setDisplayElements(false, false);
		o_curve_interpolate.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zFnNurbsCurve fnNurbs(o_curve_circle);

		zDoubleArray tParams;

		zPointArray pts_b;
		zDoubleArray tParams_b;
		fnNurbs.intersect(o_curve_self, pts, pts_b, tParams, tParams_b);

		what = string("curve/curve intersect");
	}

	if (k == '4')
	{
		d_cvs = true;

		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(true, true);
		o_curve_circle.setDisplayElements(false, false);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_interpolate.setDisplayElements(true, true);
		o_curve_sub.setDisplayElements(false, false);

		what = string("interpolate curve");
	}

	if (k == '5')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(true, true);
		o_curve_circle.setDisplayElements(false, false);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_interpolate.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(true, true);

		zFnNurbsCurve fnNurbs(o_curve);
		fnNurbs.computeSubCurve(0.3, 0.6, true, o_curve_sub);

		zFnNurbsCurve fnNurbs_sub(o_curve_sub);
		o_curve_sub.setDisplayColor(zGREEN);
		o_curve_sub.setDisplayWeight(4);
		cout << "fnNurbs_sub.numControlVertices():" << fnNurbs_sub.numControlVertices() << endl;

		pts.push_back(fnNurbs_sub.getPointAt(0.0));
		pts.push_back(fnNurbs_sub.getPointAt(1.0));
		what = string("sub curve");
	}

	if (k == 'a')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(true, true);
		o_curve_circle.setDisplayElements(false, false);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zFnNurbsCurve fn(o_curve);
		zDoubleArray t;
		fn.divideByCount(20, pts, t);

		what = string("divideByCount");
	}

	if (k == 's')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(true, true);
		o_curve_circle.setDisplayElements(false, false);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zFnNurbsCurve fn(o_curve);
		zDoubleArray t;
		fn.divideByLength(0.2, pts, t);

		what = string("divideByLength");
	}

	if (k == 'd')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(false, false);
		o_curve_circle.setDisplayElements(true, true);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zPoint pt(0, 0, 0);
		zPoint pt_out;
		double t_out;

		zFnNurbsCurve fn(o_curve_circle);
		fn.closestPoint(pt, pt_out, t_out);

		pts.clear();
		pts_1.clear();
		pts.push_back(pt_out);
		pts_1.push_back(pt);
		what = string("closestPoint");
	}

	if (k == 'f')
	{
		pts.clear();
		pts_1.clear();
		o_curve.setDisplayElements(true, true);
		o_curve_circle.setDisplayElements(true, true);
		o_curve_self.setDisplayElements(false, false);
		o_curve_line.setDisplayElements(false, false);
		o_curve_sub.setDisplayElements(false, false);

		zPoint pt;
		zPoint pt_out;
		double t;
		double t_out;

		zFnNurbsCurve fn(o_curve);
		fn.closestPoint(o_curve_circle, pt_out, pt, t_out, t);

		pts.clear();
		pts_1.clear();
		pts.push_back(pt_out);
		pts_1.push_back(pt);
		what = string("closestPointsBetweenTwoCurves");
	}

}

void mousePress(int b, int state, int x, int y)
{

	if (GLUT_LEFT_BUTTON == b && GLUT_DOWN == state)
	{

		B.performSelection(x, y);

		S.performSelection(x, y, HUDSelectOn);

	}

	if ((GLUT_LEFT_BUTTON == b && GLUT_UP == state) || (GLUT_RIGHT_BUTTON == b && GLUT_UP == state))
	{

	}
}

void mouseMotion(int x, int y)
{
	S.performSelection(x, y, HUDSelectOn);


	bool dragging = (glutGetModifiers() == GLUT_ACTIVE_ALT) ? true : false;
	int cur_msx = winW * 0.5;
	int cur_msy = winH * 0.5;
	camPt = screenToCamera(cur_msx, cur_msy, 0.2);

	//if( dragging)GS.LM.updateColorArray(lightscale, flipNormals, camPt);

}




#endif // _MAIN_
