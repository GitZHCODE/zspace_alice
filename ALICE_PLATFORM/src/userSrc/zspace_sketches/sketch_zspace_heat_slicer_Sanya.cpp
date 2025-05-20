//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsGeometry.h>

using namespace zSpace;
using namespace std;
////////////////////////////////////////////////////////////////////////// CUSTOM OBJECTS

struct zMegaPanel
{
	int id;
	zObjMesh o_panelMesh;
	zFloatArray scalars;

	zObjMeshArray o_stripMeshes;
};

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 

zUtilsCore core;

void computeVLoops(zObjMesh& oMesh, int startVertexID0, int startVertexID1, vector<zItMeshHalfEdgeArray>& v_Loops)
{

	zFnMesh fnMesh_in(oMesh);

	zItMeshHalfEdge heStart;
	bool chk = fnMesh_in.halfEdgeExists(startVertexID1, startVertexID0, heStart);



	bool flip = false;
	if (!heStart.onBoundary())
	{
		heStart = heStart.getSym();
		flip = true;
	}

	zItMeshHalfEdge he_U = heStart;
	bool exit_1 = false;
	do
	{

		zItMeshHalfEdge he_V = (!flip) ? he_U.getSym().getNext() : he_U.getSym().getPrev();
		int startVertexValence = (!flip) ? he_V.getStartVertex().getValence() : he_V.getVertex().getValence();

		zItMeshHalfEdgeArray tempV;

		bool exit_2 = false;

		do
		{
			if (flip) tempV.push_back(he_V.getSym());
			else tempV.push_back(he_V);

			he_V.getEdge().setColor(zBLUE);

			if (flip)
			{
				if (he_V.getStartVertex().checkValency(startVertexValence)) exit_2 = true;
			}
			else
			{
				if (he_V.getVertex().checkValency(startVertexValence)) exit_2 = true;
			}


			if (!exit_2) he_V = (!flip) ? he_V.getNext().getSym().getNext() : he_V.getPrev().getSym().getPrev();

		} while (!exit_2);

		v_Loops.push_back(tempV);

		he_U.getEdge().setColor(zMAGENTA);

		if (flip)
		{
			if (he_U.getStartVertex().checkValency(2)) exit_1 = true;
		}
		else
		{
			if (he_U.getVertex().checkValency(2)) exit_1 = true;
		}

		//if (he == heStart) exit_1 = true;



		if (!exit_1) he_U = (!flip) ? he_U.getNext() : he_U.getPrev();
		else
		{
			tempV.clear();
			zItMeshHalfEdge he_V1 = (!flip) ? he_U.getNext() : he_U.getPrev();
			bool exit_3 = false;
			do
			{
				if (flip)tempV.push_back(he_V1.getSym());
				else tempV.push_back(he_V1);
				he_V1.getEdge().setColor(zBLUE);

				if (flip)
				{
					if (he_V1.getStartVertex().checkValency(2)) exit_3 = true;
				}
				else
				{
					if (he_V1.getVertex().checkValency(2)) exit_3 = true;
				}


				if (!exit_3) he_V1 = (!flip) ? he_V1.getNext() : he_V1.getPrev();

			} while (!exit_3);

			v_Loops.push_back(tempV);
		}

	} while (!exit_1);
}

void computeHeatScalars(zObjMesh& oMesh, vector<zItMeshHalfEdgeArray>& v_Loops, zScalarArray& scalars)
{
	zFnMesh fnMesh(oMesh);

	scalars.clear();
	scalars.assign(fnMesh.numVertices(), -1);

	for (int l = 0; l < v_Loops.size(); l++)
	{
		float length = 0;
		for (int j = 0; j < v_Loops[l].size(); j++)
		{
			if (j == 0) scalars[v_Loops[l][j].getStartVertex().getId()] = length;
			
			length += v_Loops[l][j].getLength();
			scalars[v_Loops[l][j].getVertex().getId()] = length;
		}
	}

	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	zDomainFloat distanceDomain(minScalar, maxScalar);
	zDomainColor colDomain(zColor(1, 0, 0, 1), zColor(0, 1, 0, 1));

	for (int i = 0; i < fnMesh.numVertices(); i++)
	{
		mesh_vColors[i] = core.blendColor(scalars[i], distanceDomain, colDomain, zRGB);
	}

	fnMesh.computeFaceColorfromVertexColor();
}

