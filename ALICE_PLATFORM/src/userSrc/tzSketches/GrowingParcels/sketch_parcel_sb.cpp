//#define _MAIN_
#ifdef _MAIN_

#include "main.h"


//// zSpace Core Headers
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>
#include <headers/zApp/include/zTsPathNetworks.h> 


using namespace zSpace;



////////////////////////////////////////////////////////// 
//GLOBAL VARIABLE --> is defined outside of setup(), update(), keyPress(), mousePress() and mouseMotion();
double backColor = 0.9; // a 'variable' called 'backColor'; variables store values, so that they can be used/changed later

// 1. write code /  make change
// 2. compile aka hit green button 
// 3. run compiled code (also green button)
// see changes.

// simple sketch : calculate points and draw simulatenously
// advanced sketch : calculate points, store points, retrieve points, draw points/ lines
//                     calculate, store (variables - arrays,  int, string, bool, float, Alice::vec),retrieve,use ;




// classes - data structure .. ie a compound way to store and retrieve information, and act on them.
// encapsulation ;

// -------------------------
// zSpace (2024) is the more mature and evolved version of Alice platform (2012)
// however, Alice is easier to learn
// There are some classes taht conflict between Alice and zSpace, such as Alice::vec and zVector.
// as such we need these two conversion functions to convert from Alice::vec to zVector and vice versa.

//----------------------------

// to make use of the actions or methods available in a class,
// we need to :
// 1. make an instance of the class
// 2. provide the isntance with tthe necesssary inputs ( eg center and direction)
// 3. use the method/s of the class ;



Alice::vec zVecToAliceVec(zVector& in)
{
	return Alice::vec(in.x, in.y, in.z);
}

zVector AliceVecToZvec( Alice::vec &in)
{
	return zVector(in.x, in.y, in.z);
}

int insidePolygon(zVector* polygon, int N, zVector& p, int bound)
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
	if (__count % 2 == 0) return(0);
	//ODD
	else return(1);
}

double x[4], y[4], u, v;
zVector IntersectEdges(zVector& p1, zVector& p2, zVector& p3, zVector& p4, IntersectResult& IR)
{

	x[0] = p1.x;
	y[0] = p1.y;

	x[1] = p2.x;
	y[1] = p2.y;

	x[2] = p3.x;
	y[2] = p3.y;

	x[3] = p4.x;
	y[3] = p4.y;

	IR = Intersect_segment2d(x, y, u, v);
	return  (p1 + (p2 - p1) * u);
}

#include "spatialBin.h"
#include "Matrix3x3.h"

Matrix3x3 mat;


#define nPoly 64
#define num_centers 50
double width = 0.5;

class alignedBox
{

public:

	//declare class variables.
	zVector directionOfBox;
	zVector centerOfBox;
	float areaOfBox;

	zVector boxPoints[nPoly];
	zVector boxPointsNormals[nPoly];

	zVector centerPoints[num_centers];
	int n_cen = 0;
	zVector forces[num_centers];

	int nPoints;
	bool boolMove[nPoly];
	int id_u, id_v;

	float normScale = 1;
	float collisionRad = 1.5;
	bool flipNormals_always = false;

	float restLength = 0.2;

	void setDefaultBox(float r = 3)
	{
		
		
		nPoints = nPoly;

		float inc = TWO_PI / float(nPoly);
		//float r =  3; // ofRandom(1, 3);// *sqrt(2);

		for (int i = 0; i < nPoints; i++)
		{
			float x = r * sin( float(i) * inc) ;
			float y = r * cos( float(i) * inc) ;

			boxPoints[i] = zVector(x,y,0);
			boolMove[i] = true;
		}

		invertBox();
		computeNormals();

		normScale = 0.5;
		//collisionRad = normScale * 1;
		
		//importPrimitive();
		//computeNormals();
	}

	void importPrimitive()
	{
		zObjMesh o_fieldMesh;
		zFnMesh fnMesh(o_fieldMesh);

		fnMesh.from("data/parcelPrim.obj", zOBJ, false);

		if (fnMesh.numVertices() < nPoly)
		{
			nPoints = fnMesh.numVertices();
			
			zPointArray vertexPositions;
			fnMesh.getVertexPositions(vertexPositions);

			for (int i = 0; i < nPoints; i++)
			{
				boxPoints[i] = vertexPositions[i];
				boolMove[i] = true;
			}
		}
	}

