#define _MAIN_



#ifdef _MAIN_


#include "main.h"

#include <zSpace/zSpaceImplement.h>
#include <zMeshUtilities.h>
#include <zSpace/zDisplay.h>

#include <random>
#include <algorithm>

#include <../src_cuda/kernel_test.cuh>

using namespace zSpace;
using namespace CUDA;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
double background = 0.75;


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// Initialize objects

	const int arraySize = 5;
	const int a[arraySize] = { 1, 2, 3, 4, 5 };
	const int b[arraySize] = { 10, 20, 30, 40, 50 };
	int c[arraySize] = { 0 };

	parallelSum(c, a, b, arraySize);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);


	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);

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

	S.draw();
	B.draw();

	if (compute)
	{
		
	}

	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("kr:" + to_string(kr), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'c') compute = !compute;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
