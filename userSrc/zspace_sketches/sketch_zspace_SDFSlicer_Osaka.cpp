//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

#include <zApp/include/zTsGeometry.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

zUtilsCore core;
zColor Magenta(1, 0, 1, 1);

bool checkBiggerBounds(zPointArray& g0, zPointArray& g1)
{
	zDomain<zPoint> bb_0;	
	core.getBounds(g0, bb_0.min, bb_0.max);
	zVector dims_0 = core.getDimsFromBounds(bb_0.min, bb_0.max);
	float area_0 = dims_0.x * dims_0.y;

	zDomain<zPoint> bb_1;
	core.getBounds(g1, bb_1.min, bb_1.max);
	zVector dims_1 = core.getDimsFromBounds(bb_1.min, bb_1.max);
	float area_1 = dims_1.x * dims_1.y;

	return (area_0 > area_1);
}

void create_spacerMeshes(zObjMeshScalarField& _o_field, zObjMeshArray& _o_meshes, zItMeshHalfEdgeArray &pHEdges, int meshID, float width)
{
	zFnMeshScalarField fnField(_o_field);

	zScalar* scalars = fnField.getRawFieldValues();
	zPoint* field_vPositions = fnField.fnMesh.getRawVertexPositions();

	zPointArray vPositions;
	zIntArray pConnects, pCounts;

	zVector dir(0, 1, 0);
	zVector norm(0, 0, 1);
	zVector cross(1, 0, 0);

	for (auto &he : pHEdges)
	{
		bool interiorEdge = false;	
	
		if (scalars[he.getVertex().getId()] < 0 && scalars[he.getStartVertex().getId()] < 0)interiorEdge = true;
		

		if (interiorEdge)
		{
			zPoint v0 = field_vPositions[he.getVertex().getId()] + (cross * width * -0.5);
			int vID0 = -1;
			bool chk0 = core.checkRepeatElement(v0, vPositions, vID0);
			if (!chk0)
			{
				vID0 = vPositions.size();
				vPositions.push_back(v0);
			}

			zPoint v1 = field_vPositions[he.getStartVertex().getId()] + (cross * width * -0.5);
			int vID1 = -1;
			bool chk1 = core.checkRepeatElement(v1, vPositions, vID1);
			if (!chk1)
			{
				vID1 = vPositions.size();
				vPositions.push_back(v1);
			}

			zPoint v2 = field_vPositions[he.getStartVertex().getId()] + (cross * width * 0.5);
			int vID2 = -1;
			bool chk2 = core.checkRepeatElement(v2, vPositions, vID2);
			if (!chk2)
			{
				vID2 = vPositions.size();
				vPositions.push_back(v2);
			}

			zPoint v3 = field_vPositions[he.getVertex().getId()] + (cross * width * 0.5);
			int vID3 = -1;
			bool chk3 = core.checkRepeatElement(v3, vPositions, vID3);
			if (!chk3)
			{
				vID3 = vPositions.size();
				vPositions.push_back(v3);
			}

			pConnects.push_back(vID0);
			pConnects.push_back(vID1);
			pConnects.push_back(vID2);
			pConnects.push_back(vID3);

			pCounts.push_back(4);
		}

	}

	zFnMesh fnMesh(_o_meshes[meshID]);
	fnMesh.create(vPositions, pCounts, pConnects);

	printf("\n spacer %i %i %i ", fnMesh.numVertices(), fnMesh.numEdges(), fnMesh.numPolygons());
}

