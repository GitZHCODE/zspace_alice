//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include <headers/zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;

#define M_PI 3.14159265358979323846  /* pi */

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool reset = false;
bool display = true;
bool toFile = false;
bool goNext = false;

bool solved = false;
int currentGraph = 0;
int numGraphs = 0;

string dir_in = "data/biArc/in_single/";
string dir_out = "data/biArc/out/";

double background = 0.35;

////////////////////////////////////////////////////////////////////////// zSpace Objects

/*!<model*/
zModel model;
zUtilsCore core;

zObjMesh oMesh;
zFnMeshDynamics fnDyMesh;

zObjGraphArray oGraphs;

zFnGraphDynamics fnDyGraph;

zIntArray edgeIds;
zIntArray fixedVertexIds;
zIntArray angleVertexIds;
zIntArray vectorVertexIds;
zIntArray startVertexIds;
zColorArray edgeCols;

//float dT = 0.01;
float dT = 0.5;
double tol_len = 0.001;
double tol_ang = 0.9999;

zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor YELLOW(1, 1, 0, 1);
zColor PURPLE(0.5, 0, 0.5, 1);
zColor CYAN(0, 1, 1, 1);
zColor ORANGE(1, 0.5, 0, 1);
zColor PINK(1, 0, 0.5, 1);

unordered_map <int, zIntArray> map_column;
unordered_map <int, zIntArray> map_fixedVertexIds;
unordered_map <int, zIntArray> map_angleVertexIds;

zPlane coPlane;

void makePair()
{
	int columnID;

	zItMeshHalfEdge he_start;
	zItMeshHalfEdge he_next;
	zItMeshHalfEdge he_walk;

	for (zItMeshHalfEdge he(oMesh); !he.end(); he++) {
		if (he.onBoundary()) {
			he_start = he;
			break;
		}
	}

	he_next = he_start;
	int counter_column = 0;

	do {
		zIntArray edgeIDs;

		he_walk = he_next.getSym().getNext();
		do {
			edgeIDs.push_back(he_walk.getEdge().getId());
			he_walk = he_walk.getNext().getSym().getNext();
		} while (!he_walk.onBoundary());

		//check each column and add all edge ids to map
		map_column[counter_column] = edgeIDs;

		counter_column ++;
		he_next = he_next.getNext();
	} while (he_next.getId() != he_start.getId());
}

void setupGraph(zObjGraph& oGraph)
{
	edgeIds.clear();
	angleVertexIds.clear();
	fixedVertexIds.clear();
	vectorVertexIds.clear();
	startVertexIds.clear();
	edgeCols.clear();

	model.addObject(oGraph);
	oGraph.setDisplayElements(true, true);

	if(currentGraph>0)
		fnDyGraph.clear();

	fnDyGraph.create(oGraph, true);

	//find start he
	int counter = 0;
	zItGraphHalfEdge he_start;
	for (zItGraphHalfEdge he(oGraph); !he.end(); he++)
	{
		if (he.getVertex().checkValency(1))
			he_start = he;
	}

	//add fix vertex ids to arr
	//fixedVertexIds.push_back(he_start.getSym().getVertex().getId());
	//fnDyGraph.setFixed(fixedVertexIds);

	//add vector forces
	vectorVertexIds.push_back(he_start.getSym().getVertex().getId());
	//vectorVertexIds_vertical.push_back(he_start.getSym().getNext().getVertex().getId());
	startVertexIds.push_back(he_start.getVertex().getId());

	//add edge ids to arr
	do {
		edgeIds.push_back(he_start.getEdge().getId());

		//add angle vertex ids to arr
		if (counter % 2 != 0)
			if (!he_start.getSym().getVertex().checkValency(1))
				angleVertexIds.push_back(he_start.getSym().getVertex().getId());
		counter++;
		he_start = he_start.getPrev();
	} while (!he_start.getVertex().checkValency(1));

	for (int i = 0; i < edgeIds.size(); i += 2)
	{
		zItGraphEdge e0(oGraph, edgeIds[i]);
		zItGraphEdge e1(oGraph, edgeIds[i + 1]);

		e0.setColor(edgeCols[i]);
		e1.setColor(edgeCols[i]);
	}

	cout << endl;
	for (auto& id : angleVertexIds)
		cout << id << ",";
}

