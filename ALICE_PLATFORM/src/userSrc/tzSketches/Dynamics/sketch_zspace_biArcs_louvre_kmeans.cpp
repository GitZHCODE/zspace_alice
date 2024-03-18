//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include <headers/zApp/include/zTsGeometry.h>

#include<headers/zToolsets/data/zTsKMeans.h>

using namespace zSpace;
using namespace std;

#define M_PI 3.14159265358979323846  /* pi */
#define DEBUGGER if (0) cout

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool reset = false;
bool display = true;
bool toFile = false;
bool goNext = false;

bool solved = false;
int counterIter = 0;
int counterKmeansIter = 0;

int currentGraph = 0;
int numGraphs = 0;

string dir_in = "data/biArc/louvres/in/";
string dir_out = "data/biArc/louvres/out/";

double background = 0.35;

////////////////////////////////////////////////////////////////////////// zSpace Objects

/*!<model*/
zModel model;
zUtilsCore core;

/*!<biArc*/
zObjMesh oMesh;
zFnMeshDynamics fnDyMesh;

zObjGraphArray oGraphs;

zFnGraphDynamics fnDyGraph;
vector<zFnGraphDynamics> fnDyGraphs;

vector<zIntArray> edgeIds;
vector<zIntArray> fixedVertexIds;
vector<zIntArray> nodeVertexIds;
vector<zIntArray> arcVertexIds;
vector<zIntArray> vectorVertexIds;
vector<zIntArray> startVertexIds;
vector<zIntArray> endVertexIds;
vector<zColorArray> edgeCols;

//float dT = 0.01;
float dT = 1.0;
double tol_len = 0.001;
double tol_ang = 0.9999;
float tol_kdev_r = 0.05;
float tol_kdev_cluster = 1.0;

double dev_len_max;
double dev_ang_max;
vector<double> dev_lens;
vector<double> dev_angs;

zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor YELLOW(1, 1, 0, 1);
zColor PURPLE(0.5, 0, 0.5, 1);
zColor CYAN(0, 1, 1, 1);
zColor ORANGE(1, 0.5, 0, 1);
zColor PINK(1, 0, 0.5, 1);

/*!<kmeans*/
Eigen::MatrixXf angleData;
Eigen::MatrixXf radiusData; 
Eigen::MatrixXf chordData; 
Eigen::MatrixXf kmeansCentroids;

int numClusters = 40;
int numKmeansIter = 200;
int kmeansSpacing = 5;
zTsKMeans myKMeans;


void setupGraph(zObjGraph& oGraph, int id)
{
	model.addObject(oGraph);
	oGraph.setDisplayElements(false, true);

	fnDyGraphs[id].create(oGraph, true);
	fnDyGraphs[id].setEdgeWeight(2);

	//find start he
	int counter = 0;
	zItGraphHalfEdge he_start;

	for (zItGraphHalfEdge he(oGraph); !he.end(); he++)
	{
		if (he.getVertex().checkValency(1))
		{
			he_start = he;

			//add vector forces
			vectorVertexIds[id].push_back(he_start.getSym().getVertex().getId());
			startVertexIds[id].push_back(he_start.getSym().getVertex().getId());
			endVertexIds[id].push_back(he_start.getVertex().getId());

			//add fix vertex ids to arr
			fixedVertexIds[id].push_back(he_start.getVertex().getId());
		}
	}

	//for (zItGraphHalfEdge he(oGraph); !he.end(); he++)
	//{
	//	if (he.getVertex().checkValency(1))
	//	{
	//		he_start = he;

	//		//add vector forces
	//		vectorVertexIds[id].push_back(he_start.getSym().getNext().getVertex().getId());
	//		startVertexIds[id].push_back(he_start.getSym().getVertex().getId());
	//		endVertexIds[id].push_back(he_start.getVertex().getId());

	//		//add fix vertex ids to arr
	//		fixedVertexIds[id].push_back(he_start.getSym().getVertex().getId());
	//	}
	//}


	fnDyGraphs[id].setFixed(fixedVertexIds[id]);



	//add edge ids to arr
	he_start = he_start.getPrev();
	nodeVertexIds[id].push_back(he_start.getVertex().getId());

	do {
		edgeIds[id].push_back(he_start.getEdge().getId());

		//add angle vertex ids to arr
		if (counter % 2 != 0)
			{
			if(!he_start.getVertex().checkValency(1))
				arcVertexIds[id].push_back(he_start.getVertex().getId());
			if (!he_start.getSym().getVertex().checkValency(1))
					nodeVertexIds[id].push_back(he_start.getSym().getVertex().getId());
			}
		counter++;
		he_start = he_start.getPrev();
	} while (!he_start.getVertex().checkValency(1)/* && myCounter<2*/);

	for (int i = 0; i < arcVertexIds[id].size(); i++)
	{
		zItGraphVertex v(oGraph, arcVertexIds[id][i]);
		zIntArray ces;
		v.getConnectedEdges(ces);

		zItGraphEdge e0(oGraph, ces[0]);
		zItGraphEdge e1(oGraph, ces[1]);

		e0.setColor(edgeCols[id][i]);
		e1.setColor(edgeCols[id][i]);
	}

	fnDyGraphs[id].setFixed(nodeVertexIds[id]);

	//DEBUGGER_K << endl;
	//for (auto& id : arcVertexIds[id])
	//	DEBUGGER_K << id << ",";
}

