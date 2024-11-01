
//#define _MAIN_


#ifdef _MAIN_

#include "main.h"

//////  zSpace Library
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsStatics.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS ----------------------------------------------------
void drawTextAtVec(string s, zVector &pt)
{
	unsigned int i;
	glRasterPos3f(pt.x, pt.y, pt.z);

	for (i = 0; i < s.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
}



////////////////////////////////////////////////////////////////////////// GLOBAL VARIABLES ----------------------------------------------------
////// --- MODEL OBJECTS ----------------------------------------------------

/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

vector<zObjMesh> forceObjs;
zObjGraph formObj;
vector <zObjMesh> polytopalObjs;
zObjGraph resultObj;

/*!<Function sets*/

/*!<Tool sets*/
zTsPolytopal myPolytopal;
//zTsGraphVault myVault;


vector<int> fixedVertices;

int presFac = 1;
double minEdgefac = 0.2;
double offset = 0.1;
double angleTolerance = 11.0;
double dT = 1.0;
zIntergrationType intType = zRK4;

int nF = 0;

string dirPath = "C:/Users/vishu.b/desktop/GFP/GFP_3A";
vector<string> forceFiles;

string exportPath = "C:/Users/vishu.b/desktop/polytopals/";

bool fixBoundary = false;

int subDIVS = 0;

bool c_form = false;
bool c_force = false;

bool compute_targets = true;

bool equilibrium= false;
bool FDM = false;
bool verticalE = false;

bool c_Polytopal = false;

bool d_dualgraph = true;
bool d_polytopal = true;
bool d_ForceDiagram = true;
bool d_FormDiagram = true;
bool d_Result = true;

bool exportPolytopal = false;

bool rotate90 = true;
bool computeHE_targets = true;

bool closePolytopal = false;

int currentId = 0;
int rCnt = 0;


double vLoad = -5.0;
double forceDensity = 1.0;

////// --- GUI OBJECTS ----------------------------------------------------


char s[200],text[200], text1[200], jts[400];

double background = 0.9;
Alice::vec camPt;
string printfile;

////////////////////////////////////////////////////////////////////////// MAIN PROGRAM : MVC DESIGN PATTERN  ----------------------------------------------------




////// ---------------------------------------------------- MODEL  ----------------------------------------------------





void setup()
{

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	//////////////////////////////////////////////////////////////////////////
	model = zModel(100000);
	
	core.getFilesFromDirectory(forceFiles,dirPath, zJSON);
	printf("\n NF : %i ", forceFiles.size());
	
	forceObjs.assign(forceFiles.size(), zObjMesh());
	
	int numPolytopals = forceFiles.size() + 1;	
	polytopalObjs.assign(numPolytopals, zObjMesh());
	
	
	myPolytopal = zTsPolytopal(formObj, forceObjs, polytopalObjs);	

	
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(formObj);
	
	for (int i = 0; i < forceObjs.size(); i++)	model.addObject(forceObjs[i]);

	for (int i = 0; i < polytopalObjs.size(); i++)	model.addObject(polytopalObjs[i]);

	// set display element booleans
	formObj.setShowElements(true, true);

	for (int i = 0; i < forceObjs.size(); i++)	forceObjs[i].setShowElements(false, true, false);
	for (int i = 0; i < polytopalObjs.size(); i++)	polytopalObjs[i].setShowElements(false, true, true);

	   
	
	//////////////////////////////////////////////////////////


	S = *new SliderGroup();
	
	S.addSlider(&background, "background");

	

	S.addSlider(&offset, "offset");
	S.sliders[1].attachToVariable(&offset, 0.01, 1.00);
	
	S.addSlider(&angleTolerance, "angleTolerance");
	S.sliders[2].attachToVariable(&angleTolerance, 0.001, 20.00);

	S.addSlider(&vLoad, "vLoad");
	S.sliders[3].attachToVariable(&vLoad, -10.0, 10.0);

	S.addSlider(&minEdgefac, "minEdgefac");
	S.sliders[4].attachToVariable(&minEdgefac, 0.001, 10.0);


	/////////////////////////////

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&c_force, "c_force");

	B.addButton(&c_form, "c_form");	

	B.addButton(&equilibrium, "equilibrium");

	B.addButton(&c_Polytopal, "c_Polytopal");
	
	B.addButton(&closePolytopal, "closePolytopal");


	B.addButton(&d_polytopal, "d_polytopal");

	B.addButton(&d_ForceDiagram, "d_ForceDiagram");

	B.addButton(&d_FormDiagram, "d_FormDiagram");	

	B.addButton(&exportPolytopal, "e_polytopals");

	
	

	//////////////////////////////////////////////////////////////////////////


}

