#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

//#include <headers/zCore/zExtMesh.h>
//#include <headers/zCore/zExtPoint.h>

using namespace zSpace;
using namespace std;

class Square
{
public:
	zPoint center;
	zVector dimension;
	float weight;

	Square() : center(zPoint(0, 0, 0)), dimension(zVector(1, 1, 1)), weight(1.0f) {}

	Square(zPoint _center, zVector _dim, float _w)
	{
		center = _center;
		dimension = _dim;
		weight = _w;
	}
};

class Network
{
public:
	vector<Square> squares;
	zObjMeshScalarField o_field;
	zObjGraphArray o_contours;
	zPointArray sqrPositions;
	zVectorArray sqrDims;
	zFloatArray sqrWeights;

	Network() : squares(), o_field(), o_contours(), sqrPositions(), sqrDims(), sqrWeights() {}

	void makeSquares()
	{
		squares.assign(sqrPositions.size(), Square());

		for (int i = 0; i < sqrPositions.size(); i++)
			squares[i] = Square(sqrPositions[i], sqrDims[i], sqrWeights[i]);
	}

	void computeSquareUnion(float threshold, bool b_sqaure)
	{
		zScalarArray scalar;
		zScalarArray scalar_temp;
		zFnMeshScalarField fnField(o_field);

		scalar.assign(fnField.numFieldValues(), 1.0f);

		if (b_sqaure)
			for (int i = 0; i < sqrPositions.size(); i++)
			{
				fnField.getScalars_Square(scalar_temp, sqrPositions[i], sqrDims[i], true);

				for (auto& v : scalar_temp)
					v *= sqrWeights[i];

				zScalarArray scalar_boolean;
				fnField.boolean_union(scalar, scalar_temp, scalar_boolean);
				scalar = scalar_boolean;
			}
		else
			fnField.getScalarsAsVertexDistance(scalar, sqrPositions, true);

		// apply scalars to field
		fnField.setFieldValues(scalar, zFieldColorType::zFieldSDF, threshold);
	}

	void computeMedialAxis(float threshold, bool b_square)
	{
		// Compute scalars for squares
		zFnMeshScalarField fnField(o_field);
		std::vector<zScalarArray> scalar_squares(sqrPositions.size());

		for (int i = 0; i < sqrPositions.size(); i++) {
			scalar_squares[i] = zScalarArray(fnField.numFieldValues(), std::numeric_limits<float>::max()); // Initialize the scalar array

			if (b_square) {
				fnField.getScalars_Square(scalar_squares[i], sqrPositions[i], sqrDims[i], true);
				for (auto& v : scalar_squares[i]) {
					v = pow(v, 1.0f / sqrWeights[i]);
				}
			}
			else {
				zPointArray temp;
				temp.push_back(sqrPositions[i]);
				fnField.getScalarsAsVertexDistance(scalar_squares[i], temp, 1 - sqrWeights[i], true);
			}
		}

		// Initialize arrays to find the minimum distance to any square and the second minimum distance
		zScalarArray min_distances(fnField.numFieldValues(), std::numeric_limits<float>::max());
		zScalarArray second_min_distances(fnField.numFieldValues(), std::numeric_limits<float>::max());

		// Compute the minimum and second minimum distances at each field point
		for (int i = 0; i < sqrPositions.size(); i++) {
			for (int k = 0; k < fnField.numFieldValues(); k++) {
				float dist = scalar_squares[i][k];
				if (dist < min_distances[k]) {
					second_min_distances[k] = min_distances[k];
					min_distances[k] = dist;
				}
				else if (dist < second_min_distances[k]) {
					second_min_distances[k] = dist;
				}
			}
		}

		// Find points where the difference between the minimum and second minimum distances is small
		zScalarArray medial_axis(fnField.numFieldValues(), 0.0f);
		for (int i = 0; i < fnField.numFieldValues(); i++) {
			if (abs(min_distances[i] - second_min_distances[i]) < threshold) {
				medial_axis[i] = 1.0f; // Mark this as part of the medial axis
			}
		}

		// Apply medial axis to field
		fnField.setFieldValues(medial_axis, zFieldColorType::zFieldSDF, 0.1f);
	}