void snapeToRadius(float& L, float& ang,float&outRadius, float digits)
{
	double R; //radius
	double half_ang = ang * 0.5;
	double mul = 1.0;
	double R_round;

	if (abs(half_ang) < 1e-6)
	{
		R = L;

		mul = mul * 10 * digits;
		R_round = round(R * mul) / mul;

		//update mean length
		L = round(L * mul) / mul;

		//update angle
		half_ang = 1e-6;
		R_round = -1;
	}
	else if (abs(half_ang) > (0.5 * M_PI - 1e-6))
	{

		R = L;

		mul = mul * 10 * digits;
		R_round = round(R * mul) / mul;

		//update mean length
		L = round(L * mul) / mul;

		//update angle
		half_ang = 0.5 * M_PI - 1e-6;
		R_round = -1;

	}
	else
	{
		R = tan(half_ang) * L;

		mul = mul * 10 * digits;
		R_round = round(R * mul) / mul;

		//update mean length
		L = (float)(R_round / tan(half_ang));

		//update angle
		half_ang = atan(R_round / L);

		DEBUGGER << "tangent: " << tan(half_ang) << endl;
		DEBUGGER << "L: " << L << endl;

	}

	ang = 2 * half_ang;
	outRadius = (float)R_round;

}

void setup()
{
	{
		////////////////////////////////////////////////////////////////////////// Enable smooth display

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_POINT_SMOOTH);

		////////////////////////////////////////////////////////////////////////// Sliders

		S = *new SliderGroup();

		S.addSlider(&background, "background");
		S.sliders[0].attachToVariable(&background, 0, 1);

		////////////////////////////////////////////////////////////////////////// Buttons

		B = *new ButtonGroup(vec(50, 450, 0));

		B.addButton(&compute, "compute");
		B.buttons[0].attachToVariable(&compute);
		B.addButton(&reset, "reset");
		B.buttons[1].attachToVariable(&reset);
		B.addButton(&display, "display");
		B.buttons[2].attachToVariable(&display);
		B.addButton(&toFile, "toFile");
		B.buttons[3].attachToVariable(&toFile);
	}

	//---------------------------
	model = zModel(100000);



	//zFnGraph fnGraph(oGraph);
	//fnGraph.from("data/biArc/testGraph.json", zJSON);
	
	zStringArray files;
	numGraphs = core.getNumfiles_Type(dir_in, zJSON);
	//numGraphs = 2;

	if (numGraphs != 0)
	{
		core.getFilesFromDirectory(files, dir_in, zJSON);
		oGraphs.assign(numGraphs, zObjGraph());
	}

	for (int i = 0; i < numGraphs; i++)
	{
		string file = files[i];
		zFnGraph fnGraph(oGraphs[i]);
		fnGraph.from(file, zJSON);
	}

	edgeIds.assign(numGraphs, zIntArray());
	fixedVertexIds.assign(numGraphs, zIntArray());
	nodeVertexIds.assign(numGraphs, zIntArray()); 
	arcVertexIds.assign(numGraphs, zIntArray());
	vectorVertexIds.assign(numGraphs, zIntArray());
	startVertexIds.assign(numGraphs, zIntArray());
	endVertexIds.assign(numGraphs, zIntArray());
	edgeCols.assign(numGraphs, zColorArray());
	fnDyGraphs.assign(numGraphs, zFnGraphDynamics());

	for (int i = 0; i < numGraphs; i++)
	{
		edgeCols[i].push_back(RED);
		edgeCols[i].push_back(BLUE);
		edgeCols[i].push_back(GREEN);
		edgeCols[i].push_back(YELLOW);
		edgeCols[i].push_back(PURPLE);
		edgeCols[i].push_back(CYAN);
		edgeCols[i].push_back(ORANGE);
		edgeCols[i].push_back(PINK);

		setupGraph(oGraphs[i], i);
	}

	//---------------------------------KMEANS----------------------------------
	//int size = nodeVertexIds.size() * nodeVertexIds[0].size();
	int counter = nodeVertexIds.size() * nodeVertexIds[0].size();
	int stride = 1;

	angleData = MatrixXf(counter, stride);
	radiusData = MatrixXf(counter, stride);
	chordData = MatrixXf(counter, stride);
	//---------------------------------KMEANS----------------------------------
}

