//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include<headers/zModules/data/zSpace_kMeans.h>



using namespace zSpace;
using namespace std;

/*!<Objects*/
zUtilsCore core;

////////////////////////////////////////////////////////////////////////// CUSTOM METHODS 
bool readData(string path,int numDataPoints, int data_stride, zDoubleArray & outDataPoints)
{
	ifstream myfile;
	myfile.open(path.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	int counter = 0; 

	outDataPoints.clear();
	outDataPoints.assign(numDataPoints* data_stride, double());

	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);
		
		outDataPoints[counter ] = atof(str.c_str());
		counter++;
	
	}


	return true;
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/
zDoubleArray kMeans_data;

int numData = 19450;
int dataStride = 11;

////// --- GUI OBJECTS ----------------------------------------------------
double nClusters = 5;
double nIterations = 100;

void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	// read data
	
	bool chk = readData("data/PanelData.txt", numData, dataStride, kMeans_data);
	
	printf("\n Input Data : ");

	for (int j = 0; j < dataStride; j++)
		printf(" %1.4f ", kMeans_data[0 * dataStride + j]);


	
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&nClusters, "numClusters");
	S.sliders[1].attachToVariable(&nClusters, 0, 100);

	S.addSlider(&nIterations, "nIterations");
	S.sliders[2].attachToVariable(&nIterations, 1, 1000);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{		
		zIntArray outClusters;
		outClusters.assign(numData, int());

		zDoubleArray outMeans;
		outMeans.assign(nClusters * dataStride, double());

		kMeansClustering(&kMeans_data[0], numData, dataStride, nClusters, nIterations, &outClusters[0], &outMeans[0]);

		vector<int> clusterSize;
		clusterSize.assign(nClusters, 0);

		for (int i = 0; i < numData; i++)
		{
			clusterSize[outClusters[i]] += 1;
		}

		printf("\n \n");
		for (int i = 0; i < nClusters; i++)
		{
			printf("\n size : %i  " , clusterSize[i]);

			for (int j = 0; j < dataStride; j++)
				printf(" %1.4f ", outMeans[i * dataStride + j]);
		}

		compute = !compute;	
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

	

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
