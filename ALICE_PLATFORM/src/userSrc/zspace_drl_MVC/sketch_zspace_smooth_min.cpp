//#define _MAIN_

#ifdef _MAIN_

#include "main.h"
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

//#include <headers/zCore/zExtMesh.h>
//#include <headers/zCore/zExtPoint.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute_union = false;
bool compute_union_mul = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

vector<zObjMeshScalarField> o_fields;
zObjMeshScalarField result;
vector<zObjGraph> graphs;

int numX;
int numY;

double val_k = 0.0;

// SMIN Helper Functions
//-------------------------------------------------------------

// Exponential
inline float smin_exponential(float a, float b, float k)
{
	// from the article: k *= 1.0; (implicitly the same)
	float r = exp2(-a / k) + exp2(-b / k);
	return -k * log2(r);
}

// Root
inline float smin_root(float a, float b, float k)
{
	k *= 2.0f;
	float x = b - a;
	return 0.5f * (a + b - sqrtf(x * x + k * k));
}

// Sigmoid
inline float smin_sigmoid(float a, float b, float k)
{
	k *= logf(2.0f);
	float x = b - a;
	return a + x / (1.0f - exp2(x / k));
}

// Quadratic Polynomial
inline float smin_polyQuadratic(float a, float b, float k)
{
	k *= 4.0f;
	float h = max(k - fabs(a - b), 0.0f) / k;
	return std::min(a, b) - h * h * k * 0.25f; // (1.0/4.0)
}

// Cubic Polynomial
inline float smin_polyCubic(float a, float b, float k)
{
	k *= 6.0f;
	float h = max(k - fabs(a - b), 0.0f) / k;
	return std::min(a, b) - h * h * h * k * (1.0f / 6.0f);
}

// Quartic Polynomial
inline float smin_polyQuartic(float a, float b, float k)
{
	k *= (16.0f / 3.0f);
	float h = max(k - fabs(a - b), 0.0f) / k;
	return std::min(a, b) - h * h * h * (4.0f - h) * k * (1.0f / 16.0f);
}

// Circular
inline float smin_circular(float a, float b, float k)
{
	k *= 1.0f / (1.0f - sqrtf(0.5f));
	float h = max(k - fabs(a - b), 0.0f) / k;
	return std::min(a, b)
		- k * 0.5f * (1.0f + h - sqrtf(1.0f - h * (h - 2.0f)));
}

// Circular Geometrical
inline float smin_circularGeometrical(float a, float b, float k)
{
	k *= 1.0f / (1.0f - sqrtf(0.5f));
	float mAB = std::min(a, b);
	float dx = max(k - a, 0.0f);
	float dy = max(k - b, 0.0f);
	float l = sqrtf(dx * dx + dy * dy);

	return max(k, mAB) - l;
}

zColorArray cols =
{
	// min (white)
	zColor(1.0f, 1.0f, 1.0f, 1.0f),

	// exponential (yellow)
	zColor(1.0f, 1.0f, 0.0f, 1.0f),

	// root (magenta)
	zColor(1.0f, 0.0f, 1.0f, 1.0f),

	// sigmoid (red)
	zColor(1.0f, 0.0f, 0.0f, 1.0f),

	// polynomial_quadratic (cyan)
	zColor(0.0f, 1.0f, 1.0f, 1.0f),

	// polynomial_cubic (blue)
	zColor(0.0f, 0.0f, 1.0f, 1.0f),

	// polynomial_quartic (orange)
	zColor(1.0f, 0.5f, 0.0f, 1.0f),

	// circular (purple)
	zColor(0.5f, 0.0f, 0.5f, 1.0f),

	// circular_geometrical (green)
	zColor(0.0f, 1.0f, 0.0f, 1.0f)
};

zStringArray modeInfo =
{
	"min (white)",
	"exponential (yellow)",
	"root (magenta)",
	"sigmoid (red)",
	"polynomial_quadratic (cyan)",
	"polynomial_cubic (blue)",
	"polynomial_quartic (orange)",
	"circular (purple)",
	"circular_geometrical (green)"
};

enum MODE { min, exponential, root, sigmoid, polynomial_quadratic, polynomial_cubic, polynomial_quartic, circular, circular_geometrical };

void boolean_union(
	const zScalarArray& a,
	const zScalarArray& b,
	zScalarArray& result,
	float k,
	MODE mode = MODE::min
)
{
	for (size_t i = 0; i < result.size(); i++)
	{
		float ai = a[i];
		float bi = b[i];

		switch (mode)
		{
		case MODE::min:
			// hard minimum
			result[i] = (ai < bi) ? ai : bi;
			break;

		case MODE::exponential:
			result[i] = smin_exponential(ai, bi, k);
			break;

		case MODE::root:
			result[i] = smin_root(ai, bi, k);
			break;

		case MODE::sigmoid:
			result[i] = smin_sigmoid(ai, bi, k);
			break;

		case MODE::polynomial_quadratic:
			result[i] = smin_polyQuadratic(ai, bi, k);
			break;

		case MODE::polynomial_cubic:
			result[i] = smin_polyCubic(ai, bi, k);
			break;

		case MODE::polynomial_quartic:
			result[i] = smin_polyQuartic(ai, bi, k);
			break;

		case MODE::circular:
			result[i] = smin_circular(ai, bi, k);
			break;

		case MODE::circular_geometrical:
			result[i] = smin_circularGeometrical(ai, bi, k);
			break;

		default:
			result[i] = (ai < bi) ? ai : bi;
			break;
		}
	}
}

