//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMesh oMesh;

void triangulateMesh(zObjMesh& oMesh)
{
	zObjMesh temp;
	zFnMesh fnMesh(temp);
	zPointArray positions;
	zIntArray pCounts;
	zIntArray pConnects;

	int counter = 0;

	zItMeshFace f(oMesh);

	for (; !f.end(); f++)
	{
		zPointArray fVertices, tri_a, tri_b;
		f.getVertexPositions(fVertices);

		tri_a = { fVertices[0],fVertices[1], fVertices[2] };
		tri_b = { fVertices[1],fVertices[2], fVertices[3] };

		for (auto& v : tri_a)
		{
			int vID;
			bool check = core.checkRepeatVector(v, positions, vID);

			if (!check)
			{
				vID = positions.size();
				positions.push_back(v);
			}
			pConnects.push_back(vID);
		}
		pCounts.push_back(3);

		for (auto& v : tri_b)
		{
			int vID;
			bool check = core.checkRepeatVector(v, positions, vID);

			if (!check)
			{
				vID = positions.size();
				positions.push_back(v);
			}
			pConnects.push_back(vID);
		}
		pCounts.push_back(3);
	}

	fnMesh.create(positions, pCounts, pConnects);
	fnMesh.computeMeshNormals();

	oMesh = temp;
}

class zVectorFieldPathFinder
{
public:
	zObjMeshScalarField oField;
	zIntArray fIds_obstacles;
	zIntArray sources;
	zObjMesh dualMesh_check;

	void create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		zFnMeshScalarField fnField(oField);
		fnField.create(_minBB, _maxBB, _n_X, _n_Y, _NR, _setValuesperVertex, _triMesh);
	}

	void compute()
	{
		computeObstacles();
		cout << "obstacle completed" << endl;
		computeHeat();
		cout << "heat completed" << endl;

	}

private:
	zTsMeshParam oHeatMesh_tri;
	zFloatArray fVals;
	unordered_map<int, int> map_new_oldId;

	void computeObstacles()
	{
		zFnMeshScalarField fnField(oField);
		zItMeshFace f(*fnField.getRawMesh());
		
		fVals.assign(fnField.numFieldValues(), -1);

		zObjMesh temp;
		zFnMesh fnMesh(temp);
		zPointArray positions;
		zIntArray pCounts;
		zIntArray pConnects;

		int counter = 0;

		for (; !f.end(); f++)
		{
			auto it = find(fIds_obstacles.begin(), fIds_obstacles.end(), f.getId());
			if (it != fIds_obstacles.end())
			{
				continue;
			}
			else
			{
				fVals[f.getId()] = 10000;

				zPointArray fVertices;
				f.getVertexPositions(fVertices);

				for (auto& v : fVertices)
				{
					int vID;
					bool check = core.checkRepeatVector(v, positions, vID);

					if (!check)
					{
						vID = positions.size();
						positions.push_back(v);
					}
					pConnects.push_back(vID);
				}
				pCounts.push_back(f.getNumVertices());
				map_new_oldId[counter] = f.getId();
				counter++;
			}
		}

		fnMesh.create(positions, pCounts, pConnects);
		fnMesh.computeMeshNormals();

		zObjMesh dualMesh;
		zIntArray temp1, temp2;
		fnMesh.getDualMesh(dualMesh, temp1, temp2, true);

		zFnMesh fnDual(dualMesh);
		fnDual.triangulate();
		//triangulateMesh(dualMesh);
		fnDual.computeMeshNormals();

		//oHeatMesh_tri.o_paramTriMesh = dualMesh;
		//oHeatMesh_tri.o_TriMesh = dualMesh;

		//fnDual.getMatrices_trimesh(oHeatMesh_tri.triMesh_V, oHeatMesh_tri.triMesh_FTris);

		dualMesh_check = dualMesh;

	}

	void computeHeat()
	{
		zObjMesh* o_inMesh = oHeatMesh_tri.getRawParamMesh();
		zFloatArray geodesics;

		oHeatMesh_tri.computeGeodesics_Exact(sources, geodesics);

		cout << endl;
		cout << "geodesics size:" << geodesics.size() << endl;

		zDomainFloat startMinMax(core.zMin(geodesics), core.zMax(geodesics));
		zDomainFloat outMinMax(0, 1);

		for (int i = 0; i < geodesics.size(); i++)
		{
			geodesics[i] = core.ofMap(geodesics[i], startMinMax, outMinMax);
			fVals[map_new_oldId[i]] = geodesics[i];
		}

		cout << "geodesics min:" << core.zMin(geodesics) << endl;
		cout << "geodesics max:" << core.zMax(geodesics) << endl;

		zFnMeshField fnField(oField);
		fnField.setFieldValues(fVals);
		fnField.updateColors();
	}
};

//
//ZSPACE_INLINE void zFnMesh::deleteFace(int faceID)
//{
//	// Check if the face ID is valid
//	if (faceID < 0 || faceID >= numPolygons()) {
//		throw std::invalid_argument("error: faceID is out of range.");
//		return;
//	}
//
//	zItMeshFace face(*meshObj, faceID);
//
//	// Deactivate the face
//	face.deactivate();
//
//	// Get all half-edges of the face
//	zItMeshHalfEdgeArray fHEdges;
//	face.getHalfEdges(fHEdges);
//
//	// Store the connected vertices
//	std::unordered_set<int> connectedVertices;
//	for (auto& he : fHEdges) {
//		connectedVertices.insert(he.getVertex().getId());
//	}
//
//	// Update half-edge pointers
//	for (auto& he : fHEdges) {
//		zItMeshHalfEdge symHe = he.getSym();
//
//		// Set next and prev pointers
//		symHe.getPrev().setNext(symHe.getNext());
//		symHe.getNext().setPrev(symHe.getPrev());
//
//		// Deactivate the half-edges
//		he.deactivate();
//		symHe.deactivate();
//	}
//
//	// Check and deactivate isolated vertices
//	for (auto vID : connectedVertices) {
//		zItMeshVertex vertex(*meshObj, vID);
//		if (vertex.onBoundary()) {
//			vertex.deactivate();
//		}
//	}
//
//	// Garbage collect to clean up inactive elements
//	garbageCollection(zFaceData);
//	garbageCollection(zEdgeData);
//	garbageCollection(zVertexData);
//
//	// Recompute normals if needed
//	computeMeshNormals();
//}
//

////// --- GUI OBJECTS ----------------------------------------------------


zVectorFieldPathFinder pathFinder;

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

	pathFinder.create(zPoint(0, 0, 0), zPoint(10, 10, 0), 10, 10, 1, true, false);
	pathFinder.fIds_obstacles = { 3,10,50 };
	pathFinder.sources = { 7,82 };
	pathFinder.compute();

	pathFinder.oField.setDisplayElements(false, true, false);
	model.addObject(pathFinder.oField);
	model.addObject(pathFinder.dualMesh_check);
	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	

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

	if (display)
	{
		// zspace model draw
		model.draw();
		
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
