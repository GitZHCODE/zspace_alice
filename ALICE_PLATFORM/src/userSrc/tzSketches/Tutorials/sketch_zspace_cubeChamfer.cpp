//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#define DEBUGGER if (0) cout

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
zObjMesh outMesh;


////// --- GUI OBJECTS ----------------------------------------------------

void sortAroundVector(zPointArray& positions, zVector& origin, zVector& normal)
{
	zDoubleArray angles;
	angles.assign(positions.size(), -1);
	zVector vec_check = normal ^ zVector(1, 0, 0);

	for (int i = 0; i < positions.size(); i++)
	{
		zVector vec = positions[i] - origin;
		angles[i] = vec_check.angle360(vec, normal);
	}

	// Create a vector of pairs to store angles and corresponding points
	std::vector<std::pair<double, zPoint>> anglePointPairs;
	for (int i = 0; i < positions.size(); i++)
	{
		anglePointPairs.push_back(std::make_pair(angles[i], positions[i]));
	}

	// Sort the pairs based on angles in ascending order
	std::sort(anglePointPairs.begin(), anglePointPairs.end(),
		[](const std::pair<double, zPoint>& a, const std::pair<double, zPoint>& b) {
			return a.first < b.first;
		});

	// Update the positions with sorted points
	for (int i = 0; i < positions.size(); i++)
	{
		positions[i] = anglePointPairs[i].second;
	}
}

void uniqueArr(zIntArray& inputArray, zIntArray& uniqueArray)
{
	for (int i = 0; i < inputArray.size(); ++i) {
		int currentElement = inputArray[i];
		if (std::find(uniqueArray.begin(), uniqueArray.end(), currentElement) == uniqueArray.end()) {
			uniqueArray.push_back(currentElement);
		}
	}
}

void chamferV_legacy(zObjMesh& oMesh, const zIntArray& chamferIds, float chamferVal, zObjMesh& outMesh,bool keepChamferedFace = false, bool absoluteDist = false)
{
    // map container to store the new vertex positions for each face
    std::map<int, zVectorArray> fv_map;

	//iterate on each face
	zItMeshFace f(oMesh);
	for (f.begin(); !f.end(); f++)
	{
		//get face vertices 
		zVectorArray vPositions;
		zIntArray vIds;
		f.getVertices(vIds);
		f.getVertexPositions(vPositions);

		//map container to store the new vertex position according to vertex ids on a face
		map<int, pair<zVector,zVector>> chamferIds_onFace;

		//find all chamfer vertices on a face
		for (int i = 0; i < vIds.size(); i++)
		{
			int chamferId, prevId, nextId;

			//if face has chamfer vertices then compute new positions
			if (std::find(chamferIds.begin(), chamferIds.end(), vIds[i]) != chamferIds.end())
			{
				int chamferId = i;
				int prevId = (i > 0) ? i - 1 : vIds.size() - 1;
				int nextId = (i < vIds.size() - 1) ? i + 1 : 0;

				zVector vec_a = vPositions[nextId] - vPositions[chamferId];
				zVector vec_b = vPositions[prevId] - vPositions[chamferId];

				if (absoluteDist)
				{
					vec_a.normalize();
					vec_b.normalize();
				}
				vec_a *= chamferVal;
				vec_b *= chamferVal;

				zVector newPos_a = vPositions[chamferId] + vec_a;
				zVector newPos_b = vPositions[chamferId] + vec_b;

				chamferIds_onFace.emplace(i, make_pair(newPos_a, newPos_b));
			}
		}
		
		//add new positions reversely
		for (auto rit = chamferIds_onFace.rbegin(); rit != chamferIds_onFace.rend(); ++rit) {
			vPositions[rit->first] = rit->second.first;
			vPositions.insert(vPositions.begin() + rit->first, rit->second.second);
		}

		//store new face vertices in a map
		fv_map[f.getId()] = vPositions;
	}

	if (keepChamferedFace)
	{
		int counter = fv_map.size();
		for (auto& vId : chamferIds)
		{
			zItMeshVertex v(oMesh, vId);
			zIntArray cHes;
			v.getConnectedHalfEdges(cHes);

			for (auto& heId : cHes)
			{
				zItMeshHalfEdge he(oMesh, heId);
				zVector vec = he.getVector();
				if (absoluteDist)
					vec.normalize();

				vec *= chamferVal;
				zVector newPos = v.getPosition() + vec;
				fv_map[counter].push_back(newPos);
			}

			//keep vertex on new face if the chamfered vertex is on boundary
			if(v.onBoundary())
			{
				fv_map[counter].push_back(v.getPosition());
				zVector average;

				for (auto& p : fv_map[counter])
					average += p;

				average /= fv_map[counter].size();

				sortAroundVector(fv_map[counter], average, v.getNormal());
			}
			counter ++;
		}
	}

    // Update the mesh
    zVectorArray positions;
    zIntArray pCounts;
    zIntArray pConnects;

    for (auto& map : fv_map)
    {
        for (auto& v : map.second)
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
        pCounts.push_back(map.second.size());
    }

    zFnMesh fn(outMesh);
    fn.create(positions, pCounts, pConnects);
}

