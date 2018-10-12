
#ifndef _NACHI_

#define _NACHI_

#include "main.h"
#include "ALICE_ROBOT_DLL.h"
using namespace ROBOTICS;

//#include "graph.h"
#include "alice/Matrix3x3.h"

class EndEffector
{
public:

	Mesh M;
	Matrix4 T_w0, T_ew, T_e0, T_ew_in, T_e0_in, T_w0_inv;

	EndEffector() {};
	EndEffector(string file , Matrix4 T_wrist)
	{
		MeshFactory fac;
		M = fac.createFromOBJ(file, 1, false);
		setup(T_wrist);
	}

	void setup( Matrix4 T_wrist)
	{
		T_w0 = T_wrist;
		normaliseFrame(T_w0);

		// assumes vector vtx1-vtx0 points in positive X, and vtx2 - vtx0, points in positive Y;
		// *-1 adjustment is to get the EE to point upward. This is internal adjustment, not to be changed.
		T_e0.setColumn(0, (M.positions[1] - M.positions[0]).normalise() * -1);
		T_e0.setColumn(1, (M.positions[2] - M.positions[0]).normalise());
		T_e0.setColumn(2, T_e0.getColumn(0).cross(T_e0.getColumn(1)).normalise());
		T_e0.setColumn(3, M.positions[0]);

		T_e0_in = T_e0;
		T_e0_in.invert();

		T_w0_inv = T_w0;
		T_w0_inv.invert();
		T_ew = T_w0_inv * T_e0;

		T_ew_in = T_ew;
		T_ew_in.invert();

		//////////////////////////////////////////////////////////////////////////
		for (int i = 0; i < M.n_v; i++)M.positions[i] = T_w0_inv * M.positions[i];
	}
	
	Matrix4 inverseTransformTool( Matrix4 TOOL )
	{
		return ( TOOL * T_ew_in );
	}

	void drawAtLocation(Matrix4 &EE, bool wireframe = true)
	{
		for (int i = 0; i < M.n_v; i++)M.positions[i] = EE * M.positions[i];// to tcip

		//if (wireframe)wireFrameOn();
		draw();
		//if (wireframe)wireFrameOff();

		EE.invert();
		for (int i = 0; i < M.n_v; i++)M.positions[i] = EE * M.positions[i];// to tcip
	}

	void draw()
	{
		//Nachi_tester.draw();
		M.draw(true);
		//int i = 1 + 4 ;
		//char c[200];
		//sprintf(c, "%i ", i);
		//string s = "";
		//s += c;
		//drawString(s, M.positions[i]);

		//i = 3 + 4 ;
		//sprintf(c, "%i ", i);
		//s = "";
		//s += c;
		//drawString(s, M.positions[i]);


		//i = 7 + 4;
		//sprintf(c, "%i ", i);
		//s = "";
		//s += c;
		//drawString(s, M.positions[i]);

		//i = 0;
		//sprintf(c, "%i ", i);
		//s = "";
		//s += c;
		//drawString(s, (M.positions[7] + M.positions[11]) * 0.5);

		int i = 1;
		char c[200];
		sprintf(c, "%i ", i);
		string s = "";
		s += c;
		drawString(s, M.positions[i]);

		i = 7;
		sprintf(c, "%i ", i);
		s = "";
		s += c;
		drawString(s, M.positions[i]);


		i = 5;
		sprintf(c, "%i ", i);
		s = "";
		s += c;
		drawString(s, M.positions[i]);


		//glColor3f(1, 0, 0); drawLine(cen, cen + XA);
		//glColor3f(0, 1, 0); drawLine(cen, cen + YA);
		//glColor3f(0, 0, 1); drawLine(cen, cen + ZA);

		//glColor3f(1, 0, 0); drawLine(cen_f, cen_f + XA_f);
		//glColor3f(0, 1, 0); drawLine(cen_f, cen_f + YA_f);
		//glColor3f(0, 0, 1); drawLine(cen_f, cen_f + ZA_f);

	}
};

//
// derive your class from a base / existing class 
class pathImporter :public importer
{

#define maxPts 59999 // max nachi controller limit
public:

	////////////////////////////////////////////////////////////////////////// CLASS VARIABLES 
	// path related class variables i.e arrays 