	void invertBox(zVector u = zVector(1,1,0))
	{
		zTransform TM;
		TM.setIdentity();
		zVector v, w;
		v = u; swap(v.x, v.y); v.y *= -1;
		w = zVector(0, 0, 1);
		u.normalize(); v.normalize(); w.normalize();
		zVector c(0,0,0);


		//assign the values to the matrix
		TM.col(0) << u.x, u.y, u.z, 1;
		TM.col(1) << v.x, v.y, v.z, 1;
		TM.col(2) << w.x, w.y, w.z, 1;
		TM.col(3) << c.x, c.y, c.z, 1;

		TM.inverse();

		for (int i = 0; i < nPoints; i++)
			boxPoints[i] = boxPoints[i] * TM;
	}

	int Mod(int a, int n)
	{
		a = a % n;
		return (a < 0) ? a + n : a;
	}
	
	void shortenNormal( bool lengthen = false)
	{
		normScale *= lengthen ? 1.1 : 0.9;
		for (int i = 0; i < nPoints; i++)
		{
			boxPointsNormals[i].normalize();
			boxPointsNormals[i] *= normScale;
		}
	}
	
	void flipNormals()
	{
		for (int i = 0; i < nPoints; i++) boxPointsNormals[i] *= -1;

	}
	
	void computeNormals()
	{
		for (int i = 0; i < nPoints; i++)
		{
			int next = Mod(i+1,nPoints);// (i + 1) % nPoints;
			int prev = Mod(i - 1, nPoints); //(nPoly - 1 + i) % nPoints;
			zVector e1 = (boxPoints[i] - boxPoints[prev])^ zVector(0, 0, 1);
			zVector e2 = (boxPoints[next] - boxPoints[i]) ^ zVector(0, 0, 1);;

			zVector norm =  ((e1 + e2) * 0.5);
			zVector compareVec = boxPoints[i] - centerOfBox;
			if (norm * compareVec) norm *= -1;

			norm.normalize();
			boxPointsNormals[i] = norm * normScale * (flipNormals_always ? -1 : 1) ;
		}
	}

	float computeParcelArea()
	{
		areaOfBox = 0;
		for (int i = 0; i < nPoints; i++)
		{
			int nxt = Mod(i + i, nPoints);
			areaOfBox += ((boxPoints[nxt] - boxPoints[i]) ^ (centerOfBox - boxPoints[i])).length() * 0.5;

		}

		return areaOfBox;
	}

	void computePCA()
	{
		Matrix3x3 mat;
		zVector mean, eigenVals, eigenVecs[3];
		//double wts[3];

		mat.PCA(boxPoints, nPoints, mean, eigenVals, eigenVecs);

		for (int i = 0; i < 3; i++)
		{
			printf("%1.2f,%1.2f,%1.f  -- %i \n", eigenVecs[i].x, eigenVecs[i].y, eigenVecs[i].z, i);
		}

		cout << " ----------------- " << endl;
		
		eigenVecs[2].normalize();
		directionOfBox = eigenVecs[2] * 5;
	}
	// actions / functions / methods
	void transformBox( )
	{
		zTransform TM;
		TM.setIdentity();
		
		zVector u,v, w;
		u = directionOfBox;
		v = u; swap(v.x, v.y); v.y *= -1;
		w = zVector(0, 0, 1);

		u.normalize(); v.normalize(); w.normalize();
		zVector c = centerOfBox;
		v *= width;

		//assign the values to the matrix
		TM.col(0) << u.x, u.y, u.z, 1;
		TM.col(1) << v.x, v.y, v.z, 1;
		TM.col(2) << w.x, w.y, w.z, 1;
		TM.col(3) << c.x, c.y, c.z, 1;

		for (int i = 0; i < nPoints; i++)
			boxPoints[i] = boxPoints[i] * TM;

		computeNormals();
	}

