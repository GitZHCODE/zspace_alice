//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zInterOp.h>

using namespace zSpace;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;


////////////////////////////////////////////////////////////////////////// zSpace Objects

enum zGameObjType { slab = 0, stair = 1, slope = 2, atrium = 3 };

class zVoxel
{
	zObjMesh voxelMesh;
public:
	zVoxel() {
		zFnMesh fn(voxelMesh);
		fn.setVertexColor(zColor(240, 0, 140), true);
	}
};

class zGameObj
{
public:
	int id;
	int status; // 0-inactive; 1-active; 2-voxel mode
	string fileDir;
	string usdToken;
	zGameObjType type;
	Eigen::Matrix4f tMatrix;

	//constructor
	zGameObj(int _id, const string& _fileDir, const string& _usdToken, Eigen::Matrix4f _tMatrix, zGameObjType _type = zGameObjType.slab, int _status = 0)
		: id(_id), fileDir(_fileDir), usdToken(_usdToken), tMatrix(_tMatrix), type(_type), status(_status)
	{}
};

class zPlayground
{
public:
	zPlayground(const zObjMesh& oMesh)
		: gridMesh(oMesh)
	{
		zFnMesh fn(gridMesh);
		zIntArray temp;
		fn.getDualGraph(dualGraph, temp, temp);
	}

	void initialiseGameObj(vector<zGameObj>& gameObjArr)
	{
		zFnGraph fnGraph(dualGraph);
		zPoint* v = fnGraph.getRawVertexPositions();
		for (int i = 0; i < fnGraph.numVertices(); i++)
		{
			int id = i;
			string fileDir = "";
			string usdToken = "";
			Eigen::Matrix4f tMatrix;
			zGameObj gameObj(id, fileDir, usdToken, tMatrix);
			gameObjArr.push_back(gameObj);
		}
	}
private:
	zObjMesh gridMesh;
	zObjGraph dualGraph;
};

class zTsConfigurator
{

public:
	zPlayground playground;
	vector<zGameObj> gameObjArr;

	//configurator methods
	void initialise(const zObjMesh& gridMesh )
	{
		playground = zPlayground(gridMesh);
		playground.initialiseGameObj(gameObjArr);
	}

	//OV methods
	void OVconnect()
	{
		// Connect to OV
		bool doLiveEdit = false;
		std::string existingStage;
		string destinationPath = "omniverse://nucleus.zaha-hadid.com/Projects";
		bool chk = omniCore.isValidOmniURL(destinationPath);

		// Startup Omniverse with the default login
		if (!omniCore.startOmniverse()) exit(1);
		omniCore.printConnectedUsername(destinationPath);
	}

	void OVdisconnect()
	{
		omniCore.shutdownOmniverse();
	}


private:
	zOmniCore omniCore;

};

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

	////////////////////////////////////////////////////////////////////////// Configurator


}

void update(int value)
{
	if (compute)
	{
		auto begin = std::chrono::high_resolution_clock::now();


		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		printf("\n Entire Time: %.7f seconds.", elapsed.count() * 1e-9);

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

	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'g') compute = true;
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
