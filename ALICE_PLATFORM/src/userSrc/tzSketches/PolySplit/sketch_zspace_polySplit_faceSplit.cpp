//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <userSrc/tzSketches/PolySplit/polygon.h>



using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;
bool exportTo = false;
bool reload = false;

double background = 1.0;
double scale = 1.0;
double densityMin = 0.01;
double densityMax = 1.0;
double offsetVal = 0.0;

double targetArea = 1000;
int numInterations;
int numPolys;
double computeTime = 0;
bool readMap = false;

float mapScalar = 1.0f;

double itr_alignVec = 0;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

vector<zPointArray> inPolys;
vector<zLine> cutLines;
unordered_map <int, vector<zLine>> map_age_cutLines;

map<int,vector<double>> map_eId_factors;
map<pair<int,int>, pair<int,int>> map_eId_eId;

zObjMeshScalarField oDensityField;
zObjMesh inMesh;
zObjMesh inMesh_copy;
zFnMesh fn_inMesh;
zObjMesh densityMap;

zVector alignVec(1, 0, 0);

string path_inMesh = "data/polySplit/inPolys.json";
string path_field_density = "data/polySplit/densityMap.json";
//string path_field_density = "data/polySplit/densityMap.usd";

vector<zFloatArray> recordArr;

////// --- GUI OBJECTS ----------------------------------------------------
Vectors zPointArrayToVectors(zPointArray& vertices)
{
	Vectors vectors;
	for (auto& v : vertices)
		vectors.push_back(Vector(v.x, v.y, v.z));

	return vectors;
}

/*NOT COMPLETE*/
void split_targetArea(zPointArray& pts, double targetArea, unordered_map <int, vector<zLine>>& outLines, vector<zPolygon>& outPolys, int& outNumIterations)
{
	//zLine cutLine;
	//zPolygon myPolygon(zPointArrayToVectors(pts));

	//zPolygon outPoly1, outPoly2;
	//myPolygon.split(targetArea, outPoly1, outPoly2, cutLine);

	//cutLines.push_back(cutLine);
	//cout << cutLine.getStart().x << "," << cutLine.getStart().y << "," << cutLine.getStart().z << endl;
	//cout << cutLine.getEnd().x << "," << cutLine.getEnd().y << "," << cutLine.getEnd().z << endl;

	zPolygon inPoly(zPointArrayToVectors(pts));

	int counter = 0;

	zPolygon poly1, poly2, temp;
	zLine cutLine;
	bool cut = true;
	outPolys.clear();

	double tol = -0.01;
	do
	{
			double area = inPoly.countSquare();
			if (area > targetArea)
			{
				inPoly.split(targetArea, poly1, poly2, cutLine);

				outLines[counter].push_back(cutLine);

				if (poly1.countSquare()+tol <= targetArea)
				{
					outPolys.push_back(poly1);
					inPoly = poly2;
				}
				else if (poly2.countSquare()+tol <= targetArea)
				{
					outPolys.push_back(poly2);
					inPoly = poly1;
				}
				if (poly1.countSquare() <= targetArea && poly2.countSquare() <= targetArea)
				{
					cut = false;
				}
			}
			if (cut)
				counter++;
	} while (cut);
	outNumIterations = counter;
}

