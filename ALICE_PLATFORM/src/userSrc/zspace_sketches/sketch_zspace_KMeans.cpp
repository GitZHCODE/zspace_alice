//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zToolsets/data/zTsKMeans.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// Methods
zUtilsCore core;

bool readData(string path, MatrixXf &outData)
{
	ifstream myfile;
	myfile.open(path.c_str());

	vector<zDoubleArray> dPoints;

	if (myfile.fail())
	{
		cout << " error in opening file  " << path.c_str() << endl;
		return false;
	}

	int counter = 0;
	int stride;

	dPoints.clear();

	int linedata = 0;
	while (!myfile.eof())
	{
		string str;
		getline(myfile, str);

		zStringArray data = core.splitString(str, ",");

		if (data.size() > 0)
		{
			zDoubleArray tempData;
			for (int i = 0; i < data.size(); i++)
			{
				tempData.push_back(atof(data[i].c_str()));


				if (dPoints.size() == 0) stride++;
			}
			dPoints.push_back(tempData);
			counter++;
		}
		

		linedata++;

	}

	myfile.close();

	outData = MatrixXf(counter, stride);
	for (int i = 0; i < counter; i++)
	{
		for (int j = 0; j < stride; j++)
		{
			outData(i, j) = dPoints[i][j];
		}
	}
	
	printf("\n data read successful! ");
	printf("\n dataPoints %i stride %i ", counter, stride);

	return true;
}


////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/


zTsKMeans myKMeans;

MatrixXf dataPoints;
vector<zVector> kMeanCentroids;

double numClusters = 50;
double numIterations = 200;


vector<double> vWeights;



bool kMeans = false;



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

	// read mesh
	
	
	/*zPoint p0(1, 0, 0);
	zPoint p1(0, 1, 0);

	zDomain<zPoint> pointDomain(p0, p1);
	zDomain<float> weight(0, 1);

	zPoint p = core.ofMap(0.5, weight, pointDomain);

	cout << p;*/
	

	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&numClusters, "numClusters");
	S.sliders[1].attachToVariable(&numClusters, 1, 200);

	S.addSlider(&numIterations, "numIterations");
	S.sliders[2].attachToVariable(&numIterations, 1, 500);

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
		readData("data/radius.csv", dataPoints);

		int nC = (int)numClusters;
		int nI = (int)numIterations;
		myKMeans = zTsKMeans(dataPoints, nC, nI);

		int actualClusters;
		int out = myKMeans.getKMeansClusters(actualClusters, zTsKMeans::initialisationMethod::kmeansPlusPlus,1,2);

		printf("\n kMeans : %i | %i %i ", out, nC, actualClusters);

		cout << "\n centroids \n" << myKMeans.means;

		printf("\n  clusterIDs %i", myKMeans.clusterIDS.size());

		printf("\n \n data, ClusterID, deviations ");
		for (int i = 0; i < myKMeans.clusterIDS.size(); i++)
		{
			printf("\n %1.2f, %i, %1.2f ", myKMeans.dataPoints(i,0), myKMeans.clusterIDS[i], abs(myKMeans.dataPoints(i, 0) - myKMeans.means(myKMeans.clusterIDS[i],0)));
		}

		
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
		// zspace model draw
		model.draw();
		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
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
