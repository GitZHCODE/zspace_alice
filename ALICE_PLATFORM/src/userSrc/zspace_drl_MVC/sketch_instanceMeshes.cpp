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


//enum IntersectResult { PARALLEL, COINCIDENT, NOT_INTERESECTING, INTERESECTING };
//IntersectResult Intersect_segment2d(double* x, double* y, double& u, double& v)
//{
//	double denom = ((y[3] - y[2]) * (x[1] - x[0])) - ((x[3] - x[2]) * (y[1] - y[0]));
//	double nume_a = ((x[3] - x[2]) * (y[0] - y[2])) - ((y[3] - y[2]) * (x[0] - x[2]));
//	double nume_b = ((x[1] - x[0]) * (y[0] - y[2])) - ((y[1] - y[0]) * (x[0] - x[2]));
//
//	if (fabs(denom) < 1e-06)
//	{
//		if (fabs(nume_a) < 1e-06 && fabs(nume_b) < 1e-06)return COINCIDENT;
//
//		u = v = -0.0;
//		return PARALLEL; // lines are parallel
//	}
//	u = nume_a;// ((x[3] - x[2]) * (y[0] - y[2])) - ((y[3] - y[2]) * (x[0] - x[2]));
//	u /= denom;// ((y[3] - y[2]) * (x[1] - x[0])) - ((x[3] - x[2]) * (y[1] - y[0]));
//
//	v = nume_b;// ((x[1] - x[0]) * (y[0] - y[2])) - ((y[1] - y[0]) * (x[0] - x[2]));
//	v /= denom;// ((y[3] - y[2]) * (x[1] - x[0])) - ((x[3] - x[2]) * (y[1] - y[0]));
//
//	if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f)return INTERESECTING;
//
//	return NOT_INTERESECTING;
//}

//scalar field
// vector field = points in space + a vector attached to each of those points
// scalar field = points in space + a scalar attached to each of thise points. Scalar = 0, 0.2,-0.25
//              = sampling a data field at specific points in 3D space;

class parcel
{
public:

	zVector center, direction;
	double scalar;
	double scalar_max;
	zTransform TM;

	zVector parcelPoints[50];

	// primitive mesh arrays
	zObjMesh oMesh;
	zPointArray vertexPositions;

	// combined mesh arrays
	zPointArray positions;
	zIntArray pCounts, pConnects;
	zObjMesh o_combinedMesh;


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
		loadMeshFromDisk();
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

	void loadMeshFromDisk()
	{
		zFnMesh meshFn(oMesh); // 
		meshFn.from("data/cone.obj", zOBJ, true);
		meshFn.getVertexPositions(vertexPositions);
	}

	void transfromMesh(zPoint &pt)
	{
		zTransform _tm;
		_tm = TM;
		_tm.col(3) << pt.x, pt.y, pt.z, 1;

		zFnMesh meshFn(oMesh); 
		zPointArray vp;
		vp = vertexPositions;

		for (auto& pt : vp) pt = pt * _tm;

		meshFn.setVertexPositions(vp);
	}

	void resetMeshVertices()
	{
		zFnMesh meshFn(oMesh);
		meshFn.setVertexPositions(vertexPositions);
	}

	zPointArray tmp_positions;
	zIntArray tmp_pCounts, tmp_pConnects;
	void combineMesh(zObjMesh _om , bool createCombinedMesh = false)
	{
		// arrays to collect and store points, face and edge data from each of the meshes in alignedMesh array
		
		// go through each object in the aligned mesh array. 
		
		{
			//make an instance of a zFnMesh object, attach it to the oMesh stored in the alignedMesh object.
			zFnMesh fnMesh(_om);

			int numV = positions.size();

			// get and store the vertex positions of the mesh using fnMesh;
			tmp_positions.clear();
			fnMesh.getVertexPositions(tmp_positions);

			// add the vertex positions into the array of vertex positions of the combined mesh.
			for (auto& p : tmp_positions) positions.push_back(p);

			// similarly collect and add edge and face data from each alignedMesh object and,
			// add the data into the array of edge and faca data of the combined mesh
			tmp_pCounts.clear(); tmp_pConnects.clear();
			fnMesh.getPolygonData(tmp_pConnects, tmp_pCounts);

			for (auto& pConnect : tmp_pConnects) pConnects.push_back(pConnect + numV);
			for (auto& pCount : tmp_pCounts) pCounts.push_back(pCount);
		}

	
		
	}

