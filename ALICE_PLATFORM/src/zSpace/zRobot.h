#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <zSpace/zIO.h>


#define  PI 3.14159265359
#define  DOF 6

#define zJoint 0
#define zJointHome 1
#define zJointMaximum 2
#define zJointMinimum 3

#define zRobot_ABB 0
#define zRobot_KUKA 1
#define zRobot_NACHI 2
#define zRobot_YASKAWA 3

#define zRobot_Move_joint 0
#define zRobot_Move_linear 1

#define zRobot_EndEffetor_on 0
#define zRobot_EndEffector_off 1
#define zRobot_EndEffector_neutral 2



namespace zSpace
{

	struct zSPACE_API zDHparameter
	{
		double d;
		double a;
		double alpha;
		double theta;

	};

	struct zSPACE_API zJointRotation
	{
		double rotation;

		double home;

		double minimum;
		double maximum;

		double pulse;
		double mask;
		double offset;
	};

	struct zSPACE_API zGCode
	{
		vector<zJointRotation> rotations;
		double vel;
		int moveType; // 0 = movJ, 1 = movL 

		int endEffectorControl; // 0 - neutral , 1 - on , 2 -0ff
		
		zVector robotTCP_position;
		zVector target_position;	


		double distDifference;
		bool targetReached;

	};

	//!  A link class   
	/*!
	*/
	class zSPACE_API zLink
	{
	public:

		//---- ATTRIBUTES

		Matrix4f T;				/*!<stores transformation matrix of the link */
		Matrix4f TBase;			/*!<stores base transformation matrix of the link */
			
		//---- DH ATTRIBUTES

		zDHparameter linkDH;

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zLink();		

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_positions		- vector of type zVector containing position information of vertices.
		*	\param		[in]	edgeConnects	- edge connection list with vertex ids for each edge
		*/
		zLink(zDHparameter &DH);

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zLink();

		//---- METHODS

		/*! \brief This method update the transform T
		*/
		void updateTransform();	

	};

	//!  A 6 Axis Robot class   
	/*!
	*/

	class zSPACE_API zRobot
	{
	protected:


		//---- PROTECTED METHODS
		
		/*! \brief This method creates the joints links based on the DH Parameters.
		*/
		void createRobotJointLinks();

		/*! \brief This method exports zRobot gCode for ABB robot to TXT file format.
		*
		*	\param [in]		outfilename			- output file name including the directory path and json extension.
		*/
		void exportABB_GCode(string &outfilename);

	public:
		//----  CONSTANT ATTRIBUTES
		double robot_scale = 1.0;

		vector<zDHparameter> robot_DH;
		vector<zJointRotation> jointRotations;
		

		//----  ATTRIBUTES

		vector<zLink> Bars;

		vector<Matrix4f> robotJoint_matrices;
		vector<Matrix4f> robotJointMesh_matrices;

		Matrix4f robot_base_matrix;
		Matrix4f robot_target_matrix;

		Matrix4f robot_endEffector_matrix;



		vector<zGCode> robot_gCode;

		//---- MESH ATTRIBUTES
		vector<zMesh> robot_jointMeshes;
		vector<vector<double>> robot_jointMeshes_edgeAngle;

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zRobot();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_positions		- vector of type zVector containing position information of vertices.
		*	\param		[in]	edgeConnects	- edge connection list with vertex ids for each edge
		*/
		zRobot(vector<double>(&_d), vector<double>(&_a), vector<double>(&_alpha), vector<double>(&_theta), double _robot_scale = 1.0);

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zRobot();

		//---- METHODS

		/*! \brief This Methods sets the home rotations per joint of zRobot to the specified values.
		*
		*	\param		[in]	_homeRotations		- vector containing home rotations per joint of zRobot.
		*/
		void setHomeRotations(vector<double> &_home_Rotations);

		/*! \brief This Methods sets the home rotations per joint of zRobot to the specified values.
		*
		*	\param		[in]	_minRotations		- vector containing minimum rotations per joint of zRobot.
		*	\param		[in]	_maxRotations		- vector containing maximum rotations per joint of zRobot.
		*/
		void setRotationLimits(vector<double> &_min_Rotations, vector<double> &_max_Rotations);

