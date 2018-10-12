#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif


#include <zSpace\zUtilities.h>
#include <zSpace\zScalarField.h>

namespace zSpace
{
	class zSPACE_API zSlimeAgent
	{
	public:
		//----  ATTRIBUTES
		zVector pos;		/*!<position of the agent.*/
		zVector dir;		/*!<direction of the agent.*/

		
		
		double *SO;			/*!<Sensor offset of the agent.*/
		double *SA;			/*!<Sensor Angle of the agent.*/
		double *RA;			/*!<Agent rotation angle.*/
		
		double *depT;		/*!<deposition per step.*/
		double *pCD;		/*!<probability of random change in direction.*/
		double *sMin;		/*!<sensitivity threshold.*/


		double mass;		/*!<mass of agent.*/
		zVector force;		/*!<update force of agent.*/

		bool active;
		
		zColor tCol;			/*!<trail color of the agent.*/
		double tWeight;			/*!<trail weight of the agent.*/
		
		int tMax;				/*!<trail maximum points of the agent.*/
		int tCounter;
		vector<zVector> trail; /*!<stores trail positions of agent.*/

		Matrix3d tMat; /*!<stores trail rotation matrix of agent.*/

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zSlimeAgent();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zSlimeAgent();
				

		//---- COMPUTE METHODS

		

	

		//---- GET SET METHODS

		/*! \brief This method returns the new position of the agent based on the forces acting
		*	\param		[in]	dt				- timestep.
		*	\param		[in]	integrateType	- integration method ( zEuler / zDK4).
		*	\return 	zVector					- vector of new position.
		*/
		zVector getNewPosition(double dt, zIntergrationType integrateType = zEuler);

		/*! \brief This method updates the forces of the agent
		*/
		void clearForce();

		/*! \brief This method updates the forces of the agent by adding the input force
		*	\param		[in]	f				- input force.
		*/
		void addForce(zVector &f);

		/*! \brief This method returns the forward direction for the agent.
		*	\return 	zVector				- vector of forward direction.
		*/
		zVector getF();

		/*! \brief This method returns the forward right direction for the agent.
		*	\return 	zVector				- vector of forward right direction.
		*/
		zVector getFR();

		/*! \brief This method returns the forward left direction for the agent.
		*	\return 	zVector				- vector of forward left direction.
		*/
		zVector getFL();

		/*! \brief This method returns the direction for the agent based on input values of F, Fr and FL.
		*	\param		[in]	a_F			- value of chemA in forward direction.
		*	\param		[in]	a_FR		- value of chemA in forward right direction.
		*	\param		[in]	a_FL		- value of chemA in forward left direction.
		*	\return 	zVector				- vector of direction.
		*/
		void setDir(double a_F, double a_FR, double a_FL);

	};


	class zSPACE_API zSlimeEnvironment
	{
	public:
		//----  ATTRIBUTES
		zScalarField2D field;

		int resX;				/*!<field resolution.*/
		int resY;				/*!<field resolution.*/
		double pix;				/*!<pixel size.*/

		vector<double> chemA;		/*!<chemical A in the field. The size of the array should match fieldresoultion.*/
		vector< bool> occupied;		/*!<true if cell is occupied else false. The size of the array should match fieldresoultion.*/
		vector<bool> bAttractants;	/*!<indicates food source indices in environment.*/
		vector<bool> bRepellants;	/*!<indicates food source indices in environment.*/


		double minA, maxA;		/*!<mininmum and maximum value of A.*/
		int id_minA, id_maxA;

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zSlimeEnvironment();

		/*! \brief Overloaded constructor.
		*	\param		[in]	_res		- resolution of environment.
		*	\param		[in]	_pix		- pixel size of environment.
		*/
		zSlimeEnvironment( int _res, double _pix , int _NR = 1);