	vec path[maxPts][4]; // array to store TOOL locations ( from input text file ) or other method
	double rotations[maxPts][6];// array for joint rotations at each point along path 
	bool *reachable; // array to booleans
	int actualPathLength;
	vec min, max; // to store min - max of path bbox

	// point related class variables 
	vec tcp, tcp_x, tcp_y, tcp_z;
	int currentPointId = 0;

	//
	Robot_Symmetric Nachi;
	EndEffector E;
	Matrix4 TOOL, TOOL_req;

	Matrix4 fTrans;
	//Mesh M;

	////////////////////////////////////////////////////////////////////////// CLASS METHODS 

	pathImporter( string file_EndEffector = "data/end_effector.obj" )
	{
		currentPointId = 0;
		Nachi.addMeshes();

		//

		Nachi.ForwardKineMatics(Nachi.rot);
		Nachi.ForwardKineMatics(Nachi.rot);
		//

		E = *new EndEffector(file_EndEffector, Nachi.Bars_to_world_matrices[5]);
		//
		actualPathLength = 0;
		reachable = new bool[maxPts];

	}
	void readPath(string fileToRead = "data/path.txt", string delimiter = ",", float inc = 0)
	{
		cout << "reading file for path " << fileToRead << endl;

		////////////////////////////////////////////////////////////////////////// read file

		std::fstream fs(fileToRead.c_str(), ios::in);

		if (fs.fail())
		{
			cout << " error in file reading " << fileToRead << endl;
			return;
		}

		//actualPathLength = 0;
		while (!fs.eof() && actualPathLength < maxPts)
		{
			char str[2000];
			fs.getline(str, 2000);
			vector<string> content = splitString(str, ",");

			assignDefaultValues();

			if (content.size() >= 3)tcp = extractVecFromStringArray(0, content) * 1.0;
			if (content.size() >= 6)tcp_x = extractVecFromStringArray(3, content).normalise() * 1;
			if (content.size() >= 9)tcp_y = extractVecFromStringArray(6, content).normalise() * 1;
			if (content.size() >= 12)tcp_z = extractVecFromStringArray(9, content).normalise() * 1;

			//tcp.x += 5.0;
			tcp.z += inc;
			addPoint(tcp,tcp_x,tcp_y,tcp_z);
		}

		fs.close();

		reachable = new bool[actualPathLength];

		////////////////////////////////////////////////////////////////////////// bounding box of file

		getBoundingBox();

		//	checkReach();
		//copyPathToGraph();

	}

	
	////////////////////////////////////////////////////////////////////////// UTILITY METHODS

	vec extractVecFromStringArray(int id, vector<string> &content)
	{
		return vec(atof(content[id].c_str()), atof(content[id + 1].c_str()), atof(content[id + 2].c_str()));
	}
	void assignDefaultValues()
	{
		tcp = vec(0, 0, 0);
		tcp_x = vec(-1, 0, 0);
		tcp_y = vec(0, 1, 0);
		tcp_z = vec(0, 0, -1);
	}
	void getBoundingBox()
	{
		min = vec(pow(10, 10), pow(10, 10), pow(10, 10));
		max = min * -1;

		for (int i = 0; i < actualPathLength; i++)
		{
			tcp = path[i][0];
			max.x = MAX(tcp.x, max.x);
			max.y = MAX(tcp.y, max.y);
			max.z = MAX(tcp.z, max.z);

			min.x = MIN(tcp.x, min.x);
			min.y = MIN(tcp.y, min.y);
			min.z = MIN(tcp.z, min.z);
		}
	}
	void addPoint(vec tcp, vec tcp_x , vec tcp_y , vec tcp_z )
	{

		path[actualPathLength][0] = tcp;
		path[actualPathLength][1] = tcp_x * 1;
		path[actualPathLength][2] = tcp_y * 1;
		path[actualPathLength][3] = tcp_z * 1;
		actualPathLength++;
		if (actualPathLength > maxPts)actualPathLength = 0;
	}
	void getToolLocation(int id, Matrix4 &TOOL)
	{
		TOOL.setColumn(0, path[id][1]); // tcp_x
		TOOL.setColumn(1, path[id][2]); //tcp_y
		TOOL.setColumn(2, path[id][3]); // tcp_z
		TOOL.setColumn(3, path[id][0]); // tcp
	}
	Matrix4 getToolLocation(int id)
	{
		Matrix4 _TOOL;
		getToolLocation(id, _TOOL);
		return _TOOL;
	}
	void goToNextPoint()
	{
		TOOL = getToolLocation(currentPointId);
 		TOOL_req =  E.inverseTransformTool(TOOL);

		double rot_prev[6];
		for (int i = 0; i < 6; i++)rot_prev[i] = Nachi.rot[i];

		Nachi.inverseKinematics_analytical(TOOL_req, false);

		//angleCorrection(rot_prev);

		Nachi.ForwardKineMatics(Nachi.rot);


		currentPointId++;
		if (currentPointId >= actualPathLength - 1)currentPointId = 0;

	}



