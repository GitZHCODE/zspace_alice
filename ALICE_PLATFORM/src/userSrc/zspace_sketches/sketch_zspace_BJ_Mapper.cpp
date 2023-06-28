//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMeshArray oMeshes;


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

	// read mesh
	
	int id = 7;
	cout << "Please enter blockid: ";
	cin >> id;

	// read program

	zStringArray lineStrings;
	ifstream myfile_1;
	string fN_02 = "data/BJ_mapper/" + to_string(id) + "_print.txt";
	myfile_1.open(fN_02);

	int lineID = 0;

	bool chkFile = true;
	if (myfile_1.fail())
	{
		cout << " error in opening file  " << fN_02.c_str() << endl;
		chkFile = false;
	}

	if (chkFile)
	{
		while (!myfile_1.eof())
		{
			string str;
			getline(myfile_1, str);

			lineStrings.push_back(str);

			//if (lineID == 0) printf("\n mapID %i %s ", lineID, lineStrings[0].c_str());
			lineID++;
		}

		//printf("\n lineID %i mapIDs =  %i ", lineID, lineStrings.size());

		myfile_1.close();
	}

	// read iDS
	zIntArray mapIDs;
	zIntArray uniqueMAPIDs;

	ifstream myfile;
	string fN_01 = "data/BJ_mapper/" + to_string(id) + "_map.txt";
	myfile.open(fN_01);

	lineID = 0;

	chkFile = true;
	if (myfile.fail())
	{
		cout << " error in opening file  " << fN_01.c_str() << endl;
		chkFile = false;
	}

	if (chkFile)
	{
		while (!myfile.eof())
		{
			string str;
			getline(myfile, str);

			mapIDs.push_back(atoi(str.c_str()));

			bool chkRepeat = false;
			for (int i = 0; i < uniqueMAPIDs.size(); i++)
			{
				if (mapIDs[lineID] == uniqueMAPIDs[i])
				{
					chkRepeat = true;
					break;
				}
			}

			if (!chkRepeat)
			{
				uniqueMAPIDs.push_back(mapIDs[lineID]);				
			}

			//printf("\n mapID %i %i ", lineID, mapIDs[lineID]);
			lineID++;
		}

		myfile.close();
		//printf("\n lineID %i mapIDs =  %i ", lineID, mapIDs.size());
	}
	
	


	// map

	zStringArray outStrings;
	zDoubleArray uniqueSpeeds;
	zBoolArray uniqueMAPIDs_Visited;
	uniqueMAPIDs_Visited.assign(uniqueMAPIDs.size(), false);

	ifstream myfile_2;
	myfile_2.open(fN_02);

	int TASKcount = 0;
	while (!myfile_2.eof())
	{
		string str;
		getline(myfile_2, str);

		string delimiter_parent = " ";
		zStringArray splitText_parent = core.splitString(str, delimiter_parent);
		
		if (splitText_parent.size() > 0)
		{
			if (splitText_parent[0] == "TASK")
			{
				if (TASKcount > 0 )
				{
					if (splitText_parent[2] == "speeddata")
					{
						string delimiter_child_0 = ":";
						zStringArray splitText_child_0 = core.splitString(splitText_parent[3], delimiter_child_0);

						string delimiter_child_1 = "=[";
						zStringArray splitText_child_1 = core.splitString(splitText_child_0[1], "=[");

						string delimiter_child_2 = ",";
						zStringArray splitText_child_2 = core.splitString(splitText_child_1[0], delimiter_child_2);

						double speed = atof(splitText_child_2[0].c_str());

						bool chkRepeat = false;
						for (int i = 0; i < uniqueSpeeds.size(); i++)
						{
							if (speed == uniqueSpeeds[i])
							{
								chkRepeat = true;
								break;
							}
						}

						if (!chkRepeat)
						{
							uniqueSpeeds.push_back(speed);
						}

						string delimiter_child_3 = " Speed";
						zStringArray splitText_child_3 = core.splitString(splitText_child_0[0], "Speed");

						int id = atoi(splitText_child_3[0].c_str());
						int mapID = mapIDs[TASKcount - 1];


						if (!chkRepeat)
						{
							string out = splitText_parent[0] + delimiter_parent + splitText_parent[1] + delimiter_parent + splitText_parent[2] + delimiter_child_3 + to_string(mapID) + delimiter_child_0 + splitText_child_0[1];
							outStrings.push_back(out);
						}
					}
					else
					{
						outStrings.push_back(str);
					}

					
				}
				TASKcount++;
			}
			else if (splitText_parent[0] == "MoveL")
			{
				string delimiter_child_0 = ",";
				zStringArray splitText_child_0 = core.splitString(splitText_parent[1], delimiter_child_0);

				string delimiter_child_1 = "Speed";
				zStringArray splitText_child_1 = core.splitString(splitText_child_0[9], delimiter_child_1);

				int id = atoi(splitText_child_1[0].c_str());
				string out = splitText_parent[0];
				for (int i = 0; i < 9; i++)
				{
					out+= (splitText_child_0[i] + delimiter_child_0);
				}
				out +=  (delimiter_child_1 + to_string(mapIDs[id]) + delimiter_child_0 + splitText_child_0[10] + delimiter_child_0 + splitText_child_0[11] + " " + splitText_parent[2]);
				outStrings.push_back(out);


			}
			else
			{
				 outStrings.push_back(str);
			}
			
			
		}
		
	
	}

	myfile_2.close();
	

	// output file
	ofstream myfile_3;

	string fN_03 = "data/BJ_mapper/" + to_string(id) + "_print_out.txt";
	myfile_3.open(fN_03.c_str());

	if (myfile_3.fail())
	{
		cout << " error in opening file  " << fN_03.c_str() << endl;
		return;
	}

	for (auto& s : outStrings)
	{
		myfile_3 << "\n" << s.c_str();
		//printf("\n %s ", s.c_str());
	}
		
	myfile_3.close();

	cout << "file exported : " << fN_03.c_str() << endl;

	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	

	// set display element booleans
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{
		
		compute = !compute;	
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
		// zspace model draw
		model.draw();
		
	}

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;	
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