void chamferV(zObjMesh& oMesh, const zIntArray& chamferIds, float chamferVal, zObjMesh& outMesh, bool keepChamferedFace = false, bool absoluteDist = false)
{
	unordered_map<int, pair<bool, int>> he_v_map;
	int vCounter = 0;

	zVectorArray vPositions;
	zIntArray pConnects;
	zIntArray pCounts;

	//visit all vertcies and make new v positions & v ids
	zItMeshVertex v(oMesh);
	for (v.begin(); !v.end(); v++)
	{
		zIntArray cHes;
		v.getConnectedHalfEdges(cHes);

		//if the vertex is chamfered. add new vPositions and ids
		if (std::find(chamferIds.begin(), chamferIds.end(), v.getId()) != chamferIds.end())
		{
			for (auto& heId : cHes)
			{
				zItMeshHalfEdge he(oMesh, heId);

				zVector newPos = v.getPosition();
				zVector vec = he.getVector();

				if (absoluteDist)
					vec.normalize();

				vec *= chamferVal;
				newPos += vec;

				vPositions.push_back(newPos);
				he_v_map[heId] = make_pair(true, vCounter);
				
				vCounter++;
			}
		}
		//if the vertex is no chamfered. add original vPosition and id
		else
		{
			for (auto& heId : cHes)
			{
				zItMeshHalfEdge he(oMesh, heId);
				he_v_map[heId] = make_pair(false, vCounter);
			}

			vPositions.push_back(v.getPosition());
			vCounter++;
		}
	}

	DEBUGGER << endl;
	for (auto& p : he_v_map)
	{
		DEBUGGER << p.second.first << endl;
	}

	//visit all faces and make pCounts & pConnects based on hes
	zItMeshFace f(oMesh);
	for (f.begin(); !f.end(); f++)
	{
		zIntArray fHes;
		f.getHalfEdges(fHes);

		int extraNumV = 0;
		zIntArray fvs;
		for (auto& heId : fHes)
		{
			zItMeshHalfEdge he(oMesh, heId);

			fvs.push_back(he_v_map[he.getId()].second);
			fvs.push_back(he_v_map[he.getSym().getId()].second);

			//check if a he and its sym has new positions
			if (he_v_map[he.getId()].first)
				extraNumV++;

			if (he_v_map[he.getSym().getId()].first)
				extraNumV++;
		}

		zIntArray unique;
		uniqueArr(fvs, unique);
		pConnects.insert(pConnects.end(), unique.begin(), unique.end());

		pCounts.push_back(fHes.size() + extraNumV/2);
	}

	//make new chamfered faces
	if (keepChamferedFace)
	{
		for (auto& vId : chamferIds)
		{
			zItMeshVertex v(oMesh, vId);
			if (!v.onBoundary())
			{
				zIntArray cHes;
				v.getConnectedHalfEdges(cHes);

				for (auto& heId : cHes)
				{
					pConnects.push_back(he_v_map[heId].second);
				}
				pCounts.push_back(cHes.size());
			}
		}
	}

	DEBUGGER << endl;
	DEBUGGER << vPositions.size() << endl;
	DEBUGGER << "pos" << endl;
	for (auto& p : vPositions)
		DEBUGGER << p << endl;
	DEBUGGER << "pConnects" << endl;
	for (auto& p : pConnects)
		DEBUGGER << p << endl;
	DEBUGGER << "pCounts" << endl;
	for (auto& p : pCounts)
		DEBUGGER << p << endl;

	//make mesh
	zFnMesh fn(outMesh);
	fn.create(vPositions, pCounts, pConnects);
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
	zFnMesh fnMesh(oMesh);
	//fnMesh.from("data/cube.json", zJSON);
	fnMesh.from("data/chamferTest.json",zJSON);
	//fnMesh.from("data/chamferTest_heavy.json",zJSON);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	// set display element booleans
	oMesh.setDisplayElements(false, true, true);


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
		zIntArray chamferVIds;
		//chamferVIds.push_back(12);
		//chamferVIds.push_back(11);
		//chamferVIds.push_back(1);
		//chamferVIds.push_back(4);
		//chamferVIds.push_back(6);
		//chamferVIds.push_back(5);
		//chamferVIds.push_back(7);

		//chamferVIds.push_back(0);
		//chamferVIds.push_back(1);

		zItMeshVertex v(oMesh);
		for (v.begin(); !v.end(); v++)
		{
			if (!v.onBoundary())
				chamferVIds.push_back(v.getId());
		}

		auto begin = std::chrono::high_resolution_clock::now();

		//chamferV_legacy(oMesh, chamferVIds, 0.5, outMesh, true, false);

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n Legacy method Time: %.7f seconds.", elapsed.count() * 1e-9);

		begin = std::chrono::high_resolution_clock::now();

		chamferV(oMesh, chamferVIds, 0.25, outMesh, true, false);

		end = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n HE method Time: %.7f seconds.", elapsed.count() * 1e-9);

		oMesh = zObjMesh(outMesh);

		zFnMesh fn(outMesh);
		cout << endl << "V:" << fn.numVertices() << "E:" << fn.numEdges() << "F:" << fn.numPolygons() << endl;
		
		fn.to("data/chamferTest_out.json", zJSON);

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
		//zFnMesh fn(oMesh);
		//model.displayUtils.drawVertexIds(fn.numVertices(), fn.getRawVertexPositions(), zColor(0, 0, 1, 1));
		//zItMeshEdge e(oMesh);
		//for (e.begin(); !e.end(); e++)
		//{
		//	zPointArray vs;
		//	e.getVertexPositions(vs);
		//	model.displayUtils.drawLine(vs[0], vs[1], zColor(zMAGENTA), 2);
		//}
		//zItMeshVertex v(oMesh);
		//for (v.begin(); !v.end(); v++)
		//{
		//	model.displayUtils.drawPoint(v.getPosition(), zColor(zBLACK), 5);
		//}
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