void computeBoundaryVertices_A(zObjMesh &oMesh, int startVertexID0, int startVertexID1, zIntArray& bIDs)
{
	zFnMesh fnMesh_in(oMesh);

	bIDs.clear();

	zItMeshHalfEdge heStart;
	bool chk = fnMesh_in.halfEdgeExists(startVertexID1, startVertexID0, heStart);



	bool flip = false;
	if (!heStart.onBoundary())
	{
		heStart = heStart.getSym();
		flip = true;
	}

	zItMeshHalfEdge he_U = heStart;

	if (flip) bIDs.push_back(he_U.getVertex().getId());
	else bIDs.push_back(he_U.getStartVertex().getId());

	bool exit_1 = false;
	do
	{
		if (flip) bIDs.push_back(he_U.getStartVertex().getId());
		else bIDs.push_back(he_U.getVertex().getId());
	
		he_U.getEdge().setColor(zMAGENTA);

		if (flip)
		{
			if (he_U.getStartVertex().checkValency(2)) exit_1 = true;
		}
		else
		{
			if (he_U.getVertex().checkValency(2)) exit_1 = true;
		}
			

		if (!exit_1) he_U = (!flip) ? he_U.getNext() : he_U.getPrev();
		

	} while (!exit_1);
}

void computeBoundaryVertices(zObjMesh* o_mesh , zIntArray& vIDs, zIntArray &bIDs)
{
	bIDs.clear();

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

void computeContours(zObjMesh* o_mesh, zFloatArray& scalars_start, zFloatArray& scalars_end, int currentContourID, int totalContours, zObjGraphArray &o_contourGraphs)
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

void computeContours_A(zObjMesh &o_mesh, zFloatArray& scalars, float contourDistances, zObjGraphArray& o_contourGraphs)
{
	zScalar minScalar = core.zMin(scalars);
	zScalar maxScalar = core.zMax(scalars);

	printf("\n minScalar : %1.2f | maxScalar %1.2f ", minScalar, maxScalar);
	
	// weighted scalars
	float dDomain = maxScalar - minScalar;
	int totalContours = ceil(dDomain / contourDistances);
	float distanceIncrements = dDomain / totalContours;

	printf("\n total Contours : %i | distance %1.2f | dDomain %1.2f", totalContours, distanceIncrements, dDomain);

	o_contourGraphs.clear();
	o_contourGraphs.assign(totalContours, zObjGraph());

	// Generate the isocontour using the threshold value
	zFnMesh fnMesh(o_mesh);
	int pres = 3;

	for (int i = 1; i < totalContours; i++)
	{
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vColors;
		
		
		fnMesh.getIsoContour(scalars, minScalar + (distanceIncrements * i), positions, edgeConnects, vColors, pres, pow(10, -1 * pres));	

		// Create graph from the isocontour
		zFnGraph tempFn(o_contourGraphs[i]);
		tempFn.create(positions, edgeConnects);
		tempFn.setEdgeColor(zColor(255, 255, 255, 1));
		tempFn.setEdgeWeight(2);
		tempFn.setVertexColors(vColors, false);

	}
	

	

	// color mesh
	zColor* mesh_vColors = fnMesh.getRawVertexColors();

	

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

void computeContours_Strips(zObjMesh& o_mesh, zFloatArray& scalars,float &minScalar, float &maxScalar, int &totalContours, float &distanceIncrements, float &contourDistances, float &gap, zObjMeshArray& o_contourMeshes)
{

	o_contourMeshes.clear();
	o_contourMeshes.assign(totalContours, zObjMesh());

	// Generate the isocontour using the threshold value
	zFnMesh fnMesh(o_mesh);
	int pres = 3;

	zDomainFloat contoursDomain(0, totalContours);
	zDomainColor contourColor(zMAGENTA, zBLUE);

	int activeStrips = 0;
	for (int i = 1; i < totalContours; i++)
	{
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vColors;

		float lowThreshold = minScalar + (distanceIncrements * (i - 1)) + (gap * 0.5);
		float highThreshold = minScalar + (distanceIncrements * i) + (gap * -0.5);

		fnMesh.getIsobandMesh(scalars, lowThreshold, highThreshold, o_contourMeshes[i]);

		zFnMesh fnStripMesh(o_contourMeshes[i]);
		//printf("\n strips %i %i %i ", fnStripMesh.numVertices(), fnStripMesh.numEdges(), fnStripMesh.numPolygons());

		//zColor col = core.blendColor((float)i, contoursDomain, contourColor, zHSV);

		if (fnStripMesh.numPolygons()>0)
		{
			activeStrips++;
			zColor col = (i % 2 == 0) ? contourColor.min : contourColor.max;
			fnStripMesh.setFaceColor(col);
		}
		
	}

	printf("\n num Active Strips : %i ", activeStrips);

}

void computeMegaPanel(zObjMesh& o_inmesh, string path, zFloatArray& scalars, vector< zMegaPanel> &megaPanels)
{
	json j;
	core.readJSON(path, j);

	zInt2DArray panel_faceIDS;
	bool chk = core.readJSONAttribute(j, "PanelFaces", panel_faceIDS);

	printf("\n megapanels %i ", panel_faceIDS.size());

	zFnMesh fnMesh(o_inmesh);
	zPoint* vPositions = fnMesh.getRawVertexPositions();
	zColor* fColors = fnMesh.getRawFaceColors();

	if (chk)
	{
		megaPanels.clear();
		megaPanels.assign(panel_faceIDS.size(), zMegaPanel());

		for (int i = 0; i < panel_faceIDS.size(); i++)
		{
			zIntArray vMap;
			vMap.assign(fnMesh.numVertices(), -1);

			zFnMesh fnMeshPanel(megaPanels[i].o_panelMesh);

			megaPanels[i].scalars.clear();
			megaPanels[i].id = i;

			zPointArray positions;
			zIntArray pCounts, pConnects;
			zColorArray faceColors;

			for (int j = 0; j < panel_faceIDS[i].size(); j++)
			{
				zItMeshFace f(o_inmesh, panel_faceIDS[i][j]);

				zIntArray fVerts;
				f.getVertices(fVerts);

				for (int k = 0; k < fVerts.size(); k++)
				{
					if (vMap[fVerts[k]] == -1)
					{
						vMap[fVerts[k]] = positions.size();
						positions.push_back(vPositions[fVerts[k]]);
						megaPanels[i].scalars.push_back(scalars[fVerts[k]]);
						
					}
					
					pConnects.push_back(vMap[fVerts[k]]);
				}

				faceColors.push_back(fColors[f.getId()]);
				pCounts.push_back(fVerts.size());
			}

			fnMeshPanel.create(positions, pCounts, pConnects);
			fnMeshPanel.setFaceColors(faceColors,false);
		}
	}

	
}

void computeMegaPanel_Strips(vector< zMegaPanel>& megaPanels, zFloatArray& globalScalars, float contourDistances, float contourGaps )
{
	zScalar minScalar = core.zMin(globalScalars);
	zScalar maxScalar = core.zMax(globalScalars);

	printf("\n minScalar : %1.2f | maxScalar %1.2f ", minScalar, maxScalar);

	float dDomain = maxScalar - minScalar;
	int totalContours = ceil(dDomain / contourDistances);
	float distanceIncrements = dDomain / totalContours;

	printf("\n total Contours : %i | distance %1.2f | dDomain %1.2f", totalContours, distanceIncrements, dDomain);

	for (auto& mp : megaPanels)
	{
		printf("\n ---- \n megaPanel %i ", mp.id);

		computeContours_Strips(mp.o_panelMesh, mp.scalars, minScalar, maxScalar, totalContours, distanceIncrements, contourDistances, contourGaps, mp.o_stripMeshes);
	}
}

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

////////////////////////////////////////////////////////////////////////// General

bool compute_heat = false;
bool compute_contour = false;
bool exportMeshes = false;
bool compute_color_start = true;

bool d_inMesh = true;
bool d_inMeshFace = true;
bool d_MegaPanel = true;
bool d_MegaPanel_strips = true;
bool d_AllPanels = false;

double background = 0.35;


int currentPanelId = 0;
int totalPanels = 60;

int startID0 = 1344;
int startID1 = 320;

float contourDistance = 0.2;
float contourGap = 0.02;

string filePath = "data/sanya/inMesh_2.json";

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTsMeshParam myMeshParam;

zObjMesh o_inMesh;
vector<zMegaPanel> myPanels;

vector<zItMeshHalfEdgeArray> vLoops;
zScalarArray dScalars;

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


	// read
	
	string inPath;
	cout << "Please enter file path: ";
	cin >> inPath;

	zStringArray split = core.splitString(inPath, "\"");
	filePath = split[0];
		

	ifstream jsonFile(inPath);
	json j;
	jsonFile >> j;
	startID0 = j["StartEnd"][0];
	startID1 = j["StartEnd"][1];
	contourDistance = j["ContourDist"];
	contourGap = j["ContourGap"];


	//cout << "Please enter start vertexID 0: ";
	//cin >> startID0;

	//cout << "Please enter start vertexID 1: ";
	//cin >> startID1;

	//cout << "Please enter contour distance: ";
	//cin >> contourDistance;

	//cout << "Please enter contour gap: ";
	//cin >> contourGap;

	zFnMesh fnInmesh(o_inMesh);
	fnInmesh.from(filePath, zJSON);
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	//model.addObject(oMesh);

	// set display element booleans	
	o_inMesh.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_heat, "compute_heat");
	B.buttons[0].attachToVariable(&compute_heat);

	B.addButton(&d_inMesh, "d_inMesh");
	B.buttons[1].attachToVariable(&d_inMesh);

	B.addButton(&d_inMeshFace, "d_inMeshFace");
	B.buttons[2].attachToVariable(&d_inMeshFace);

	B.addButton(&d_MegaPanel, "d_MegaPanel");
	B.buttons[3].attachToVariable(&d_MegaPanel);

	B.addButton(&d_MegaPanel_strips, "d_MegaPanel_strips");
	B.buttons[4].attachToVariable(&d_MegaPanel_strips);
	
	

}

