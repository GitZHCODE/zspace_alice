//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#include <headers/zApp/include/zTsDigitalFabrication.h>

using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General


double background = 0.9;
double increment = 0.1;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;
zObjMeshArray r_meshObjs;
zObjGraph r_graphObj;

zTransform robotTarget;
zTransform robotEE;

string robName = "ABB_IRB_4600";
string Actions;

string dirPath = "data/ABB_IRB_4600";
vector<string> robotFile;


/*!<Toolsets*/
zTsRobot myRobot;

/*!<GUI*/

bool zRobot_IMPORT = true;

bool zRobot_IK = false;


////// --- GUI OBJECTS ----------------------------------------------------


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(2000000);

	// zROBOT import

	core.getFilesFromDirectory(robotFile, dirPath, zJSON);

	int nF = core.getNumfiles_Type(dirPath, zOBJ);
	if (nF < 8) nF = 8;

	r_meshObjs.assign(nF, zObjMesh());

	myRobot = zTsRobot(r_graphObj, r_meshObjs);

	cout << endl << robotFile[0];
	myRobot.createRobotfromFile(robotFile[0], zJSON);

	//myRobot.createTargetsfromFile(dirPath + "/targets.txt", zTXT);
	//printf("\n %i ", myRobot.robotTargets.size());

	myRobot.createRobotJointMeshesfromFile(dirPath, zOBJ, true);

	/////////////////////

	// set target transform
	
	robotTarget.setIdentity();
	robotTarget(0, 0) = -1; robotTarget(0, 1) = 0; robotTarget(0, 2) = 0;
	robotTarget(1, 0) = 0; robotTarget(1, 1) = 1; robotTarget(1, 2) = 0;
	robotTarget(2, 0) = 0; robotTarget(2, 1) = 0; robotTarget(2, 2) = -1;
	robotTarget(3, 0) = 2.0; robotTarget(3, 1) = 0; robotTarget(3, 2) = 0;

	// set EE transform
	robotEE.setIdentity();

	robotEE(0, 0) = 0; robotEE(0, 1) = 0; robotEE(0, 2) = -1;
	robotEE(1, 0) = 0; robotEE(1, 1) = 1; robotEE(1, 2) = 0;
	robotEE(2, 0) = 1; robotEE(2, 1) = 0; robotEE(2, 2) = 0;
	robotEE(3, 0) = -0.2; robotEE(3, 1) = 0; robotEE(3, 2) = -0.346;


	myRobot.setEndEffector(robotEE);


	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(r_graphObj);

	for (int i = 0; i < r_meshObjs.size(); i++)
	{
		model.addObject(r_meshObjs[i]);

	}

	// set display element booleans
	for (int i = 0; i < r_meshObjs.size(); i++)
	{
		r_meshObjs[i].setDisplayElements(false, true, true);


		if (i != 7) r_meshObjs[i].appendToBuffer(myRobot.robot_jointMeshes_edgeAngle[i], true, 90);
		else r_meshObjs[i].appendToBuffer();

		r_meshObjs[i].setDisplayTransform(true);

	}

	model.setShowBufLines(true, false);
	model.setShowBufTris(true, true);

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();

	S.addSlider(&myRobot.jointRotations[0].rotation, "J1");
	S.addSlider(&myRobot.jointRotations[1].rotation, "J2");
	S.addSlider(&myRobot.jointRotations[2].rotation, "J3");
	S.addSlider(&myRobot.jointRotations[3].rotation, "J4");
	S.addSlider(&myRobot.jointRotations[4].rotation, "J5");
	S.addSlider(&myRobot.jointRotations[5].rotation, "J6");

	S.sliders[0].attachToVariable(&myRobot.jointRotations[0].rotation, myRobot.jointRotations[0].minimum, myRobot.jointRotations[0].maximum);
	S.sliders[1].attachToVariable(&myRobot.jointRotations[1].rotation, myRobot.jointRotations[1].minimum, myRobot.jointRotations[1].maximum);
	S.sliders[2].attachToVariable(&myRobot.jointRotations[2].rotation, myRobot.jointRotations[2].minimum, myRobot.jointRotations[2].maximum);
	S.sliders[3].attachToVariable(&myRobot.jointRotations[3].rotation, myRobot.jointRotations[3].minimum, myRobot.jointRotations[3].maximum);
	S.sliders[4].attachToVariable(&myRobot.jointRotations[4].rotation, myRobot.jointRotations[4].minimum, myRobot.jointRotations[4].maximum);
	S.sliders[5].attachToVariable(&myRobot.jointRotations[5].rotation, myRobot.jointRotations[5].minimum, myRobot.jointRotations[5].maximum);


	S.addSlider(&background, "background");

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&zRobot_IMPORT, "Import");

	B.addButton(&zRobot_IK, "IK");
		

}