void update(int value)
{
	
	formObj.setShowObject(d_FormDiagram);
	
	for (int i = 0; i < forceObjs.size(); i++)
	{
		forceObjs[i].setShowObject(d_ForceDiagram);
	}

	for (int i = 0; i < polytopalObjs.size(); i++)
	{
		polytopalObjs[i].setShowObject(d_polytopal);
	}

	myPolytopal.setVertexOffset(offset);
	
	if (c_force)
	{
		myPolytopal.createForceFromFiles(forceFiles, zJSON);

		myPolytopal.getPrimal_GlobalElementIndicies(zForceDiagram, -1, 3);	
		
		//myPolytopal.createSectionPoints(4, 0.05);
		
		
		/*zSparseMatrix C_ev;
		myPolytopal.getPrimal_EdgeVertexMatrix(zForceDiagram, C_ev);
		cout << "\n C_ev " << endl << C_ev << endl;*/

		/*VectorXd q;
		myPolytopal.getDual_ForceDensities_LPA(q, 0.1);*/

		//MatrixXd A;
		//myPolytopal.get_EquilibriumMatrix(zForceDiagram, A);		

		//// Q & s
		//real_1d_array Q;
		//real_1d_array s;

		//Q.setlength(A.cols());
		//s.setlength(A.cols());

		//for (int i = 0; i < Q.length(); i++) fAreas.push_back(myPolytopal.primalFaceAreas[myPolytopal.internalFaceIndex_primalFace[i]]);

		//for (int i = 0; i < A.cols(); i++)
		//{
		//	Q(i) = 1.0;
		//	s(i) = 1;
		//}

		//real_2d_array c;
		//c.setlength(A.rows(), A.cols() + 1);

		//integer_1d_array ct;
		//ct.setlength(A.rows());

		////AQ =0
		//for (int i = 0; i < A.rows(); i++)
		//{
		//	// coefficients
		//	for (int j = 0; j < A.cols(); j++)
		//	{
		//		c(i, j) = A(i, j);
		//	}

		//	// right part
		//	c(i, A.cols()) = 0.0;

		//	ct(i) = 1;
		//}

		//// Q >=1
		//real_1d_array bndl; 
		//bndl.setlength(A.cols());

		//real_1d_array bndu; 
		//bndu.setlength(A.cols());

		//for (int i = 0; i < A.cols(); i++)
		//{
		//	bndl(i) = 0.1;
		//	bndu(i) = 100.0;
		//}

		////printf("\n %s\n", Q.tostring(2).c_str());
		////printf("\n C \n %s\n", c.tostring(2).c_str());
		////printf("\n CT \n %s\n", ct.tostring().c_str());

		//minbleicstate state;
		//double epsg = 0.000001;
		//double epsf = 0;
		//double epsx = 0.000001;
		//ae_int_t maxits = 0;
		//double diffstep = 1.0e-6;

		//minbleiccreatef(Q, diffstep,state);
		//minbleicsetbc(state, bndl, bndu);
		//minbleicsetlc(state, c, ct);
		//minbleicsetscale(state, s);
		//minbleicsetcond(state, epsg, epsf, epsx, maxits);

		//minbleicoptguardsmoothness(state);
		////minbleicoptguardgradient(state, 0.001);

		//minbleicreport rep;
		//alglib::minbleicoptimize(state, loadPath_func);
		//minbleicresults(state, Q, rep);
		//printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4
		//printf("%s\n", Q.tostring(2).c_str()); // EXPECTED: [-1,1]


		/*zSparseMatrix C_ev;
		myPolytopal.getPrimal_EdgeVertexMatrix(zForceDiagram, C_ev);
		cout << "\n C_ev " << endl << C_ev << endl;

		zSparseMatrix C_ef;
		myPolytopal.getPrimal_EdgeFaceMatrix(zForceDiagram, C_ef);
		cout << "\n C_ef " <<  endl  << C_ef << endl;*/


		/*zSparseMatrix C_fvol;
		myPolytopal.getPrimal_FaceVolumeMatrix(zForceDiagram, C_fvol);
		cout << "\n C_ec " << endl << C_fvol << endl;
*/

		//MatrixXd A;
		//myPolytopal.get_EquilibriumMatrix(zForceDiagram, A);
		
		//cout << "\n A " << endl << A << endl;

		//FullPivLU<MatrixXd> lu_decomp(A);

		//lu_decomp.setThreshold(0.0001);
		//cout << "\n RANK:  " << lu_decomp.rank() << endl;		
		

		//MatrixXd upper = lu_decomp.matrixLU().triangularView<Eigen::Upper>();
		//cout << "\n upper:  " << endl << upper << endl;
		

		//string outfilename = "C:/Users/vishu.b/desktop/upper.csv";
		//ofstream myfile;
		//myfile.open(outfilename.c_str());

		//for (int i =0; i <  upper.rows(); i++)
		//{
		//	myfile << "\n";
		//	for (int j= 0; j < upper.cols(); j++)
		//	myfile << upper(i,j) << ",";

		//}



		//cout << "\n Here is a matrix whose columns form a basis of the null-space of A:\n"
			//<< lu_decomp.kernel() << endl;	
		
		//cout << "Here is a matrix whose columns form a basis of the column-space of A:\n"
			//<< (lu_decomp.image(A).col(0) - A.col(0)).isZero() << endl; // yes, have to pass the original A
		
	

		/*float A_RREF[24][12];
		for (int i = 0; i < 24; i++)
		{
			for (int j = 0; j < 12; j++) A_RREF[i][j] = upper(i, j);
		}

		to_reduced_row_echelon_form(A_RREF);
		


		std::cout << "\n \n  A RREF \n ";
		for (int i = 0; i < 24; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				if(A_RREF[i][j] >= 0 && A_RREF[i][j] < 0.0000001) std::cout << 0 << '\t';
				else if (A_RREF[i][j] <= 0 && A_RREF[i][j] > -0.0000001) std::cout << 0 << '\t';
				else std::cout << A_RREF[i][j] << '\t';
			}
				
			std::cout << "\n";
		}*/

		//for (int i = 0; i < myPolytopal.fnForces.size(); i++)
		//{
		//	//printf("\n  %i numP: %i ",i, myPolytopal.fnForces[i].numPolygons());

		//	vector<zVector> fCenters;
		//	myPolytopal.fnForces[i].getCenters(zFaceData, fCenters);

		//	forceObjs[i].setShowFaceNormals(true, fCenters, 0.1);
		//}

	

		c_force = !c_force;
	}

	
	if (c_form)
	{
		myPolytopal.createFormFromForce(zColor(0,1,0,1) , false, 0.1);
		
		printf("\n form :  v %i e %i ", myPolytopal.fnForm.numVertices(), myPolytopal.fnForm.numEdges());
	
		

		//myPolytopal.getDual(0.3, true, 0.1);

		//myPolytopal.createFormFromForce(offset,presFac);

		//compute_targets = true;

		printf("\n numE: %i ", myPolytopal.fnForm.numEdges());
		
		c_form = !c_form;
	}

	

	if (equilibrium)
	{
		//bool out = myPolytopal.equilibrium(compute_targets, minEdgefac, dT,intType,10,angleTolerance,true,true);;
		zDomainDouble dev;

		bool out = myPolytopal.equilibrium(compute_targets, minEdgefac, dev, dT, intType, 10, angleTolerance, true, true);;

		

		//if (out) myPolytopal.setFormEdgeWeightsfromForce(zDomainDouble(2.0, 10.0));

		if (out) myPolytopal.setDualEdgeWeightsfromPrimal(zDomainDouble(2.0, 10.0));

		equilibrium = !out;

	}

	if (c_Polytopal)
	{		
		myPolytopal.createPolytopalsFromForce_profile(4, 0.1, 0.05, 0.3, subDIVS);

		//myPolytopal.createPolytopalsFromForce(0.1, 0.3, subDIVS);

		if (closePolytopal) myPolytopal.closePolytopals();

		c_Polytopal = !c_Polytopal;
	}
	

	if (exportPolytopal)
	{
		/*for (int i = 0; i < myPolytopal.fnPolytopals.size(); i++)
		{
			string path1 = exportPath + "/polytopals_" + to_string(i) + ".json";

			myPolytopal.fnPolytopals[i].to(path1, zJSON);

			
		}*/

		myPolytopal.exportBisectorPlanes("C:/Users/vishu.b/Desktop/test_edgeCutPlanes.json");
		myPolytopal.fnForm.to("C:/Users/vishu.b/Desktop/test_Graph.json", zJSON);

		exportPolytopal = !exportPolytopal;
	}
	

}


