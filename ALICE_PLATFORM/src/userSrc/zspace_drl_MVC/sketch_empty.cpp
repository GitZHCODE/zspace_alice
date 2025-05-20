#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

//#include <headers/include/zCore.h>
//#include <headers/include/zGeometry.h>
//#include <headers/include/zDisplay.h>
//#include <headers/include/zData.h>
//#include <headers/include/zIO.h> 
//
using namespace zSpace;

////////////////////////////////////////////////////////////////////////// Untilities 

Alice::vec zVecToAliceVec(zVector& in)
{
	return Alice::vec(in.x, in.y, in.z);
}

zVector AliceVecToZvec(Alice::vec& in)
{
	return zVector(in.x, in.y, in.z);
}

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zObjMesh oMesh;
zObjGraph oGraph;

class VectorField2D {
public:
	static const int RES = 25;
	zVector grid[RES][RES];
	zVector field[RES][RES];
	float spacing = 0.5;

	void setupField(float halfSize = 3.0f) {
		for (int i = 0; i < RES; ++i) {
			for (int j = 0; j < RES; ++j) {
				float x = ofMap(i, 0, RES - 1, -halfSize, halfSize);
				float y = ofMap(j, 0, RES - 1, -halfSize, halfSize);
				grid[i][j] = zVector(x, y, 0);

				// Define the vector field: f = {-1 - x^2 + y, 1 + x - y^2}
				float fx = -1.0 - x * x + y;
				float fy = 1.0 + x - y * y;
				field[i][j] = zVector(fx, fy, 0);
			}
		}
	}

	void drawField() {
		glColor3f(0.2, 0.2, 0.8);
		for (int i = 0; i < RES; ++i) {
			for (int j = 0; j < RES; ++j) {
				zVector p = grid[i][j];
				zVector v = field[i][j];
				v.normalize();
				drawLine(zVecToAliceVec(p), zVecToAliceVec(p + v * spacing));
			}
		}
	}
};

VectorField2D myField;



void setup()
{
	model = zModel(100000);

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

	////////////////////////////////////////////////////////////////////////// Intialize the variables

	myField.setupField(); // Initialize the vector field


}

void update(int value)
{
	if (compute)
	{

		
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
		model.draw();
		myField.drawField(); // Draw vector arrows
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
