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


#define num 150
zVector points[num];
zVector forces[num];


void setup() // events // eventHandles
{
	// assign values to global variables ;
	

	for (int i = 0; i < num; i++)
	{
		points[i] = zVector(ofRandom(-2, 2), ofRandom(-2, 2), 0);

	}
	
}



bool compute = false;

void update(int value) // an event that run 100 times a second, automatically, without any user input
{

	if (compute == true)
	{
		// reset forces
		for (int i = 0; i < num; i++)forces[i] = zVector(0, 0, 0);

		//calculate & store repulsive force per point
		for (int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; j++)
			{
				if (i == j) continue;

				zVector e = points[j] - points[i];
				float d = points[j].distanceTo(points[i]);
				
				if (d > 1e-2 )
				{
					e.normalize();
					e /= d * d;
					forces[i] -= e;
				}
				
			}
		}

		// calculate the maximum and minimum magnitude of reuplisve force
		double force_max, force_min;
		force_min = 1e6; force_max = -force_min;

		for (int i = 0; i < num; i++)
		{
			float d = forces[i].length();
			force_max = MAX(force_max, d);
			force_min = MIN(force_min, d);
		}

		// re-scale all forces to be within 0 & 1
		for (int i = 0; i < num; i++)
		{
			float d = forces[i].length();
			forces[i].normalize();
			forces[i] *= ofMap(d, force_min, force_max, 0,1);

		}

		// move each of the points, by applying their respective forces, if the magnitude of force is less than 1 and the point is whtin a radius of 10 from the origin;
		for (int i = 0; i < num; i++)
			if (forces[i].length() < 1 && points[i].length() < 10)
				points[i] += forces[i];

		
	}



}


void draw() // an event that run 100 times a second, automatically, without any user input
{

	//drawing code here
	backGround(0.8);
	drawGrid(50);


	glPointSize(5);

		for (int i = 0; i < num; i++)
		{
			drawPoint( zVecToAliceVec(points[i]) );

			drawLine(zVecToAliceVec(points[i]), zVecToAliceVec(points[i] + forces[i]));
		}

	glPointSize(1);


}

int np = 0;

void keyPress(unsigned char k, int xm, int ym) // events
{



	compute = !compute;

	if (k == 'm')
	{
		compute = true;

		update(100);
		compute = false;
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