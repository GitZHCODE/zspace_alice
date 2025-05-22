//#define _MAIN_

#define _HAS_STD_BYTE 0

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// Custom Methods

zUtilsCore core;

enum zMedialType
{
	//columnUp, 
	//column,
	Bottom, //<< lower part of the bay (column and arch)
	Top //<< top part of the bay (top arch)
};

struct zHalfMedial
{
	zObjGraph o_medial;
	zObjGraphArray o_features;
	vector<zPlane> splitPlanes;

	zItMeshHalfEdgeArray meshHEStartEnd;


	zObjNurbsCurve o_medial_nurbs;
	zObjNurbsCurveArray o_features_nurbs;
	zObjPlaneArray o_splitPlanes;


	zHalfMedial* nextMedial = nullptr;
	zHalfMedial* prevMedial = nullptr;
	zMedialType type;

	zIntArray blockIDs;


	zHalfMedial* symMedial = nullptr;
};

zPoint getContourPosition(float& threshold, zVector& vertex_lower, zVector& vertex_higher, float& thresholdLow, float& thresholdHigh)
{
	float scaleVal = core.ofMap(threshold, thresholdLow, thresholdHigh, 0.0000f, 1.0000f);

	zVector e = vertex_higher - vertex_lower;
	double edgeLen = e.length();
	e.normalize();

	return (vertex_lower + (e * edgeLen * scaleVal));
}

void isoContour(zObjGraph& o_graph, zScalarArray &vertexScalars, float threshold, zPointArray& contourPoints)
{
	zFnGraph fnGraph(o_graph);
	zPoint* vPositions = fnGraph.getRawVertexPositions();

	contourPoints.clear();

	for (zItGraphEdge e(o_graph); !e.end(); e++)
	{
		zIntArray eVerts;
		e.getVertices(eVerts);

		float s0 = vertexScalars[eVerts[0]];
		float s1 = vertexScalars[eVerts[1]];

		bool contour = false;
		if (s0 <= threshold && s1 >= threshold)contour = true;
		if (s0 >= threshold && s1 <= threshold)contour = true;

		if (!contour) continue;

		zPoint v0 = vPositions[eVerts[0]];
		zPoint v1 = vPositions[eVerts[1]];	

		zPoint pos1 = getContourPosition(threshold, v1, v0, s1, s0);
		contourPoints.push_back(pos1);
	}
}

void intersect_graphPlane(zObjGraph& o_graph, zPlane &inPlane, bool closestPoint, zPointArray &outPoints)
{
	outPoints.clear();

	zScalarArray vertexScalars;

	zPoint O(inPlane(3, 0), inPlane(3, 1), inPlane(3, 2));
	zVector N(inPlane(2, 0), inPlane(2, 1), inPlane(2, 2));

	for (zItGraphVertex v(o_graph); !v.end(); v++)
	{
		zPoint P = v.getPosition();
		float minDist_Plane = core.minDist_Point_Plane(P, O, N);
		vertexScalars.push_back(minDist_Plane);
	}

	zPointArray contourPoints;
	isoContour(o_graph, vertexScalars,0.0, contourPoints);

	if (closestPoint)
	{
		float dist = 1000000;
		zPoint cPoint;
		for (auto& p : contourPoints)
		{
			if (p.distanceTo(O) < dist)
			{
				dist = p.distanceTo(O);
				cPoint = p;
			}

		}

		outPoints.push_back(cPoint);
	}
	else outPoints = contourPoints;

}

void createGraphFromHalfEdgeLoop(zObjGraph &o_graph, zItMeshHalfEdgeArray &heLoop, zColor eColor)
{
	zPointArray positions;
	zIntArray eConnects;

	for (int i = 0; i< heLoop.size(); i++)
	{
		positions.push_back(heLoop[i].getStartVertex().getPosition());

		if (positions.size() > 1)
		{
			eConnects.push_back(positions.size() - 2);
			eConnects.push_back(positions.size() - 1);
		}

		if (i == heLoop.size() - 1)
		{
			positions.push_back(heLoop[i].getVertex().getPosition());

			eConnects.push_back(positions.size() - 2);
			eConnects.push_back(positions.size() - 1);
		}
	}

	zFnGraph fnGraph(o_graph);
	fnGraph.create(positions, eConnects);

	fnGraph.setEdgeColor(eColor);
	//printf("\n g: %i %i ", fnGraph.numVertices(), fnGraph.numEdges());
}


