#define _MAIN_
#ifdef _MAIN_

#include "main.h"

//#include "alice/spatialBin.h"
//// zSpace Core Headers
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

using namespace zSpace;

Alice::vec zVecToAliceVec(zVector& in)
{
	return Alice::vec(in.x, in.y, in.z);
}

zVector AliceVecToZvec(Alice::vec& in)
{
	return zVector(in.x, in.y, in.z);
}

int outsidePolygon(zVector* polygon, int N, zVector& p, int bound)
{
	//cross points count of x
	int __count = 0;

	//neighbour bound vertices
	zVector p1, p2;

	//left vertex
	p1 = polygon[0];

	//check all rays
	for (int i = 1; i <= N; ++i)
	{
		//point is an vertex
		if (p == p1) return bound;

		//right vertex
		p2 = polygon[i % N];

		//ray is outside of our interests
		if (p.y < MIN(p1.y, p2.y) || p.y > MAX(p1.y, p2.y))
		{
			//next ray left point
			p1 = p2; continue;
		}

		//ray is crossing over by the algorithm (common part of)
		if (p.y > MIN(p1.y, p2.y) && p.y < MAX(p1.y, p2.y))
		{
			//x is before of ray
			if (p.x <= MAX(p1.x, p2.x))
			{
				//overlies on a horizontal ray
				if (p1.y == p2.y && p.x >= MIN(p1.x, p2.x)) return bound;

				//ray is vertical
				if (p1.x == p2.x)
				{
					//overlies on a ray
					if (p1.x == p.x) return bound;
					//before ray
					else ++__count;
				}

				//cross point on the left side
				else
				{
					//cross point of x
					double xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

					//overlies on a ray
					if (fabs(p.x - xinters) < EPS) return bound;

					//before ray
					if (p.x < xinters) ++__count;
				}
			}
		}
		//special case when ray is crossing through the vertex
		else
		{
			//p crossing over p2
			if (p.y == p2.y && p.x <= p2.x)
			{
				//next vertex
				const zVector& p3 = polygon[(i + 1) % N];

				//p.y lies between p1.y & p3.y
				if (p.y >= MIN(p1.y, p3.y) && p.y <= MAX(p1.y, p3.y))
				{
					++__count;
				}
				else
				{
					__count += 2;
				}
			}
		}

		//next ray left point
		p1 = p2;
	}

	//EVEN
	if (__count % 2 == 0) return(1);
	//ODD
	else return(0);
}


//scalar field
// vector field = points in space + a vector attached to each of those points
// scalar field = points in space + a scalar attached to each of thise points. Scalar = 0, 0.2,-0.25
//              = sampling a data field at specific points in 3D space;

zModel myModel;

class parcel
{
public:

	zVector center, direction;
	double scalar;
	double scalar_max;
	zTransform TM;

	zVector parcelPoints[50];
	
	//
	zObjMesh oMesh;
	zPointArray originalVertexPositions;

	void createDefaultShape( float _r = 6)
	{
		//---
		float inc = TWO_PI / float(50);
		float r = _r;// *sqrt(2);

		for (int i = 0; i < 50; i++)
		{
			float x = r * sin(float(i) * inc);
			float y = r * cos(float(i) * inc);

			parcelPoints[i] = zVector(x, y, 0);

		}

		TM.setIdentity();

		scalar = center.length();
		//
		
	}

	void transformShape(zVector _c, zVector _dir)
	{
		direction = _dir;
		// scale the circle in x-axis
		TM.setIdentity();
		TM(0, 0) *= 1.5;

		for (int i = 0; i < 50; i++)
		{
			parcelPoints[i] = parcelPoints[i] * TM;
		}

		// move the scaled ellipse to a new center; 	
		zVector u, v, w;
		u = _dir;
		v = u; swap(v.x, v.y); v.x *= -1;
		w = zVector(0, 0, 1);
		center = _c;

		u.normalize(); v.normalize(); w.normalize();

		TM.col(0) << u.x, u.y, u.z, 1.0;
		TM.col(1) << v.x, v.y, v.z, 1.0;
		TM.col(2) << w.x, w.y, w.z, 1.0;
		TM.col(3) << center.x, center.y, center.z, 1.0;

		for (int i = 0; i < 50; i++)
		{
			parcelPoints[i] = parcelPoints[i] * TM;
		}

		scalar =  center.length();
		scalar_max = 60;
	}

	///----------------------------------------

	void loadMeshFromDisk( string file)
	{
		
		zFnMesh fnMesh(oMesh);
		fnMesh.from( file, zOBJ);

		
		fnMesh.getVertexPositions(originalVertexPositions);
		myModel.addObject(oMesh);
	}