		/*! \brief Overloaded constructor.
		*	\param		[in]	_res		- resolution of environment.
		*	\param		[in]	_minBB		- bounding box minimum.
		*	\param		[in]	_maxBB		- bounding box maximum.
		*/
		zSlimeEnvironment(int _resX, int _resY, zVector _minBB, zVector _maxBB, int _NR = 1);

		/*! \brief Overloaded constructor.
		*	\param		[in]	_minBB		- bounding box minimum.
		*	\param		[in]	_maxBB		- bounding box maximum.
		*	\param		[in]	_pix		- pixel size of environment.
		*/
		zSlimeEnvironment( zVector _minBB, zVector _maxBB, int _pix, int _NR = 1);

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zSlimeEnvironment();

		//---- GET SET METHODS
		
		/*! \brief This method return the value of chem A at the input position.
		*	\param		[in]	pos		- input position .
		*	\return				double	- value of ChemA.
		*/
		double getChemAatPosition(zVector &pos);


		//---- METHODS

		/*! \brief This method diffuses chemA in the environment.
		*/
		void diffuseEnvironment( double decayT,  double diffuseDamp , zDiffusionType diffType = zAverage);



		//---- UTILITY METHODS

		/*! \brief This method computes the minimum and maximum value of chemcial A in the environment.
		*/
		void minMax_chemA(double min = 0, double max = 1);

	

		
	};


	class zSPACE_API zSlime
	{
	public:
		//----  ATTRIBUTES
		zSlimeEnvironment environment;		/*!<slime environment .*/
		vector<zSlimeAgent>  agents;		/*!<slime agents.*/
		vector<int> attractants;			/*!<indicates attractant food source indices in environment.*/
		vector<int> repellants;			/*!<indicates repellant food source indices in environment.*/

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/					
		zSlime();


		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zSlime();

		//---- METHODS

		/*! \brief This method defines the motor stage of the simulation.
		*/
		void slime_Motor(double dt, zIntergrationType integrateType = zPixel , bool agentTrail = false);

		/*! \brief This method defines the motor stage of the simulation.
		*/
		void slime_Sensor();

		/*! \brief This method contains the agent with in the bounds of the environment.
		*	\param		[in]	agent		- input zSlimeAgent .
		*/
		void containBounds(zSlimeAgent &agent, double dt, zIntergrationType integrateType);

		/*! \brief This method adds a food source at the input index of the environment.
		*	\param		[in]	id		- environment ID to be made a fod source.
		*	\param		[in]	depA	- deposition rate of chem A at food source .
		*/
		bool addFoodSource(int id, double depA, int nR = 1, bool attractant = true);

		/*! \brief This method adds a repellant food source at boundary cells of the environment.
		*	\param		[in]	depA				- deposition rate of chem A at food source .
		*	\param		[in]	distFromBoundary	- number of cells from the boundary to made a food source.
		*/
		void makeBoundaryRepellant(double depA, int distFromBoundary);

		/*! \brief This method deposits chemical A at the inpt environment Id.
		*	\param		[in]	id		- environment ID.
		*	\param		[in]	depA	- deposition rate of chem A.
		*/
		void depositChemicalA(int id, double depA);

		void depositAtFoodSource(double depA, double wProj);

		int randomUnoccupiedCell(int boundaryOffset = 3);

		void clearFoodSources( bool Attractants = true, bool Repelants = true);

		void randomRemoveAgents(int minNum );

		//---- UTILITY METHODS

		/*! \brief This method computes the color value of each cell in the environment based on chemical A.
		*	\param		[out]	minA		- minimum of chemical A .
		*	\param		[out]	maxA		- maximum of chemical A .
		*/
		void computeEnvironmentColor(vector<zSlimeAgent> &agents, bool dispAgents = false);


		void updateAgentParametersFromField( int agentId, vector<double> &fieldScalars, double &parameterValue ,double minVal, double maxVal, zSlimeParameter parm);
		

		zVector computeGradientForce(int agentId, vector<double> &fieldScalars);
	};
}

