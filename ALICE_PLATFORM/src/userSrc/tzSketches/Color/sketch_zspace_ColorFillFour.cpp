//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include <headers/zToolsets/geometry/zTsColorSplit.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = false;
bool exportToFile = false;

double background = 0.35;

////////////////////////////////////////////////////////////////////////// zSpace Objects

/*!<model*/
zModel model;
//zModel resultModel;
/*!<Objects*/
zObjMesh oMesh;
zObjMeshPointerArray result;

zTsColorSplit cs;

int numMesh;
int currentMeshID;

double block = 1;
double brace = 1;

zUtilsCore core;

zColor RED(1, 0, 0, 1);
zColor BLUE(0, 0, 1, 1);
zColor GREEN(0, 1, 0, 1);
zColor MAGENTA(1, 0, 1, 1);

class ColorFillFour
{

public:

	zObjMesh* oMesh;
	zIntArray cols;
	unordered_map<int, vector<int>> dualNode_faces;
	zItMeshHalfEdgeArray edgeArr;

	zObjMesh oTempMesh;
	zPointArray pVertices;
	zIntArray pConnects;
	zIntArray pCounts;

	zItMeshHalfEdge se;

	void setMesh(zObjMesh& _meshObj)
	{
		oMesh = &_meshObj;
	}

	void computeColors(bool _BSF)
	{

		zObjGraph dualGraph;
		zIntArray inToDual;
		zIntArray dualToIn;

		zFnMesh fnMesh(*oMesh);
		fnMesh.getDualGraph(dualGraph, inToDual, dualToIn, true, false, false);
		const int numF = fnMesh.numPolygons();

		//initialise data
		cols.clear();
		cols.assign(numF, -1);

		int vNow = 0;
		bool tested = false;

		zIntArray bsf;
		zItGraphVertex sv(dualGraph, 0);
		zIntPairArray vertexPairs;

		if (_BSF) sv.getBSF(bsf, vertexPairs);
		else for (int i = 0; i < numF; i++) bsf[i] = i;

		do
		{
			tested = checkExisted(bsf[vNow], cols);
			if (!tested)
			{
				int id;
				int cvCounter = 0;
				zItGraphVertex v(dualGraph, bsf[vNow]);
				cols[v.getId()] = findColor(v.getId(), dualGraph, cols);
				//cout << cols[v.getId()] << endl;

				zIntArray connectedV;
				v.getConnectedVertices(connectedV);
				do
				{
					zItGraphVertex cv(dualGraph, connectedV[cvCounter]);
					if (cols[cv.getId()] != -1)
						cols[cv.getId()] = findColor(cv.getId(), dualGraph, cols);

					cvCounter++;
				} while (cvCounter < connectedV.size());

			}
			else vNow++;
		} while (vNow != numF);

		//do
		//{
		//	tested = checkExisted(vNow, cols);
		//	if (!tested)
		//	{
		//		int id;
		//		int cvCounter = 0;
		//		zItGraphVertex v(dualGraph, vNow);
		//		cols[v.getId()] = findColor(v.getId(), dualGraph, cols);
		//		//cout << cols[v.getId()] << endl;

		//		zIntArray connectedV;
		//		v.getConnectedVertices(connectedV);
		//		do
		//		{
		//			zItGraphVertex cv(dualGraph, connectedV[cvCounter]);
		//			if (cols[cv.getId()] != -1)
		//				cols[cv.getId()] = findColor(cv.getId(), dualGraph, cols);

		//			cvCounter++;
		//		} while (cvCounter < connectedV.size());

		//	}
		//	else vNow++;
		//} while (vNow != numF);
	}