	void transformMesh( zVector _c )
	{
		zTransform transMatrix;
		transMatrix = TM;// testParcel.TM;


		zVector center = _c;

		if (!outsidePolygon( parcelPoints, 50, center, 0))
		{
			transMatrix.col(3) << center.x, center.y, center.z, 1;

		}

		// get vertexpositions and multiply with TM and set vertex positions;
		zPointArray vp;
		vp = originalVertexPositions;

		zFnMesh fnMesh(oMesh);
		//Mesh.getVertexPositions(vp);

		// multiply each vertex position with the transformation matrix
		for (int i = 0; i < vp.size(); i++)
		{
			vp[i] = vp[i] * transMatrix;
		}

		// set vertex positions to the transfromed points
		fnMesh.setVertexPositions(vp);

		// step 3 : display the transfomred mesh;

		//myModel.addObject(oMesh);

	}

	double getAreaOfParcel()
	{
		double sumOfAreas = 0;
		for (int i = 0; i < 50; i++)
		{
			zVector curPt = parcelPoints[i];
			zVector nextPt = parcelPoints[(i + 1) % 50];

			zVector a = curPt - center;
			zVector b = nextPt - curPt;

			double areaOfTriangle = (a ^ b).length();
			sumOfAreas += areaOfTriangle;
		}

		return sumOfAreas;
	}

	void display()
	{
		// draw the parcel
		glColor3f(0, 0, 0);
		for (int i = 0; i < 50; i++)
		{

			zVector curPt = parcelPoints[i];
			zVector nextPt = parcelPoints[(i + 1) % 50]; //parcelPoints[i + 1];modulo = remainder

			drawLine(zVecToAliceVec(curPt), zVecToAliceVec(nextPt));
		}


		//transform and draw the grid of points, only if the trasnformed point lies within the parcel polygon 

		glPointSize(5);
		glColor3f(0.5, 0, 0);
		for (int i = -50; i < 50; i++)
			for (int j = -50; j < 50; j++)
			{
				zVector gridPt(i, j, 0);// gridPt;
				gridPt = gridPt * TM; // transform the gridPt;

				if (!(outsidePolygon(parcelPoints, 50, gridPt, 0))) // check if trasnfomred gridPt is outside the parcel polygon
					drawPoint(zVecToAliceVec(gridPt)); // if not, drawPoint

			}


		glPointSize(1);

		// draw a line that is representative of the value of the scalar;

		glLineWidth(5);

		double redChannel = ofMap(scalar, 0, scalar_max, 0, 1);

		glColor3f(redChannel,0,0);
		
			double remapped_scalar = ofMap(scalar, 0, scalar_max, 1, 20);

			drawLine( zVecToAliceVec(center), zVecToAliceVec(center + zVector(0, 0, remapped_scalar)) );
		//	glColor3f(0, 0, 1); drawLine(zVecToAliceVec(center), zVecToAliceVec(center + zVector(0, 0, scalar)));
		
		glLineWidth(1);
		
		//reset color to white
		glColor3f(1,1, 1);

		//

		glColor3f(0, 0, 0);
		glLineWidth(5);

			drawCircle(zVecToAliceVec(center), ofMap(scalar, 0, scalar_max, 1, 5), 32);

		glLineWidth(1);

	}
};



parcel testParcel;
zObjMesh oMesh;
zObjMesh o_Meshes[5];

parcel parcelArray[25];



void setup() // events // eventHandles
{
	// assign values to global variables ;
	double backColor = 0.0;
	backColor = 0.0; // set an initial value for backgroun color

	testParcel.createDefaultShape(32);
	testParcel.transformShape( zVector(0, 0, 0), zVector(1, 2, 0) );
	//testParcel.transformShape( zVector(5,5,0) ) ; 

	// create default shape and transform each of the parcels in parcelArray
	int cnt = 0;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			parcelArray[cnt].createDefaultShape();
			parcelArray[cnt].transformShape(zVector(i * 20, j * 20, 0), zVector(i, j, 0));

			cnt++;
		}
}



bool compute = false;

void update(int value) // an event that run 100 times a second, automatically, without any user input
{

	if (compute == true)
	{

	}



}


void draw() // an event that run 100 times a second, automatically, without any user input
{

	//drawing code here
	backGround(0.8);
	drawGrid(50);

	//testParcel.display();

	//// display the array of parcels;
	for (int i = 0; i < 25; i++)
	{
		parcelArray[i].display();

	}

	///

	myModel.draw();


}

int np = 0;

// PSUEDO algo

// when i press the letter 't', 
// 1. import a mesh froom the hard disk
// 2. transform the mesh from origin to one of the grid points inside the 'testParcel'
// 3. dsiplay the mesh i.e add the mesh to the model

//

int mesh_counter = 0;