void split_averageArea(zPointArray& pts, double targetArea, unordered_map <int, vector<zLine>>& outLines, vector<zPolygon>& outPolys, int& outNumIterations)
{
	outPolys.clear();
	zPolygon inPoly(zPointArrayToVectors(pts));

	vector<zPolygon> polys;
	vector<zPolygon> temp;
	polys.push_back(inPoly);

	int counter = 0;
	bool exit;
	do
	{
		temp.clear();
		exit = true;

		for (auto& poly : polys)
		{
			double area = poly.countSquare();
			Vector center = poly.countCenter();
			zVector zCenter(center.x, center.y, center.z);

			zScalar val = 1;
			if (readMap)
			{
				zFnMeshScalarField fnField(oDensityField);
				fnField.getFieldValue(zCenter, zFieldNeighbourWeighted, val);
			}

			val *= targetArea;
			//cout << "val:" << val << endl;

			if (area * 0.5 > val)
			{
				exit = false;

				zPolygon poly1, poly2;
				zLine cutLine;

				//Vector alignVec = counter < itr_alignVec ? Vector(1, 0, 0) : Vector(0, 0, 0);
				Vector vec = counter < itr_alignVec ? Vector(alignVec.x, alignVec.y, alignVec.z) : Vector(0, 0, 0);

				poly.split(area * 0.5, poly1, poly2, cutLine, vec);
				temp.push_back(poly1);
				temp.push_back(poly2);
				outLines[counter].push_back(cutLine);
			}
			else
			{
				temp.push_back(poly);
			}
		}

		polys = temp;

		if (counter == 0)
			outPolys = polys;

		if (!exit)
		{
			outPolys = polys;
			counter++;
		}

	} while (!exit);
	outNumIterations = counter;
}

int split_averageArea_face(zPointArray& pts, double targetArea, int& e0, int& e1, double& f0, double& f1, unordered_map <int, vector<zLine>>& outLines)
{
	zPolygon poly(zPointArrayToVectors(pts));

	double area = poly.countSquare();
	Vector center = poly.countCenter();
	zVector zCenter(center.x, center.y, center.z);

	zScalar val = 1;
	if (readMap)
	{
		zFnMeshScalarField fnField(oDensityField);
		fnField.getFieldValue(zCenter, zFieldNeighbourWeighted, val);
	}

	val *= targetArea;
	//cout << "val:" << val << endl;

	if (area * 0.5 > val)
	{
		zPolygon poly1, poly2;
		zLine cutLine;

		Vector vec = counter < itr_alignVec ? Vector(alignVec.x, alignVec.y, alignVec.z) : Vector(0, 0, 0);

		poly.split(area * 0.5, poly1, poly2, cutLine, vec);
		outLines[counter].push_back(cutLine);

		poly.getData(e0, e1, f0, f1);

		return 1;
	}
	else
		return 0;
}


void fromFile(string& file, vector<zPointArray>& inPolys)
{
	inPolys.clear();

	json j;
	bool chk = core.json_read(file, j);

	if (chk)
	{
		zFnMesh fnMesh(inMesh);

		fnMesh.from(file, zJSON);
		zItMeshFace f(inMesh);

		inPolys.assign(fnMesh.numPolygons(), zPointArray());
		for (; !f.end(); f++)
		{
			zIntArray vIds;
			f.getVertices(vIds);

			zPointArray pts;
			pts.assign(vIds.size(), zPoint());
			for (int i = 0; i < vIds.size(); i++)
			{
				zItMeshVertex v(inMesh, vIds[i]);
				pts[i] = v.getPosition();
			}
			inPolys[f.getId()] = pts;
		}
	}
}

void exportBlocksToFile()
{
	//string outPath = "data/polySplit/out/blocks";

	//// Cleanup existing files in the output directory
	//fs::remove_all(outPath);
	//// Recreate the output directory
	//fs::create_directories(outPath);

	//int counter_numFile = 0;
	//int counter1 = 0;
	//for (const auto& polys : outPolys)
	//{
	//	string folder = outPath + "/block" + "_" + to_string(counter1);
	//	if (!fs::exists(folder))
	//		fs::create_directory(folder);

	//	int counter2 = 0;

	//	for (const auto& poly : polys)
	//	{
	//		// Generate the output path
	//		string filePath = folder + "/tile" + "_" + to_string(counter2) + ".json";

	//		// Export the vectors to the JSON file
	//		std::ofstream outFile(filePath);
	//		if (!outFile.is_open())
	//		{
	//			std::cerr << "Failed to open file for writing: " << filePath << std::endl;
	//			continue; // Skip to the next iteration
	//		}

	//		// Export the fnGraph to the JSON file
	//		json j;
	//		Vectors pos = poly.getVectors();

	//		for (const auto& p : pos)
	//		{
	//			json point = {
	//				p.x,p.y,p.z
	//			};
	//			j.push_back(point);
	//		}

	//		// Write JSON to file
	//		outFile << std::setw(4) << j << std::endl;
	//		outFile.close();

	//		counter_numFile++;
	//		counter2++;

	//		cout << counter_numFile << "/" << numPolys << endl;
	//	}
	//	counter1++;
	//}

	//cout << endl;
	//cout << "All files have been exported to: " + outPath << endl;
}

