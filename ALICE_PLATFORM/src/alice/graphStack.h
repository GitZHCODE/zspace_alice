
#include "main.h"
#include "ALICE_ROBOT_DLL.h"
using namespace ROBOTICS;

#include "graph.h"
#include "ActiveGraph.h"
#include "metaMesh.h"

class graphStack
{
public:

	RenderMesh LM;
	activeGraph PrintStack[500];
	Graph G;
	vec minV, maxV;
	double dMin, dMax;
	double threshold;
	int currentStackLayer;
	metaMesh MM;
	bool convertedToToroidal;
	int filecnt = 0;
	////---------------------------------------------------- CONSTRUCTOR --------------------------------------
	graphStack( )
	{
		//---------
		//PrintStack.reserve(25);
		//PrintStack = * new Graph[25]
	}

	////---------------------------------------------------- COMPUTE  --------------------------------------
	void readGraphAndCreateDataMesh(string fileToImport, float scale = 1.0)
	{
		//---------
		cout << "--imp" << endl;
		importer imp = *new importer(fileToImport, 10000, 1.0);
		imp.readEdges();
		cout << "--impF" << endl;

		//---------
		G.reset();
		for (int i = 0; i < imp.nCnt; i++)G.createVertex(imp.nodes[i].pos);
		for (int i = 0; i < imp.eCnt; i++)G.createEdge(G.vertices[imp.edges[i].n0], G.vertices[imp.edges[i].n1]);

		// ------------ scale to fit 
		G.boundingbox(minV, maxV);

		Matrix4 trans;
		//double preferedDiag = (minV.distanceTo(maxV));
		//trans.scale(preferedDiag / (minV.distanceTo(maxV)));
		//trans.translate((minV + maxV) * 0.5);
		//for (int i = 0; i < G.n_v; i++) G.positions[i] = trans * G.positions[i];
		//cout << " ACUTAL DIAG " << minV.distanceTo(maxV) << endl;
		G.boundingbox(minV, maxV);

		trans.identity();
		//trans.translate(vec(45, 0, 0) - (minV + maxV) * 0.5);
		for (int i = 0; i < G.n_v; i++) G.positions[i] = trans * G.positions[i];
		minV = minV * trans;
		maxV = maxV * trans;

		G.boundingbox(minV, maxV);
		minV -= (maxV - minV).normalise() * 12.5;
		maxV += (maxV - minV).normalise() * 12.5;
		
	
		//--------- dateGrid
		cout << " dataGrid create " << endl;
		createDataMeshGrid(G);
		//createIsoContourGraph(0.1);
		cout << " dataGrid created " << endl;
		//
		currentStackLayer = 0;
		convertedToToroidal = false; 

	}


	void createDataMeshGrid( Graph &G)
	{
		MeshFactory fac;
		//MM = metaMesh(fac.createFromOBJ("data/in.obj", 1.0, false, false));
		//MM.assignScalars("z");
		MM = MM.createFromPlane(minV, maxV, 100);
		cout << "create from plane" << endl;
		MM.assignScalarsAsLineDistanceField(G,0.0,1500,true);
		MM.getMinMaxOfScalarField(dMin, dMax);
	
	}

	void createIsoContourGraph( double threshold )
	{
		MM.createIsoContourGraph(threshold);
		
	}
	void convertContourToToroidalGraph()
	{
		MM.convertContourToToroidalGraph();
		convertedToToroidal = true;
	}

	void smoothCurrentGraph( double breakAngle )
	{
		//MM.G.smooth_connectedVertices();
		MM.G.smoothGraph(breakAngle);
	}

	void inflateCurrentGraph( bool medialAxis = true )
	{
		//MM.G.inflateVertices();
		double prevZ = G.positions[0].z;
		for (int i = 0; i < G.n_v; i++)G.positions[i].z = MM.G.positions[0].z;
		
		( medialAxis ) ? MM.G.inflateWRTMedialAxis(G) : MM.G.inflateVertices();

		for (int i = 0; i < G.n_v; i++) G.positions[i].z = prevZ;
	}

