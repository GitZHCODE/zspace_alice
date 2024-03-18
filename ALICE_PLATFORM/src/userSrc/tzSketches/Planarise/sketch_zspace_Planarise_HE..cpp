//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <iostream>
#include <execution>
#include <algorithm>


#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace zSpace;
using namespace std;


////////////////////////////////////////////////////////////////////////// General

bool computePlanar = false;
bool computeConical = false;
bool computeParallel = false;
bool exportMeshes = false;
bool display = false;
bool toFile = false;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMesh oMesh;

zFnMeshDynamics fnDyMesh;
zDoubleArray devs_planarity;
zDoubleArray devs_conical;
zIntArray fixedVertexIds;

double tol_d = 0.001;
double dT = 0.1;

int numMeshes;
int myCounter = 0;
//string dir_in = "data/zPanelData/forPlanarity/joined-0.10step-v1/";
string dir_in = "data/pSolver/in/joined-0.10step-v1/joined-0.10step-v1.json";
//string dir_out = "data/zPanelData/forPlanarity/OUT_joined-0.10step-v1/";
//string dir_out = "data/pSolver/in/joined-0.10step-v1_out/";
string dir_out = "omniverse://nucleus.zaha-hadid.com/Projects/1453_CODE/UserTests/TZ/nanshaRoofAnim.usda";


////// --- GUI OBJECTS ----------------------------------------------------
//omniverse
zOmniCore omniCore;
UsdStageRefPtr stage;
UsdPrim usd;
int timeStamp = 0;
int timeSpace = 500;

void to(UsdPrim& usd, zObjMesh& meshObj, UsdTimeCode timeCode = UsdTimeCode::Default())
{
	zFnMesh fnMesh(meshObj);
	//mesh attributes
	VtArray<GfVec3f> points;
	VtArray<GfVec3f> normals;
	VtArray<int> fVCounts;
	VtArray<int> fVIDs;
	GfMatrix4d transform;

	//custom attributes
	VtArray<GfVec3f> vCols;
	VtArray<float> opacity;

	VtArray<GfVec3f> vCols_unique;
	VtArray<float> opacity_unique;
	VtArray<int> vCols_unique_index;

	//set mesh vertex attributes
	int numV = fnMesh.numVertices();
	zPoint* rawVPositions = fnMesh.getRawVertexPositions();
	zColor* rawVColor = fnMesh.getRawVertexColors();

	//get & set transformation
	zTransform t;
	fnMesh.getTransform(t);

	transform.Set(t(0, 0), t(1, 0), t(2, 0), t(3, 0),
		t(0, 1), t(1, 1), t(2, 1), t(3, 1),
		t(0, 2), t(1, 2), t(2, 2), t(3, 2),
		t(0, 3), t(1, 3), t(2, 3), t(3, 3));

	// v positions and color
	for (int i = 0; i < numV; i++)
	{
		GfVec3f v_attr, c_attr;
		//set vertex positions
		v_attr.Set(rawVPositions[i].x, rawVPositions[i].y, rawVPositions[i].z);
		points.push_back(v_attr);

		opacity.push_back(1.0);
		c_attr.Set(rawVColor[i].r, rawVColor[i].g, rawVColor[i].b);
		vCols.push_back(c_attr);

		int id = -1;
		bool chkRepeat = false;

		for (int j = 0; j < vCols_unique.size(); j++)
		{
			if (vCols_unique[j] == c_attr)
			{
				chkRepeat = true;
				id = j;
				break;
			}
		}

		if (chkRepeat)
		{
			vCols_unique_index.push_back(id);
		}
		else
		{
			vCols_unique_index.push_back(vCols_unique.size());
			vCols_unique.push_back(c_attr);
			opacity_unique.push_back(1.0);

		}

	}

	//set mesh face attributes
	for (zItMeshFace f(meshObj); !f.end(); f++)
	{
		zVector fNorm = f.getNormal();

		fVCounts.push_back(f.getNumVertices());

		zIntArray v_idx;
		f.getVertices(v_idx);
		for (auto& o : v_idx)
		{
			fVIDs.push_back(o);

			GfVec3f n_attr;
			n_attr.Set(fNorm.x, fNorm.y, fNorm.z);
			normals.push_back(n_attr);
		}
	}



	//create default attr
	UsdGeomMesh usdMesh(usd);
	TfToken interpolationType = pxr::UsdGeomTokens->uniform;

	if (timeCode.IsDefault() || timeCode == 0)
	{
		//create points & normals
		usdMesh.CreateFaceVertexCountsAttr().Set(fVCounts);
		usdMesh.CreateFaceVertexIndicesAttr().Set(fVIDs);

		usdMesh.CreatePointsAttr().Set(points, timeCode);
		usdMesh.CreateNormalsAttr().Set(normals, timeCode);

		//create display color
		auto displayColorPrimvar = usdMesh.CreateDisplayColorPrimvar();
		displayColorPrimvar.Set(vCols_unique, timeCode);
		displayColorPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayColorPrimvar.SetInterpolation(interpolationType);

		//create opacity
		auto displayOpacityPrimvar = usdMesh.CreateDisplayOpacityPrimvar();
		displayOpacityPrimvar.Set(opacity_unique, timeCode);
		displayOpacityPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayOpacityPrimvar.SetInterpolation(interpolationType);
	}
	else
	{
		//update points & normals
		usdMesh.GetPointsAttr().Set(points, timeCode);
		usdMesh.GetNormalsAttr().Set(normals, timeCode);

		//update display color
		auto displayColorPrimvar = usdMesh.GetDisplayColorPrimvar();
		displayColorPrimvar.Set(vCols_unique, timeCode);
		displayColorPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayColorPrimvar.SetInterpolation(interpolationType);

		//update opacity
		auto displayOpacityPrimvar = usdMesh.GetDisplayOpacityPrimvar();
		displayOpacityPrimvar.Set(opacity_unique, timeCode);
		displayOpacityPrimvar.SetIndices(vCols_unique_index, timeCode);
		displayOpacityPrimvar.SetInterpolation(interpolationType);
	}

	//attr = meshPrim.AddTransformOp();
	//attr.Set(transform);
}

