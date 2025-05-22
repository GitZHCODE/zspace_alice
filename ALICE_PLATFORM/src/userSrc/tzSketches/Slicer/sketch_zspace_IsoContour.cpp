//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/local_basis.h>
//#include <igl/readOFF.h>
#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/PI.h>

#include <unordered_set>
#include <unordered_map>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool d_inMesh = true;
bool d_paramMesh = true;
bool d_graph = true;

double background = 0.35;
int boundaryID = 0;
int numGraphs = 20;

int displayIndex = 0;

//not used in current method
double user_weight = 0.0;


////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjGraphArray o_sectionGraphs;

////// --- GUI OBJECTS ----------------------------------------------------

////// --- igl Objects --------------------------------------------------
// Mesh
Eigen::MatrixXd V;
Eigen::MatrixXi F;

// Constrained faces id
Eigen::VectorXi b;

// Cosntrained faces representative vector
Eigen::MatrixXd bc;

class IsoContour
{
public:
	void setMesh(zTsMeshParam& _oMeshParam) {
		oMesh = _oMeshParam.getRawInMesh();
	}

	unordered_map<int, vector<int>> getBoundaries() {
		return boundaries;
	}

	//compute single graph
	void compute(zObjGraphArray& o_sectionGraphs, int _boundaryID, int _numGraphs) {
		findBoundaries();
		computeHeatMap(_boundaryID);

		vector<int> v = boundaries[_boundaryID];

		for (int i = 0; i < v.size(); i++)
		{
			scalars[v[i]] = 0.0;
		}

		computeIsoContour(o_sectionGraphs, _numGraphs);

		//for (int j = 0; j < scalars.size(); j++) {
		//	if (find(v.begin(), v.end(), scalars[j]) != v.end()) {
		//		scalars[j] = 0.0;
		//	}
		//	v = boundaries[1];
		//	if (find(v.begin(), v.end(), scalars[j]) != v.end()) {
		//		scalars[j] = 0.0;
		//	}
		//}

		//vector<int> v = boundaries[_boundaryID];

		//for (int i = 0; i < v.size(); i++)
		//{
		//	scalars[v[i]] = 0.0;
		//}
	}

	//compute two interpolated graphs
	void compute(zObjGraphArray& o_sectionGraphs, int displayIndex, int _boundaryID_start, int _boundaryID_end, float _weight, int _numGraphs) {

		findBoundaries();

		zFloatArray scalars_start, scalars_end;
		float min_start, max_start;

		//compute each map of picked boundary
		computeHeatMap(_boundaryID_start);
		scalars_start = scalars;

		computeHeatMap(_boundaryID_end);
		scalars_end = scalars;

		vector<int> v_start = boundaries[_boundaryID_start];
		vector<int> v_end = boundaries[_boundaryID_end];

		for (int i = 0; i < v_start.size(); i++)
		{
			scalars_start[v_start[i]] = 0.0;
		}

		for (int i = 0; i < v_end.size(); i++)
		{
			scalars_end[scalars_end[i]] = 0.0;
		}

		computeInterpolatedIsoContour(o_sectionGraphs, displayIndex, _numGraphs, scalars_start, scalars_end);
	}



private:
	zTsMeshParam* oMeshParam;
	zObjMesh* oMesh;
	unordered_map<int, vector<int>> boundaries;
	zFloatArray scalars;
	zUtilsCore coreUtils;

	void findBoundaries()
	{
		/* find all boundary vertices */
		zIntArray boundaryVertices;

		// Loop through all vertices and add boundary vertices to the boundaryVertices array
		zItMeshVertex v(*oMesh);
		for (v.begin(); !v.end(); v++) {
			if (v.onBoundary()) boundaryVertices.push_back(v.getId());
		}


		unordered_set<int> visited;
		int numBoundaries = 0;

		// Loop until all boundary vertices have been visited
		while (visited.size() < boundaryVertices.size())
		{
			// Find an unvisited boundary vertex to start the boundary loop
			auto startVertex = find_if(boundaryVertices.begin(), boundaryVertices.end(), [&](int id) {
				return visited.count(id) == 0;
				});

			// If there are no unvisited boundary vertices, exit the loop
			if (startVertex == boundaryVertices.end())
				break;

			// Create a vector to store the current boundary
			vector<int> currentBoundary;

			// Find a half-edge on the boundary starting at the current vertex
			zItMeshVertex v(*oMesh, *startVertex);
			zItMeshHalfEdge start(*oMesh), next(*oMesh);

			zIntArray connectedHalfEdges;
			v.getConnectedHalfEdges(connectedHalfEdges);

			auto he = find_if(connectedHalfEdges.begin(), connectedHalfEdges.end(), [&](int id) {
				return zItMeshHalfEdge(*oMesh, id).onBoundary();
				});
			start = zItMeshHalfEdge(*oMesh, *he);

			// Mark the starting vertex as visited and add it to the current boundary
			visited.insert(start.getVertex().getId());
			currentBoundary.push_back(start.getVertex().getId());
			next = start.getNext();

			// Loop through all the half-edges on the boundary and add their vertices to the current boundary
			while (next != start) {
				visited.insert(next.getVertex().getId());
				currentBoundary.push_back(next.getVertex().getId());
				next = next.getNext();
			}

			// Add the current boundary to the boundaries map and increment the number of boundaries
			boundaries[numBoundaries++] = currentBoundary;
		}

		//// Print the boundaries with their keys
		//for (auto it = boundaries.begin(); it != boundaries.end(); it++) {
		//	cout << "Boundary " << it->first << ": ";
		//	for (int i = 0; i < it->second.size(); i++) {
		//		cout << it->second[i] << " ";
		//	}
		//	cout << std::endl;
		//}
	}