	void transformCurrentGraphBy( double ang = 5 , int axis = 0 , vec trans = vec(0,0,0) )
	{
		Matrix4 T , T_inv;
		plane Pl = MM.G.getCurrentPlane();
		vec c, u, v, n;
		n = Pl.normal;
		c = MM.G.centroid();
		u = (MM.G.positions[1] - MM.G.positions[0]).normalise();
		v = u.cross(n).normalise();
		n.normalise();

		T.setColumn(0, u);
		T.setColumn(1, v);
		T.setColumn(2, n);
		T.setColumn(3, c);

		T_inv = T;
		T_inv.invert();
		for (int i = 0; i < MM.G.n_v; i++)MM.G.positions[i] = T_inv * MM.G.positions[i];

		T_inv.identity();
		if( axis == 0)T_inv.rotateX(ang);
		if (axis == 1)T_inv.rotateY(ang);
		if (axis == 2)T_inv.rotateZ(ang);
		T_inv.translate( trans );

		for (int i = 0; i < MM.G.n_v; i++)MM.G.positions[i] = T_inv * MM.G.positions[i];

		for (int i = 0; i < MM.G.n_v; i++)MM.G.positions[i] = T * MM.G.positions[i];

	}

	void translateCurrentGraph()
	{


	}

	////---------------------------------------------------- UTILITIES  --------------------------------------

	void reducePointsOnContourGraph( int inc = 3)
	{
		toroidalGraph A;
		A.constructFromGraph(MM.G , inc);

		MM.G.reset();
		MM.G.constructFromGraph(A);

	}

	void addCurrentContourGraphToPrintStack( float layersize = 0.1 , float baseOffset = 1.0 )
	{
		
		//---------
		activeGraph AG;
		AG = /** new */activeGraph();
		AG.reset();
		
		AG.constructFromGraph(MM.G);
		//AG.fixEnds();
		/*AG.populateRigidBodies(1.0,layersize);*/
		plane prevPl;
		prevPl.cen = AG.centroid();
		prevPl.cen.z += layersize;
		prevPl.normal = vec(0, 0, 1);
		AG.populateRigidBodies(LM, prevPl, layersize, layersize);
		
		PrintStack[currentStackLayer] = AG;

		///
		currentStackLayer++;
		if (currentStackLayer >= 500 )currentStackLayer = 0;
	}

	void popPrintstack()
	{
		currentStackLayer--;
		double z = PrintStack[currentStackLayer].positions[0].z;
		for (int i = 0; i < MM.G.n_v; i++)MM.G.positions[i].z = z;// currentStackLayer * layersize + baseOffset;

	}


	void ConvertContourStackToPrintPath(pathImporter &path)
	{
		path.actualPathLength = 0;
		for (int i = 0; i < currentStackLayer; i++)
			for (int j = 0; j < PrintStack[i].n_v; j++)
				path.addPoint( PrintStack[i].positions[j] );
		
	}

	//void writeStackToObj(string outFileName = "data/stack.txt")
	//{
	//	Mesh M;
	//	for (int i = 0; i < currentStackLayer; i++)
	//		for (int j = 0; j < PrintStack[i].n_v; j+= 1)
	//		{
	//			M.createVertex(PrintStack[i].positions[j]);
	//		}

	//	Vertex *fv[4];

	//	for (int i = 0; i < currentStackLayer -1; i++)
	//		for (int j = 0; j < PrintStack[i].n_v-1; j+= 1)
	//		{

	//			fv[0] = &M.vertices[PrintStack[i].n_v * i + j];
	//			fv[1] = &M.vertices[PrintStack[i].n_v * i + j+1];
	//			fv[2] = &M.vertices[PrintStack[i].n_v * (i+1) + j+1];
	//			fv[3] = &M.vertices[PrintStack[i].n_v * (i+1) + j];
	//			M.createFace(fv, 4);
	//		}

	//	for (int i = 0; i < M.n_f; i++)M.faces[i].faceVertices();

	//	M.writeOBJ( outFileName, "", M.positions, false);
	//}

