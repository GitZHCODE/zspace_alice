#define _MAIN_

#ifdef _MAIN_

#include "main.h"
#include <vector>

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool edgeSelection = false;
bool faceSelection = false;
bool vertexSelection = false;
bool faceSelectionFromVertex = false;
bool display = true;
bool myButton = false;

double background = 0.35;

string mystring;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMesh oMesh;

zItMeshVertexArray currentSet;
zItMeshVertexArray newSet;
zItMeshVertexArray currentVSet;
zItMeshVertexArray currentFSet;

int startVertexID = 13;

int startEdge_v0 = 16;
int startEdge_v1 = 17;

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

	// read mesh

	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/TestData/alicetest3.obj", zOBJ); // zJSON , zUSD , zCSV


	zItMeshVertex v(oMesh, startVertexID);
	currentSet.push_back(v);
	currentFSet.push_back(v);
	currentVSet.push_back(v);

	zItMeshHalfEdge start_he;
	bool chk = fnMesh.halfEdgeExists(startEdge_v0, startEdge_v1, start_he);

	zColor myRGB(1, 0, 1, 1); // rgba
	zColor myHSV(300, 1, 1); // hsv

	zItMeshHalfEdge he_V;
	zItMeshHalfEdgeArray cHEdges;

	zItMeshVertex v0(oMesh, startEdge_v0);
	v0.getConnectedHalfEdges(cHEdges);

	for (zItMeshHalfEdge cHE : cHEdges)
	{
		if (!cHE.getEdge().onBoundary()) he_V = cHE;
	}



	//// U Loop	
	//he_V.getEdge().setColor(myRGB);

	//	
	//int boundaryCount = 0;
	//while (boundaryCount < 2) {

	//	
	//	zItMeshHalfEdge start_he = he_V.getPrev();
	//	//he_V.getEdge().setColor(myRGB);

	//	if (start_he.onBoundary())
	//	{
	//		zItMeshHalfEdge he_U = start_he;

	//		do
	//		{
	//			he_U.getEdge().setColor(myRGB);
	//			he_U = he_U.getNext();

	//		} while (he_U != start_he);
	//	}
	//	else
	//	{

	//		zItMeshHalfEdge he_U = start_he;

	//		do
	//		{
	//			he_U.getEdge().setColor(myRGB);
	//			he_U = he_U.getNext().getSym().getNext();

	//		} while (he_U != start_he);
	//	}

	//	/////


	//	he_V = he_V.getNext().getSym().getNext();

	//	if (he_V.getStartVertex().onBoundary()) {
	//		boundaryCount++;
	//	}

	//}



	if (chk) {
		//// U Loop	
		//he_V.getEdge().setColor(myRGB);
		//int boundaryCount = 0;
		//while (boundaryCount < 2) {


		//	zItMeshHalfEdge start_he = he_V.getPrev();
		//	//he_V.getEdge().setColor(myRGB);

		//	if (start_he.onBoundary())
		//	{
		//		zItMeshHalfEdge he_U = start_he;

		//		do
		//		{
		//			he_U.getEdge().setColor(myRGB);
		//			he_U = he_U.getNext();

		//		} while (he_U != start_he);
		//	}
		//	else
		//	{

		//		zItMeshHalfEdge he_U = start_he;

		//		do
		//		{
		//			he_U.getEdge().setColor(myRGB);
		//			he_U = he_U.getNext().getSym().getNext();

		//		} while (he_U != start_he);
		//	}

		//	/////


		//	he_V = he_V.getNext().getSym().getNext();

		//	if (he_V.getStartVertex().onBoundary()) {
		//		boundaryCount++;
		//	}

		//}

		//// V Loop	
		//zItMeshHalfEdge start_he = he_V.getPrev();
		//zItMeshHalfEdge he_U = start_he.getSym();

		//int boundaryCount = 0;
		//do {
		//	do {


		//		he_V.getEdge().setColor(myRGB);
		//		he_V = he_V.getNext().getSym().getNext();

		//		if (he_V.getVertex().onBoundary()) {
		//			boundaryCount++;
		//		}
		//	} while (boundaryCount < 2);
		//	boundaryCount = 0;
		//	he_U = he_U.getNext();
		//	he_V = he_U.getSym().getNext();
		//} while (he_U != start_he.getSym());






	}


	

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh); // add the aobjects you want display

	// set display element booleans
	oMesh.setDisplayElements(false, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0.0, 1.0); // has to be a double 

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	B.addButton(&myButton, "myButton");
	B.buttons[2].attachToVariable(&myButton); // has to be a boolean

}