		/*! \brief This Methods sets the rotations to pulse conversion per joint of zRobot to the specified values.
		*
		*	\param		[in]	_rotationsToPulse		- vector containing rotation to pulse conversion value per joint of zRobot.
		*/
		void setRotationsToPulse(vector<double> &_rotationsToPulse);

		/*! \brief This Methods sets the rotation masks per joint of zRobot to the specified values.
		*
		*	\param		[in]	_rotationsBitMask		- vector containing rotation mask per joint of zRobot.
		*/
		void setRotationsMask(vector<double> &_rotationsBitMask);

		/*! \brief This Methods sets the rotation offsets per joint of zRobot to the specified values.
		*
		*	\param		[in]	_rotationsOffsets		- vector containing rotation offsets per joint of zRobot.
		*/
		void setRotationsOffsets(vector<double> &_rotationsOffsets);

		/*! \brief This method sets the rotation per joint of zRobot to the specified values.
		*
		*	\param		[in]	_rotation		- vector containing rotation offsets per joint of zRobot.
		*/
		void setRotations(vector<double> &_rotation);

		/*! \brief This method gets the rotation per joint of zRobot.
		*
		*	\return				rotations		- joint rotations of the  robot stored in a vector of type double.
		*/
		vector<double> getRotations();

		/*! \brief This Methods computes the joint positionsof the zRobot.
		*
		*	\param		[in]	type		- zJoints / zHomeJoints. Specifies if the calulation for joints or joints in home position.
		*/
		void computeJoints();

		//---- FORWARD KINEMATICS

		/*! \brief This Methods computes the forward kinematics chain of zRobot for the specified joint rotation values.
		*
		*	\param		[in]	rotations		- vector containing rotations per joint of zRobot.
		*/
		zVector forwardKinematics(int type = zJoint);

		//---- INVERSE KINEMATICS

		/*! \brief This Methods computes the inverse kinematics chain of zRobot give the target plane as a matrix.
		*
		*	\param		[in]	rotations		- vector containing rotations per joint of zRobot.
		*/
		zVector inverseKinematics();


		zVector  optimiseIK( double &angle, int frame);

		//---- JOINT MESH METHODS
		
		/*! \brief This method imports joint zMesh from input file given in JSON file format.
		*
		*	\param [in]		infilename			- input file name including the directory path.
		*/
		void importOBJ_robotJointMeshes(string directoryName = "C:/Users/vishu.b/Desktop");

		/*! \brief This method update the positions of the joint zMesh based on joint positions.
		*
		*	\param [in]		infilename			- input file name including the directory path.
		*/
		void update_robotJointMeshes();

		
		//---- GCODE METHODS


		/*! \brief This method stores the gcode of the zRobot in a vector of type zGCode.
		*	
		*	\param [in]		inGCode			- output file name including the directory path and json extension.
		*/
		void store_robotGCode(zVector &target_position, double velocity, int moveType = zRobot_Move_joint, int endEffectorControl = zRobot_EndEffector_neutral);

		/*! \brief This method exports zRobot gCode for he specified robot type
		*
		*	\param [in]		outfilename			- output file name including the directory path and json extension.
		*	\param [in]		type				- robot type to work on zRobot_ABB or zRobot_KUKA or zRobot_NACHI or zRobot_YASKAWA
		*/
		void export_robotGCode(string directoryName = "C:/Users/vishu.b/Desktop", int type = zRobot_ABB);

		//---- IMPORT METHODS

		/*! \brief This method imports zRobot attributes and meshes from the input JSON file.
		*
		*	\param		[in]	fileDirectory	- output file directory.  ( e.g | C:/Users/vishu.b/Desktop).
		*	\param		[in]	robotFilename	- output file name including file extension.
		*/
		void importJSON_robot(string directory = "C:/Users/vishu.b/Desktop", string robotFilename = "ABB_IRB_4600.json");

		/*! \brief This method imports robot atributes from input file given in JSON file format.
		*
		*	\param [in]		infilename			- input file name including the directory path.
		*/
		void importJSON_robotAttributes(string infileName = "C:/Users/vishu.b/Desktop/ABB_IRB_4600.json");

		vector<Matrix4f> importTXT_robotTargets(string infileName = "C:/Users/vishu.b/Desktop/print.txt");


		
		
};
}