void create_slicedMeshes(zObjMeshScalarField& _o_field, zObjGraphArray& _o_graphs, zObjMeshArray& _o_meshes, zObjMeshArray& _o_meshes_spacers, zItMeshHalfEdgeArray& pHEdges, int numSmooth, float sWidth, float threshold)
{
	
	zFnMeshScalarField fnField(_o_field);
	cout<< "\n numVerts : " << fnField.fnMesh.numVertices() << endl;

	int numGraphs = _o_graphs.size();

	_o_meshes.clear();
	_o_meshes.assign(numGraphs, zObjMesh());

	_o_meshes_spacers.clear();
	_o_meshes_spacers.assign(numGraphs, zObjMesh());

	//string folderName = "data/Slicer/out/contours";
	//for (const auto& entry : std::filesystem::directory_iterator(folderName)) std::filesystem::remove_all(entry.path());

	int graphID = 0;
	zPointArray vPositions, vPositions_next;
	zScalarArray scalars;

	for (int k =0; k< numGraphs; k++)
	{
		//if (k > 0) continue;


		zFnGraph fnIsoGraph(_o_graphs[k]);
		zPoint* raw_vPositions = fnIsoGraph.getRawVertexPositions();

		bool chkVoid = false;
		if (k < numGraphs - 1)
		{
			zFnGraph fnIsoGraph_next(_o_graphs[k + 1]);
			zPoint* raw_vPositions_next = fnIsoGraph_next.getRawVertexPositions();

			if (abs(raw_vPositions[0].z - raw_vPositions_next[0].z) < EPS) chkVoid = true;
		}
		
		//override for some instances
		//chkVoid = false;
				
		vPositions.clear();
		fnIsoGraph.getVertexPositions(vPositions);

		for (int i = 0; i < fnIsoGraph.numVertices(); i++) raw_vPositions[i].z = 0;
		
		zScalarArray polyField_0;
		fnField.getScalars_Polygon(polyField_0, _o_graphs[k], false);

		if (chkVoid)
		{
			zFnGraph fnIsoGraph_next(_o_graphs[k + 1]);
			zPoint* raw_vPositions_next = fnIsoGraph_next.getRawVertexPositions();

			vPositions_next.clear();
			fnIsoGraph_next.getVertexPositions(vPositions_next);

			for (int i = 0; i < fnIsoGraph_next.numVertices(); i++) raw_vPositions_next[i].z = 0;

			zScalarArray polyField_1;
			fnField.getScalars_Polygon(polyField_1, _o_graphs[k + 1], false);


			bool chkArea = checkBiggerBounds(vPositions, vPositions_next);

			zScalarArray boolean_0;
			if (chkArea) fnField.boolean_subtract(polyField_0, polyField_1, boolean_0, false);
			else fnField.boolean_subtract(polyField_1, polyField_0, boolean_0, false);

			fnField.smoothField(boolean_0, numSmooth);

			fnField.setFieldValues(boolean_0);


			for (int i = 0; i < fnIsoGraph_next.numVertices(); i++) raw_vPositions_next[i].z = vPositions_next[i].z;

		}
		else
		{
			fnField.smoothField(polyField_0, numSmooth);
			fnField.setFieldValues(polyField_0);
		}

		// Contour
		fnField.updateColors();
		for (int i = 0; i < fnIsoGraph.numVertices(); i++) raw_vPositions[i].z = vPositions[i].z;

		printf("\n slice %i : ", k);
		fnField.getIsolineMesh(_o_meshes[k], threshold);

		zFnMesh fnMesh(_o_meshes[k]);
		
		if (fnMesh.numVertices() > 0)
		{
			zPoint* mVPositions = fnMesh.getRawVertexPositions();
			for (int i = 0; i < fnMesh.numVertices(); i++) mVPositions[i].z = vPositions[0].z;
		}
		
		create_spacerMeshes(_o_field, _o_meshes_spacers, pHEdges, k, sWidth);
		
		zFnMesh fnMesh_spacer(_o_meshes_spacers[k]);

		if (fnMesh_spacer.numVertices() > 0)
		{
			zPoint* mVPositions = fnMesh_spacer.getRawVertexPositions();
			for (int i = 0; i < fnMesh_spacer.numVertices(); i++) mVPositions[i].z = vPositions[0].z;
		}

		if (chkVoid) k++;
	}

}