void update(int value)
{
	if (compute)
	{

		//-------------ADD FORCES---------------
		zObjGraph* oGraph(&oGraphs[currentGraph]);

		DEBUGGER << endl << "currentGraph:" << currentGraph << endl;

		float restAngle = M_PI;
		float meanLength;
		double dev_len;
		double dev_ang;

		solved = true;

		//add arc forces
		for (int i = 0; i < arcVertexIds[currentGraph].size(); i++)
		{
			int id = arcVertexIds[currentGraph][i];
			zItGraphVertex v(*oGraph, id);
			zItGraphHalfEdgeArray cHEdges;
			v.getConnectedHalfEdges(cHEdges);

			zVector he_0 = cHEdges[0].getVector();
			zVector he_1 = cHEdges[1].getVector();

			meanLength = (he_0.length() + he_1.length()) * 0.5;
			dev_len = abs(he_0.length() - he_1.length());

			//snap minimum length to 1m
			if (meanLength < 1) meanLength = 1.0f;

			DEBUGGER << endl;
			DEBUGGER << "dev_len_" << i << ":" << dev_len << endl;

			//180 degree force
			zPoint* vPositions = fnDyGraphs[currentGraph].getRawVertexPositions();
			zVector vec0, vec1;
			zIntArray cvs;
			v.getConnectedVertices(cvs);
			vec0 = vPositions[cvs[0]] - *v.getRawPosition();
			vec1 = vPositions[cvs[1]] - *v.getRawPosition();

			vec0.normalize();
			vec1.normalize();

			dev_ang = abs(vec0 * vec1);

			dev_lens.push_back(dev_len);

			if (dev_len > tol_len)
			{
				fnDyGraphs[currentGraph].addSpringForce(0.8, cHEdges[0].getEdge().getId(), meanLength);
				fnDyGraphs[currentGraph].addSpringForce(0.8, cHEdges[1].getEdge().getId(), meanLength);

				solved = false;
			}
		}


		//add node force
		zPoint* vPositions = fnDyGraphs[currentGraph].getRawVertexPositions();
		for (int i = 0; i < nodeVertexIds[currentGraph].size(); i++)
		{
			int id = nodeVertexIds[currentGraph][i];
			zItGraphVertex v(*oGraph, id);

			zVector vec0, vec1;
			zIntArray cvs;
			v.getConnectedVertices(cvs);
			vec0 = vPositions[cvs[0]] - *v.getRawPosition();
			vec1 = vPositions[cvs[1]] - *v.getRawPosition();

			vec0.normalize();
			vec1.normalize();

			dev_ang = abs(vec0 * vec1);
			dev_angs.push_back(dev_ang);

			if (dev_ang < tol_ang)
			{
				fnDyGraphs[currentGraph].addAngleForce(1.0, id, restAngle, false);
				solved = false;
			}
			
			DEBUGGER << endl;
			DEBUGGER << "dev_ang_" << i << ":" << dev_ang << endl;
		}

		//add vector forces
		for (int i = 0; i < vectorVertexIds[currentGraph].size(); i++)
		{
			int id = vectorVertexIds[currentGraph][i];

			int id_start = startVertexIds[currentGraph][i];
			int id_end = endVertexIds[currentGraph][i];

			zItGraphVertex v_start(*oGraph, id_start);
			zItGraphVertex v_end(*oGraph, id_end);

			zVector alignVector(v_end.getPosition() - v_start.getPosition());
			alignVector *= -1;

			fnDyGraphs[currentGraph].addVectorForce(1.0, id, vPositions[id_end], alignVector);
		}

		DEBUGGER << "------------" << endl;

		fnDyGraphs[currentGraph].update(dT, zEuler, true, true, true);

		compute = !compute;
		goNext = !goNext;

		//check deviation at the end of each iteration
		if (currentGraph % numGraphs == 0)
		{
			dev_lens.clear();
			dev_angs.clear();
		}
		if (currentGraph == numGraphs - 1)
		{
			dev_len_max = core.zMax(dev_lens);
			dev_ang_max = core.zMin(dev_angs);
		}

		/* if solved stop computing;
		export solved graph to file;
		go next;*/
		if (solved && counterIter > 0 && currentGraph == numGraphs - 1)
		{
			toFile = !toFile;
			compute = !compute;
		}
	}

	if (reset)
	{
		//oGraph_copy = oGraph;

		reset = !reset;
	}

	if (toFile)
	{
		//zFnGraph fn(oGraphs[currentGraph]);
		//fn.to(dir_out + "out_" + to_string(currentGraph) + ".json", zJSON);

		for (int i = 0; i < numGraphs; i++)
		{
			zFnGraph fn(oGraphs[i]);
			fn.to(dir_out + "out_" + to_string(i) + ".json", zJSON);
		}

		toFile = !toFile;
	}

	if (goNext)
	{
		currentGraph++;

		if (currentGraph > numGraphs - 1)
		{
			currentGraph = 0;
			counterIter++;
			solved = true;
		}

		//setupGraph(oGraphs[currentGraph]);
		compute = !compute;
		goNext = !goNext;
	}
}