void boolean_union_multiple(
	const std::vector<zScalarArray>& inputs,
	zScalarArray& result,
	float k,
	MODE mode = MODE::min
)
{
	result = inputs[0];

	for (size_t i = 1; i < inputs.size(); i++)
	{
		boolean_union(result, inputs[i], result, k, mode);
	}
}

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

	o_fields.assign(3, zObjMeshScalarField());
	graphs.assign(9, zObjGraph());

	numX =numY = 40;

	zFnMeshScalarField fnField(result);
	fnField.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), numX, numY, 1, true, false);
	result.setDisplayElements(true, false, false);
	model.addObject(result);

	for (int i = 0; i < graphs.size(); i++)
	{
		graphs[i].setDisplayElements(false, true);
		model.addObject(graphs[i]);
	}


	for (auto& field : o_fields)
	{
		zFnMeshScalarField fnField(field);
		fnField.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), numX, numY, 1, true, false);
		zDomainColor colDomain(zRED, zGREEN);
		fnField.setFieldColorDomain(colDomain);
	}


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object

	// set display element booleans

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&val_k, "k");
	S.sliders[1].attachToVariable(&val_k, -1, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_union, "compute_union");
	B.buttons[0].attachToVariable(&compute_union);
	B.addButton(&compute_union_mul, "compute_union_mul");
	B.buttons[1].attachToVariable(&compute_union_mul);
	B.addButton(&display, "display");
	B.buttons[2].attachToVariable(&display);


}

void update(int value)
{
	if (compute_union)
	{
		zScalarArray scalars_a, scalars_b, scalars_result;
		scalars_a.reserve(numX * numY);
		scalars_b.reserve(numX * numY);
		scalars_result.assign(numX * numY, -1);

		zFnMeshScalarField fnField_a(o_fields[0]);
		fnField_a.getScalars_Circle(scalars_a, zVector(-4, -4, 0), 2);

		zFnMeshScalarField fnField_b(o_fields[1]);
		fnField_b.getScalars_Square(scalars_b, zVector(2, 2, 0), zVector(2, 2, 0));

		zFnMeshScalarField fnField_result(result);
		// Original zspace min method
		//fnField_result.boolean_union(scalars_a, scalars_b, scalars_result);
		

		for (int i = 0; i < graphs.size(); i++)
		{
			boolean_union(scalars_a, scalars_b, scalars_result, val_k, static_cast<MODE>(i));

			fnField_result.normliseValues(scalars_result);
			fnField_result.setFieldValues(scalars_result, zFieldColorType::zFieldSDF, 0.01f);
			fnField_result.getIsocontour(graphs[i], 0.01f);

			//for (auto& v : scalars_result) cout << v << "\n";

			zFnGraph fnGraph(graphs[i]);
			fnGraph.setEdgeColor(cols[i], true);
		}

		compute_union = !compute_union;
	}

	if (compute_union_mul)
	{
		vector<zScalarArray> inputs;
		zScalarArray scalars_result;
		inputs.assign(3, zScalarArray());
		scalars_result.assign(numX * numY, -1);

		zFnMeshScalarField fnField_a(o_fields[0]);
		fnField_a.getScalars_Circle(inputs[0], zVector(-4, -4, 0), 2);

		zFnMeshScalarField fnField_b(o_fields[1]);
		fnField_b.getScalars_Square(inputs[1], zVector(2, 2, 0), zVector(2, 2, 0));

		zFnMeshScalarField fnField_c(o_fields[2]);
		fnField_c.getScalars_Circle(inputs[2], zVector(5, -3, 0), 2);

		zFnMeshScalarField fnField_result(result);

		for (int i = 0; i < graphs.size(); i++)
		{
			boolean_union_multiple(inputs, scalars_result, val_k, static_cast<MODE>(i));

			fnField_result.normliseValues(scalars_result);
			fnField_result.setFieldValues(scalars_result, zFieldColorType::zFieldSDF, 0.01f);
			fnField_result.getIsocontour(graphs[i], 0.01f);

			zFnGraph fnGraph(graphs[i]);
			fnGraph.setEdgeColor(cols[i], true);
		}

		compute_union_mul = !compute_union_mul;
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	model.draw();

	if (display)
	{
	
	}


	//////////////////////////////////////////////////////////

	setup2d();

	for (size_t i = 0; i < modeInfo.size(); i++)
	{
		glColor3f(cols[i].r, cols[i].g, cols[i].b);
		drawString(modeInfo[i], Alice::vec(winW - 350, winH - 750 + i * 20, 0));
	}

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	//if (k == 'p') compute = true;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