void computeMedialGraph(zObjMesh& o_mesh,int startVID, int numFeatureLoops, int numStride, vector<int> &_He_Medial_Map, vector<zHalfMedial> &_medials )
{
	_medials.clear();
	_He_Medial_Map.clear();

	zFnMesh fnMesh(o_mesh);
	_He_Medial_Map.assign(fnMesh.numHalfEdges(), -1);

	// get corner vertex
	zItMeshVertex v(o_mesh, startVID);

	zItMeshHalfEdgeArray cHEdges;
	v.getConnectedHalfEdges(cHEdges);

	zItMeshHalfEdge startHe;
	for (auto& he : cHEdges)
	{
		if (!he.getEdge().onBoundary()) startHe = he;
	}

	zItMeshHalfEdge he = startHe;

	zItMeshHalfEdge he_top_Start;
	bool compHeTopStart = true;

	int numMedials = 0;

	//////////  COMPUTE NUMBER OF MEDIALS;
	//bottom loops
	do
	{
		he.getEdge().setColor(zRED);
		zItMeshHalfEdge heSec = he;

		// store he for top loop walk
		if (compHeTopStart)
		{
			if (he.getStartVertex().getValence() > 4)
			{
				he_top_Start = he.getSym();
				he_top_Start = he_top_Start.getNext().getSym().getNext();

				compHeTopStart = false;
			}
		}

		// add
		if (he.getStartVertex().getValence() > 4) numMedials++;

		for (int i = 0; i < numFeatureLoops; i++)
		{
			for (int j = 0; j < numStride; j++) heSec = heSec.getPrev().getPrev().getSym();

			heSec.getEdge().setColor(zBLUE);
		}

		if (he.getVertex().onBoundary())
		{
			he = he.getSym();
			numMedials++;
		}
		else
		{
			he = he.getNext().getSym().getNext();
		}

	} while (he != startHe);


	// top loop
	he = he_top_Start;

	do
	{
		he.getEdge().setColor(zYELLOW);
		zItMeshHalfEdge heSec = he;

		// add
		if (he.getStartVertex().getValence() > 4) numMedials++;

		for (int i = 0; i < numFeatureLoops; i++)
		{
			for (int j = 0; j < numStride; j++) heSec = heSec.getPrev().getPrev().getSym();

			heSec.getEdge().setColor(zGREEN);

		}

		he = he.getNext().getSym().getNext();

	} while (he != he_top_Start);

	printf("\n num Medials %i ", numMedials);

	_medials.assign(numMedials, zHalfMedial());

	////////// CREATE MEDIAL GRAPHS
	he = startHe;
	int medialCounter = 0;

	zItMeshHalfEdgeArray tmp_medials;
	vector<zItMeshHalfEdgeArray> tmp_features;
	tmp_features.assign(numFeatureLoops, zItMeshHalfEdgeArray());

	do
	{
		zItMeshHalfEdge heSec = he;

		// store
		tmp_medials.push_back(he);
		_He_Medial_Map[he.getId()] = medialCounter;

		// add medial graph
		if (he.getVertex().getValence() > 4 || he.getVertex().onBoundary())
		{
			//printf("\n medial graph ");
			createGraphFromHalfEdgeLoop(_medials[medialCounter].o_medial, tmp_medials, zRED);
			_medials[medialCounter].meshHEStartEnd.assign(2, zItMeshHalfEdge());
			_medials[medialCounter].meshHEStartEnd[0] = tmp_medials[0];
			_medials[medialCounter].meshHEStartEnd[1] = tmp_medials[tmp_medials.size() - 1];


			_medials[medialCounter].nextMedial = &_medials[medialCounter + 1];
			_medials[medialCounter + 1].prevMedial = &_medials[medialCounter];

			_medials[medialCounter].type = zMedialType::Bottom;


			tmp_medials.clear();
			medialCounter++;
		}

		for (int i = 0; i < numFeatureLoops; i++)
		{
			for (int j = 0; j < numStride; j++) heSec = heSec.getPrev().getPrev().getSym();
			tmp_features[i].push_back(heSec);

		}

		if (he.getVertex().onBoundary())
		{
			// add feature graph
			_medials[medialCounter - 3].o_features.assign(numFeatureLoops, zObjGraph());
			_medials[medialCounter - 2].o_features.assign(numFeatureLoops, zObjGraph());
			_medials[medialCounter - 1].o_features.assign(numFeatureLoops, zObjGraph());

			for (int i = 0; i < numFeatureLoops; i++)
			{
				//printf("\n feature graph ");
				createGraphFromHalfEdgeLoop(_medials[medialCounter - 3].o_features[i], tmp_features[i], zBLUE);
				createGraphFromHalfEdgeLoop(_medials[medialCounter - 2].o_features[i], tmp_features[i], zBLUE);
				createGraphFromHalfEdgeLoop(_medials[medialCounter - 1].o_features[i], tmp_features[i], zBLUE);
				tmp_features[i].clear();
			}

			// walk
			he = he.getSym();

		}
		else
		{
			he = he.getNext().getSym().getNext();
		}

	} while (he != startHe);

	_medials[0].prevMedial = &_medials[medialCounter - 1];
	_medials[medialCounter - 1].nextMedial = &_medials[0];
	// top loop
	he = he_top_Start;

	int startTopInd = medialCounter;
	do
	{
		he.getEdge().setColor(zYELLOW);
		zItMeshHalfEdge heSec = he;

		// store
		tmp_medials.push_back(he);
		_He_Medial_Map[he.getId()] = medialCounter;

		for (int i = 0; i < numFeatureLoops; i++)
		{
			for (int j = 0; j < numStride; j++) heSec = heSec.getPrev().getPrev().getSym();
			tmp_features[i].push_back(heSec);
		}

		// add medial and feature graph
		if (he.getVertex().getValence() > 4)
		{
			//printf("\n medial graph ");
			createGraphFromHalfEdgeLoop(_medials[medialCounter].o_medial, tmp_medials, zYELLOW);

			_medials[medialCounter].o_features.assign(numFeatureLoops, zObjGraph());
			for (int i = 0; i < numFeatureLoops; i++)
			{
				createGraphFromHalfEdgeLoop(_medials[medialCounter].o_features[i], tmp_features[i], zGREEN);
				tmp_features[i].clear();
			}

			_medials[medialCounter].meshHEStartEnd.assign(2, zItMeshHalfEdge());
			_medials[medialCounter].meshHEStartEnd[0] = tmp_medials[0];
			_medials[medialCounter].meshHEStartEnd[1] = tmp_medials[tmp_medials.size() - 1];


			_medials[medialCounter].nextMedial = &_medials[medialCounter + 1];
			_medials[medialCounter + 1].prevMedial = &_medials[medialCounter];

			_medials[medialCounter].type = zMedialType::Top;


			tmp_medials.clear();
			medialCounter++;
		}

		he = he.getNext().getSym().getNext();

	} while (he != he_top_Start);
	_medials[startTopInd].prevMedial = &_medials[medialCounter - 1];
	_medials[medialCounter - 1].nextMedial = &_medials[startTopInd];

	//  set sym medial indicies
	for (int i = 0; i < _He_Medial_Map.size(); i += 2)
	{
		if (_He_Medial_Map[i] != -1)
		{
			//printf("\n %i %i | %i %i ", i, _He_Medial_Map[i], i + 1, _He_Medial_Map[i + 1]);
			_medials[_He_Medial_Map[i]].symMedial = &_medials[_He_Medial_Map[i + 1]];
			_medials[_He_Medial_Map[i + 1]].symMedial = &_medials[_He_Medial_Map[i]];
		}
	}

	/// NURBS
	/*for (int i = 0; i < _medials.size(); i++)
	{
		zFnNurbsCurve fnN(_medials[i].o_medial_nurbs);
		fnN.create(_medials[i].o_medial, 0, 3, false, true, 100);

		_medials[i].o_features_nurbs.clear();
		_medials[i].o_features_nurbs.assign(_medials[i].o_features.size(), zObjNurbsCurve());
		for (int f = 0; f < _medials[i].o_features.size(); f++)
		{
			zFnNurbsCurve fn(_medials[i].o_features_nurbs[f]);
			fn.create(_medials[i].o_features[f], 0, 3, false, true, 100);
		}
	}*/
	
}