void draw()
{
	//glEnable(GL_MULTISAMPLE); //anti-aliasing

	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	zPoint* vPositions = fnDyGraphs[currentGraph].getRawVertexPositions();

		for (int i = 0; i < fnDyGraphs[currentGraph].numVertices(); i++){
			bool nodeV = std::find(std::begin(nodeVertexIds[currentGraph]), std::end(nodeVertexIds[currentGraph]), i) != std::end(nodeVertexIds[currentGraph]);
			bool fixedV = std::find(std::begin(fixedVertexIds[currentGraph]), std::end(fixedVertexIds[currentGraph]), i) != std::end(fixedVertexIds[currentGraph]);
			bool alignedV = std::find(std::begin(vectorVertexIds[currentGraph]), std::end(vectorVertexIds[currentGraph]), i) != std::end(vectorVertexIds[currentGraph]);
			bool angleV = std::find(std::begin(arcVertexIds[currentGraph]), std::end(arcVertexIds[currentGraph]), i) != std::end(arcVertexIds[currentGraph]);

			if (nodeV)
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 0, 1, 1), 5);
			else if (fixedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(1, 0, 0, 1), 5);
			else if (alignedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 1, 0, 1), 5);
			else if (angleV)
				model.displayUtils.drawPoint(vPositions[i], zColor(1, 1, 0, 1), 5);
			else
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 0, 0, 1), 5);
		}


	if (display)
	{
		
	}

	//////////////////////////////////////////////////////////
	model.draw();

	setup2d();

	glColor3f(0, 0, 0);

	if (solved && counterIter > 0 && currentGraph == numGraphs - 1)
		drawString("Solved!", vec(winW - 350, winH - 450, 0));

	drawString("Current graph #:" + to_string(currentGraph) + "/" + to_string(numGraphs - 1), vec(winW - 350, winH - 475, 0));
	//drawString("Current num clusters #:" + to_string(myKMeans.numClusters), vec(winW - 350, winH - 500, 0));
	drawString("Iterations:" + to_string(counterIter), vec(winW - 350, winH - 525, 0));

	drawString("Dev_len_max:" + to_string(dev_len_max), vec(winW - 350, winH - 550, 0));
	drawString("Dev_ang_max:" + to_string(dev_ang_max), vec(winW - 350, winH - 575, 0));





	restore3d();
	
}

void keyPress(unsigned char k, int xm, int ym)
{

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