	void computeColors(zObjMesh& _meshObj, zIntArray& _cols)
	{
		zObjGraph dualGraph;
		zIntArray inToDual;
		zIntArray dualToIn;

		zFnMesh fnMesh(_meshObj);
		fnMesh.getDualGraph(dualGraph, inToDual, dualToIn, true, false, false);
		const int numF = fnMesh.numPolygons();

		//initialise data
		cols.clear();
		cols.assign(numF, -1);

		int vNow = 0;
		bool tested = false;

		do
		{
			tested = checkExisted(vNow, cols);
			if (!tested)
			{
				int id;
				int cvCounter = 0;
				zItGraphVertex v(dualGraph, vNow);
				cols[v.getId()] = findColor(v.getId(), dualGraph, cols);

				//cout << "tempCols[v.getId()]" << cols[v.getId()] << endl;
				zIntArray connectedV;
				v.getConnectedVertices(connectedV);
				do
				{
					zItGraphVertex cv(dualGraph, connectedV[cvCounter]);
					if (cols[cv.getId()] != -1)
						cols[cv.getId()] = findColor(cv.getId(), dualGraph, cols);

					cvCounter++;
				} while (cvCounter < connectedV.size());

			}
			else vNow++;
		} while (vNow != numF);

		_cols = cols;
	}

	void computeColorsByStride(int _braceStride, int _blockStride, bool _isSkeleton, int _v0, int _v1)
	{
		dualNode_faces.clear();
		pVertices.clear();
		pConnects.clear();
		pCounts.clear();
		cols.clear();
		edgeArr.clear();

		if (_braceStride != 1 && _blockStride != 1)
		{
			zFnMesh fnMesh(*oMesh);
			int numF = fnMesh.numPolygons();
			int size = _braceStride * _blockStride;
			size = numF / size;

			zItMeshHalfEdge he_startBrace;
			bool isSkeletonMesh = _isSkeleton;

			//check if mesh is skeleton mesh and return the first half edge on node
			//otherwise take a corner edge
			findStartHalfedge(he_startBrace, isSkeletonMesh, _v0, _v1);

			if (isSkeletonMesh)
			{
				zItMeshHalfEdge he_brace = he_startBrace;
				zItMeshHalfEdge he_block = he_brace;

				se = he_startBrace;
				cout << endl << "SE" << se.getVertex().getPosition() << "," << se.getSym().getVertex().getPosition() << endl;

				int counter_f = 0;
				do
				{
					do
					{
						do
						{
							edgeArr.push_back(he_block);
							zItMeshVertexArray vertices;
							zItMeshHalfEdge nextStart;
							walk2d(he_block, _braceStride, _blockStride, counter_f, vertices, nextStart);
							addMeshData(vertices);

							//cout << "fid" << counter_f << endl;
							he_block = nextStart;
							counter_f++;
						} while (!he_block.onBoundary() && he_block != he_brace);

						for (int i = 0; i < _braceStride; i++) he_brace = he_brace.getNext().getSym().getNext();
						he_block = he_brace;

					} while (!he_brace.onBoundary() && counter_f < size);
					he_brace = he_brace.getSym().getNext();
					he_block = he_brace;

				} while (he_brace != he_startBrace);
				
			}
			else
			{
				zItMeshHalfEdge he_brace = he_startBrace;
				zItMeshHalfEdge he_block = he_brace;

				se = he_startBrace;
				cout << endl << "SE" << se.getVertex().getPosition() << "," << se.getSym().getVertex().getPosition() << endl;

				int counter_f = 0;

				do
				{
					do
					{
						edgeArr.push_back(he_block);
						zItMeshVertexArray vertices;
						zItMeshHalfEdge nextStart;
						walk2d(he_block, _braceStride, _blockStride, counter_f, vertices, nextStart);
						addMeshData(vertices);

						//cout << "fid" << counter_f << endl;
						he_block = nextStart;
						counter_f++;
					} while (!he_block.onBoundary() && he_block != he_brace);

					for (int i = 0; i < _braceStride; i++) he_brace = he_brace.getNext().getSym().getNext();
					he_block = he_brace;

				} while (!he_brace.onBoundary() && counter_f < size);
			}

			//generate mesh
			zFnMesh fm(oTempMesh);
			fm.clear();
			fm.create(pVertices, pCounts, pConnects);

			cout << endl << "V:" << fm.numVertices() << "," << "E:" << fm.numEdges() << "," << "F:" << fm.numPolygons() << endl;


			zIntArray tempCols;
			computeColors(oTempMesh, tempCols);

			cols.clear();
			cols.assign(fnMesh.numPolygons(), -1);

			//cout << "tempColSize" << tempCols.size();
			//cout << "colsSize" << cols.size();

			for (int i = 0; i < tempCols.size(); i++)
			{
				auto gotFace = dualNode_faces.find(i);
				if (gotFace != dualNode_faces.end())
				{
					int colId = gotFace->first;
					zIntArray faces = gotFace->second;

					for (int j = 0; j < faces.size(); j++)
					{
						cols[faces[j]] = tempCols[colId];
						//cout << "cols:" << o << endl;

					}
				}
			}
		}
		else computeColors(true);
	}