////// ---------------------------------------------------- VIEW  ----------------------------------------------------

void draw()
{
	
	

	backGround(background);
	//drawGrid(20.0);

	//glDisable(GL_CULL_FACE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);


	glColor3f(1, 0, 0);
	

	S.draw();
	B.draw();
	// ------------------------ draw the path points / Tool orientations 
	//int i = 0;
	//for (auto &pos : myPolytopal.primalVertexPositions)
	//{
	//	drawTextAtVec(to_string(i), pos);
	//	i++;
	//}

	//glColor3f(1, 0, 0);

	// i = 0;
	//for (auto &pos : myPolytopal.primalFaceCenters)
	//{
	//	/*if (!myPolytopal.GFP_SSP_Face[i])
	//	{
	//		drawTextAtVec(to_string(myPolytopal.primal_internalFaceIndex[i]), pos);
	//	}*/
	//	
	//	drawTextAtVec(to_string(i), pos);
	//	
	//	i++;
	//}

	//for (auto &pos : myPolytopal.userSection_points)
	//{
	//	model.displayUtils.drawPoint(pos, zColor(0, 0, 1, 1));		
	//}

	/*
	for (int i = 0; i < myPolytopal.primalFaceCenters.size(); i++)
	{
		if (myPolytopal.GFP_SSP_Face[i])continue;

		zVector temp = myPolytopal.primalFaceCenters[i] + (myPolytopal.primalFaceNormals[i] * 0.1);
		model.displayUtils.drawLine(myPolytopal.primalFaceCenters[i], temp);
	}

	glColor3f(0, 1, 0);

	for (int j = 0; j < myPolytopal.internalEdgeIndex_primalEdge.size(); j++)
	{
		int eId = myPolytopal.internalEdgeIndex_primalEdge[j];

		zVector v0 = myPolytopal.primalVertexPositions[myPolytopal.primalEdge_PrimalVertices[eId][0]];
		zVector v1 = myPolytopal.primalVertexPositions[myPolytopal.primalEdge_PrimalVertices[eId][1]];

		zVector pos = (v0 + v1) * 0.5;
		drawTextAtVec(to_string(j), pos);
	
	}*/

	
	
	/*glColor3f(0, 1, 0);
	for (zItGraphEdge e(*myPolytopal.formObj); !e.end(); e.next())
	{
		zVector v0 = e.getHalfEdge(0).getStartVertex().getVertexPosition();
		zVector v1 = e.getHalfEdge(0).getVertex().getVertexPosition();;

		zVector pos = (v0 + v1) * 0.5;
		drawTextAtVec(to_string(e.getId()), pos);
	}*/

	model.draw();	
	

	//////////////////////////////////////////////////////////



	glColor3f(0, 0, 0);
	setup2d();

	AL_drawString(s, winW * 0.5, winH - 50);
	AL_drawString(text, winW * 0.5, winH - 75);
	AL_drawString(jts, winW * 0.5, winH - 100);
	AL_drawString(printfile.c_str(), winW * 0.5, winH - 125);
	
	

	int hts = 25;
	int wid = winW * 0.75;



	restore3d();
	//drawVector(camPt, vec(wid, hts + 25, 0), "cam");

}

////// ---------------------------------------------------- CONTROLLER  ----------------------------------------------------

void keyPress(unsigned char k, int xm, int ym)
{

	///// GRAPH GENERTOR PROGRAM 
	if (k == 'i')setCamera(15, -40, 60, -2, 4);
	
	if (k == 'w') if (subDIVS < 3) subDIVS++;
	if (k == 's') if (subDIVS >= 1) subDIVS--;

}

void mousePress(int b, int state, int x, int y)
{

	if (GLUT_LEFT_BUTTON == b && GLUT_DOWN == state)
	{
		
		B.performSelection(x, y);

		S.performSelection(x, y, HUDSelectOn);

	}

	if((GLUT_LEFT_BUTTON == b && GLUT_UP == state) || (GLUT_RIGHT_BUTTON == b && GLUT_UP == state))
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