	////////////////////////////////////////////////////////////////////////// COMPUTE METHODS

	void checkReach()
	{
		getBoundingBox();

		//----- print global warnings if any

		cout << " ------------------------------ START - PATH RELATED  WARNINGS ------------------------------ " << endl;

		if (min.mag() >= 70 || max.mag() >= 70) cout << " some or all points out of range" << endl;
		if (min.mag() <= 20 || max.mag() <= 20) cout << " some or all points are too close to robot " << endl;

		cout << " ------------------------------  END - PATH RELATED  WARNINGS ------------------------------ " << endl;

		//----- print point specific warnings if any

		cout << " ------------------------------ START - POINT-SPECIFIC   WARNINGS ------------------------------ " << endl;
		checkPathForReachability();
		cout << " ------------------------------ END - POINT-SPECIFIC   WARNINGS ------------------------------ " << endl;
	}
	void checkPathForReachability()
	{

		currentPointId = 0;
		for (int i = 0; i < actualPathLength - 1; i++)
		{
			// get TOOL information at current point i
			goToNextPoint();

			//-170	170
			//	65 - 150
			//	- 70.001	90
			//	190.001 - 190.001
			//	- 109.106	109.106
			////	360.001 - 360.001

			//angleCorrection(rot_prev);


			//Nachi_tester.rot[0] = ofClamp(Nachi_tester.rot[0], -170, 170);
			//Nachi_tester.rot[1] = ofClamp(Nachi_tester.rot[1], -65, 150);
			//Nachi_tester.rot[2] = ofClamp(Nachi_tester.rot[2], -70, 90);
			//Nachi_tester.rot[3] = ofClamp(Nachi_tester.rot[3], -170, 170);
			//Nachi_tester.rot[4] = ofClamp(Nachi_tester.rot[4], -109, 109);
			//Nachi_tester.rot[5] = ofClamp(Nachi_tester.rot[5], -360, 360);

			// store corresponding rotations at each point along path
			// for later use such as gcode export & graph-generation etc.
			for (int n = 0; n < 6; n++)rotations[i][n] = Nachi.rot[n];

		}

	}
	void angleCorrection(double * rot_prev)
	{
		if (fabs(rot_prev[3] - Nachi.rot[3]) > 180)
		{
		//	cout << Nachi_tester.rot[3] << " J3_fk " << endl;

			if (rot_prev[3] < 0 && Nachi.rot[3] > 0) Nachi.rot[3] -= 360;
			if (rot_prev[3] > 0 && Nachi.rot[3] < 0) Nachi.rot[3] += 360;

		//	cout << Nachi_tester.rot[3] << " J3_new " << endl;
		}

	//	cout << Nachi_tester.rot[3] << " J4_new_after " << endl;


		if (fabs(rot_prev[5] - Nachi.rot[5]) > 180)
		{

			//cout << rot_prev[5] << " J5_prev " << endl;
			//cout << Nachi_tester.rot[5] << " J5_fk " << endl;

			if (rot_prev[5] < 0 && Nachi.rot[5] > 0) Nachi.rot[5] -= 360;
			if (rot_prev[5] > 0 && Nachi.rot[5] < 0) Nachi.rot[5] += 360;

			//cout << Nachi_tester.rot[5] << " J5_new " << endl;
		}

		/*Nachi_tester.rot[0] = ofClamp(Nachi_tester.rot[0], -170, 170);
		Nachi_tester.rot[1] = ofClamp(Nachi_tester.rot[1], -65, 150);
		Nachi_tester.rot[2] = ofClamp(Nachi_tester.rot[2], -70, 90);
		Nachi_tester.rot[3] = ofClamp(Nachi_tester.rot[3], -150, 150);
		Nachi_tester.rot[4] = ofClamp(Nachi_tester.rot[4], -109, 109);
		Nachi_tester.rot[5] = ofClamp(Nachi_tester.rot[5], -360, 360);*/
	}
	void exportGCode(string fileToWrite = "data/MZ07-01-A.080")
	{
		
		int counter = 0;
		//----- check & compute all necessary values
		checkReach();

		cout << "--------------------------- EXPORT GCODE ----------------- " << endl;
		cout << "Ensure you have inspected robot reach at all points previously " << endl;
		cout << "un-reachable points revert to previous reach-able points" << endl;

		//----- instance ofstream for file IO
		ofstream myfile_write;
		char gcode[600];

		//- open file
		myfile_write.open(fileToWrite.c_str(), ios::out);
		if (myfile_write.fail())cout << " error in opening file  " << fileToWrite << endl;

		float e_x, e_y, e_z, r, p, y;
		//- iterate through path
		//for (int i = actualPathLength - 2; i >= 0; i--) // reverse order
		for (int i = 0; i < actualPathLength - 1; i++)
		{
			// get corresponding joint rotations
			e_x = rotations[i][0];
			e_y = rotations[i][1];
			e_z = rotations[i][2];
			r = rotations[i][3];
			p = rotations[i][4];
			y = rotations[i][5];


			//format as per nachi language
			//sprintf_s(gcode, "MOVEX A=6,AC=0,SM=0,M1J,P,( %1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f),T=0.1,H=3,MS, CONF=0001", e_x, e_y, e_z, r, p, y);
			sprintf_s(gcode, "MOVEX A=6,AC=0,SM=0,M1J,P,( %1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f),S=300.0,H=3,MS, CONF=0001", e_x, e_y, e_z, r, p, y);

			// output string to open file
			myfile_write << gcode << endl;

			if (i % 1000 == 0)
			{
				myfile_write.close();

				string file = "data/MZ07-01-A.08";
				char s[20];
				itoa(counter, s, 10);
				file += s;

				//- open file
				myfile_write.open(file, ios::out);
				if (myfile_write.fail())cout << " error in opening file  " << fileToWrite << endl;

				counter++;
			}
		}

		//-close file
		myfile_write.close();
		cout << "--------------------------- EXPORT GCODE COMPLETE ----------------- " << endl;
	}
	void exportGCode_3dp(string fileToWrite = "data/bl_1.src")
	{

		checkReach();
		int counter = 0;
	
		//----- instance ofstream for file IO
		ofstream myfile_write;
		char gcode[600];

		//- open file
		myfile_write.open(fileToWrite.c_str(), ios::out);
		if (myfile_write.fail())cout << " error in opening file  " << fileToWrite << endl;

		// --------------------------- copy header from base file
		std::fstream fs("data/3DP_header.txt", ios::in);

			if (fs.fail())cout << " error in file reading " << fileToRead << endl;

		int lcnt = 0;
		while (!fs.eof() && lcnt < 1000)
		{
			char str[2000];
			fs.getline(str, 2000);

			myfile_write << str << endl;
			lcnt++;
		}
		fs.close();
		
		// --------------------------- 
		//- iterate through path
		
		vec tcp;
		double scale = 10;
		tcp = path[0][0]; tcp *= scale;
		sprintf_s(gcode, "PTP{ E6POS: X %1.4f, Y %1.4f, Z %1.4f, A 0, B 90, C 0, E1 0, E2 0, E3 0, E4 0, S 'B 110' } C_PTP", tcp.x,tcp.y,tcp.z);
		myfile_write << gcode << endl;
		
		tcp = path[1][0]; tcp *= scale;
		sprintf_s(gcode, "LIN{ E6POS: X %1.4f, Y %1.4f, Z %1.4f, A 0, B 90, C 0, E1 0, E2 0, E3 0, E4 0 }", tcp.x, tcp.y, tcp.z);
		myfile_write << gcode << endl;

		for (int i = 2; i < actualPathLength - 1; i++)
		{
		
			tcp = path[i][0]; tcp *= scale;
			sprintf_s(gcode, "LIN{ E6POS: X %1.4f, Y %1.4f, Z %1.4f, A 0, B 90, C 0, E1 0, E2 0, E3 0, E4 0 } C_DIS", tcp.x, tcp.y, tcp.z);
			myfile_write << gcode << endl; 
		}

		//------close file
		myfile_write << "HALT" << endl;
		myfile_write << "END" << endl;
		myfile_write.close();
		cout << "--------------------------- EXPORT GCODE COMPLETE ----------------- " << endl;
	}