	void setColor()
	{
		zFnMesh fnMesh(*oMesh);
		for (zItMeshFace f(*oMesh); !f.end(); f++)
		{
			int id = f.getId();
			//cout << cols[id] << endl;
			if (cols[id] == 0) f.setColor(RED);
			else if (cols[id] == 1) f.setColor(BLUE);
			else if (cols[id] == 2) f.setColor(GREEN);
			else if (cols[id] == 3) f.setColor(MAGENTA);
		}
	}

	void draw()
	{
		//oMesh->setDisplayElements(false, false, true);
		//oMesh->draw();
		oTempMesh.setDisplayElements(false, true, true);
		oTempMesh.draw();

		zFnMesh fn(oTempMesh);
		zObjGraph g;
		zIntArray inToDual;
		zIntArray dualToIn;
		fn.getDualGraph(g, inToDual, dualToIn, true, false, false);
		g.draw();

		//model.displayUtils.drawLine(se.getVertex().getPosition(), se.getSym().getVertex().getPosition(), zColor(1, 0, 0, 1), 8);


		for (auto& e : edgeArr)
			model.displayUtils.drawLine(e.getVertex().getPosition(), e.getSym().getVertex().getPosition(), zColor(1, 0, 0, 1), 8);
	}

private:

	void findStartHalfedge(zItMeshHalfEdge& _startBraceHalfEdge, bool& _isSkeletonMesh, int _v0, int _v1)
	{

		if (_isSkeletonMesh)
		{
			zItMeshHalfEdgeArray cHes;
			zItMeshVertex v(*oMesh, _v0);
			v.getConnectedHalfEdges(cHes);

			for (auto& he : cHes)
			{
				if (he.getVertex().getId() == _v1)
				{
					_startBraceHalfEdge = he;
					break;
				}
			}

			////check skeleton mesh
			//for (v.begin(); !v.end(); v++)
			//{
			//	if (!v.checkValency(4) && !v.onBoundary())
			//	{
			//		_startBraceHalfEdge = v.getHalfEdge().getSym();
			//		goto skeletonMesh;
			//	}
			//}
		}

		else
		{
			zItMeshVertex v(*oMesh);

			//check non-periodic mesh
			for (v.begin(); !v.end(); v++)
			{
				if (v.checkValency(2) && v.onBoundary())
				{
					zItMeshHalfEdgeArray edges;
					v.getConnectedHalfEdges(edges);
					for (auto& e : edges)
					{
						if (!e.onBoundary())
						{
							_startBraceHalfEdge = e;
							break;
						}
					}
				}
			}

			//check periodic mesh
			for (v.begin(); !v.end(); v++)
			{
				if (v.checkValency(3) && v.onBoundary() && v.getId() == 2563)
				{
					zItMeshHalfEdgeArray edges;
					v.getConnectedHalfEdges(edges);
					for (auto& e : edges)
					{
						e = e.getSym();
						if (e.onBoundary())
						{
							_startBraceHalfEdge = e.getSym().getPrev().getSym();
							break;
						}
					}
				}
			}
		}
	}