void update(int value)
{
	o_inMesh.setDisplayFaces(d_inMeshFace);

	if (compute_heat)
	{
		computeVLoops(o_inMesh, startID0, startID1, vLoops);		
		computeHeatScalars(o_inMesh, vLoops, dScalars);
		
		computeMegaPanel(o_inMesh, filePath, dScalars, myPanels);

		totalPanels = myPanels.size();

		compute_heat = !compute_heat;
	}

	if (compute_contour)
	{
		//computeContours_Strips(o_inMesh, dScalars, contourDistance, contourGap, o_stripMeshes);
		computeMegaPanel_Strips(myPanels, dScalars, contourDistance, contourGap);
		
		compute_contour = !compute_contour;
	}

	if (exportMeshes)
	{
		string DIR = "data/sanya/out/";

		bool chkDir = dirExists(DIR);
		if (!chkDir) _mkdir(DIR.c_str());

		for (int i = 0; i < myPanels.size(); i++)
		{
			string subDIR = "data/sanya/out/panels_" + to_string(i);

			bool chk_subDir = dirExists(subDIR);
			if (!chk_subDir) _mkdir(subDIR.c_str());
			
			for (const auto& entry : std::filesystem::directory_iterator(subDIR)) std::filesystem::remove_all(entry.path());

			for (int j = 0; j < myPanels[i].o_stripMeshes.size(); j++)
			{
				zFnMesh fnMesh(myPanels[i].o_stripMeshes[j]);

				string fileName = subDIR + "/strips_" + to_string(j) + ".json";
				fnMesh.to(fileName, zJSON);
			}
		}

		exportMeshes = !exportMeshes;
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
		o_inMesh.draw();		
	}
		
	
	if (d_MegaPanel)
	{

		if (d_AllPanels)
		{
			for (auto& m : myPanels)
			{
				m.o_panelMesh.setDisplayElements(false, false, true);
				m.o_panelMesh.draw();


			}
		}
		else
		{
			int i = currentPanelId;
			if (myPanels.size() > 0 )
			{
				myPanels[i].o_panelMesh.setDisplayElements(false, false, true);
				myPanels[i].o_panelMesh.draw();
			}

		}
	}

	if (d_MegaPanel_strips)
	{
		if (d_AllPanels)
		{
			for (auto& m : myPanels)
			{
				for (auto& strip : m.o_stripMeshes)
				{
					strip.setDisplayElements(false, false, true);
					strip.draw();
				}
			}
		}
		else
		{
			int i = currentPanelId;
			if (myPanels.size() > 0)
			{
				for (auto& strip : myPanels[i].o_stripMeshes)
				{
					strip.setDisplayElements(false, false, true);
					strip.draw();
				}
			}
		}

	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);

	drawString("Total Panels #:" + to_string(totalPanels), vec(winW - 350, winH - 800, 0));
	drawString("Current Panel #:" + to_string(currentPanelId), vec(winW - 350, winH - 775, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("p - compute geodesics", vec(winW - 350, winH - 625, 0));
	drawString("o - compute contours", vec(winW - 350, winH - 600, 0));
	drawString("e - exportGeometry", vec(winW - 350, winH - 575, 0));
	

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute_heat = true;;
	if (k == 'o') compute_contour = true;;
	if (k == 'e') exportMeshes = true;

	if (k == 'w')
	{
		if (currentPanelId < myPanels.size() - 1)currentPanelId++;;
	}
	if (k == 's')
	{
		if (currentPanelId > 0)currentPanelId--;;
	}

	if (k == 'd') d_AllPanels = !d_AllPanels;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