	void computeContours()
	{
		zFnMeshScalarField fnField(o_field);
		int size = o_contours.size();
		cout << "o_contours.size()"<<o_contours.size() << endl;

		// contours
		for (int i = 0; i < size; i++)
		{
			float val = (float)i / (float)size;

			cout << "val:" << val << endl;

			fnField.getIsocontour(o_contours[i], val);

			zFnGraph fnGraph(o_contours[i]);
			fnGraph.setEdgeColor(zWHITE);
			printf("\n v %i , e %i ", fnGraph.numVertices(), fnGraph.numEdges());
		}
	}

};

////////////////////////////////////////////////////////////////////////// General

bool compute_union = false;
bool compute_medial = false;
bool compute_contour = false;
bool display = true;
bool d_vals = false;
bool b_square = true;
bool b_move = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

int numContours = 20;
double transitionValue = 0.01f;

float squareX = 2.0f;
float squareY = 1.0f;

Network network;


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


	// setup field
	int numX;
	int numY;
	numX =numY = 200;
	zFnMeshScalarField fnField(network.o_field);
	fnField.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), numX, numY, 1, true, false);

	zDomainColor colDomain(zRED, zGREEN);
	fnField.setFieldColorDomain(colDomain);

	//setup contours
	network.o_contours.assign(numContours, zObjGraph());

	//setup squares
	network.sqrPositions = { zPoint(-5,-4,0), zPoint(5,-4,0),zPoint(-2,5,0) };
	network.sqrDims = { zVector(2,3,0),zVector(3,3,0),zVector(5,2,0) };
	network.sqrWeights = { 1.0,1.0,1.0 };

	//network.sqrPositions = { zPoint(-5,0,0), zPoint(5,0,0) };
	//network.sqrDims = { zVector(1,1,0),zVector(1,1,0) };
	//network.sqrWeights = { 1.0,1.0 };

	network.makeSquares();

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(network.o_field);

	for (auto& g : network.o_contours)
	{
		model.addObject(g);
		g.setDisplayElements(false, true);
	}

	// set display element booleans
	network.o_field.setDisplayElements(false, true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);
	S.addSlider(&transitionValue, "transitionValue");
	S.sliders[1].attachToVariable(&transitionValue, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute_union, "compute_union");
	B.buttons[0].attachToVariable(&compute_union);
	B.addButton(&compute_medial, "compute_medial");
	B.buttons[1].attachToVariable(&compute_medial);
	B.addButton(&compute_contour, "compute_contour");
	B.buttons[2].attachToVariable(&compute_contour);
	B.addButton(&display, "display");
	B.buttons[3].attachToVariable(&display);
	B.addButton(&b_square, "b_square");
	B.buttons[4].attachToVariable(&b_square);
	B.addButton(&b_move, "b_move");
	B.buttons[5].attachToVariable(&b_move);

}

void update(int value)
{
	if (compute_union)
	{
		// compute scalars
		network.computeSquareUnion(0.1f, b_square);

		if (b_move)
		{
			zFnMeshScalarField fnField(network.o_field);
			zObjMesh* m = fnField.getRawMesh();
			zFloatArray values;
			fnField.getFieldValues(values);
			for (int i = 0; i < fnField.numFieldValues(); i++)
			{
				zItMeshVertex v(*m, i);
				zVector* vec = v.getRawPosition();
				vec->z = values[i] * 5;
			}
		}

		compute_union = !compute_union;
	}
	if (compute_medial)
	{
		network.computeMedialAxis(transitionValue, b_square);

		compute_medial = !compute_medial;
	}
	if (compute_contour)
	{
		network.computeContours();

		compute_contour = !compute_contour;
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
		// zspace model draw
		network.o_field.setDisplayObject(true);
	}
	if (!display)
	{
		network.o_field.setDisplayObject(false);
	}
	if (d_vals)
	{
		zFnMeshScalarField fnField(network.o_field);
		float* val = fnField.getRawFieldValues();

		for (int i = 0; i < fnField.numFieldValues(); i++)
		{
			zItMeshVertex v(*fnField.getRawMesh(),i);
			model.displayUtils.drawTextAtPoint(to_string(val[i]), v.getPosition());
		}
	}



	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	//if (k == 'p') compute = true;
	if (k == 'v') d_vals = !d_vals;
	if (k == 's') b_square = !b_square;
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