	void expand(vector<alignedBox>& AB)
	{
		zVector np;
		for (int i = 0; i < nPoints; i++)
		{
			np = boxPointsNormals[i];

			if (boolMove[i])boxPoints[i] += np;

			for (auto& parcel : AB)
			{
				if (parcel.id_u != id_u)
					for (int j = 0; j < nPoints; j++)
					{
						float dist = boxPoints[i].distanceTo(parcel.boxPoints[j]);

						if (dist < collisionRad)
						{
							boolMove[i] = false;

						}
					}
			}
		}


		//computeNormals();
	}

	int *nborIds;
	void expand(spaceGrid* SG)
	{

		zVector np;
		for (int i = 0; i < nPoints; i++)
		{
			np = boxPointsNormals[i];

			if (boolMove[i])boxPoints[i] += np;

			//for (auto& parcel : AB)
			{
				//if (parcel.id_u != id_u)
				int num_nbors = SG->getNeighBors(nborIds, boxPoints[i], collisionRad*2  );

					/// collision test with vertices.
					for (int j = 0; j < num_nbors; j++)
					{
						if (nborIds[j] >= (id_u * nPoints) && nborIds[j] < (id_u + 1) * nPoints)continue;

						float dist = boxPoints[i].distanceTo( SG->positions[ nborIds[j]] );
						
						if (dist < collisionRad)boolMove[i] = false;

					}

					if (!(num_nbors > 0))continue;

					/// look-up intersection test with edges.
					for (int j = 0 ; j < num_nbors ; j++)
					{
						if (nborIds[j] >= (id_u * nPoints) && nborIds[j] < (id_u + 1) * nPoints)continue;

						int next_j = (j == num_nbors - 1) ? nborIds[j] + 1 : nborIds[j+1];// !!! this assumes the nbors[j] ids are in consequtive order;
						if (next_j >= SG->np)continue;

						np.normalize();
						IntersectResult IR;
						IntersectEdges(boxPoints[i], boxPoints[i] + np * collisionRad * 2, SG->positions[nborIds[j]], SG->positions[next_j], IR);

						if (IR == INTERESECTING)
						{
							boolMove[i] = false;
							//boxPoints[i] -= np * collisionRad * 0.5;;

						}
					}
			}
		}


		//computeNormals();
	}

	void expand_withNormalCheck(vector<alignedBox>& AB, bool expandBeforeNormalCheck = false , spaceGrid *SG = NULL)
	{
		
		//if( expandBeforeNormalCheck)expand(AB);
		if (expandBeforeNormalCheck)expand(SG);

		zVector np;
		 

		//double x[4], y[4], u, v;
		//for (int i = 0; i < nPoints; i++)
		//{
		//	np = boxPointsNormals[i];
		//	np.normalize();
		//	
		//	for (auto& parcel : AB)
		//	{
		//		

		//		if (parcel.id_u != id_u)
		//			for (int j = 0; j < nPoints; j++)
		//			{
		//				
		//				int next_j = Mod(j + i, nPoints);// (j + 1) % nPoly;
		//				IntersectResult IR;
		//				IntersectEdges( boxPoints[i], boxPoints[i] + np * collisionRad, parcel.boxPoints[j], parcel.boxPoints[next_j], IR);

		//				if (IR == INTERESECTING)
		//				{
		//					boolMove[i] = false;
		//					boxPoints[i] -= np * collisionRad * 0.5;;

		//				}
		//			}
		//	}
		//}

		//computeNormals();
	}

	void parcel_All_parcel_intersect(vector<alignedBox>& AB)
	{
		for (int i = 0; i < nPoints; i++)
		{
			int nxt = Mod(i + 1, nPoints);// (i + 1) % nPoly;

			for (auto& parcel : AB)
			{
				parcel_parcel_intersect(parcel);
			}

		}

	}