	void walk2d(zItMeshHalfEdge _startBraceHalfEdge, int _braceStride, int _blockStride, int _fId, zItMeshVertexArray& _vertices, zItMeshHalfEdge& _nextStart)
	{
		/*
	2-------1
	|	|	|
	--------- ^
	|	|	| | startBraceHalfEdge
	3-------0
*/

		zItMeshHalfEdge he_brace = _startBraceHalfEdge;
		zItMeshHalfEdge he_block = he_brace;
		zItMeshHalfEdge nextStart;

		zItMeshVertexArray vertices;
		vertices.assign(4, zItMeshVertex());

		vertices[0] = he_brace.getSym().getVertex(); //update va
		for (int i = 0; i < _braceStride; i++)
		{
			for (int j = 0; j < _blockStride; j++)
			{
				dualNode_faces[_fId].push_back(he_block.getFace().getId());
				he_block = he_block.getPrev().getPrev().getSym();

				//safe trigger
				if (he_block.onBoundary()) break;
			}
			if (i < 1)
			{
				nextStart = he_block;
				vertices[3] = he_block.getSym().getVertex(); //update vb

			}
			vertices[1] = he_brace.getVertex(); //update vd
			vertices[2] = he_block.getVertex(); //update vc

			he_brace = he_brace.getNext().getSym().getNext();
			he_block = he_brace;

			//safe trigger
			if (he_brace.onBoundary()) break;
		}

		//zItMeshHalfEdge he_brace = _startBraceHalfEdge;
		//zItMeshHalfEdge he_block = he_brace.getPrev();
		//zItMeshHalfEdge nextStart;

		//zItMeshVertexArray vertices;
		//vertices.assign(4, zItMeshVertex());

		//vertices[0] = he_brace.getSym().getVertex(); //update va
		//for (int i = 0; i < _braceStride; i++)
		//{
		//	for (int j = 0; j < _blockStride; j++)
		//	{
		//		if (j > 0)
		//			he_block = he_block.getPrev().getSym().getPrev();

		//		dualNode_faces[_fId].push_back(he_block.getFace().getId());
		//	}
		//	if (i < 1)
		//	{
		//		vertices[3] = he_block.getPrev().getVertex(); //update vb
		//		nextStart = he_block.getPrev().getSym();
		//	}
		//	vertices[1] = he_brace.getVertex(); //update vd
		//	vertices[2] = he_block.getPrev().getSym().getVertex(); //update vc

		//	he_brace = he_brace.getNext().getSym().getNext();
		//	he_block = he_brace.getPrev();
		//}

		//output
		_vertices = vertices;
		_nextStart = nextStart;
	}

	void addMeshData(zItMeshVertexArray& _vertices)
	{
		for (auto& v : _vertices)
		{
			int vID;
			bool check = core.checkRepeatVector(v.getPosition(), pVertices, vID);

			if (!check)
			{
				vID = pVertices.size();
				pVertices.push_back(v.getPosition());
			}
			pConnects.push_back(vID);
		}
		pCounts.push_back(_vertices.size());
	}

	bool checkExisted(int _id, zIntArray& _cols)
	{
		bool found = false;
		for (int i = 0; i < cols.size(); i++)
			if (_cols[_id] != -1)
			{
				found = true;
				goto end;
			}
	end:
		return found;
	}

	int findColor(int _id, zObjGraph& _dualGraph, zIntArray& _cols)
	{
		zItGraphVertex v(_dualGraph, _id);
		zIntArray connectedV;
		v.getConnectedVertices(connectedV);

		int max = -1;
		int i = 0;
		do
		{
			if (_cols[connectedV[i]] - max == 1)
			{
				max++;
				i = 0;
			}
			else
				i++;

		} while (i != connectedV.size());

		max += 1;
		return max;
	}
};

