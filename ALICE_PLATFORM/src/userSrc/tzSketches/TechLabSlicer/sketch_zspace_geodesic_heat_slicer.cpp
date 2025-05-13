#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

#include "zSpatialField.cpp"
#include "GraphProcessor.cpp"

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;



void computeBoundaryVertices(zObjMesh* o_mesh , zIntArray& vIDs, zIntArray &bIDs)
{
	bIDs.clear();

	if(vIDs.size()>0)
	for (int j = 0; j < vIDs.size(); j++)
	{
		zItMeshVertex v(*o_mesh, vIDs[j]);
		zItMeshHalfEdgeArray cHEdges;
		v.getConnectedHalfEdges(cHEdges);

		zItMeshHalfEdge startHe, He;
		for (auto& cHE : cHEdges)
		{
			if (cHE.onBoundary())startHe = cHE;
		}

		He = startHe;

		do
		{
			bIDs.push_back(He.getStartVertex().getId());
			He = He.getNext();

		} while (He != startHe);
	}
	

}

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars, int currentContourID, int totalContours, zObjGraphArray &o_contourGraphs)
{

	if (currentContourID >= o_contourGraphs.size())
	{
		cout << "Error: currentContourID greater than or eual to size of o_contourGraphs." << endl;
		return;
	}

	// weighted scalars
	float weight = ((float)(currentContourID + 1) / (float)totalContours);

	// Generate the isocontour using the threshold value
	zPointArray positions;
	zIntArray edgeConnects;
	zColorArray vColors;
	int pres = 6;
	zFnMesh fnMesh(*o_mesh);

	fnMesh.getIsoContour(scalars, weight, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

	// Create graph from the isocontour
	zFnGraph tempFn(o_contourGraphs[currentContourID]);
	tempFn.create(positions, edgeConnects);

	// color mesh
	zColor *mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars_start, zFloatArray& scalars_end, int currentContourID, int totalContours, zObjGraphArray& o_contourGraphs)
{

	if (currentContourID >= o_contourGraphs.size())
	{
		cout << "Error: currentContourID greater than or eual to size of o_contourGraphs." << endl;
		return;
	}

	zFloatArray scalars;
	scalars.assign(scalars_start.size(), -1);

	// weighted scalars
	float weight = ((float)(currentContourID + 1) / (float)totalContours);

	for (int j = 0; j < scalars.size(); j++)
	{
		scalars[j] = weight * scalars_start[j] - (1 - weight) * scalars_end[j];
	}

	// Generate the isocontour using the threshold value
	zPointArray positions;
	zIntArray edgeConnects;
	zColorArray vColors;
	int pres = 3;
	zFnMesh fnMesh(*o_mesh);
	fnMesh.getIsoContour(scalars, 0.0, positions, edgeConnects, vColors, pres, pow(10, -1 * pres));

	// Create graph from the isocontour
	zFnGraph tempFn(o_contourGraphs[currentContourID]);
	tempFn.create(positions, edgeConnects);
	tempFn.setEdgeColor(zColor(255, 255, 255, 1));
	tempFn.setEdgeWeight(2);
	tempFn.setVertexColors(vColors, false);

	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}

void colorMesh(zObjMesh* o_mesh, zFloatArray& scalars)
{
	zFnMesh fnMesh(*o_mesh);

	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();

}


////////////////////////////////////////////////////////////////////////// General

bool compute_heat = false;
bool compute_contour = false;
bool compute_color = false;
bool compute_color_start = true;

bool d_inMesh = true;
bool d_paramMesh = true;
bool d_sliceMesh = true;
bool d_ContourGraphs = true;
bool d_AllGraphs = false;

bool reload = false;
bool exportTo = false;

double background = 0.35;

double sampleDist = 0.01;
int currentGraphId = 0;
double totalGraphs = 50;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjMesh o_sliceMesh;
zObjGraphArray o_contours;
zFloatArray geodesics_start;
zFloatArray geodesics_end;
zIntArray start_boundary_ids;
zIntArray end_boundary_ids;

vector<zObjGraphArray> separatedContours;
zSpatialField spatialField;
GraphProcessor graphProcessor;

//string inPath_obj = "data/Slicer/chair.obj";
//string inPath = "data/Slicer/chair.json";
string outPath = "data/TechLabSlicer/outFolder";
//string inPath_usda = "data/TechLabSlicer/chair_seat.usda";
//string inPath_usda = "data/TechLabSlicer/pp_one_one.usda";
string inPath_usda = "data/TechLabSlicer/pp_one_mul.usda";
//string inPath_usda = "data/TechLabSlicer/pp_two_mul.usda";

////// --- GUI OBJECTS ----------------------------------------------------

void fromFile()
{
	//clear current set of things
	zFnMesh fnSliceMesh(o_sliceMesh);
	geodesics_start.clear();
	geodesics_end.clear();
	start_boundary_ids.clear();
	end_boundary_ids.clear();

	//read mesh
	//fnSliceMesh.from(inPath_obj, zOBJ);
	//myMeshParam = zTsMeshParam();
	//myMeshParam.setFromFile(inPath_obj, zOBJ);

	fnSliceMesh.from(inPath_usda, zUSD);
	myMeshParam = zTsMeshParam();
	myMeshParam.setFromFile(inPath_usda, zUSD);

	cout << "mesh loaded \n";

	//read properties
	//json j;
	//bool chk = core.json_read(inPath, j);

	//if (chk)
	//{
	//	const auto& j_startVIds = j["StartVIds"].get<vector<int>>();
	//	const auto& j_numGraphs = j["NumGraphs"].get<int>();

	//	//assign start vids
	//	for (auto& id : j_startVIds)
	//		start_boundary_ids.push_back(id);

	//	//assign total num graphs
	//	totalGraphs = j_numGraphs;
	//}
	
	for (zItMeshVertex v(*myMeshParam.getRawInMesh()); !v.end(); v++)
	{
		//if (v.onBoundary())
		if (v.getColor().r > 0.9)
		{
			start_boundary_ids.push_back(v.getId());
		}
		else if (v.getColor().b > 0.9)
		{
			end_boundary_ids.push_back(v.getId());
		}
	}

	//for (zItMeshEdge e(*myMeshParam.getRawInMesh()); !e.end(); e++)
	//{
	//	//if (v.onBoundary())
	//	if (e.getWeight()>0)
	//	{
	//		zIntArray vs;
	//		e.getVertices(vs);
	//		start_boundary_ids.push_back(vs[0]);
	//	}
	//}

	o_contours.clear();
	o_contours.assign(totalGraphs, zObjGraph());
}

void toFile()
{
	// Cleanup existing files in the output directory
	fs::remove_all(outPath);
	// Recreate the output directory
	fs::create_directories(outPath);

	int counter_folder = 0;

	//export mesh
	zFnMesh fnMesh(*myMeshParam.getRawInMesh());
	fnMesh.to(outPath + "/outMesh.json", zJSON);

	//export graphs
	//for (auto& contour : o_contours)
	//{
	//	//make file
	//	std::ostringstream ss;
	//	ss << std::setw(4) << std::setfill('0') << counter_file;
	//	string file = outPath + "/graph_" + ss.str() + ".json";

	//	zFnGraph fnGraph(contour);
	//	fnGraph.to(file, zJSON);
	//	counter_file++;
	//}

	int counter_layer = 0;
	for (auto& contours : separatedContours)
	{
		int counter_file = 0;
		for (auto& contour : contours)
		{

			//make file
			std::ostringstream s_layer;
			s_layer << std::setw(4) << std::setfill('0') << counter_layer;

			std::ostringstream s_file;
			s_file << std::setw(4) << std::setfill('0') << counter_file;
			string file = outPath + "/graph_" + s_layer.str() + "_" + s_file.str() + ".json";

			zFnGraph fnGraph(contour);
			fnGraph.to(file, zJSON);
			counter_file++;
		}
		counter_layer++;
	}

	cout << endl;
	cout << "All files have been exported to: " + outPath << endl;
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

	//fnSliceMesh.from("data/Slicer/sliceMesh3.obj", zOBJ);
	//myMeshParam.setFromFile("data/Slicer/sliceMesh3.obj", zOBJ);

	fromFile();
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans

	o_sliceMesh.setDisplayElements(false, true, false);

	zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
	o_inMesh->setDisplayElements(false, false, true);

	zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
	o_paramMesh->setDisplayElements(false, true, false);

	zFnMesh fnMesh(*o_inMesh);
	fnMesh.computeMeshNormals();
	fnMesh.computeVertexNormalfromFaceNormal();

	zPoint minBB, maxBB;
	o_inMesh->getBounds(minBB, maxBB);
	spatialField.create(minBB, maxBB, 50, 50, 50);

	zItMeshFace f(*o_inMesh);
	for (; !f.end(); f++)
	{
		spatialField.setValue(f.getCenter(), f.getId());
	}

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&totalGraphs, "totalGraphs");
	S.sliders[1].attachToVariable(&totalGraphs, 0, 800);

	S.addSlider(&sampleDist, "sampleDist");
	S.sliders[2].attachToVariable(&sampleDist, 0, 10);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_heat, "compute_heat");
	B.buttons[0].attachToVariable(&compute_heat);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_paramMesh, "d_paramMesh");
	B.buttons[2].attachToVariable(&d_paramMesh);

	B.addButton(&d_sliceMesh, "d_sliceMesh");
	B.buttons[3].attachToVariable(&d_sliceMesh);

	B.addButton(&d_ContourGraphs, "d_ContourGraphs");
	B.buttons[4].attachToVariable(&d_ContourGraphs);
	
	B.addButton(&reload, "reload");
	B.buttons[5].attachToVariable(&reload);

	B.addButton(&exportTo, "exportTo");
	B.buttons[6].attachToVariable(&exportTo);
	

}