void snapeToRadius(float& L,float& angle, float digits)
{
	float R = L * tan(angle / 2);
	float mul = 1.0f;
	mul = mul * 10 * digits;

	float R_round = round(R * mul) / mul;

	L = R_round / tan(angle / 2);

	cout << "R_round: " << R_round << endl;

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

	edgeCols.push_back(RED);
	edgeCols.push_back(BLUE);
	edgeCols.push_back(GREEN);
	edgeCols.push_back(YELLOW);
	edgeCols.push_back(PURPLE);
	edgeCols.push_back(CYAN);
	edgeCols.push_back(ORANGE);
	edgeCols.push_back(PINK);

	//zFnGraph fnGraph(oGraph);
	//fnGraph.from("data/biArc/testGraph.json", zJSON);
	
	zStringArray files;
	numGraphs = core.getNumfiles_Type(dir_in, zJSON);

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

	setupGraph(oGraphs[currentGraph]);

}

void update(int value)
{
	if (compute)
	{
		//-------------ADD FORCES---------------
		zObjGraph* oGraph(&oGraphs[currentGraph]);

		float restAngle = M_PI / 2;
		float meanLength;
		double dev_len;
		double dev_ang;

		solved = true;

		//add spring force
		for (int i = 0; i < edgeIds.size(); i+=2)
		{
			zItGraphEdge e0(*oGraph, edgeIds[i]);
			zItGraphEdge e1(*oGraph, edgeIds[i+1]);

			meanLength = (e0.getLength() + e1.getLength()) * 0.5;
			dev_len = abs(e0.getLength() - e1.getLength());

			float ang = e0.getVector().angle(e1.getVector());
			ang = 180 - ang;

			float rad = ang * M_PI / 180;
			cout << "angle: " << ang << endl;

			//snap minimum length to 1m
			if (meanLength < 1) meanLength = 1.0f;

			snapeToRadius(meanLength, rad, 1);
			cout << "meanLength: " << meanLength << endl;


			if (dev_len > tol_len)
			{
				fnDyGraph.addSpringForce(1.0, e0.getId(), meanLength);
				fnDyGraph.addSpringForce(1.0, e1.getId(), meanLength);

				solved = false;
			}

			cout << endl;
			cout << "dev_len_" << i << ":" << dev_len << endl;

		}

		//add angle force
		zPoint* vPositions = fnDyGraph.getRawVertexPositions();

		for (int i = 0; i < angleVertexIds.size(); i++)
		{
			int id = angleVertexIds[i];
			zItGraphVertex v(*oGraph, id);

			zVector vec0, vec1;
			zIntArray cvs;
			v.getConnectedVertices(cvs);
			vec0 = vPositions[cvs[0]] - *v.getRawPosition();
			vec1 = vPositions[cvs[1]] - *v.getRawPosition();

			vec0.normalize();
			vec1.normalize();

			dev_ang = abs(vec0 * vec1);

			if (dev_ang < tol_ang)
			{
				fnDyGraph.addAngleForce(1.0, id, restAngle, true);
				solved = false;
			}
			
			cout << endl;
			cout << "dev_ang_" << i << ":" << dev_ang << endl;
		}

		//add vector forces
		for (int i = 0; i < vectorVertexIds.size(); i++)
		{
			int id = vectorVertexIds[i];
			zVector alignVector(0, 0, 1);
			fnDyGraph.addVectorForce(1, id, vPositions[startVertexIds[0]], alignVector);
		}

		cout << "------------" << endl;


		fnDyGraph.update(dT, zEuler, true, true, true);

		/*
		if solved stop computing;
		export solved graph to file;
		go next;
		*/
		if (solved)
		{
			toFile = !toFile;
			solved = !solved;
			compute = !compute;

			if (currentGraph < numGraphs - 1)
				goNext = !goNext;
		}
	}

	if (reset)
	{
		//oGraph_copy = oGraph;

		reset = !reset;
	}

	if (toFile)
	{
		zFnGraph fn(oGraphs[currentGraph]);
		fn.to(dir_out + "out_" + to_string(currentGraph) + ".json", zJSON);

		toFile = !toFile;
	}

	if (goNext)
	{
		currentGraph++;

		setupGraph(oGraphs[currentGraph]);
		compute = !compute;
		goNext = !goNext;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	zPoint* vPositions = fnDyGraph.getRawVertexPositions();

		for (int i = 0; i < fnDyGraph.numVertices(); i++){
			bool angleV = std::find(std::begin(angleVertexIds), std::end(angleVertexIds), i) != std::end(angleVertexIds);
			bool fixedV = std::find(std::begin(fixedVertexIds), std::end(fixedVertexIds), i) != std::end(fixedVertexIds);
			bool alignedV = std::find(std::begin(vectorVertexIds), std::end(vectorVertexIds), i) != std::end(vectorVertexIds);
				
			if (angleV)
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 0, 1, 1), 5);
			else if (fixedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(1, 0, 0, 1), 5);
			else if (alignedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(0, 1, 0, 1), 5);
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
	drawString("Current graph #:" + to_string(currentGraph) + "/" + to_string(numGraphs - 1), vec(winW - 350, winH - 500, 0));

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