ColorFillFour cf;
zItMeshVertex dv;
zItMeshHalfEdge dhe;

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(200000);
	// read mesh
	zFnMesh fnMesh(oMesh);
	//fnMesh.from("data/testColorMesh.json", zJSON);
	//fnMesh.from("data/10X10GridMesh.obj", zOBJ);
	//fnMesh.from("data/20X20GridMesh.obj", zOBJ);
	//fnMesh.from("data/skeletonGridMesh.obj", zOBJ);
	//fnMesh.from("data/striatus_noColor.obj", zOBJ);
	fnMesh.from("data/HZ/aquaticRoof_2.obj", zOBJ);

	model.addObject(oMesh);
	oMesh.setDisplayElements(false, true, true);

	zItMeshVertex v(oMesh);
	zItMeshHalfEdge he;
	dv = v;

	if (v.checkValency(2) && v.onBoundary())
	{
		zItMeshHalfEdgeArray edges;
		v.getConnectedHalfEdges(edges);
		for (auto& e : edges)
		{
			if (!e.onBoundary())
			{
				dhe = e;
				cout << "FOUND" << endl;
			}
		}
		//if (he_startBrace.onBoundary())
		//	he_startBrace = he_startBrace.getSym();
	}

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&brace, "brace");
	S.sliders[1].attachToVariable(&brace, 1, 10);
	S.addSlider(&block, "block");
	S.sliders[2].attachToVariable(&block, 1, 10);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&exportToFile, "export");
	B.buttons[2].attachToVariable(&exportToFile);


}

void update(int value)
{
	if (compute)
	{
		auto begin = std::chrono::high_resolution_clock::now();

		cf.setMesh(oMesh);
		//cf.computeColors(true);

		bool isSkeleton = false;
		//int v0 = 25;
		//int v1 = 92;
		int v0 = 480;
		int v1 = 794;
		cf.computeColorsByStride((int)brace, (int)block, isSkeleton, v0, v1);
		cf.setColor();


		// Stop measuring time
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n Time: %.7f seconds.", elapsed.count() * 1e-9);

		cs.setInMesh(oMesh);
		cs.compute();
		result = cs.getRawSplitMesh(numMesh);

		compute = !compute;
	}

	if (exportToFile)
	{
		if (numMesh > 0)
		{
			string dir = "data/HZ/out_2/";
			cs.exportTo(dir, zJSON);
		}
		exportToFile = !exportToFile;

	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	model.draw();
	S.draw();
	B.draw();


	if (display)
	{
		if (numMesh > 0)
			for (auto& m : result)
			{
				m->setDisplayElements(true, true, true);
				m->draw();
			}

		cf.draw();

		/*model.displayUtils.drawPoint(dv.getPosition(), zColor(0, 0, 0, 1), 10);
		model.displayUtils.drawPoint(dhe.getVertex().getPosition(), zColor(1, 0, 0, 1), 10);
		model.displayUtils.drawLine(dhe.getVertex().getPosition(), dhe.getSym().getVertex().getPosition(), zColor(1, 0, 0, 1), 5);
		model.displayUtils.drawLine(dhe.getPrev().getVertex().getPosition(), dhe.getPrev().getSym().getVertex().getPosition(), zColor(0, 0, 1, 1), 5);*/

	}
	else
	{
		int i = currentMeshID;

		if (numMesh > 0 && i >= 0 && i < numMesh)
		{
			result[i]->setDisplayElements(true, true, true);
			result[i]->draw();
		}

	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));
	drawString("Total num of meshes #:" + to_string(numMesh), vec(50, 200, 0));
	drawString("Current mesh #:" + to_string(currentMeshID), vec(50, 225, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{

	if (k == 'w')
	{
		if (currentMeshID < numMesh - 1)currentMeshID++;;
	}
	if (k == 's')
	{
		if (currentMeshID > 0)currentMeshID--;;
	}
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