void patternCurves(zObjMeshScalarField& _o_field, int startVID, zItMeshHalfEdgeArray& pHEdges, int numSteps = 3, zVector dir = zVector(1,0,0))
{
	zFnMeshScalarField fnField(_o_field);

	pHEdges.clear();

	zFnMesh fnMesh(_o_field);

	zItMeshVertex v(_o_field, startVID);
	zItMeshHalfEdgeArray cHEdges;

	v.getConnectedHalfEdges(cHEdges);

	zItMeshHalfEdge startHe;;
	float ang = 360;
	for (auto& cHE : cHEdges)
	{
		if (cHE.getVector().angle(dir) < ang)
		{
			startHe = cHE;
			ang = cHE.getVector().angle(dir);
		}
	}

	bool flip = false;
	if (!startHe.onBoundary())
	{
		startHe = startHe.getSym();
		flip = true;
	}

	for(int i =0; i< numSteps; i++)	 startHe = (flip) ? startHe.getPrev() : startHe.getNext();;

	zItMeshHalfEdge he = startHe;

	bool exitV = false;

	do
	{
		if (flip)
		{
			if (he.getStartVertex().checkValency(2)) exitV = true;
		}
		else
		{
			if (he.getVertex().checkValency(2)) exitV = true;
		}

		zItMeshHalfEdge heU = (flip) ? he.getSym().getPrev() : he.getSym().getNext();

		bool exit = false;
		do
		{
			if (flip)
			{
				if (heU.getStartVertex().onBoundary()) exit = true;
			}
			else
			{
				if (heU.getVertex().onBoundary()) exit = true;
			}
			

			heU.getEdge().setColor(Magenta);
			pHEdges.push_back(heU);

			heU = (flip) ? heU.getPrev().getSym().getPrev() : heU.getNext().getSym().getNext();

		} while (!exit);

		//he.getEdge().setColor(zColor(1, 1, 0, 1));

		for (int i = 0; i < numSteps; i++)
		{
			he = (flip) ? he.getPrev() : he.getNext();;

			if (flip)
			{
				if (he.getStartVertex().checkValency(2)) exitV = true;
			}
			else
			{
				if (he.getVertex().checkValency(2)) exitV = true;
			}
		}

	} while (!exitV);

}

void checkLoops(zObjGraphArray& _o_graphs)
{
	int numGraphs = _o_graphs.size();
	

	// vertex sequence
	zIntArray vSequence;
	zItGraphVertexArray vArray;

	for (int i = 0; i < numGraphs; i++)
	{
		vArray.clear();

		for (zItGraphVertex v(_o_graphs[i]); !v.end(); v++)
		{
			if (!v.checkValency(2))
			{
				vArray.push_back(v);

				v.setColor(zColor(0, 1, 0, 1));
			}
		}

		//printf("\n %i - valence 2 verts  %i", i, vArray.size());

		zFnGraph fnIsoGraph(_o_graphs[i]);

		if (fnIsoGraph.numEdges() > 0)
		{
			if (vArray.size() == 0) fnIsoGraph.setEdgeColor(zColor(1, 0, 0, 1));
			if (vArray.size() == 1) fnIsoGraph.setEdgeColor(zColor(0, 1, 0, 1));
			if (vArray.size() == 2) fnIsoGraph.setEdgeColor(zColor(0, 0, 1, 1));
			if (vArray.size() == 3) fnIsoGraph.setEdgeColor(zColor(1, 1, 1, 1));
			if (vArray.size() > 3) fnIsoGraph.setEdgeColor(zColor(0, 0, 0, 1));
		}
				
	}

	
}

void smoothGraphs(zObjGraphArray& _o_graphs, int steps)
{
	int numGraphs = _o_graphs.size();

	// vertex sequence
	for (int i = 0; i < numGraphs; i++)
	{
		zFnGraph fnIsoGraph(_o_graphs[i]);
		fnIsoGraph.averageVertices(steps);	
	}


}

////////

bool computeFRAMES = false;
bool computeSDF = false;
bool computeSMOOTH = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zObjMeshScalarField o_field;
zObjGraphArray o_graphs;
zObjMeshArray o_meshes;
zObjMeshArray o_meshes_spacers;