void keyPress(unsigned char k, int xm, int ym) // events
{



	compute = !compute;

	if (k == '2')
	{
		//testParcel.loadMeshFromDisk();
		//testParcel.transformMesh();

		for (int i = 0; i < 25; i++)
		{
			if (i < 3 )
			{
				parcelArray[i].loadMeshFromDisk("data/merged_tube.obj");
			}
			else
			{
				parcelArray[i].loadMeshFromDisk("data/cube.obj");
			}

		}

	}

	if (k == '3')
	{
		//testParcel.transformMesh( zVector( mesh_counter * 5 ,0,0) );

		for (int i = 0; i < 25; i++)
		{
			parcelArray[i].transformMesh( parcelArray[i].center + zVector(mesh_counter * 0.1, 0, 0) );

		}

		mesh_counter += 1;
	}
	
	///
	if (k == '1')
	{
		zFnMesh fnMesh(oMesh);
		fnMesh.from("data/cube.obj", zOBJ);

		//myModel.addObject(oMesh);
	}

	if (k == 't')
	{

		
		// step 1 import the mesh
		//zObjMesh oMesh;
	/*	zFnMesh fnMesh(oMesh);
		fnMesh.from("data/cube.obj", zOBJ);*/
		
		o_Meshes[mesh_counter] = oMesh;
		zFnMesh fnMesh(o_Meshes[mesh_counter]);
		
		// step 2 : transform the mesh...
		
		// get or construct a transformation matrix;
		zTransform transMatrix;
		transMatrix = testParcel.TM;

	
		zVector center(mesh_counter * 5.0f, 0, 0);

		if (  !outsidePolygon( testParcel.parcelPoints, 50, center, 0) )
		{
			transMatrix.col(3) << center.x, center.y, center.z, 1;
			cout << "line 395" << endl;
			cout << center.x << endl;
		}
		

		// get mesh vertex positions
		zPointArray vp;
		fnMesh.getVertexPositions(vp);

		// multiply each vertex position with the transformation matrix
		for (int i = 0; i < vp.size(); i++)
		{
			vp[i] = vp[i] * transMatrix;
		}

		// set vertex positions to the transfromed points
		fnMesh.setVertexPositions(vp);

		// step 3 : display the transfomred mesh;

		myModel.addObject(o_Meshes[mesh_counter]);

		//
		mesh_counter = mesh_counter + 1;
		if (mesh_counter > 4)
		{
			mesh_counter = 0;
		}
	}
	

	//when i press key 'a', change all the sscalars to the area of the parcel
	if (k == 'a')
	{
		//how to find the max vlaue in a series of numbers;
		double max = -1 * 1000000;
		for (int i = 0; i < 25; i++)
		{
			// left hand side = right hand side
			// LHS can store the result of RHS;
			parcelArray[i].scalar = parcelArray[i].getAreaOfParcel(); // 0.25,2.25 

			if (parcelArray[i].scalar > max)
			{
				max = parcelArray[i].scalar;// update max value to the newly found
			}
		}
		// update the scalar_max value of all hte parcels with the newly calculated max scalar i.e maximum area of parcel;

		for (int i = 0; i < 25; i++)
		{
			// left hand side = right hand side
			// LHS can store the result of RHS;
			parcelArray[i].scalar_max = max;
		}


	}

	if (k == 'b')
	{
		//how to find the max vlaue in a series of numbers;
		double max = -1 * 1000000;
		for (int i = 0; i < 25; i++)
		{
			// left hand side = right hand side
			// LHS can store the result of RHS;
			parcelArray[i].scalar = parcelArray[i].direction.angle(zVector(1, 0, 0));

			cout << parcelArray[i].scalar << endl;

			if (parcelArray[i].scalar > max)
			{
				max = parcelArray[i].scalar;// update max value to the newly found
			}
		}
		// update the scalar_max value of all hte parcels with the newly calculated max scalar i.e maximum area of parcel;

		for (int i = 0; i < 25; i++)
		{
			// left hand side = right hand side
			// LHS can store the result of RHS;
			parcelArray[i].scalar_max = max;
		}


	}


}

void mousePress(int b, int state, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}


//void combineMeshes(zObjMeshArray& _oMeshes, zObjMesh& _oMesh)
//{
//	
//	for (auto& m : _oMeshes)
//	{
//		zFnMesh fnMesh(m);
//		int numV = positions.size();
//		zPointArray tmp_positions;
//		fnMesh.getVertexPositions(tmp_positions);
//		for (auto& p : tmp_positions) positions.push_back(p);
//		zIntArray tmp_pCounts, tmp_pConnects;
//		fnMesh.getPolygonData(tmp_pConnects, tmp_pCounts);
//
//		for (auto& pConnect : tmp_pConnects) pConnects.push_back(pConnect + numV);
//		for (auto& pCount : tmp_pCounts) pCounts.push_back(pCount);
//	}
//	
//}
#endif // _MAIN_