template <typename T>
void addProperty(UsdPrim& usd, TfToken& token, SdfValueTypeName typeName, const T& propertyValue, UsdTimeCode timeCode = UsdTimeCode::Default())
{
	UsdGeomPrimvarsAPI primvar(usd);

	if (timeCode.IsDefault() || timeCode == 0)
	{
		primvar.CreatePrimvar(token, typeName).Set(propertyValue, timeCode);
	}
	else
	{
		primvar.GetPrimvar(token).Set(propertyValue, timeCode);
	}
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

	zStringArray files;
	zFnMesh fnMesh(oMesh);
	fnMesh.from(dir_in, zJSON);

	fnDyMesh.create(oMesh, false);
	model.addObject(oMesh);
	oMesh.setDisplayElements(true, true, true);

	//make fixed vertices
	zColorArray vertexColors;
	fnMesh.getVertexColors(vertexColors);

	for (auto& cols : vertexColors)
		//cout << "cols:" << cols.r << endl;

		for (int j = 0; j < vertexColors.size(); j++)
		{
			devs_planarity.push_back(-1);
			devs_conical.push_back(-1);

			//cout << "col:" << vertexColors[j].r << endl;
			if (vertexColors[j].r == 1)
				fixedVertexIds.push_back(j);
		}

	fnDyMesh.setFixed(fixedVertexIds);

	// Connect to OV
	bool doLiveEdit = false;
	std::string existingStage;
	string destinationPath = "omniverse://nucleus.zaha-hadid.com/Projects/1453_CODE/UserTests/TZ";
	bool chk = omniCore.isValidOmniURL(destinationPath);

	// Startup Omniverse with the default login
	if (!omniCore.startOmniverse()) exit(1);
	omniCore.printConnectedUsername(destinationPath);

	//--------------------- 3 - WRITE TO FILE --------------------------
	stage = UsdStage::CreateNew(dir_out);
	stage->SetMetadata(TfToken("defaultPrim"), VtValue("World"));
	stage->SetMetadata(TfToken("upAxis"), VtValue("Z"));
	stage->SetMetadata(TfToken("metersPerUnit"), VtValue(1));

	string s_root, s_layer, s_prim;
	s_root = "/World";
	s_layer = s_root + "/Geometry";
	s_prim = s_layer + "/mesh";
	UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath(s_root));
	UsdGeomXform layer = UsdGeomXform::Define(stage, SdfPath(s_layer));
	UsdGeomMesh meshPrim = UsdGeomMesh::Define(stage, SdfPath(s_prim));

	usd = meshPrim.GetPrim();
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object


	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));
	int buttonCounter = 0;
	B.addButton(&computePlanar, "computePlanar");
	B.buttons[buttonCounter++].attachToVariable(&computePlanar);
	B.addButton(&computeConical, "computeConical");
	B.buttons[buttonCounter++].attachToVariable(&computeConical);
	B.addButton(&computeParallel, "computeParallel");
	B.buttons[buttonCounter++].attachToVariable(&computeParallel);
	B.addButton(&display, "displayMesh");
	B.buttons[buttonCounter++].attachToVariable(&display);
	B.addButton(&exportMeshes, "exportMeshes");
	B.buttons[buttonCounter++].attachToVariable(&exportMeshes);

}
//void processMesh(int myCounter, vector<zObjMesh>& oMeshes, vector<zFnMeshDynamics>& fnDyMeshes,
//	vector<double>& devs_planarity, double tol_d)
//{
//	bool exit_planar = false;
//	zObjMesh* oMesh = &oMeshes[myCounter];
//	zFnMeshDynamics* fnDyMesh = &fnDyMeshes[myCounter];
//	zVectorArray forceDir;
//
//	fnDyMesh->addPlanarityForce(1.0, tol_d, zVolumePlanar, devs_planarity[myCounter], forceDir, exit_planar);
//	fnDyMesh->update(dT, zRK4, true, true, true);
//
//	for (zItMeshFace f(*oMesh); !f.end(); f++)
//	{
//		if (devs_planarity[myCounter][f.getId()] < tol_d)
//			f.setColor(zColor(0, 1, 0, 1));
//		else
//			f.setColor(zColor(1, 0, 1, 1));
//	}
//}