zItMeshHalfEdgeArray patternHEdges;

zDomain<zPoint> bb(zPoint(-15, -8, 0), zPoint(15, 22, 0));
int resX = 193;
int resY = 193;

int numSteps = 3;
int numSmooth = 2;

float spacerWidth = 0.05;
float sdf_threshold = 0.1;

bool dSliceMesh = true;
bool dGradientMesh = true;
bool dPrintPlane = false;
bool dSectionGraphs = true;
bool dContourGraphs = false;
bool dTrimGraphs = false;
bool dField = false;
bool exportSDF = false;

bool displayAllGraphs = false;
int currentGraphId = 0;
int totalGraphs = 0;

float printPlaneSpace = 0.1;
float printLayerWidth = 0.01;

zDomainFloat printHeightDomain(0.05, 0.05);


int SDFFunc_Num = 4;
bool SDFFunc_NumSmooth = 0;

int numSDFLayers = 3;
bool allSDFLayers = true;

/*!<Tool sets*/
zTsSDFSlicer mySlicer;



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

	// read graphs
	zStringArray files;
	string fileDir = "data/Slicer/in/";
	core.getFilesFromDirectory(files, fileDir, zJSON);

	o_graphs.assign(files.size(), zObjGraph());

	for (auto& s : files)
	{
		zStringArray split_0 = core.splitString(s, ".");
		zStringArray split_1 = core.splitString(split_0[0], "_");

		int id = atoi(split_1[split_1.size() - 1].c_str());

		//printf("\n %i | %s ", id, s.c_str());

		zObjGraph o_temp;
		zFnGraph fnTemp(o_temp);
		fnTemp.from(s, zJSON);

		zPointArray vPositions;
		fnTemp.getVertexPositions(vPositions);
		vPositions.pop_back();

		zIntArray eConnects;
		fnTemp.getEdgeData(eConnects);
		eConnects[eConnects.size() - 1] = 0;


		zFnGraph fnGraph(o_graphs[id]);
		fnGraph.create(vPositions, eConnects);
	}
	
	totalGraphs = o_graphs.size();

	//--- FIELD

	zFnMeshScalarField fnField(o_field);
	fnField.create(bb.min, bb.max, resX, resY, 1, true, false);

	zDomainColor dCol(zColor(1,0,0,1), zColor(0, 1, 0, 1));
	fnField.setFieldColorDomain(dCol);

	patternCurves(o_field, 0, patternHEdges, numSteps, zVector(1,0,0));
	printf("\n pattern Edges : %i ", patternHEdges.size());

	//--- closest point test 
	//zPointArray testPts;
	//testPts.push_back(zPoint(5.66, 4.94, 6.40));
	//zIntArray faceIDs ;
	//zPointArray cPts;
	//mySlicer.computeClosestPointToGradientMesh(testPts, faceIDs, cPts);

	//printf("\n %i | %1.2f %1.2f %1.2f ", faceIDs[0], cPts[0].x, cPts[0].y, cPts[0].z);
		
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&computeFRAMES, "computeFRAMES");
	B.buttons[0].attachToVariable(&computeFRAMES);

	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	B.addButton(&dSliceMesh, "dSliceMesh");
	B.buttons[2].attachToVariable(&dSliceMesh);

	B.addButton(&dSliceMesh, "dGradientMesh");
	B.buttons[3].attachToVariable(&dGradientMesh);		

	B.addButton(&dPrintPlane, "dPrintPlane");
	B.buttons[4].attachToVariable(&dPrintPlane);

	B.addButton(&dSectionGraphs, "dSectionGraphs");
	B.buttons[5].attachToVariable(&dSectionGraphs);

	B.addButton(&dContourGraphs, "dContourGraphs");
	B.buttons[6].attachToVariable(&dContourGraphs);

	B.addButton(&dTrimGraphs, "dTrimGraphs");
	B.buttons[7].attachToVariable(&dTrimGraphs);	

	B.addButton(&dField, "dField");
	B.buttons[8].attachToVariable(&dField);
	
}