void update(int value)
{
	if (compute_heat)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();

		zDomainFloat outMinMax(0, 1);

		cout << endl;
		cout << "boundary start:" << start_boundary_ids.size() << endl;
		cout << "boundary end:" << end_boundary_ids.size() << endl;

		if (start_boundary_ids.size() > 0)
		{
			myMeshParam.computeGeodesics_Exact(start_boundary_ids, geodesics_start);
			zDomainFloat startMinMax(core.zMin(geodesics_start), core.zMax(geodesics_start));
			for (auto& v : geodesics_start)
				v = core.ofMap(v, startMinMax, outMinMax);

			cout << "start min:" << core.zMin(geodesics_start) << endl;
			cout << "start max:" << core.zMax(geodesics_start) << endl;
		}

		if (end_boundary_ids.size() > 0)
		{
			myMeshParam.computeGeodesics_Exact(end_boundary_ids, geodesics_end);
			zDomainFloat endMinMax(core.zMin(geodesics_end), core.zMax(geodesics_end));
			for (auto& v : geodesics_end)
				v = core.ofMap(v, endMinMax, outMinMax);

			cout << "end min:" << core.zMin(geodesics_end) << endl;
			cout << "end max:" << core.zMax(geodesics_end) << endl;
		}

		compute_heat = !compute_heat;
	}

	if (compute_contour)
	{
		o_contours.clear();
		o_contours.assign(totalGraphs, zObjGraph());

		separatedContours.clear();
		separatedContours.assign(totalGraphs, zObjGraphArray());

		if (geodesics_start.size() > 0 && geodesics_end.size() > 0)
		{
			for (int i = 0; i < totalGraphs; i++)
				computeContours(&o_sliceMesh, geodesics_start,geodesics_end, i, totalGraphs, o_contours);
		}
		else
		{
			for (int i = 0; i < totalGraphs; i++)
				computeContours(&o_sliceMesh, geodesics_start, i, totalGraphs, o_contours);
		}


		for (int i = 0; i < o_contours.size(); i++)
		{
			separatedContours[i].reserve(10);
			separatedContours[i] = graphProcessor.separateGraph(o_contours[i]);
			separatedContours[i].shrink_to_fit();

			for (auto& contours : separatedContours)
			{
				for (auto& contour : contours)
				{
					graphProcessor.mergeVertices(contour, sampleDist);
					//graphProcessor.resampleGraph(contour, sampleDist);
				}
			}
		}

		compute_contour = !compute_contour;
	}

	if (compute_color)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		colorMesh(o_inMesh, geodesics_start);

		compute_color_start = !compute_color_start;
		compute_color = !compute_color;
	}

	if (reload)
	{
		fromFile();
		reload = !reload;
	}

	if (exportTo)
	{
		toFile();
		exportTo = !exportTo;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (d_inMesh)
	{
		zObjMesh* o_inMesh = myMeshParam.getRawInMesh();
		//o_inMesh->setDisplayElements(true, false, false);
		o_inMesh->draw();
		
	}

	if (d_sliceMesh)
	{
		o_sliceMesh.draw();
	}

	if (d_paramMesh)
	{
		zObjMesh* o_paramMesh = myMeshParam.getRawParamMesh();
		//o_paramMesh->setDisplayElements(true, false, false);
		o_paramMesh->draw();
	}
	
	if (d_ContourGraphs && separatedContours.size()>0)
	{
		if (d_AllGraphs)
		{
			//for (auto& g : o_contours)
			//{
			//	g.draw();
			//}

			for (auto& contours : separatedContours)
				for (auto& g : contours)
					g.draw();
		}
		else
		{
			int i = currentGraphId;			
			if (totalGraphs > 0 && i >= 0 && i < totalGraphs)
			{
				//o_contours[i].draw();

				for (auto& g : separatedContours[i])
					g.draw();
			}

		}

		//for (auto& contours : separatedContours)
		//	for (auto& g : contours)
		//	{
		//		zItGraphVertex v(g);
		//		for (; !v.end(); v++)
		//		{
		//			//if (v.checkValency(1))
		//				model.displayUtils.drawPoint(v.getPosition(), zWHITE, 5);
		//		}
		//	}

		for (auto& contours : separatedContours)
			for (auto& g : contours)
			{
				g.draw();

				zFnGraph fn(g);
				zPoint* pos = fn.getRawVertexPositions();
				for (size_t i = 0; i < fn.numVertices(); i++)
				{
					int id = spatialField.getValue(pos[i]);

					if (id != -1)
					{
						zItMeshFace f(*myMeshParam.getRawInMesh(), id);
						zVector norm = f.getNormal();
						norm.normalize();
						norm *= 0.02;
						model.displayUtils.drawLine(pos[i], pos[i]+ norm, zBLUE, 1);
					}
				}
			}

			//for (auto& g : o_contours)
			//{
			//	zItGraphVertex v(g);
			//	for (; !v.end(); v++)
			//	{
			//		if (v.checkValency(1))
			//			model.displayUtils.drawPoint(v.getPosition(), zCYAN, 5);
			//	}
			//}
	}


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Layers #:" + to_string(totalGraphs), vec(winW - 350, winH - 800, 0));
	drawString("Current Layer #:" + to_string(currentGraphId), vec(winW - 350, winH - 775, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("p - compute geodesics", vec(winW - 350, winH - 625, 0));
	drawString("o - compute contours", vec(winW - 350, winH - 600, 0));
	drawString("c - compute colors of flip fields", vec(winW - 350, winH - 575, 0));
	

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute_heat = true;;
	if (k == 'o') compute_contour = true;;
	if (k == 'c') compute_color = true;

	if (k == 'f')
	{
		zPoint minBB, maxBB;
		o_sliceMesh.getBounds(minBB, maxBB);
		zPoint focus = (minBB + maxBB) * 0.5;
		updateCamera(Alice::vec(focus.x, focus.y, focus.z));
	}

	if (k == 'w')
	{
		if (currentGraphId < totalGraphs - 1)currentGraphId++;;
	}
	if (k == 's')
	{
		if (currentGraphId > 0)currentGraphId--;;
	}

	if (k == 'd') d_AllGraphs = !d_AllGraphs;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
