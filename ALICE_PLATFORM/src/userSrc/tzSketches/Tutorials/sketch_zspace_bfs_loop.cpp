//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjGraph graph;

//define container of output halfedge ids


////// --- GUI OBJECTS ----------------------------------------------------

int bfs_loop_detect(zObjGraph& graph) {
	std::set<int> visited;
	vector<bool> onStack;
	int loopCount = 0;

	zFnGraph fnGraph(graph);
	int numVertices = fnGraph.numVertices();
	onStack.resize(numVertices, false);

	zItGraphVertex v(graph);
	for (; !v.end(); v++) {
		int startVertex = v.getId();

		if (visited.find(startVertex) == visited.end()) {
			std::queue<std::pair<int, int>> q;  // pair<vertexId, parent>
			q.push({ startVertex, -1 });
			visited.insert(startVertex);

			while (!q.empty()) {
				int currentVertex = q.front().first;
				int parent = q.front().second;
				q.pop();

				onStack[currentVertex] = true;

				zIntArray neighbors;
				v.getConnectedVertices(neighbors);
				for (auto& neighbor : neighbors) {
					if (neighbor != parent) {
						if (visited.find(neighbor) == visited.end()) {
							visited.insert(neighbor);
							q.push({ neighbor, currentVertex });
						}
						else if (onStack[neighbor]) {
							//detected
							loopCount++;
						}
					}
				}

				onStack[currentVertex] = false;
			}
		}
	}

	return loopCount / 2;
}

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
	zPointArray pos = {
		zPoint(0, 0, 0),
		zPoint(1, 0, 0),
		zPoint(1, 1, 0),
		zPoint(0, 1, 0),

		zPoint(2, 0, 0),
		zPoint(2, -1, 0),
		zPoint(3, -1, 0),
		zPoint(3, 0, 0),
		zPoint(2, -2, 0),

	};

	zIntArray pConnects = {
		0,1,
		1,2,
		2,3,
		3,0,
		1,4,

		4,5,
		5,6,
		6,7,
		7,4,
		5,8
	};

	zFnGraph fnGraph(graph);
	fnGraph.create(pos, pConnects);

	auto begin = std::chrono::high_resolution_clock::now();
	double computeTime = 0;

	int numLoops = bfs_loop_detect(graph);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	//printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);
	computeTime = elapsed.count() * 1e-9;

	cout << "\n" <<  numLoops <<" loops detected within " <<computeTime << " seconds" << "\n";

	model.addObject(graph);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	graph.setDisplayElements(true, true);

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


		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	// zspace model draw
	model.draw();

	if (display)
	{

		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;	
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