	////////////////////////////////////////////////////////////////////////// DISPLAY METHODS
	void transformRobotMeshes()
	{
		Matrix4 transformMatrix;


		for (int i = 0; i < DOF; i++)
		{
			if (i > 4)continue;

			transformMatrix = Nachi.Bars_to_world_matrices[i];
			for (int j = 0; j < Nachi.link_meshes[i].n_v; j++)Nachi.link_meshes[i].positions[j] = transformMatrix * Nachi.link_meshes[i].positions[j];
		}

		// transform meshes EE --> world
		for (int i = 0; i < E.M.n_v; i++)E.M.positions[i] = Nachi.Bars_to_world_matrices[5] * E.M.positions[i];// to tcip

	}

	void invertTransformRobotMeshes()
	{
		Nachi.invertTransformMeshesToLocal();

		Matrix4 tw = Nachi.Bars_to_world_matrices[5];
		tw.invert();
		for (int i = 0; i < E.M.n_v; i++)E.M.positions[i] = tw * E.M.positions[i];// to tcip
	}
	void drawHistograms( int j ,vec cen = vec (50, winH - 50,0), float r = 5.0)
	{
		AL_glLineWidth(1);
		
		float mn, mx;
		mn = 1e10;
		mx = mn * -1;
		for (int i = 0; i < actualPathLength - 1; i++)
		{
			mn = MIN(mn, rotations[i][j]);
			mx = MAX(mn, rotations[i][j]);
		}
		
		setup2d();



		//glBegin(GL_LINES);
			for (int i = 0; i < actualPathLength - 1; i+= 1 )
			{
				double ang = ((PI * 2.0 / actualPathLength) * float(i)) * 1.0;
				vec4 clr = getColour(rotations[i][j], mn, mx);
				double d = ofMap(rotations[i][j], mn, mx, 0, r);
				d = ofClamp(d,0, r);
				glColor3f(clr.r, clr.g, clr.b);
				drawLine(cen, cen + (vec(sin(ang), cos(ang), 0).normalise() * d));
				vec radial = cen + vec(sin(ang), cos(ang), 0).normalise() * d;
				//glVertex2f(cen.x, cen.y);
				//glVertex2f(radial.x, radial.y);
			}
		//glEnd();

		restore3d();
	}
	void draw(bool drawRobot = true , bool drawEndEffector = true)
	{
		// ------------------- taskGraph
		//taskGraph.draw();

		// ------------------- TCP locations
		glPointSize(3);
		for (int i = 0; i < actualPathLength; i++)
		{
			reachable[i] ? glColor3f(0, 0, 1) : glColor3f(1, 0, 0);
			drawPoint(path[i][0]);
			/*char c[200];
			sprintf(c, "%i ", i);
			string s = "";
			s += c;
			drawString(s, path[i][0]+ vec(0,0,0.2));*/
		}
		glPointSize(1);

		// ------------------- TCP orientation axes

		for (int i = 0; i < actualPathLength; i++)drawFrame(getToolLocation(i), .1);



		//////////////////////////////////////////////////////////////////////////
		// get TOOL information at current point i

		if (drawEndEffector)
		{
			Matrix4 EE = Nachi.Bars_to_world_matrices[5];
			E.drawAtLocation(EE);

			drawFrame(TOOL_req, 3);
			drawCircle(TOOL_req.getColumn(3), 3, 32);
			drawFrame(TOOL, 3);
		}

	
		//////////////////////////////////////////////////////////////////////////
		// ------------------- draw bounding box ;

		wireFrameOn();
			drawCube(min, max);
		wireFrameOff();

		// ------------------- draw Robot ;

		if (drawRobot)Nachi.draw();




		for (int i = 0; i < 6; i++)
			drawHistograms(i, vec( i*200 + 150 , winH - 300,0),75);

	}


};
#endif // !_NACHI_