	void computeHeatMap(int _boundaryID)
	{
		zIntArray vertices = boundaries[_boundaryID];
		myMeshParam.computeGeodesics_Heat(vertices, scalars);
		//scalars = myMeshParam.getGeodesicsHeatDistance();
	}

	void computeIsoContour(zObjGraphArray& o_sectionGraphs, int _numGraphs)
	{
		// Find min and max value of the scalar
		float min = scalars[0];
		float max = scalars[0];
		for (int i = 1; i < scalars.size(); i++) {
			if (scalars[i] > max) {
				max = scalars[i];
			}
			if (scalars[i] < min) {
				min = scalars[i];
			}
		}

		// Generate isosurfaces and create graphs
		// create a vector of zObjGraph objects to store the graphs
		zObjGraph tempGraph;
		o_sectionGraphs.assign(_numGraphs, tempGraph);

		for (int i = 0; i < _numGraphs; i++)
		{
			// Calculate the threshold value for the current isosurface
			float threshold = min + i * ((max - min) / _numGraphs);

			// Generate the isosurface using the threshold value
			zPointArray positions;
			zIntArray edgeConnects;
			zColorArray vColors;
			int pres = 3;

			zFnMesh fnMesh(*oMesh);
			fnMesh.getIsoContour(scalars, threshold, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

			// Create graph from the isosurface
			zFnGraph tempFn(o_sectionGraphs[i]);
			tempFn.create(positions, edgeConnects);
			tempFn.setEdgeColor(zColor(255, 255, 255));
			tempFn.setVertexColors(vColors, false);
		}
	}


	void computeInterpolatedIsoContour(zObjGraphArray& o_sectionGraphs, int displayIndex, int _numGraphs, zFloatArray& scalars_start, zFloatArray& scalars_end)
	{

		// Generate isosurfaces and create graphs
		// create a vector of zObjGraph objects to store the graphs
		zObjGraph tempGraph;
		o_sectionGraphs.assign(_numGraphs, tempGraph);

		//for (int i = 0; i < _numGraphs; i++) {
		zFloatArray currentScalars;
		currentScalars.assign(scalars_start.size(), -1);


		zColor col(0.8, 0, 0, 1);
		zFnMesh fn(*oMesh);

		float weight = ((float)(displayIndex + 1) / (float)_numGraphs);
		for (int j = 0; j < currentScalars.size(); j++) {
			currentScalars[j] = weight * scalars_start[j] - (1 - weight) * scalars_end[j];

		}

		cout << endl;
		cout << "boundaries.size():" << boundaries.size() << endl;

		cout << "currentScalars:" << currentScalars[0] << endl;
		cout << "scalars_start:" << scalars_start[0] << endl;
		cout << "scalars_end:" << scalars_end[0] << endl;
		cout << "weight:" << weight << endl;
		cout << "_numGraphs:" << _numGraphs << endl;
		cout << "displayIndex:" << displayIndex << endl;
		cout << "(displayIndex / _numGraphs):" << (displayIndex / _numGraphs) << endl;



		computeColor(currentScalars);

		// Calculate the threshold value for the current isosurface
		float threshold = 0.0;

		// Generate the isosurface using the threshold value
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vColors;
		int pres = 3;

		zFnMesh fnMesh(*oMesh);
		fnMesh.getIsoContour(currentScalars, threshold, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));


		// Create graph from the isosurface
		zFnGraph tempFn(o_sectionGraphs[displayIndex]);
		tempFn.create(positions, edgeConnects);
		tempFn.setEdgeColor(zColor(255, 255, 255));
		tempFn.setVertexColors(vColors, false);
		//}
	}