//float getInternalAngle(zItMeshFace& face, int vertexId)
//{
//	printf("\n getInternalAngle 0");
//	zItMeshVertexArray fVerts;
//	face.getVertices(fVerts);
//
//
//	zIntArray fVertsIds;
//	face.getVertices(fVertsIds);
//
//	// Ensuring that the given vertexId belongs to this face
//	if (std::find(fVertsIds.begin(), fVertsIds.end(), vertexId) == fVertsIds.end()) return 0;
//
//
//	int prevVert, nextVert;
//	int counter = 0;
//	zItMeshHalfEdgeArray heArray;
//	zPoint vCurr, vPrev, vNext;
//	for (int i = 0; i < fVertsIds.size(); i++)
//	{
//		printf("\n getInternalAngle 1, %i", counter++);
//
//		if (fVertsIds[i] == vertexId)
//		{
//
//			zItMeshHalfEdgeArray he;
//			fVerts[i].getConnectedHalfEdges(he);
//			vCurr = fVerts[i].getPosition();
//			for (auto& e : he)
//			{
//				zPoint start, end;
//				start = e.getStartVertex().getPosition();
//				end = e.getVertex().getPosition();
//
//				bool chk = start.x == vCurr.x && start.y == vCurr.y && start.z == vCurr.z;
//				if (chk)
//				{
//					heArray.push_back(e);
//				}
//				else
//				{
//					////check if the edge is part of the face
//					//auto itFace = std::find_if(fVerts.begin(), fVerts.end(), [&](zItMeshVertex& v)
//					//	{
//					//		return v.getPosition().x == end.x && v.getPosition().y == end.y && v.getPosition().z == end.z;
//					//	});
//
//
//					auto it = std::find_if(fVerts.begin(), fVerts.end(), [&](zItMeshVertex& v)
//						{
//							return v.getPosition().x == end.x && v.getPosition().y == end.y && v.getPosition().z == end.z;
//						});
//					if (it != fVerts.end()) //means it exist
//					{
//
//						heArray.push_back(e.getSym());
//					}
//
//
//				}
//			}
//
//
//
//			prevVert = fVertsIds[(i + fVertsIds.size() - 1) % fVertsIds.size()]; // Get previous vertex in the loop
//			nextVert = fVertsIds[(i + 1) % fVertsIds.size()];                  // Get next vertex in the loop
//			break;
//		}
//	}
//	printf("\n getInternalAngle 1");
//
//
//
//	////zFnMesh fnMesh(*meshObj);
//	//zFnMesh fnMesh(*meshObj);
//	//zPointArray positions;
//	//fnMesh.getVertexPositions(positions);
//
//	//zPoint vCurr = meshObj->getVertexPosition(vertexId);
//	//zPoint vPrev = meshObj->getVertexPosition(prevVert);
//	//zPoint vNext = meshObj->getVertexPosition(nextVert);
//	/*zPoint vCurr = positions[vertexId];
//	zPoint vPrev = positions[prevVert];
//	zPoint vNext = positions[nextVert];	*/
//
//
//	vCurr = fVerts[vertexId].getPosition();
//	vPrev = fVerts[prevVert].getPosition();
//	vNext = fVerts[nextVert].getPosition();
//	printf("\n getInternalAngle 2");
//
//
//
//	zVector vec1 = vPrev - vCurr;
//	zVector vec2 = vNext - vCurr;
//	printf("\n getInternalAngle 3");
//
//	vec1.normalize();
//	vec2.normalize();
//
//	double angle = acos(vec1 * vec2);  // Dot product to get the cosine of the angle
//	printf("\n getInternalAngle 4");
//
//	return angle;
//}
//
//void addConicalForce(zFnMeshDynamics& fnDynamics, double strength, double tolerance, zDoubleArray& conicalDeviations, zVectorArray& forceDir, bool& exit)
//{
//	printf("\n addConicalForce 0");
//	if (forceDir.size() != fnDynamics.numVertices())
//	{
//		forceDir.clear();
//		forceDir.assign(fnDynamics.numVertices(), zVector());
//	}
//	printf("\n addConicalForce 1");
//
//	exit = true;
//	zUtilsCore core;
//	zPoint* vPositions = fnDynamics.getRawVertexPositions();
//
//	if (conicalDeviations.size() != fnDynamics.numVertices())
//	{
//		conicalDeviations.clear();
//		conicalDeviations.assign(fnDynamics.numVertices(), -1);
//	}
//	printf("\n addConicalForce 2");
//	int counter = 0;
//	// Iterate through all vertices
//	for (zItMeshVertex v(*fnDynamics.meshObj); !v.end(); v++)
//	{
//		int vID = v.getId();
//		zIntArray vFaces;
//		v.getConnectedFaces(vFaces);
//		double sumAngles = 0;
//
//		printf("\n addConicalForce 3, %i", counter++);
//
//		// Calculate the total angles made by adjacent faces to the vertex
//		for (int i = 0; i < vFaces.size(); i++)
//		{
//			zItMeshFace f(*fnDynamics.meshObj, vFaces[i]);
//			sumAngles +=  getInternalAngle(f, vID);
//		}
//
//		// Deviation from 2pi (360 degrees in radians)
//		conicalDeviations[vID] = abs(2 * M_PI - sumAngles);
//		printf("\n addConicalForce 4, %i", counter++);
//
//		// If the deviation is above the given tolerance, apply forces to correct it
//		if (conicalDeviations[vID] > tolerance)
//		{
//			exit = false;
//			printf("\n addConicalForce 5, %i", counter++);
//
//			zItMeshFaceArray connectedFaces;
//			v.getConnectedFaces(connectedFaces);
//
//			float targetAngle = 2.0f * static_cast<float>(M_PI);  // Explicitly casting M_PI to float
//			//float summedAngles = ...;  // You'd calculate this from the getInternalAngle() function for all neighboring faces
//			float summedAngles = 0.0f;
//			for (auto& face : connectedFaces)
//			{
//				float angle = getInternalAngle(face, vID);
//				summedAngles += angle;
//			}
//
//			// Determine the difference
//			float angleDiff = summedAngles - targetAngle;
//
//			// Get the average normal for the vertex by averaging face normals of surrounding faces
//			//zVector avgNormal = getAverageNormalForVertex(vertexId);  // This is a hypothetical function you'd need to define
//
//			zVector avgNormal(0.0f, 0.0f, 0.0f); // Initialize an empty vector
//
//			int counter2 = 0;
//			for (auto& face : connectedFaces)
//			{
//				printf("\n addConicalForce 6, %i - %i", counter++, counter2++);
//
//				zVector faceNormal = face.getNormal(); // retrieve the normal of a face
//				avgNormal += faceNormal;
//			}
//
//			avgNormal /= static_cast<float>(connectedFaces.size()); // Average out the summed vectors
//
//			printf("\n addConicalForce 7, %i", counter++);
//
//
//			avgNormal.normalize();
//
//			// Calculate the force's magnitude and direction
//			zVector vForce = avgNormal * angleDiff * strength;  // 'strength' is a scaling factor you can tune
//
//			printf("\n addConicalForce 8, %i", counter++);
//
//			//zVector vForce = ... // Determine the force based on the deviation and the local geometry
//			vForce = vForce * strength;
//
//			fnDynamics.fnParticles[vID].addForce(vForce);
//			forceDir[vID] += vForce;
//
//			printf("\n addConicalForce 9, %i", counter++);
//
//		}
//		printf("\n addConicalForce 10");
//
//	}
//
//	for (auto& fDir : forceDir)
//	{
//		if (fDir.length() > 0) fDir.normalize();
//	}
//}
//