	void parcel_parcel_intersect( alignedBox &otherBox)
	{
		for (int i = 0; i < nPoints; i++)
		{
			int nxt = (i + 1) % nPoints;

			//for (auto& parcel : AB)
			//auto parcel = otherBox;
			{
				if (otherBox.id_u != id_u)
					for (int j = 0; j < nPoints; j++)
					{
						int next_j = (j + 1) % nPoints;

						IntersectResult IR;
						zVector int_pt = IntersectEdges(boxPoints[i], boxPoints[nxt], otherBox.boxPoints[j], otherBox.boxPoints[next_j], IR);

						if (IR == INTERESECTING)
						{
							//boxPoints[i] -= np;
							boolMove[i] = false;
							boolMove[nxt] = false;
							otherBox.boolMove[j] = false; 
							otherBox.boolMove[next_j] = false;
						}
						
						bool i_in, nxt_in;
						i_in = insidePolygon(otherBox.boxPoints, nPoints, boxPoints[i], 0);
						nxt_in = insidePolygon(otherBox.boxPoints, nPoly, boxPoints[nxt], 0);

						zVector np = boxPointsNormals[i];

						if (i_in)
						{
							boxPoints[i] -= boxPointsNormals[i];
							boolMove[i] = false;
						
						}
						if (nxt_in)
						{
							boxPoints[nxt] -= boxPointsNormals[nxt];
							boolMove[nxt] = false;
						}
						

						glPointSize(8);
							if (IR == INTERESECTING) drawPoint( zVecToAliceVec(int_pt) );
						glLineWidth(1);
					}
			}

		}

	}

	void equaliseEdgeLengths()
	{
		for (int i = 0; i < nPoints; i++)
		{
			int nxt = Mod(i + 1, nPoints);
			zVector edge = boxPoints[nxt] - boxPoints[i];
			double displacement = edge.length() - restLength;

			edge.normalize();
			if(boolMove[i])
				boxPoints[i] += edge * displacement * 0.4;	
			
			if (boolMove[nxt])
				boxPoints[nxt] -= edge * displacement * 0.4;
		}

	}

	void smooth()
	{

		for (int i = 0; i < nPoints; i++)
		{
			int prev = Mod(i - 1, nPoints);// (i + nPoints - 1) % nPoints;
			int next = Mod(i + 1, nPoints);// (i + 1) % nPoints;
			if( !boolMove[i])
				boxPoints[i] = boxPoints[prev] * 0.15 + boxPoints[i]*0.7 + boxPoints[next] * 0.15;
			else
				boxPoints[i] = boxPoints[prev] * 0.3 + boxPoints[i] * 0.4 + boxPoints[next] * 0.3;
		}

	}

	void addCenter()
	{
		
		centerPoints[n_cen++] = centerOfBox + zVector(ofRandom(-1, 1), ofRandom(-1, 1), 0);
		if (n_cen >= num_centers)n_cen = 0;
	}
	
	void makeCentersEquiDistant()
	{
		// reset forces
		for (int i = 0; i < n_cen; i++)forces[i] = zVector(0, 0, 0);

		//calculate & store repulsive force per point
		for (int i = 0; i < n_cen; i++)
		{
			for (int j = 0; j < n_cen; j++)
			{
				if (i == j) continue;

				zVector e = centerPoints[j] - centerPoints[i];
				float d = centerPoints[j].distanceTo(centerPoints[i]);

				if (d > 1e-2)
				{
					e.normalize();
					e /= d * d;
					forces[i] -= e;
				}

			}
		}

		// calculate the maximum and minimum magnitude of reuplisve force
		normaliseForces();

		// move each of the points, by applying their respective forces, if the magnitude of force is less than 1 and the point is with a radius of 10 from the origin;
		for (int i = 0; i < n_cen; i++)
			if (forces[i].length() < 1)
			{
				centerPoints[i] += forces[i];
				if (!insidePolygon(boxPoints, nPoints, centerPoints[i], 0))centerPoints[i] -= forces[i]*2;
				
			}
				
				
	}

	void normaliseForces()
	{
		double force_max, force_min;
		force_min = 1e6; force_max = -force_min;

		for (int i = 0; i < n_cen; i++)
		{
			float d = forces[i].length();
			force_max = MAX(force_max, d);
			force_min = MIN(force_min, d);
		}

		// re-scale all forces to be within 0 & 1
		for (int i = 0; i < n_cen; i++)
		{
			float d = forces[i].length();
			forces[i].normalize();
			forces[i] *= ofMap(d, force_min, force_max, 0, 1);

		}
	}