void update(int value)
{
	
	
	if (computeFRAMES)
	{
		checkLoops(o_graphs);

		computeFRAMES = !computeFRAMES;
	}

	if (computeSDF)
	{
		create_slicedMeshes(o_field, o_graphs, o_meshes,o_meshes_spacers, patternHEdges, numSmooth, spacerWidth, sdf_threshold);
		computeSDF = !computeSDF;
	}

	if (computeSMOOTH)
	{
		smoothGraphs(o_graphs, 1);

		computeSMOOTH = !computeSMOOTH;
	}

	if (exportSDF)
	{

		int numGraphs = totalGraphs;
		
		
		string folderName = "data/Slicer/out_mesh/slabs";
		for (const auto& entry : std::filesystem::directory_iterator(folderName)) std::filesystem::remove_all(entry.path());

		int meshID = 0;
		for (auto& m : o_meshes)
		{
			zFnMesh fnMesh(m);
			if (fnMesh.numVertices() == 0)
			{
				meshID++;
				continue;
			}

			string outName1 = folderName + "/outSlab_";
			outName1 += to_string(meshID) + ".json";

			fnMesh.to(outName1, zJSON);
			meshID++;
			
		}
			
		printf("\n %i slab meshes exported. ", meshID);

		string folderName_1 = "data/Slicer/out_mesh/spacers";
		for (const auto& entry : std::filesystem::directory_iterator(folderName_1)) std::filesystem::remove_all(entry.path());

		meshID = 0;
		for (auto& m : o_meshes_spacers)
		{
			zFnMesh fnMesh(m);
			if (fnMesh.numVertices() == 0)
			{
				meshID++;
				continue;
			}

			string outName1 = folderName_1 + "/outSpacer_";
			outName1 += to_string(meshID) + ".json";

			fnMesh.to(outName1, zJSON);
			meshID++;

		}

		printf("\n %i spacer meshes exported. ", meshID);


		exportSDF = !exportSDF;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		
	}



	if (dSectionGraphs)
	{
		int numGraphs = totalGraphs;
		

		if (displayAllGraphs)
		{
			for (auto& g : o_graphs)
			{
				g.setDisplayElements(true, true);
				g.draw();
			}
		}
		else
		{
			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{	
				o_graphs[i].setDisplayElements(true, true);
				o_graphs[i].draw();
			}

		}
			
		
	}

	if (dContourGraphs)
	{
		int numGraphs = totalGraphs;


		if (displayAllGraphs)
		{
			for (auto& m : o_meshes)
			{
				m.setDisplayElements(false, true, false);
				m.draw();
			}
		}
		else
		{
			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				o_meshes[i].setDisplayElements(false, true, false);
				o_meshes[i].draw();
			}

		}

		
	}

	if (dTrimGraphs)
	{
		int numGraphs = totalGraphs;


		if (displayAllGraphs)
		{
			for (auto& m : o_meshes_spacers)
			{
				m.setDisplayElements(false, true, true);
				m.draw();
			}
		}
		else
		{
			int i = currentGraphId;

			if (numGraphs > 0 && i >= 0 & i < numGraphs)
			{
				o_meshes_spacers[i].setDisplayElements(false, true, true);
				o_meshes_spacers[i].draw();
			}

		}
	}

	if (dField)
	{	

		o_field.setDisplayElements(false, true, true);
		o_field.draw();
				
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Layers #:" + to_string(totalGraphs), vec(winW - 350, winH - 800, 0));
	drawString("Current Layer #:" + to_string(currentGraphId), vec(winW - 350, winH - 775, 0));

	
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computeFRAMES = true;;
	if (k == 'o') computeSDF = true;;
	if (k == 'a') computeSMOOTH = true;

	if (k == 'w')
	{
		if (currentGraphId < totalGraphs - 1)currentGraphId++;;
	}
	if (k == 's')
	{
		if(currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') displayAllGraphs = !displayAllGraphs;

	if (k == 'e') exportSDF = true;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