void update(int value)
{
	if (computePlanar)
	{
		bool exit_planar = false;

		double mxDev = 0;
		zVectorArray forceDir;

		//fnDyMesh.addPlanarityForce(1.0, tol_d, zQuadPlanar, devs_planarity[myCounter],forceDir, exit_planar);
		fnDyMesh.addPlanarityForce(1.0, tol_d, zVolumePlanar, devs_planarity, forceDir, exit_planar);
		fnDyMesh.update(dT, zRK4, true, true, true);

		for (zItMeshFace f(oMesh); !f.end(); f++)
		{
			if (devs_planarity[f.getId()] < tol_d)
				f.setColor(zColor(0, 1, 0, 1));
			else f.setColor(zColor(1, 0, 1, 1));
		}
		zFnMesh fn(oMesh);
		fn.computeVertexColorfromFaceColor();

		double fPlanar_max = core.zMax(devs_planarity);
		double fPlanar_min = core.zMin(devs_planarity);
		printf("\n planar devs : %1.6f %1.6f \n", fPlanar_max, fPlanar_min);

		//to usd
		if (timeStamp % 10 == 0)
		{
			int timeCode = timeStamp / 10;
			to(usd, oMesh, timeCode);

			VtArray<float> out_planarity;
			out_planarity.assign(devs_planarity.size(), 0.0f);
			for (int i = 0; i < devs_planarity.size(); i++) {
				out_planarity[i] = static_cast<float>(devs_planarity[i]);
			}
			addProperty(usd, TfToken("planarity"), SdfValueTypeNames->FloatArray, out_planarity, timeCode);
		}
		timeStamp++;


		if (exit_planar)
		{
			computePlanar = !computePlanar;
			toFile = true;
		}
	}
	
	if (exportMeshes)
	{
		exportMeshes != exportMeshes;
		toFile = true;
	}
	if (toFile)
	{

		//{
		//	zFnMesh fn(oMesh);
		//	fn.to(dir_out + "out_" + to_string(i) + ".json", zJSON);
		//}

		stage->SetStartTimeCode(0.0);
		stage->SetEndTimeCode(timeStamp - 1);
		stage->SetFramesPerSecond(24.0);
		stage->SetTimeCodesPerSecond(24.0);

		stage->GetRootLayer()->Save();
		cout << endl << "stage saved as: " << dir_out << endl;

		toFile = !toFile;
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
		
		/*zPoint* vPositions = fnDyMeshes[myCounter].getRawVertexPositions();

		for (int i = 0; i < fnDyMeshes[myCounter].numVertices(); i++) {
			bool fixedV = std::find(std::begin(fixedVertexIds[myCounter]), std::end(fixedVertexIds[myCounter]), i) != std::end(fixedVertexIds[myCounter]);

			if (fixedV)
				model.displayUtils.drawPoint(vPositions[i], zColor(1, 0, 0, 1), 5);
		}*/
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') computePlanar = true;;	
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
