//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/cityEngine/polygon.h>
#include <userSrc/tzSketches/cityEngine/zTsCityEngine.h>

using namespace zSpace;
using namespace std;

#define LOG_DEV 0 && std::cout
#define LOG_BUILD 1 && std::cout


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = false;
bool displayField = false;
bool displayVector = false;
bool exportTo = false;
bool reload = false;

double background = 1.0;
double scale = 1.0;
double densityMin = 0.01;
double densityMax = 1.0;

double targetArea = 5000;
int numIterations;
int numPolys;
double computeTime = 0;
bool readMap = true;

float mapScalar = 1.0f;

int e0 = -1;
int e1 = -1;
double f0 = -1.0;
double f1 = -1.0;

double itr_alignVec = 0;
double maxNumItr = 5;

double offsetX = 0.5;
double offsetY = 0.5;
bool longside = true;

double alignVecX = 0.0;
double alignVecY = 1.0;
//zVector alignVec;

double eId1 = 0;
double eId2 = -1;


////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;
zUtilsCore core;

zTsCityEngine engine;
zBlock block;
zLine cutLine;

zPointArray polygon_pos;
zIntArray polygon_counts;
vector<zAnchorData> anchorData;
vector<zPointArray> anchorPlots;
zPointArray bbox;

vector<string> files;
int numFiles;
int fileCounter = 0;

vector<double> eVals;
vector<int> eIds;
vector<int>edgeLevels;

vector<double> targetAreas;
vector<zVector> alignVectors;

zObjMeshScalarField renderField;

/*!<Objects*/

zUtilsCore coreUtil;

void drawPolygon(zPointArray pts, zColor col, double wt)
{
	if (pts.size() > 1)
	{
		for (int i = 0; i < pts.size() - 1; i++)
		{
			model.displayUtils.drawLine(pts[i], pts[i + 1], col, wt);
		}
		model.displayUtils.drawLine(pts[pts.size() - 1], pts[0], col, wt);
	}
}

void readBlockData(string path, zPointArray& blockPos, zIntArray& blockCount, vector<zAnchorData>& anchorData)
{
	blockPos.clear();
	blockCount.clear();
	anchorData.clear();

	json j;
	coreUtil.json_read(path, j);

	vector<vector<double>> pos = j["BlockPositions"].get<vector<vector<double>>>();

	blockPos.assign(pos.size(), zPoint());
	for (int i = 0; i < pos.size(); i++)
	{
		zPoint p(pos[i][0], pos[i][1], pos[i][2]);
		blockPos[i] = p;
	}

	blockCount = j["BlockCount"].get<vector<int>>();
	
	for (const auto& ad : j["AnchorData"]) {
		zAnchorData data;
		data.blockId = ad["BlockId"];
		data.dimX = ad["DimX"];
		data.dimY = ad["DimY"];
		data.eId1 = ad["EId1"];
		data.eId2 = ad["EId2"];
		anchorData.push_back(data);
	}

	edgeLevels = j["EdgeLevels"].get<vector<int>>();

	//block.targetArea = j["TargetArea"].get<double>();
	//vector<double> vec = j["NorthVector"].get<vector<double>>();
	//block.blockVector = zVector(vec[0], vec[1], vec[2]);

	targetAreas = j["TargetArea"].get<vector<double>>();

	vector<vector<double>> vecs = j["NorthVector"].get<vector<vector<double>>>();
	for (auto& vec : vecs)
	{
		zVector vector(vec[0], vec[1], vec[2]);
		alignVectors.push_back(vector);
	}
}