void update(int value)
{
	bool isFirstIteration = true;
	// At first iteration, we have to go through current set
	// after that we can always go with the newSet and keep going
	// for first iteration, we need to set a boolean to make sure that
	// we use it just once
	zItMeshVertexArray newSetThisIteration;
	zItMeshVertexArray currentIterationSet; 
	
	if (isFirstIteration)
	{
		currentIterationSet = currentSet;  // Use 'currentSet' for the first iteration
		isFirstIteration = false;  // Set flag to false after first iteration
	}
	else
	{
		currentIterationSet = newSet;  // Use 'newSet' for subsequent iterations
	}
	
	if (edgeSelection)
	{
		int currentSize = currentSet.size();
		int precision = 0;

		for (int i = 0; i < currentSize; i++)
		{
			zItMeshHalfEdgeArray cHEdges;
			vector<int> currentSetIDs;
			// Gets the edges and stores it in cHEdges
			currentIterationSet[i].getConnectedHalfEdges(cHEdges);
			zColor myRGB(1, 0, 1, 1); // rgba
			zColor myHSV(300, 1, 1); // hsv
			int index;
			// REMOVE DUPLICATES USING ZSPACE NAMESPACE
			for (auto& cHE : cHEdges) // for each loop
			{

				cHE.getEdge().setColor(myHSV);
				zItMeshVertex eV = cHE.getVertex();
				int index = 0;

				for (int j = 0; j < currentSet.size(); j++) {
					currentSetIDs.push_back(currentSet[j].getId());
				}
				int vertexIndex = eV.getId();

				if (!core.checkRepeatElement(vertexIndex, currentSetIDs, index, precision)) {
					currentSet.push_back(eV);
					newSetThisIteration.push_back(eV);
				}
			}

			// REMOVE DUPLICATES USING STD NAMESPACE
			//for (auto& cHE : cHEdges) // for each loop
			//{
			//  //printf("%i ", cHE.getId());
			//  cHE.getEdge().setColor(myHSV);
			//  zItMeshVertex eV = cHE.getVertex();
			//  eV.getId();
			//  bool isPresent = false;
			//  for (int j = 0; j < currentSet.size(); j++) {
			//      if (currentSet[j].getId() == eV.getId()) {
			//          isPresent = true;
			//      }
			//  }
			//  if (!isPresent) {
			//      currentSet.push_back(eV); // only add unique ids... coreutils
			//      printf("\n pushback currentSet Size %i ", currentSet.size());
			//  }
			//}

		}
		//printf("\n currentSet Size %i ", currentSet.size());
		cout << "\ncurrentSet Size " << currentSet.size() << " " << "newSet size" << newSetThisIteration.size();
		newSet = newSetThisIteration;
		newSetThisIteration.clear();
		edgeSelection = !edgeSelection;  // boolean toggle 
		
	}

	set<int> uniqueFaceIDs;
	zColor currentColor(300, 1, 1); // hsv
	bool useMagenta = true;

	//if (faceSelectionFromVertex)
	//{
	//	

	//	int currentSize = currentSet.size();
	//	int precision = 0;
	//	zItMeshHalfEdgeArray cHEdges;

	//	for (int i = 0; i < currentSize; i++)
	//	{
	//		vector<int> currentSetIDs;
	//		// Gets the edges and stores them in cHEdges
	//		currentIterationSet[i].getConnectedHalfEdges(cHEdges);

	//		for (auto& cHE : cHEdges)
	//		{
	//			zItMeshVertex eV = cHE.getVertex();
	//			// Retrieve faces connected to this vertex
	//			zItMeshFaceArray connectedFaces;
	//			eV.getConnectedFaces(connectedFaces);

	//			// Check and store unique faces
	//			for (auto& face : connectedFaces)
	//			{
	//				int faceID = face.getId();
	//				if (uniqueFaceIDs.find(faceID) == uniqueFaceIDs.end())  // If the face is not already stored
	//				{
	//					uniqueFaceIDs.insert(faceID);
	//					 
	//					face.setColor(currentColor);
	//				}
	//			}

	//			for (int j = 0; j < currentSet.size(); j++)
	//			{
	//				currentSetIDs.push_back(currentSet[j].getId());
	//			}
	//			int index = 0;
	//			int vertexIndex = eV.getId();

	//			if (!core.checkRepeatElement(vertexIndex, currentSetIDs, index, precision))
	//			{
	//				currentSet.push_back(eV);
	//				newSetThisIteration.push_back(eV);
	//			}
	//		}
	//		
	//	}

	//	cout << "\ncurrentSet Size " << currentSet.size() << " " << "newSet size" << newSetThisIteration.size();
	//	newSet = newSetThisIteration;
	//	newSetThisIteration.clear();
	//	faceSelectionFromVertex = !faceSelectionFromVertex;  // boolean toggle 
	//	
	//	
	//}

	//if (faceSelectionFromVertex)
	//{
	//	std::queue<zItMeshVertex> bfsQueue;
	//	for (auto& elem : currentSet) bfsQueue.push(elem);  // Initialize the queue with the starting set

	//	while (!bfsQueue.empty())
	//	{
	//		zItMeshVertex currentVertex = bfsQueue.front();  // Get the current vertex
	//		bfsQueue.pop();

	//		vector<int> currentSetIDs;

	//		zItMeshHalfEdgeArray cHEdges;
	//		currentVertex.getConnectedHalfEdges(cHEdges);

	//		for (auto& cHE : cHEdges)
	//		{
	//			zItMeshVertex eV = cHE.getVertex();

	//			zItMeshFaceArray connectedFaces;
	//			eV.getConnectedFaces(connectedFaces);

	//			for (auto& face : connectedFaces)
	//			{
	//				int faceID = face.getId();
	//				if (uniqueFaceIDs.find(faceID) == uniqueFaceIDs.end())
	//				{
	//					uniqueFaceIDs.insert(faceID);
	//					face.setColor(currentColor);
	//				}
	//			}

	//			int vertexIndex = eV.getId();

	//			// If this vertex hasn't been processed yet
	//			if (uniqueFaceIDs.find(vertexIndex) == uniqueFaceIDs.end())
	//			{
	//				bfsQueue.push(eV);  // enqueue this vertex for BFS
	//				uniqueFaceIDs.insert(vertexIndex);  // mark this vertex as processed (or visited)
	//			}
	//		}
	//	}

	//	faceSelectionFromVertex = !faceSelectionFromVertex;
	//}
	

ItMesh mesh(meshObj);

set<int> coloredFaces;
set<int> visitedVertices;

const int BFS_DEPTH_LIMIT = 2; // Adjust as needed based on your mesh's structure

// We now have a BFS queue that holds both vertex pointer and its BFS depth
queue<pair<zItMeshVertex*, int>> bfsQueue;

// Initialize BFS queue with the current vertices and depth 0
for (int id : currentSet)
{
	zItMeshVertex v(mesh, id);
	bfsQueue.push({ &v, 0 });
}

while (!bfsQueue.empty())
{
	zItMeshVertex* currentVertex = bfsQueue.front().first;
	int currentDepth = bfsQueue.front().second;
	bfsQueue.pop();

	if (visitedVertices.find(currentVertex->getId()) != visitedVertices.end())
	{
		continue; // Vertex already processed, skip
	}
	visitedVertices.insert(currentVertex->getId());

	// Don't expand beyond the BFS depth limit
	if (currentDepth >= BFS_DEPTH_LIMIT)
	{
		continue;
	}

	// Get connected faces and color them
	vector<zItMeshFace> cFaces = currentVertex->getConnectedFaces();
	cout << "Vertex " << currentVertex->getId() << " has " << cFaces.size() << " connected faces." << endl;

	for (auto& f : cFaces)
	{
		if (coloredFaces.find(f.getId()) == coloredFaces.end())
		{
			cout << "Coloring face with ID: " << f.getId() << endl;
			f.setColor(zColor(1, 0, 0, 1));
			coloredFaces.insert(f.getId());
		}
		else
		{
			cout << "Face with ID " << f.getId() << " already colored." << endl;
		}
	}

	// For each edge of the current vertex, get the other vertex and enqueue it for BFS
	vector<zItMeshHalfEdge> cEdges = currentVertex->getConnectedHalfEdges();
	for (auto& e : cEdges)
	{
		zItMeshVertex eV = e.getStartVertex();
		if (currentVertex->getId() == eV.getId())
		{
			eV = e.getSym().getStartVertex();
		}

		// If this vertex hasn't been visited, enqueue it with incremented BFS depth
		if (visitedVertices.find(eV.getId()) == visitedVertices.end())
		{
			bfsQueue.push({ &eV, currentDepth + 1 });
		}
	}
}


	if (faceSelection)
	{
		int currentSize = currentFSet.size();
		int precision = 0;
		vector<zItMeshFace> uniqueFaces;
		vector<int> uniqueVertexIDs;
		vector<int> uniqueFaceIDs;


		for (int i = 0; i < currentSize; i++)
		{

			zItMeshFaceArray cFaces;
			vector<int> currentSetIDs;

			// Gets the faces and stores it in cFaces
			currentFSet[i].getConnectedFaces(cFaces);

			int index;

			// REMOVE DUPLICATES USING ZSPACE NAMESPACE

			for (auto& cF : cFaces)
			{
				cF.setColor(zColor(300, 1, 1));

				int faceIndex = cF.getId();

				if (!core.checkRepeatElement(faceIndex, currentSetIDs, index, precision))
				{
					uniqueFaceIDs.push_back(faceIndex);
					
					zItMeshVertexArray vertexGrowth;
					cF.getVertices(vertexGrowth);
					for (auto& vertex : vertexGrowth)
					{
						int index1;
						int vertexIndex = vertex.getId();
						// Check if the vertex ID is unique using checkRepeatElement
						if (!core.checkRepeatElement(vertexIndex, uniqueVertexIDs, index1, precision))
						{
							uniqueVertexIDs.push_back(vertexIndex); // Add the vertex ID to the set
							currentFSet.push_back(vertex);
							
						}
					}
					//printf("\n cFaces Size %i ", cFaces.size());
				}
			}
			
		}
		
		faceSelection = !faceSelection;	 // boolean toggle 
		
	}
	if (vertexSelection)
	{
		int currentSize = currentVSet.size();
		int precision = 0;


		for (int i = 0; i < currentSize; i++)
		{
			zItMeshHalfEdgeArray cHEdges;
			vector<int> currentSetIDs;

			// Gets the edges and stores it in cHEdges
			currentVSet[i].getConnectedHalfEdges(cHEdges);

			
			zColor myRGB(1, 0, 1, 1); // rgba
			zColor myHSV(300, 1, 1); // hsv
			int index; 

			// REMOVE DUPLICATES USING ZSPACE NAMESPACE

			for (auto& cHE : cHEdges) // for each loop
			{
				//printf("%i ", cHE.getId());
				cHE.getVertex().setColor(myHSV);

				zItMeshVertex eV = cHE.getVertex();
				int index = 0;

				for (int j = 0; j < currentVSet.size(); j++) {
					currentSetIDs.push_back(currentVSet[j].getId());
				}
				int vertexIndex = eV.getId();

				if (!core.checkRepeatElement(vertexIndex, currentSetIDs, index, precision)) {
					currentVSet.push_back(eV);
				}
			}
		}
		printf("\n currentVSet Size %i ", currentVSet.size());

		vertexSelection = !vertexSelection;	 // boolean toggle 

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

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'd' || k == 'D') edgeSelection = true;
	if (k == 'f' || k == 'F') faceSelection = true;
	if (k == 'v' || k == 'V') vertexSelection = true;
	if (k == 'g' || k == 'G') faceSelectionFromVertex = true;


}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