void exportCutsToFile()
{
	string outPath = "data/polySplit/out/cuts";

	// Cleanup existing files in the output directory
	fs::remove_all(outPath);
	// Recreate the output directory
	fs::create_directories(outPath);

	int counter_numFile = 0;
	int counter1 = 0;

	for (const auto& it : map_age_cutLines)
	{
		string folder = outPath + "/lvl" + "_" + to_string(counter1);
		if (!fs::exists(folder))
			fs::create_directory(folder);

		int counter2 = 0;

		for (const auto& cutLine : it.second)
		{
			// Generate the output path
			string filePath = folder + "/cut" + "_" + to_string(counter2) + ".json";

			// Export the vectors to the JSON file
			std::ofstream outFile(filePath);
			if (!outFile.is_open())
			{
				std::cerr << "Failed to open file for writing: " << filePath << std::endl;
				continue; // Skip to the next iteration
			}

			// Export the fnGraph to the JSON file
			json j;
			Vectors pos;
			pos.push_back(cutLine.getStart());
			pos.push_back(cutLine.getEnd());

			for (const auto& p : pos)
			{
				json point = {
					p.x,p.y,p.z
				};
				j["pos"].push_back(point);
			}
			j["age"].push_back(it.first);

			// Write JSON to file
			outFile << std::setw(4) << j << std::endl;
			outFile.close();

			counter_numFile++;
			counter2++;

			cout << counter_numFile << "/" << numPolys << endl;
		}
		counter1++;
	}

	cout << endl;
	cout << "All files have been exported to: " + outPath << endl;
}

void exportBlocksToFile_mesh()
{
	//string outPath = "data/polySplit/outMesh.json";

	//zPointArray positions;
	//zIntArray pConnects;
	//zIntArray pCounts;

	//int offset = 0;

	//for (const auto& polys : outPolys)
	//{
	//	for (const auto& poly : polys)
	//	{
	//		for (const auto& p : poly.getVectors())
	//		{
	//			positions.push_back(zPoint(p.x, p.y, p.z));
	//		}

	//		int size = poly.getVectors().size();
	//		for (int i = offset; i < offset + size; i++)
	//		{
	//			pConnects.push_back(i);
	//		}
	//		pCounts.push_back(size);

	//		offset += size;
	//	}
	//}

	////for (auto& it : positions)
	////	cout << it << endl;

	////cout << "--pconnects--" << endl;

	////for (auto& it : pConnects)
	////	cout << it << endl;

	////cout << "--pcounts--" << endl;

	////for (auto& it : pCounts)
	////	cout << it << endl;

	//zObjMesh outMesh;
	//zFnMesh fn(outMesh);
	//fn.create(positions, pCounts, pConnects);
	//fn.to(outPath, zJSON);

	//cout << endl;
	//cout << "All files have been exported to: " + outPath << endl;
}

void exportRecord(string& outPath)
{
	cout << "record size: " << recordArr.size() << endl;

	string filePath = outPath + "/record.json";

	std::ofstream outFile(filePath);
	if (!outFile.is_open())
	{
		std::cerr << "Failed to open file for writing: " << outPath << std::endl;
	}

	json j;

	for (const auto& vals : recordArr)
	{
		json j_vals = {
			vals[0],
			vals[1],
			vals[2],
			vals[3],
			vals[4],
			vals[5],
		};
		j["values"].push_back(j_vals);
	}
	// Write JSON to file
	outFile << std::setw(4) << j << std::endl;
	outFile.close();

	cout << endl;
	cout << "All files have been exported to: " + outPath << endl;
}