string dir_block = "data/CityEngine/blockData";
string dir_field = "data/CityEngine/fieldData";
//string dir_block = "data/CityEngine";
string dir_out = "data/CityEngine/outFolder";
string filename;

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

	model.addObject(block.blockMesh);
	block.blockMesh.setDisplayElements(false, true, false);

	model.addObject(block.blockField);
	block.blockField.setDisplayElements(false, true, true);
	zFnMeshScalarField fnField(block.blockField);
	zItMeshVertex v(*fnField.getRawMesh());
	for (; !v.end(); v++)
		*v.getRawPosition() += zVector(0, 0, -5);

	//readBlockData(dir_block, polygon_pos, polygon_counts, anchorData);

	
	numFiles = core.getNumfiles_Type(dir_block, zJSON);

	files.assign(numFiles, string());

	core.getFilesFromDirectory(files, dir_block, zJSON);
	
	//// Cleanup existing files in the output directory
	//fs::remove_all(dir_out);
	//// Recreate the output directory
	//fs::create_directories(dir_out);



	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object


	// set display element booleans



	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&scale, "scale");
	S.sliders[1].attachToVariable(&scale, 0, 1);
	//S.addSlider(&offsetX, "offsetX");
	//S.sliders[2].attachToVariable(&offsetX, 0, 5);
	//S.addSlider(&offsetY, "offsetY");
	//S.sliders[3].attachToVariable(&offsetY, 0, 5);
	//S.addSlider(&alignVecX, "alignVecX");
	//S.sliders[4].attachToVariable(&alignVecX, -1, 1);
	//S.addSlider(&alignVecY, "alignVecY");
	//S.sliders[5].attachToVariable(&alignVecY, -1, 1);
	//S.addSlider(&eId1, "eId1");
	//S.sliders[6].attachToVariable(&eId1, -2, 4);
	//S.addSlider(&eId2, "eId2");
	//S.sliders[7].attachToVariable(&eId2, -2, 4);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&displayField, "displayField");
	B.buttons[2].attachToVariable(&displayField);
	B.addButton(&displayVector, "displayVector");
	B.buttons[3].attachToVariable(&displayVector);
	//B.addButton(&reload, "reload");
	//B.buttons[3].attachToVariable(&reload);
	B.addButton(&exportTo, "exportTo");
	B.buttons[4].attachToVariable(&exportTo);
	//B.addButton(&longside, "longside");
	//B.buttons[5].attachToVariable(&longside);

}

