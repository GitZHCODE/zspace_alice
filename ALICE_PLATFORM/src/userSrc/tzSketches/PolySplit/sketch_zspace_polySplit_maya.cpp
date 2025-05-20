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
double offsetVal = 0.2;

double targetArea = 5000;
int numInterations;
int numPolys;
double computeTime = 0;
bool readMap = false;

float mapScalar = 1.0f;
////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

vector<zPointArray> inPolys;
vector<vector<zPolygon>> outPolys;
vector<zLine> cutLines;
unordered_map <int, vector<zLine>> map_age_cutLines;

zObjMeshScalarField oDensityField;
zObjMesh oMesh;
zObjMesh densityMap;

string path_inMesh = "data/polySplit/inPolys.json";
string path_field_density = "data/polySplit/densityMap.json";
string path_field_programme = "data/polySplit/densityMap.json";

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
				poly.split(area * 0.5, poly1, poly2, cutLine);
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

void fromFile(string& file, vector<zPointArray>& inPolys)
{
	inPolys.clear();

	json j;
	bool chk = core.json_read(file, j);

	if (chk)
	{
		zObjMesh inMesh;
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
	string outPath = "data/polySplit/out/blocks";

	// Cleanup existing files in the output directory
	fs::remove_all(outPath);
	// Recreate the output directory
	fs::create_directories(outPath);

	int counter_numFile = 0;
	int counter1 = 0;
	for (const auto& polys : outPolys)
	{
		string folder = outPath + "/block" + "_" + to_string(counter1);
		if (!fs::exists(folder))
			fs::create_directory(folder);

		int counter2 = 0;

		for (const auto& poly : polys)
		{
			// Generate the output path
			string filePath = folder + "/tile" + "_" + to_string(counter2) + ".json";

			// Export the vectors to the JSON file
			std::ofstream outFile(filePath);
			if (!outFile.is_open())
			{
				std::cerr << "Failed to open file for writing: " << filePath << std::endl;
				continue; // Skip to the next iteration
			}

			// Export the fnGraph to the JSON file
			json j;
			Vectors pos = poly.getVectors();

			for (const auto& p : pos)
			{
				json point = {
					p.x,p.y,p.z
				};
				j.push_back(point);
			}

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

	if (fs::exists(path_field_density))
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
			fnField.getNeighbour_Contained(v.getPosition(), neighbours);

			for (auto& id : neighbours)
			{
				zItMeshScalarField it(oField, id);
				fVals[id] += v.getColor().r;
			}
		}

		fnField.setFieldValues(fVals);

		setFieldScalar(densityMin, densityMax);
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

	// read mesh
	//pts.push_back(zPoint(0, 0, 0));
	//pts.push_back(zPoint(2, 0, 0));
	//pts.push_back(zPoint(1, 1, 0));
	//pts.push_back(zPoint(0, 1, 0));

	//pts.push_back(zPoint(0, 0, 0));
	//pts.push_back(zPoint(20, 0, 0));
	//pts.push_back(zPoint(10, 10, 0));
	//pts.push_back(zPoint(0, 10, 0));

	//zFnMesh fnMesh(oMesh);
	////fnMesh.from("data/polySplit/cityBaseMesh3.json", zJSON);
	//fnMesh.from("data/polySplit/blockBase.json", zJSON);

	//zItMeshFace f(oMesh);
	//for (; !f.end(); f++)
	//{
	//	zPointArray poly;
	//	f.getVertexPositions(poly);
	//	inPolys.push_back(poly);
	//}

	fromFile(path_inMesh, inPolys);

	fieldFromMap_density(path_field_density, oDensityField, 100, 100);

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oDensityField);
	model.addObject(densityMap);

	oDensityField.setDisplayElements(false, false, false);
	densityMap.setDisplayElements(false, false, false);

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
	S.sliders[2].attachToVariable(&targetArea, 0, 50000);
	S.addSlider(&densityMin, "densityMin");
	S.sliders[3].attachToVariable(&densityMin, 0, 1);
	S.addSlider(&densityMax, "densityMax");
	S.sliders[4].attachToVariable(&densityMax, 0, 1);
	S.addSlider(&offsetVal, "offsetVal");
	S.sliders[5].attachToVariable(&offsetVal, 0, 1);

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
		outPolys.clear();
		auto begin = std::chrono::high_resolution_clock::now();

		setFieldScalar(densityMin, densityMax);

		//cutLines.clear();
		map_age_cutLines.clear();
		numInterations = 0;
		numPolys = 0;

		//split_targetArea(pts, targetArea);


		for (auto& poly : inPolys)
		{
			vector<zPolygon> polyInBlock;
			int tempNumIteration;
			//split_targetArea(poly, targetArea, map_age_cutLines, polyInBlock, numInterations);
			split_averageArea(poly, targetArea, map_age_cutLines, polyInBlock, tempNumIteration);
			
			for (auto& poly : polyInBlock)
				poly = offsetPoly(poly, offsetVal);

			outPolys.push_back(polyInBlock);
			numPolys += polyInBlock.size();

			numInterations = tempNumIteration > numInterations ? tempNumIteration : numInterations;
		}


		
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

		exportBlocksToFile();
		exportCutsToFile();
		exportTo = !exportTo;
	}

	if (reload)
	{
		inPolys.clear();
		outPolys.clear();
		cutLines.clear();
		map_age_cutLines.clear();

		fromFile(path_inMesh, inPolys);
		fieldFromMap_density(path_field_density, oDensityField, 100, 100);

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

		for (auto& pts : inPolys)
		{
			for (int i = 0; i < pts.size() - 1; i++)
			{
				model.displayUtils.drawLine(pts[i], pts[i + 1], zBLACK, 3);
			}
			model.displayUtils.drawLine(pts[pts.size() - 1], pts[0], zBLACK, 3);
		}

		for (auto& polys : outPolys)
		{
			for (auto& poly : polys)
			{
				zPointArray pts;
				for (auto& vec : poly.getVectors())
					pts.push_back(zPoint(vec.x, vec.y, vec.z));

					for (int i = 0; i < pts.size() - 1; i++)
					{
						model.displayUtils.drawLine(pts[i], pts[i + 1], zBLACK, 1);
					}
					model.displayUtils.drawLine(pts[pts.size() - 1], pts[0], zBLACK, 1);
			}

		}

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
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