	zVector norm;
	void drawBox()
	{
		glPointSize(5);

		//drawLine and drawPoint accept data of type : Alice::vec
		// so we need to convert zVector to Alice::vec;

		drawPoint(zVecToAliceVec(centerOfBox));
		drawLine(zVecToAliceVec(centerOfBox), zVecToAliceVec(centerOfBox + directionOfBox * 3));

		for (int i = 0; i < nPoints; i++)
		{
			//drawLine and drawPoint accept data of type : Alice::vec
			// so we need to convert zVector to Alice::vec;
			glColor3f(1, 0, 0);
			drawLine(zVecToAliceVec(boxPoints[i]), zVecToAliceVec(boxPoints[(i + 1) % nPoints]));
			norm = boxPointsNormals[i];
			

			//drawLine and drawPoint accept data of type : Alice::vec
			// so we need to convert zVector to Alice::vec;
			(boolMove[i]) ? glColor3f(1, 0, 0) : glColor3f(0, 0, 1);

			drawPoint(zVecToAliceVec(boxPoints[i]));
			drawLine(zVecToAliceVec(boxPoints[i]), zVecToAliceVec(boxPoints[i] + norm ));


			/*for (int i = 0; i < nPoints; i++)
				if (boolMove[i])
				{
					(boolMove[i]) ? glColor3f(1, 0, 0) : glColor3f(0, 0, 1);
					drawCircle(zVecToAliceVec(boxPoints[i]), collisionRad, 32);
				}*/
			
		}

		for (int i = 0; i < n_cen; i++)
		{
			drawPoint(zVecToAliceVec(centerPoints[i]));
			drawLine(zVecToAliceVec(centerPoints[i]), zVecToAliceVec(centerPoints[i] + forces[i]));
		}

		glPointSize(1);


	}
};


vector<alignedBox> parcels;
zObjMesh o_fieldMesh;
spaceGrid SG;
zModel model;

zTsGraphShortestPath  graphSP;
zObjGraph o_graph;
vector<zObjGraph> o_shortestPaths;
zObjMesh o_pathMesh;
zObjMesh o_fixeBoundaryMesh;

// global declaration
zVector cen(0, 0, 0);
zVector cen1(-15, -15, 0);
zVector cen2(12, 12, 0);

bool compute = false;
bool equalise = false;
bool equaliseD = false;


void setup() // events // eventHandles
{
	// assign values to global variables ;
	double backColor = 0.0;
	backColor = 0.0; // set an initial value for backgroun color


	S.addSlider(&width, "wd");// make a slider control for the variable called width;
	//S.sliders[0].maxVal = 10;


	zFnMesh fnMesh(o_fieldMesh);
	fnMesh.from("data/field_square.obj", zOBJ, false);

	zPointArray vp;
	fnMesh.getVertexPositions(vp);
		for (auto & p : vp)p *= 1;
	fnMesh.setVertexPositions(vp);

	//zColorArray clrAr;
	//fnMesh.getFaceColors(clrAr);

	zPointArray faceCenters;
	fnMesh.getCenters(zFaceData, faceCenters);

	int nump = 0;
	parcels.clear();

	for (auto& P : faceCenters)
	{
		alignedBox AB;

		zVector gridPt = P;

		float dist1 = gridPt.distanceTo(cen1);
		float dist2 = gridPt.distanceTo(cen2);

		cen = (dist1 < dist2) ? cen1 : cen2;
		
		zVector ed = (gridPt - cen);
		ed.normalize(); // gridPt + Alice::vec(i * 0.1, j * 0.05, 0);


		AB.centerOfBox = gridPt;
		AB.directionOfBox = ed;
		AB.setDefaultBox();
		AB.transformBox();
		AB.id_u = nump;

		parcels.push_back(AB);

		nump++;

	}

	SG = *new spaceGrid();
	//spaceDiv.PartitionParticlesToBuckets();

	//cout << spaceDiv.np << endl;
	//cout << spaceDiv.getNumVoxelsContainingParticles() << endl;


	



	//model.addObject(o_shortestPath);
	
}