void setFieldScalar(double& _min, double& _max)
{
	zFnMeshScalarField fnField(oDensityField);
	zFloatArray fVals;
	fnField.getFieldValues(fVals);

	float min = core.zMin(fVals);
	float max = core.zMax(fVals);
	zDomainFloat inDomain(min, max);
	zDomainFloat outDomain(_min, _max);
	for (auto& val : fVals)
	{
		val = core.ofMap(val, inDomain, outDomain);
	}

	fnField.setFieldValues(fVals);

	cout << endl;
	cout << "min:" << min << endl;
	cout << "max:" << max << endl;
}

void fieldFromMap_density(string& path, zObjMeshScalarField& oField, int resX, int resY)
{
	readMap = false;

	if (fs::exists(path))
	{
		readMap = true;

		zFnMesh fn(densityMap);
		fn.from(path, zJSON);
		zPoint minBB, maxBB;
		fn.getBounds(minBB, maxBB);

		zFnMeshScalarField fnField(oField);
		fnField.create(minBB, maxBB, resX, resY, 1, true, false);

		zFloatArray fVals;
		fVals.assign(fnField.numFieldValues(), 0);

		zItMeshVertex v(densityMap);
		for (; !v.end(); v++)
		{

			zIntArray neighbours;
			
				//fnField.getNeighbour_Contained(zPoint(-170.463, -80.5909, 2.23821e-13), neighbours);
			zPoint vPos(v.getPosition().x, v.getPosition().y, 0);
			fnField.getNeighbour_Contained(vPos, neighbours);

			//cout << "numNeighbours:" << neighbours.size() << endl;
			//cout << "pos:" << v.getPosition() << endl;
			//cout << "v value:" << v.getColor().r << endl;

			for (auto& id : neighbours)
			{
				zItMeshScalarField it(oField, id);
				fVals[id] += v.getColor().r;

	/*			cout << "i:" << id << endl;
				cout << "v value:" << v.getColor().r << endl;
				cout << "f value:" << fVals[id] << endl;*/
			}
		}

		fnField.setFieldValues(fVals);

		setFieldScalar(densityMin, densityMax);
		fnField.updateColors();

	}
}

zPolygon offsetPoly(const zPolygon& poly, float offsetVal)
{
	vector<Vector> pts;
	Vector centroid = poly.countCenter();
	for (auto& p : poly.getVectors())
	{
		Vector vec = centroid - p;
		vec *= offsetVal;
		pts.push_back(p + vec);
	}
	zPolygon outPoly(pts);
	return outPoly;
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

	fn_inMesh = zFnMesh(inMesh);
	fn_inMesh.from(path_inMesh, zJSON);
	inMesh_copy = zObjMesh(inMesh);

	fieldFromMap_density(path_field_density, oDensityField, 100, 100);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oDensityField);
	model.addObject(densityMap);
	model.addObject(inMesh);

	oDensityField.setDisplayElements(false, false, false);
	densityMap.setDisplayElements(false, false, false);
	inMesh.setDisplayElements(false, true, false);
	
	zPointArray edgeCenters;
	zFnMesh fn(inMesh);
	fn.getCenters(zHEData::zEdgeData, edgeCenters);
	inMesh.setEdgeCenters(edgeCenters);
	inMesh.setDisplayElementIds(true, true, false);


	//model.addObject(oMesh);
	// set display element booleans
	//oMesh.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&scale, "scale");
	S.sliders[1].attachToVariable(&scale, 0, 1);
	S.addSlider(&targetArea, "targetArea");
	S.sliders[2].attachToVariable(&targetArea, 0, 1000);
	S.addSlider(&densityMin, "densityMin");
	S.sliders[3].attachToVariable(&densityMin, 0, 1);
	S.addSlider(&densityMax, "densityMax");
	S.sliders[4].attachToVariable(&densityMax, 0, 1);
	S.addSlider(&offsetVal, "offsetVal");
	S.sliders[5].attachToVariable(&offsetVal, 0, 1);
	//S.addSlider(&itr_alignVec, "itr_alignVec");
	//S.sliders[6].attachToVariable(&itr_alignVec, 0, 10);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);
	B.addButton(&reload, "reload");
	B.buttons[2].attachToVariable(&reload);
	B.addButton(&exportTo, "exportTo");
	B.buttons[3].attachToVariable(&exportTo);

}

