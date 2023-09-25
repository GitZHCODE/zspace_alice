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
	if (edgeSelection)
	{
		int currentSize = currentSet.size();
		int precision = 0;

		
		for (int i = 0; i < currentSize; i++)
		{
			zItMeshHalfEdgeArray cHEdges;			
			vector<int> currentSetIDs;			

			// Gets the edges and stores it in cHEdges
			currentSet[i].getConnectedHalfEdges(cHEdges);				

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
					
				}
			}
				
			// REMOVE DUPLICATES USING STD NAMESPACE

			//for (auto& cHE : cHEdges) // for each loop
			//{
			//	//printf("%i ", cHE.getId());
			//	cHE.getEdge().setColor(myHSV);

			//	zItMeshVertex eV = cHE.getVertex();
			//	eV.getId();
			//	bool isPresent = false;
			//	for (int j = 0; j < currentSet.size(); j++) {
			//		if (currentSet[j].getId() == eV.getId()) {
			//			isPresent = true;
			//		}
			//	}
			//	if (!isPresent) {
			//		currentSet.push_back(eV); // only add unique ids... coreutils
			//		printf("\n pushback currentSet Size %i ", currentSet.size());
			//	}
			//}

			
		}

		//printf("\n currentSet Size %i ", currentSet.size());
		cout << "currentSet Size " << currentSet.size() << endl;

		edgeSelection = !edgeSelection;	 // boolean toggle 
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
							printf("\n currentFSet Size %i ", currentFSet.size());
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


}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