	void computeInterpolatedIsoContour_user(zObjGraphArray& o_sectionGraphs, int _numGraphs, float _weight, zFloatArray scalars_start, zFloatArray scalars_end)
	{

		for (int i = 0; i < scalars.size(); i++) {
			scalars[i] = _weight * scalars_end[i] + (1 - _weight) * scalars_start[i];
		}

		// Find min and max value of the start scalar
		float min = scalars[0];
		float max = scalars[0];
		for (int i = 1; i < scalars.size(); i++) {
			if (scalars[i] > max) {
				max = scalars[i];
			}
			if (scalars[i] < min) {
				min = scalars[i];
			}
		}

		// Generate isosurfaces and create graphs
		// create a vector of zObjGraph objects to store the graphs
		zObjGraph tempGraph;
		o_sectionGraphs.assign(_numGraphs, tempGraph);

		for (int i = 0; i < _numGraphs; i++) {

			// Calculate the threshold value for the current isosurface
			float threshold = min + i * ((max - min) / _numGraphs);

			// Generate the isosurface using the threshold value
			zPointArray positions;
			zIntArray edgeConnects;
			zColorArray vColors;
			int pres = 3;

			zFnMesh fnMesh(*oMesh);
			fnMesh.getIsoContour(scalars, threshold, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

			// Create graph from the isosurface
			zFnGraph tempFn(o_sectionGraphs[i]);
			tempFn.create(positions, edgeConnects);
			tempFn.setEdgeColor(zColor(255, 255, 255));
			tempFn.setVertexColors(vColors, false);
		}
	}

	void computeColor(zFloatArray& scalars) {
		// compute vertex color from Geodesic Distance
		zFnMesh fnTriMesh(*oMesh);
		zColor* vColors = fnTriMesh.getRawVertexColors();

		float min, max;
		min = coreUtils.zMin(scalars);
		max = coreUtils.zMax(scalars);

		zDomainFloat distanceDomain(min, max);
		zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

		for (int i = 0; i < fnTriMesh.numVertices(); i++)
		{
			vColors[i] = coreUtils.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
		}

		fnTriMesh.computeFaceColorfromVertexColor();

	}

	void blendColor()
	{
		// compute vertex color from Geodesic Distance
		zFnMesh fnTriMesh(*oMesh);
		fnTriMesh.setVertexColor(zColor(0, 0, 0));
		zColor* vColors = fnTriMesh.getRawVertexColors();

		// Find min and max value of the scalar
		float min = scalars[0];
		float max = scalars[0];
		for (int i = 1; i < scalars.size(); i++) {
			if (scalars[i] > max) {
				max = scalars[i];
			}
			if (scalars[i] < min) {
				min = scalars[i];
			}
		}

		zDomainFloat distanceDomain(min, max);
		zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

		for (int i = 0; i < fnTriMesh.numVertices(); i++)
		{
			vColors[i] = coreUtils.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
		}

		fnTriMesh.computeFaceColorfromVertexColor();

	}
};

IsoContour iso;

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
	//myMeshParam.setFromFile("data/Parameterization/heatTest_extrude.obj", zOBJ);
	myMeshParam.setFromFile("data/Parameterization/heatTest_ring.obj", zOBJ);
	//myMeshParam.setFromFile("data/Parameterization/cylinder.obj", zOBJ);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans
	zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	o_paramMesh->setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&user_weight, "user_weight");
	S.sliders[1].attachToVariable(&user_weight, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[2].attachToVariable(&d_paramMesh);

	B.addButton(&d_paramMesh, "d_graph");
	B.buttons[3].attachToVariable(&d_graph);

}

void update(int value)
{
	if (compute)
	{
		//// Harmonic Parameterization
		//myMeshParam.computeParam_Harmonic();

		//// ARAP Parameterization
		//myMeshParam.computeParam_ARAP();

		// LSCM Parameterization
		//myMeshParam.computeParam_LSCM();

		// Geodesics heat method
		// Compute geodesics heat distance
		iso.setMesh(myMeshParam);
		//iso.compute(o_sectionGraphs, boundaryID, numGraphs);

		zIntArray boundaries = { 0 ,1 };
		iso.compute(o_sectionGraphs, displayIndex, boundaries[0], boundaries[1], (float)user_weight, numGraphs);




		compute = !compute;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	if (d_inMesh)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		o_inMesh->draw();

	}

	if (d_paramMesh)
	{
		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
		o_paramMesh->draw();

	}

	if (d_graph)
	{
		for (auto& o : o_sectionGraphs)
			o.draw();
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;
	if (k == '+')
	{
		boundaryID++;
		compute = true;
	}
	if (k == '-')
	{
		boundaryID--;
		compute = true;
	}

	if (k == '8')
	{
		displayIndex++;
		compute = true;
	}
	if (k == '2')
	{
		displayIndex--;
		compute = true;
	}
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