void update(int value)
{
	
	if (zRobot_IK)
	{
		myRobot.setTarget(robotTarget);
		zVector pos = myRobot.inverseKinematics();
	}

	if (!zRobot_IK)
	{
		zVector pos = myRobot.forwardKinematics(zJoint);
	}

	// update buffer
	for (int i = 0; i < DOF + 2/*myRobot.fnMeshJoints.size()*/; i++)
	{

		if (myRobot.fnMeshJoints[i].numVertices() == 0) continue;

		model.displayUtils.bufferObj.updateVertexPositions(myRobot.fnMeshJoints[i].getRawVertexPositions(), myRobot.fnMeshJoints[i].numVertices(), myRobot.fnMeshJoints[i].getVBOVertexIndex());
	}
		
		
	
}

void draw()
{
	backGround(background);
	drawGrid(50);

	S.draw();
	B.draw();

	
	// zspace model draw
	model.draw();
		
	model.displayUtils.drawTransform(robotTarget, 1);


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	//drawString("Vectors #:" + to_string(numVectors), vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'w')
	{
		robotTarget(3, 0) += increment;
	}
	if (k == 'W')robotTarget(3, 0) -= increment;

	if (k == 'y') robotTarget(3, 1) += increment;
	if (k == 'Y') robotTarget(3, 1) -= increment;

	if (k == 'z') robotTarget(3, 2) += increment;
	if (k == 'Z') robotTarget(3, 2) -= increment;

	if (k == 'p')
	{

		zTransform originalTargetMatrix = robotTarget;


		zVector rtarget_X(originalTargetMatrix(0, 0), originalTargetMatrix(1, 0), originalTargetMatrix(2, 0));
		zVector rtarget_Y(originalTargetMatrix(0, 1), originalTargetMatrix(1, 1), originalTargetMatrix(2, 1));
		zVector rtarget_Z(originalTargetMatrix(0, 2), originalTargetMatrix(1, 2), originalTargetMatrix(2, 2));


		zVector new_rtarget_Y = rtarget_Y.rotateAboutAxis(rtarget_Z, 10);
		zVector new_rtarget_X = rtarget_X.rotateAboutAxis(rtarget_Z, 10);

		//robotTarget(0, 2) = new_rtarget_Z.x; robotTarget(1, 2) = new_rtarget_Z.y; robotTarget(2, 2) = new_rtarget_Z.z;
		robotTarget(0, 1) = new_rtarget_Y.x; robotTarget(1, 1) = new_rtarget_Y.y; robotTarget(2, 1) = new_rtarget_Y.z;
		//zOperateRobot.robot_target_matrix(0, 1) = new_rtarget_Y.x; zOperateRobot.robot_target_matrix(1, 1) = new_rtarget_Y.y; zOperateRobot.robot_target_matrix(2, 1) = new_rtarget_Y.z;
		robotTarget(0, 0) = new_rtarget_X.x; robotTarget(1, 0) = new_rtarget_X.y; robotTarget(2, 0) = new_rtarget_X.z;



	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