void update(int value) // an event that run 100 times a second, automatically, without any user input
{
	
	if (compute)
	{
		long start_time = GetTickCount();

	
		//for (auto& parcel : parcels)parcel.expand(parcels);
		for (auto& parcel : parcels)parcel.expand_withNormalCheck(parcels,true, &SG);

		/*for (int i = 0; i < parcels.size(); i++)
			for (int j = i + 1; j < parcels.size(); j++)
				parcels[i].parcel_parcel_intersect(parcels[j]);*/

		for (auto& parcel : parcels)parcel.smooth();
		//for (auto& parcel : parcels)parcel.computeNormals();

		
		keyPress('b', 0, 0);


		long end_time = GetTickCount();
		long elapsed_time = end_time - start_time;

		cout << elapsed_time << " --- ELTIME" << endl;
		
	}

	if (equalise)
	{
		for (auto& parcel : parcels)parcel.equaliseEdgeLengths();

		keyPress('b', 0, 0);
	}
	
	if (equaliseD)
	{
		keyPress('D', 0, 0);
	}
	
}

void draw() // an event that run 100 times a second, automatically, without any user input
{
	
	//drawing code here
	backGround(backColor);
	drawGrid(50);

	
	//glDisable(GL_LINE_STIPPLE);
	

	//
	glColor3f(0, 1, 0.5);
	drawCircle( zVecToAliceVec(cen1), 5, 32);
	drawCircle( zVecToAliceVec(cen2), 5, 32);

	//testBox.drawBox();

	if(!compute)
	for (auto& parcel : parcels)parcel.drawBox();


	wireFrameOn();
		model.draw();
	

	/*for (int i = 0; i < parcels.size(); i++)
		for (int j = i + 1; j < parcels.size(); j++)
			parcels[i].parcel_parcel_intersect(parcels[j]);*/

	//SG.drawBuckets();
	SG.drawParticlesInBuckets();

	wireFrameOff();

	//FD.display();
	
}