void readJSON_Plane(string path, zObjPlane& o_plane_left, zObjPlane& o_plane_right)
{
	json j;
	core.json_read(path, j);

	vector<vector<double>> endPlanes;
	vector<vector<double>> startPlanes;
	core.json_readAttribute(j, "plane_end", endPlanes);

	for (auto &f : endPlanes)
	{
		printf("\n e  plane : ");
		for(auto &f1 : f)cout << f1 << ", ";
	}

	core.json_readAttribute(j, "plane_start", startPlanes);


	for (auto& f : startPlanes)
	{
		printf("\n s  plane : ");
		for (auto& f1 : f)cout << f1 << ", ";
	}
}

void readJSONS(string dirPath,zObjMesh &o_mesh, zObjNurbsCurveArray& o_featureCurves, vector<zObjPlane>& o_splitPlanes)
{
	zStringArray filepaths;

	core.getFilesFromDirectory(filepaths, dirPath, zJSON);

	int numFeatures = 0;
	int numPlanes = 0;

	for (auto& fp : filepaths)
	{
		zStringArray splits = core.splitString(fp, "/");

		zStringArray splits_1 = core.splitString(splits[splits.size() - 1], "_");


		if (splits_1.size() > 0)
		{
			
			if (splits_1[0] == "feature") numFeatures++;

			else if (splits_1[0] == "interface") numPlanes++;
		}		
		
	}

	o_featureCurves.clear();
	o_featureCurves.assign(numFeatures, zObjNurbsCurve());

	o_splitPlanes.clear();
	o_splitPlanes.assign(numPlanes * 2, zObjPlane());

	numFeatures = 0;
	numPlanes = 0;

	for (auto& fp : filepaths)
	{
		zStringArray splits = core.splitString(fp, "/");

		zStringArray splits_1 = core.splitString(splits[splits.size() - 1], "_");


		if (splits_1.size() > 0)
		{
			if (splits_1[0] == "mesh")
			{
				zFnMesh fnMesh(o_mesh);
				fnMesh.from(fp, zJSON);				
			}

			else if (splits_1[0] == "feature")
			{
				zFnNurbsCurve fnCurve(o_featureCurves[numFeatures]);
				fnCurve.from(fp, zJSON);

				fnCurve.setDisplayColor(zRED);
				fnCurve.setDisplayWeight(3);
				numFeatures++;
			}

			else if (splits_1[0] == "interface")
			{

				readJSON_Plane(fp, o_splitPlanes[numPlanes * 2], o_splitPlanes[numPlanes * 2 + 1]);
				numPlanes++;
			}

		}

	}
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool d_Mesh = true;

bool d_Medial = false;
bool d_allMedials = false;
int medialID = 0;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

//zUtilsCore core;
zObjMesh o_inMesh_inner;

zObjMesh o_Mesh;
zObjNurbsCurveArray o_featureCurves;
vector<zObjPlane> o_splitPlanes;

vector<int> He_Medial_Map;
vector<zHalfMedial> medials;


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

	// read json
	//readJSONS("C:/Users/vishu.b/source/repos/GitZHCODE/zspace_alice/ALICE_PLATFORM/x64/Release/EXE/data/Natpower/inner", o_Mesh, o_featureCurves, o_splitPlanes);
	
	zFnMesh fnMesh_inner(o_inMesh_inner);
	fnMesh_inner.from("C:/Users/vishu.b/source/repos/GitZHCODE/zspace_alice/ALICE_PLATFORM/x64/Release/EXE/data/Natpower/mesh_inner.json", zJSON);

	computeMedialGraph(o_inMesh_inner, 42, 2, 1, He_Medial_Map, medials);


	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(o_Mesh);
	model.addObject(o_inMesh_inner);

	// set display element booleans
	o_Mesh.setDisplayElements(false, false, true);

	o_inMesh_inner.setDisplayElements(false, true, false);

	for (auto& o_curve : o_featureCurves)
	{
		model.addObject(o_curve);
		o_curve.setDisplayElements(false, true);
	}

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&d_Mesh, "d_Mesh");
	B.buttons[1].attachToVariable(&d_Mesh);

	B.addButton(&d_Medial, "d_Medial");
	B.buttons[2].attachToVariable(&d_Medial);

}

void update(int value)
{
	o_inMesh_inner.setDisplayObject(d_Mesh);

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

	model.draw();

	if (d_Medial)
	{
		if (d_allMedials)
		{
			for (auto& m : medials)
			{
				m.o_medial.draw();

				for (auto& g : m.o_features) g.draw();
			}
		}
		else
		{
			
			medials[medialID].o_medial.draw();
			for (auto& g : medials[medialID].o_features) g.draw();
			
		}
	}


	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;	

	if (k == 'w')
	{
		if (medialID < medials.size() - 1)medialID++;;
	}
	if (k == 's')
	{
		if (medialID > 0)medialID--;;
	}

	if (k == 'd') d_allMedials = !d_allMedials;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