	void writeStackToObj(string outFileName = "data/stack.txt")
	{
		ofstream myfile;
		myfile.open(outFileName.c_str(), ios::out);

		if (myfile.fail())
		{
			myfile << " error in opening file  " << outFileName.c_str() << endl;
			return;
		}


		for (int i = 0; i < currentStackLayer; i++)
			for (int j = 0; j < PrintStack[i].n_v; j += 1)
			{
				char s[200];
				sprintf(s, "v %1.4f %1.4f %1.4f ", PrintStack[i].positions[j].x, PrintStack[i].positions[j].y, PrintStack[i].positions[j].z);

				myfile << s << endl;
			}

		Vertex *fv[4];

		for (int i = 0; i < currentStackLayer - 1; i++)
			for (int j = 0; j < PrintStack[i].n_v - 1; j += 1)
			{


				string str;
				str = "f ";
				char s[200];
				{
					
					itoa(PrintStack[i].n_v * i + j + 1, s, 10);
					str += s;
					str += "//";
					str += s;
					str += " ";
				}
				//
				{

					itoa(PrintStack[i].n_v * i + j + 1 + 1, s, 10);
					str += s;
					str += "//";
					str += s;
					str += " ";
				}
				//
				{

					itoa(PrintStack[i].n_v * (i + 1) + j + 1 + 1, s, 10);
					str += s;
					str += "//";
					str += s;
					str += " ";
				}
				//
				{

					itoa(PrintStack[i].n_v * (i + 1) + j + 1, s, 10);
					str += s;
					str += "//";
					str += s;
					str += " ";
				}


				myfile << str.c_str() << endl;
			}

		myfile.close();

	}

	void writeStackToFile(string outFileName = "data/stack.txt")
	{
		printf(" ----------- writing graphs : stack \n ");

		ofstream myfile;
		myfile.open(outFileName.c_str(), ios::out);


		if (myfile.fail())
		{
			myfile << " error in opening file  " << outFileName.c_str() << endl;
			return;
		}


		for (int i = 0; i < currentStackLayer; i++)
		{
			string layer = "";
			layer += "layer";
			layer += "_";
			char s[20];
			itoa(i, s, 10);
			layer += s;

			myfile << layer << endl;
			myfile << "/*" << endl;

			PrintStack[i].writeVerticeToFile(myfile);

			myfile << "*/" << endl;
		}

		myfile.close();
		printf(" ----------- writing done : stack \n ");

	}

	void writeCurrentGraph()
	{
		string file = "";
		file += "data/graph";
		file += "_";
		char s[20];
		itoa(filecnt, s, 10);
		file += s;
		file += ".txt";

		MM.G.writeGraph(1.0,file);
		filecnt++;
	}

	////---------------------------------------------------- DISPLAY  --------------------------------------



	void draw( bool showMesh = false, bool showMeshWire = false,bool showData = false )
	{
		

		{
			Matrix4 T;
			plane Pl = MM.G.getCurrentPlane();
			vec c, u, v, n;
			n = Pl.normal;
			c = MM.G.centroid();
			u = (MM.G.positions[1] - MM.G.positions[0]).normalise();
			v = u.cross(n).normalise();
			n.normalise();

			T.setColumn(0, u);
			T.setColumn(1, v);
			T.setColumn(2, n);
			T.setColumn(3, c);

			drawFrame(T);
		}
		
		
		wireFrameOn();

			
			/// ------------------------- TODO : clean this section -> encapsulate

		glColor3f(1, 0, 0);
		glLineWidth(3);
			G.draw();
		glLineWidth(1);

			MM.G.computeIslandsAsEdgeAndVertexList();
			//MM.G.drawConnectedEdgeList( true );
			
			convertedToToroidal ? glColor3f(1, 0, 0) : glColor3f(0, 0, 0);
			MM.G.draw();
			//MM.drawIsoContoursInRange(threshold, 1.0);

			glColor3f(0, 0, 0);
			/// ------------------------- TODO : clean this section

		wireFrameOff();

		//----------------- drawDataGridMesh
		if(showData)MM.display(showData, true, false);

		//----------------- drawDataGridMesh
		if (showMesh)LM.draw(showMeshWire);

		glColor3f(0, 0, 0);
		glLineWidth(1);
			for (int i = 0; i < currentStackLayer; i++)PrintStack[i].dilay();// draw();//draw RC
			for (int i = 0; i < currentStackLayer; i++)PrintStack[i].draw(false);// draw();
		glLineWidth(1);
		


		//draw stats
		char s[200];
		sprintf_s(s, " num points in contour : %i", MM.G.n_v);

		setup2d();

		AL_drawString(s, winW * 0.5, 50);
			sprintf_s(s, " num points in stack : %i", MM.G.n_v * currentStackLayer);
		AL_drawString(s, winW * 0.5, 75);
		
		sprintf_s(s, " numlayers : %i total height : %1.2f", currentStackLayer, currentStackLayer * 0.1);
		AL_drawString(s, winW * 0.5, 100);

		{
			sprintf_s(s, " numBricks : %i ", LM.numCubes);
			AL_drawString(s, winW * 0.5, 125);
		}
		
		restore3d();


	}

};