int np = 0;
void keyPress(unsigned char k, int xm, int ym) // events
{

	if(k =='1')
	{
		for (auto& p : parcels)p.computePCA();
	}


	if (k == 'C')
	{
		for (auto& p : parcels)
		{
			for (int i = 0; i < num_centers-1; i++)
				p.addCenter();
		}
				
	}

	if (k == 'D')
	{
		for (auto& p : parcels)p.makeCentersEquiDistant();
	}

	if (k == 'B')
	{
		//zObjMesh o_fixeBoundaryMesh;
		zFnMesh fnMesh(o_fixeBoundaryMesh);
		fnMesh.from("data/fixed.obj", zOBJ, false);


	}
	
	
	
	if (k == '+')
	{
		
		vector<alignedBox> newParcels;
		alignedBox AB;
		for (auto &parcel : parcels)
		{
			if (parcel.computeParcelArea() < 3) continue;

			for (int i = 0; i < parcel.n_cen; i++)
			{
				if (!insidePolygon(parcel.boxPoints, parcel.nPoints, parcel.centerPoints[i] + parcel.forces[i] * 2, 0))continue;

				AB.centerOfBox = parcel.centerPoints[i];
				AB.directionOfBox = parcel.directionOfBox;
				AB.setDefaultBox(0.5);
				AB.transformBox();
				AB.id_u = parcels.size() + newParcels.size();
				newParcels.push_back(AB);

			}
			
		}

		copy(newParcels.begin(), newParcels.end(), back_inserter(parcels));
		
		/*alignedBox AB;

		int sz = parcels.size();
		int rnd_parcel = ofRandom(0, sz - 1);

		zVector cen = parcels[rnd_parcel].centerOfBox;
		zVector dir = parcels[rnd_parcel].centerOfBox;
		dir.normalize();

		zVector cen1 = cen + dir * 5;
		zVector cen2 = cen - dir * 5;

		{
			AB.centerOfBox = cen1;
			AB.directionOfBox = dir;
			AB.setDefaultBox();
			AB.transformBox();
			AB.id_u = parcels.size();
			parcels.push_back(AB);
		}

		{
			AB.centerOfBox = cen2;
			AB.directionOfBox = dir;
			AB.setDefaultBox();
			AB.transformBox();
			AB.id_u = parcels.size();
			parcels.push_back(AB);
		}*/


		
	}

	//-------------  smooth paths

	if (k == 'h')
	{
		alignedBox ab;
		zPointArray vp;
		for (auto& o_g : o_shortestPaths)
		{
			zFnGraph fnGraph(o_g);
			vp.clear();
			fnGraph.getVertexPositions(vp);

			for (int i = 0 ; i < vp.size() ; i++)
			{
				if (i == 0 || i == vp.size() - 1) continue;

				int nxt = ab.Mod(i + 1, vp.size() );
				int prev = ab.Mod(i - 1, vp.size() );
				vp[i] = vp[prev] * 0.2 + vp[i] * 0.6 + vp[nxt] * 0.2;
			}
			
			fnGraph.setVertexPositions(vp);
			
			
		}
	}

	//-------------  compute paths

	if (k == 'g')
	{
		//o_shortestPath.setDisplayVertices(true);
		zFnMesh fnMesh(o_pathMesh);
		fnMesh.from("data/path_grid.obj", zOBJ, false);

		zPointArray vp;
		fnMesh.getVertexPositions(vp);

		
		for( auto&p : vp)
			for (auto& parcel : parcels)
			{
				bool inside = insidePolygon(parcel.boxPoints, parcel.nPoints, p, 0);
					
				if (inside)p.z = 100;
			}
			
		fnMesh.setVertexPositions(vp);


		///  ------------------------------
		zFnGraph fnGraph(o_graph);
		fnGraph.createFromMesh(o_pathMesh); // or create using arrays.
		graphSP = zTsGraphShortestPath(o_graph);
		
		int vertices[4] = { 0, 100,10100,10200 };
		int num_combinations = (4 * 3)/2;
		o_shortestPaths.reserve(num_combinations);
		o_shortestPaths.resize(num_combinations);

		int path_cnt = 0;
		cout << endl;
		
		for (int i = 0; i < 3; i++)
		{
			//int i = 3;
			int source = vertices[i];
			for (int j = i; j < 4; j++)
			{
				if (i == j)continue; 
				if(path_cnt >= (num_combinations-1))continue; 

				graphSP.getShortestPathGraph(source, vertices[j], o_shortestPaths[path_cnt]);
				
				zFnGraph fn_gs(o_shortestPaths[path_cnt]);
				if (fn_gs.numVertices() > 0 )
				{
					fn_gs.setEdgeWeight(3);
					o_shortestPaths[path_cnt].setDisplayVertices(true);

					model.addObject(o_shortestPaths[path_cnt]);

					cout << i << "--" << j << "--" << path_cnt << "--- PC" << endl;
					path_cnt++;
				}
		
			}

		}

		//model.addObject(o_pathMesh);

		

	}

	/////////////// update spaceGrid ;
	if (k == 'b')
	{

		//SG = *new spaceGrid(parcels[0].boxPoints, parcels[0].nPoints);
		SG.clearBuckets();
		SG.np = 0;

		for( auto &parcel : parcels)
			for (int i = 0; i < parcel.nPoints; i++)
			{
				SG.addPosition(parcel.boxPoints[i]);
			}
		
		
		//.. add graph points

		zPointArray vp;
		for (auto& o_g : o_shortestPaths)
		{
			zFnGraph fnGraph(o_g);
			vp.clear();
			fnGraph.getVertexPositions(vp);

			for (int i = 0; i < vp.size(); i++)
				SG.addPosition(vp[i]);
		}

		// .. add fixed boundary

		vp.clear();
		zFnMesh fnMesh(o_fixeBoundaryMesh);
		fnMesh.getVertexPositions(vp);

			for (int i = 0; i < vp.size(); i++)
				SG.addPosition(vp[i]);

		
		// add dummy point ;
		SG.addPosition(zVector(0, 0, 1));

		
		SG.PartitionParticlesToBuckets();
	}

	

	//---------------------------------------
	
	if(k == 'r')
		compute = !compute;

	if(k == 'e')
		equalise = !equalise;

	if (k == 'E')
		equaliseD = !equaliseD;

	if (k == 's')
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
#endif // _MAIN_