	void instance_and_combine_MeshesOnGrid()
	{
		clear_combinedMeshArrays();
		//
		
		for (int i = -50; i < 50; i+=20)
			for (int j = -50; j < 50; j+=20)
			{
				zVector gridPt(i, j, 0);// gridPt;
				gridPt = gridPt * TM; // transform the gridPt;

				if (!(outsidePolygon(parcelPoints, 50, gridPt, 0))) // check if trasnfomred gridPt is outside the parcel polygon
				{
					transfromMesh(gridPt);
					combineMesh(oMesh, false);
					resetMeshVertices();

				}
			}

		//
		createCombinedMesh();
	}

	void createCombinedMesh()
	{
		
		zFnMesh fnMesh(o_combinedMesh);
		fnMesh.create(positions, pCounts, pConnects);

	}

	void clear_combinedMeshArrays()
	{
		positions.clear();
		pCounts.clear(); pConnects.clear();
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

		//glPointSize(5);
		//glColor3f(0.5, 0, 0);
		//for (int i = -50; i < 50; i++)
		//	for (int j = -50; j < 50; j++)
		//	{
		//		zVector gridPt(i, j, 0);// gridPt;
		//		gridPt = gridPt * TM; // transform the gridPt;

		//		if (!(outsidePolygon(parcelPoints, 50, gridPt, 0))) // check if trasnfomred gridPt is outside the parcel polygon
		//			drawPoint(zVecToAliceVec(gridPt)); // if not, drawPoint

		//	}


		//glPointSize(1);

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
parcel parcelArray[25];

zModel model;

zVector p[4];

#define num_cen 50
zVector resiCenters[num_cen];
zVector forces[num_cen];
zVector vel[num_cen];


// when i press the key 'e'
// 1. create and open a file on the hard disk.
// 2. write text into the file , one line at a time.
// 3. close the file.


void setup() // events // eventHandles
{
	// assign values to global variables ;
	double backColor = 0.0;
	backColor = 0.0; // set an initial value for backgroun color

	testParcel.createDefaultShape(32);
	testParcel.transformShape(zVector(0, 0, 0), zVector(1, 1, 0));
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

	p[0] = zVector(0, 0, 0);
	p[1] = zVector(15, 15, 0);
	p[2] = zVector(0, 15, 0);
	p[3] = zVector(15,0, 0);

	///

	for (int i = 0; i < num_cen; i++)
	{
		resiCenters[i] = zVector(Alice::ofRandom(-10, 10), Alice::ofRandom(-10, 10), Alice::ofRandom(-10, 10));

	}


	
}



bool compute = false;

void update(int value) // an event that run 100 times a second, automatically, without any user input
{

	if (compute == true)
	{
		// reset forces
		for (int i = 0; i < num_cen; i++)forces[i] = zVector(0, 0, 0);

		// attract to origin behavior
		for (int i = 0; i < num_cen; i++)
		{
			zVector A = resiCenters[i];
			zVector B = zVector(0,0,0);
			zVector e = B - A;
			e.normalize();
			
			forces[i] = forces[i] + e;

		}
		
		

		

		for (int i = 0; i < num_cen; i++)
		{
			zVector A = resiCenters[i];
			zVector E_total;

			for (int j = 0; j < num_cen; j++)
			{
				if (i == j)continue;

				zVector B = resiCenters[j];
				zVector e = B - A;
				e.normalize();

				
				e *= -1;
				float d = B.distanceTo(A);;
				e /= d*d;

				if (d > 0.0001 /*&& d < 5*/) E_total += e;

				
				//forces[j] -= e;
			}

			forces[i] = forces[i] + E_total;

		}

		//
		// update position and velocity ;
		for (int i = 0; i < num_cen; i++)
		{
			zVector A = forces[i] / 1.0;
			
			//A = deltaV / deltaT ; let say deltaT = 0.1
			float deltaT = 1;// 0.1;
			zVector deltaV = A * deltaT;
			// V = deltaP / deltaT ;
			zVector deltaP = vel[i] * deltaT;

	
			

			resiCenters[i] += deltaP;
			vel[i] = deltaV;
		}
	}



}


void draw() // an event that run 100 times a second, automatically, without any user input
{

	//drawing code here
	backGround(0.8);
	drawGrid(50);

	testParcel.display();

	glPointSize(5);
	glColor3f(1, 0, 0);

		for (int i = 0; i < num_cen; i++)
		{
			drawPoint( zVecToAliceVec(resiCenters[i]) );

		}
	glPointSize(1); // reset pointsize to 1;
	glColor3f(0, 0, 0); // reset color to black;


	/// debug draw 
	for (int i = 0; i < num_cen; i++)
	{
	
		drawLine(zVecToAliceVec(resiCenters[i]), zVecToAliceVec(resiCenters[i] + forces[i]));

	}






	//// display the array of parcels;
	for (int i = 0; i < 25; i++)
	{
		//parcelArray[i].display();

	}


	/*drawLine(zVecToAliceVec(p[0]), zVecToAliceVec(p[1]));
	drawLine(zVecToAliceVec(p[2]), zVecToAliceVec(p[3]));

	double x[4], y[4], u, v;
	for (int i = 0; i < 4; i++)
	{
		x[i] = p[i].x;
		y[i] = p[i].y;


	}*/

	//IntersectResult IR = Intersect_segment2d(x, y, u, v);
	//
	//glPointSize(5);
	//if (IR == INTERESECTING)
	//{
	//	drawPoint(zVecToAliceVec(p[0] + (p[1] - p[0]) * u));

	//	glLineWidth(5);
	//		drawLine(zVecToAliceVec(p[0]), zVecToAliceVec(p[0] + (p[1] - p[0]) * u));
	//		drawLine(zVecToAliceVec(p[2]), zVecToAliceVec(p[2] + (p[3] - p[2]) * v));

	//	glLineWidth(1);
	//}

	//glPointSize(1);
	//model.draw();
}

int np = 0;

void keyPress(unsigned char k, int xm, int ym) // events
{



	compute = !compute;

	if (k == 'm')
	{
		/*zVector A = resiCenters[0];
		zVector B = resiCenters[1];
		zVector e = B - A;
		e.normalize();
		e *= -0.1;

		resiCenters[0] = resiCenters[0] + e;
		resiCenters[1] = resiCenters[1] - e;*/


		for (int i = 0; i < num_cen; i++)
		{
			zVector A = resiCenters[i];
			zVector E_total; 
			
			for (int j = 0; j < num_cen; j++)
			{
				if (i == j)continue;

				zVector B = resiCenters[j];
				zVector e = B - A;
				e.normalize();
				e *= -1 / B.squareDistanceTo(A);

				E_total += e;
			}

			resiCenters[i] = resiCenters[i] + E_total;
			
		}
	}

	if (k == 'e')
	{
		std::fstream myFileStream; // crreate instance of class fstream;

		// use instance of the class, to execute action "open"
		myFileStream.open("data/test.txt", std::fstream::in | std::fstream::out ); // app --> append, i.e add to the file, not replace;

		//extract data from parcels, and write into open text file.
		for (int i = 0; i < 25; i++)
		{
			myFileStream << "data:" << parcelArray[i].scalar << endl;

			zVector parcelDir = parcelArray[i].direction;
			zVector parcelCenter = parcelArray[i].center;

			myFileStream << "direction:" << parcelDir.x << "," << parcelDir.y << "," << parcelDir.z << endl;
			myFileStream << "center:" << parcelCenter.x << "," << parcelCenter.y << "," << parcelCenter.z << endl;

			for (int j = 0; j < 50; j++)
			{
				zVector parcelPt = parcelArray[i].parcelPoints[j];

				myFileStream << parcelPt.x << "," << parcelPt.y << "," << parcelPt.z << endl;
			}

			myFileStream << " ---------------- " << endl;
		}

		// close the file
		myFileStream.close();
	}


	///------------------------
	if (k == '1')
	{
		p[2].x += 0.1;
	}

	if (k == 't')
	{
		testParcel.instance_and_combine_MeshesOnGrid();
		
		model.addObject(testParcel.o_combinedMesh);
		
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