void update(int value)
{
	if (compute)
	{
		recordArr.clear();

		auto begin = std::chrono::high_resolution_clock::now();

		inMesh = zObjMesh(inMesh_copy);
		inMesh.setDisplayElements(false, true, false);
		//zPointArray edgeCenters;
		//zFnMesh fn(inMesh);
		//fn.getCenters(zHEData::zEdgeData, edgeCenters);
		//inMesh.setEdgeCenters(edgeCenters);
		//inMesh.setDisplayElementIds(true, true, false);

		setFieldScalar(densityMin, densityMax);
		map_age_cutLines.clear();
		numInterations = 0;
		numPolys = 0;

		bool exit;
		do
		{
			exit = true;
			bool addNumIter = true;
			int numFaces = fn_inMesh.numPolygons();

			for (int i = 0; i < numFaces; i++)
			{
				int e0, e1;
				double f0, f1;
				zItMeshFace f(inMesh, i);
				zPointArray fVs;
				zIntArray fHes;
				f.getVertexPositions(fVs);
				f.getHalfEdges(fHes);

				vector<zPolygon> polyInBlock;
				int split = split_averageArea_face(fVs, targetArea, e0, e1, f0, f1, map_age_cutLines);

				if (split)
				{
					zItMeshHalfEdge he0(inMesh, fHes[e0]);
					zItMeshHalfEdge he1(inMesh, fHes[e1]);

					e0 = he0.getEdge().getId();
					e1 = he1.getEdge().getId();

					cout << endl;
					cout << "fId:" << i << endl;
					cout << "e0:" << e0 << ",f0:" << f0 << endl;
					cout << "e1:" << e1 << ",f1:" << f1 << endl;

					fn_inMesh.splitFace(i, e0, e1, f0, f1);

					zFloatArray temp;
					temp.push_back(numInterations);
					temp.push_back(i);
					temp.push_back(e0);
					temp.push_back(e1);
					temp.push_back(f0);
					temp.push_back(f1);

					recordArr.push_back(temp);

					zPointArray edgeCenters;
					fn_inMesh.getCenters(zHEData::zEdgeData, edgeCenters);
					inMesh.setEdgeCenters(edgeCenters);

					if (addNumIter)
					{
						addNumIter = false;
					}
					exit = false;
				}
			}
			if(addNumIter)
				numInterations++;

		} while (!exit);
		numPolys = fn_inMesh.numPolygons();

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		//printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);
		computeTime = elapsed.count() * 1e-9;

		compute = !compute;	
	}

	if (exportTo)
	{
		string outPath = "data/polySplit/out";
		fs::remove_all(outPath);
		fs::create_directories(outPath);

		//exportBlocksToFile();
		//exportCutsToFile();
		//exportBlocksToFile_mesh();
		fn_inMesh.to(outPath + "/outMesh.json", zJSON);
		fn_inMesh.to(outPath + "/outMesh.obj", zOBJ);

		exportRecord(outPath);

		exportTo = !exportTo;
	}

	if (reload)
	{
		inPolys.clear();
		cutLines.clear();
		map_age_cutLines.clear();

		fn_inMesh = zFnMesh(inMesh);
		fn_inMesh.from(path_inMesh, zJSON);
		inMesh_copy = zObjMesh(inMesh);

		zPointArray edgeCenters;
		fn_inMesh.getCenters(zHEData::zEdgeData, edgeCenters);
		inMesh.setEdgeCenters(edgeCenters);

		fieldFromMap_density(path_field_density, oDensityField, 100, 100);

		//zPointArray edgeCenters;
		//zFnMesh fn(inMesh);
		//fn.getCenters(zHEData::zEdgeData, edgeCenters);
		//inMesh.setEdgeCenters(edgeCenters);
		//inMesh.setDisplayElementIds(true, true, false);

		compute = !compute;

		reload = !reload;
	}
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();


	glPushMatrix();
	glScalef(scale, scale, scale);
	if (display)
	{
		// zspace model draw
		model.draw();

		//model.displayUtils.drawLine(zPoint(0, 0, 0), zPoint(0, 0, 0) + alignVec*100, zMAGENTA, 4);

		//for (auto& pts : inPolys)
		//{
		//	for (int i = 0; i < pts.size() - 1; i++)
		//	{
		//		model.displayUtils.drawLine(pts[i], pts[i + 1], zBLACK, 3);
		//	}
		//	model.displayUtils.drawLine(pts[pts.size() - 1], pts[0], zBLACK, 3);
		//}

		//for (auto& polys : outPolys)
		//{
		//	for (auto& poly : polys)
		//	{
		//		zPointArray pts;
		//		for (auto& vec : poly.getVectors())
		//			pts.push_back(zPoint(vec.x, vec.y, vec.z));

		//			for (int i = 0; i < pts.size() - 1; i++)
		//			{
		//				model.displayUtils.drawLine(pts[i], pts[i + 1], zBLACK, 1);
		//			}
		//			model.displayUtils.drawLine(pts[pts.size() - 1], pts[0], zBLACK, 1);
		//	}

		//}

		//if (e0 != -1 && e1 != -1)
		//{
		//	zItMeshEdge edge0(inMesh, e0);
		//	zItMeshHalfEdge he0 = edge0.getHalfEdge(0);
		//	if (he0.onBoundary()) he0 = he0.getSym();
		//	zPoint p0 = he0.getStartVertex().getPosition();
		//	p0 = p0 + he0.getVector() * f0;

		//	model.displayUtils.drawPoint(p0, zRED, 10);

		//	zItMeshEdge edge1(inMesh, e1);
		//	zItMeshHalfEdge he1 = edge1.getHalfEdge(0);
		//	if (he1.onBoundary()) he1 = he1.getSym();
		//	zPoint p1 = he1.getStartVertex().getPosition();
		//	p1 = p1 + he1.getVector() * f1;

		//	model.displayUtils.drawPoint(p1, zRED, 10);
		//}


		//zDomainColor domainCol(zRED, zBLUE);
		zDomainColor domainCol(zRED, zBLUE);
		zDomainFloat domain(1.0f, numInterations);

		for (auto& it : map_age_cutLines)
		{
			zColor col = core.blendColor(it.first, domain, domainCol, zRGB);

			for (auto& cutLine : it.second)
			{
				zPoint a(cutLine.getStart().x, cutLine.getStart().y, cutLine.getStart().z);
				zPoint b(cutLine.getEnd().x, cutLine.getEnd().y, cutLine.getEnd().z);
				model.displayUtils.drawLine(a, b, col, 2);
			}
		}
	}
	glPopMatrix();

	


	//////////////////////////////////////////////////////////
	glColor3f(0, 0, 0);
	setup2d();

	drawString("numInterations:" + to_string(numInterations), vec(winW - 350, winH - 475, 0));
	drawString("numPolys:" + to_string(numPolys), vec(winW - 350, winH - 450, 0));
	drawString("compute time:" + to_string(computeTime) + "s", vec(winW - 350, winH - 425, 0));

	restore3d();
}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;	

	if (k == '1')
	{
		oDensityField.setDisplayElements(false, true, true);
	}
	if (k == '2')
	{
		densityMap.setDisplayElements(false, true, true);
	}
	if (k == '`')
	{
		oDensityField.setDisplayElements(false, false, false);
		densityMap.setDisplayElements(false, false, false);
	}

	if (k == 's')
	{
		//zFnMesh fn(inMesh);
		//fn.splitFace(0, e0, e1, f0, f1);
		//cout << "split face" << endl;

		//zPointArray edgeCenters;
		//fn.getCenters(zHEData::zEdgeData, edgeCenters);
		//inMesh.setEdgeCenters(edgeCenters);
	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