void update(int value)
{
	if (compute)
	{
		//for (auto& file : files)
		{
			/*-------create a new block so you can 'refresh'------*/
			block = zBlock();
			string file;

			/*-------only wrong the latest blockData in the input folder------*/
			bool lastOne = true;
			//cout <<"\n" << "run last one?" << "\n";
			//cin >> lastOne;
			if (lastOne)
			{
				file = files[files.size() - 1];
				size_t pos = file.find_last_of("/\\");
				filename = (pos != std::string::npos) ? file.substr(pos + 1) : file;
			}
			else
			{
				LOG_DEV << "\n" << "filename?" << "\n";
				cin >> filename;
				file = dir_block + "/" + filename;
			}

			auto begin = std::chrono::high_resolution_clock::now();

			/*-------read polygon data from json------*/
			readBlockData(file, polygon_pos, polygon_counts, anchorData);

			//alignVec = block.blockVector;
			//alignVec = zVector(alignVecX, alignVecY, 0);

			block.create(polygon_pos, polygon_counts);
			block.blockMesh.setDisplayElements(false, true, false);

			block.setEdgeLevels(edgeLevels);
			block.setTargetAreas(targetAreas);
			block.setAlignVectors(alignVectors);
			LOG_DEV << "targetAreas.size():" << targetAreas.size() << endl;
			LOG_DEV << "alignVectors.size():" << alignVectors.size() << endl;

			string fieldFile = dir_field + "/" + filename;
			block.createField(fieldFile);

			LOG_DEV << file << endl;
			LOG_DEV << fieldFile << endl;
			LOG_DEV << "fileCounter: " << fileCounter << endl;

			/*-------keep this 0.1 to avoid overlapping line intersections------*/
			float offset = 0.1;

			/*-------this chunk computes the splits around anchor plots------*/
			block.anchorPlotIds.clear();
			anchorPlots.assign(polygon_counts.size(), zPointArray());

			if (anchorData.size() != 0)
				for (auto& data : anchorData)
				{
					if (data.blockId != -1)
					{
						zItMeshFace f(block.blockMesh, data.blockId);
						zIntArray heIds;
						zIntArray eIds;
						f.getHalfEdges(heIds);

						for (auto& id : heIds)
						{
							zItMeshHalfEdge he(block.blockMesh, id);
							eIds.push_back(he.getEdge().getId());
						}

						LOG_DEV << "numEdges: " << eIds.size() << endl;
						//cout << "f: " << data.blockId << " e0:" << data.eId1 << " e1:" << data.eId2 << endl;
						block.split_anchor_edge_aligned(data.blockId, data.dimX, data.dimY, offset, data.eId1, data.eId2);
						anchorPlots[data.blockId] = VectorsToPointArray(block.anchorPlot.getVectors());
					}
				}

			/*-------split the rest of the blocks after anchor is done------*/
			block.split_block_rest(longside, true, true);

			for (auto& id : block.anchorPlotIds)
				LOG_DEV << "anchorID: " << id << endl;


			//block.dumpJSON(dir_out + "/" + filename);
			//cout << "exported " << fileCounter << "/" << numFiles - 1 << endl;
			fileCounter++;

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
			////printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);
			computeTime = elapsed.count() * 1e-9;

			//for (auto& it : block.map_eId_value)
			//{
			//	cout << "eId: " << it.first << ", val: " << it.second << "\n";
			//}

			numPolys = block.getNumBlocks();
			numIterations = block.getNumIterations();
		}
		compute = !compute;
	}

	if (exportTo)
	{
		block.dumpJSON(dir_out + "/" + filename);

		//zFnMesh fn(block.blockMesh);
		//fn.to(dir_out + "/temp.obj", zOBJ);

		exportTo = !exportTo;
	}

	if (reload)
	{
		readBlockData(dir_block, polygon_pos, polygon_counts, anchorData);

		for (auto& data : anchorData)
		{
			cout << "blockId:" << data.blockId << endl;
			cout << "dimX:" << data.dimX << endl;
			cout << "dimY:" << data.dimY << endl;
			cout << "eId1:" << data.eId1 << endl;
			cout << "eId2:" << data.eId2 << endl;
			cout << "---------" << endl;
		}

		compute = !compute;
		reload = !reload;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();


	glPushMatrix();
	glScalef(scale, scale, scale);

	// zspace model draw
	model.draw();


	//draw polygon
	//drawPolygon(polygon,zBLACK,2);

	for (auto& plot : anchorPlots)
	{
		if (plot.size() > 0)
		drawPolygon(plot, zMAGENTA, 2);
	}

	//drawPolygon(bbox,zMAGENTA,2);

	//draw cut line
	zPoint p_start(cutLine.getStart().x, cutLine.getStart().y, cutLine.getStart().z);
	zPoint p_end(cutLine.getEnd().x, cutLine.getEnd().y, cutLine.getEnd().z);

	model.displayUtils.drawLine(p_start, p_end, zRED, 5);
	model.displayUtils.drawLine(zVector(0, 0, 0), zVector(alignVecX * 100, alignVecY * 100, 0), zRED, 2);

	//zItMeshEdge e(block.blockMesh);
	//for (; !e.end(); e++)
	//{
	//	model.displayUtils.drawTextAtPoint(to_string(block.map_eId_age[e.getId()]), e.getCenter());
	//}

	if (display)
	{
		zItMeshFace f(block.blockMesh);
		for (; !f.end(); f++)
		{
			double fArea = f.getPlanarFaceArea();
			std::ostringstream stream;
			stream << std::fixed << std::setprecision(2) << fArea;
			std::string formattedArea = stream.str();
			model.displayUtils.drawTextAtPoint(formattedArea,f.getCenter());
		}
	}
	if (displayVector)
	{
		zItMeshFace f(block.blockMesh);
		for (; !f.end(); f++)
		{
			zVector vec = block.map_fId_alignVector[f.getId()];
			vec.normalize();
			vec *= 20;

			if(vec.z>0)
				model.displayUtils.drawLine(f.getCenter(), f.getCenter() + vec, zMAGENTA, 2);
			else
				model.displayUtils.drawLine(f.getCenter(), f.getCenter() + vec, zBLUE, 2);
		}
	}

	if (displayField)
	{
		block.blockField.setDisplayObject(true);
	}
	else
	{
		block.blockField.setDisplayObject(false);
	}
	glPopMatrix();

	


	//////////////////////////////////////////////////////////
	glColor3f(0, 0, 0);
	setup2d();

	drawString("numInterations:" + to_string(numIterations), vec(winW - 350, winH - 475, 0));
	drawString("numPolys:" + to_string(numPolys), vec(winW - 350, winH - 450, 0));
	drawString("compute time:" + to_string(computeTime) + "s", vec(winW - 350, winH - 425, 0));

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
