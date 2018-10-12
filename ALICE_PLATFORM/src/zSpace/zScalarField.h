// DESCRIPTION:
//      Scalar Field
//

#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include<zSpace/zVector.h>
#include <vector>



namespace zSpace
{
	

	struct zSPACE_API zScalar
	{
		int id;

		zVector position;
		zVector direction;
		
		double weight;
		zColor color;

		vector<int> ringNeighbours;
		vector<int> adjacentNeighbours;

	};

	class zSPACE_API zScalarField2D
	{
	public:

		int n_X;
		int n_Y;

		double unit_X;
		double unit_Y;

		zVector minBB; 
		zVector maxBB;


		vector<zScalar> scalars;

		//---- CONSTRUCTOR

		// default constructor
		zScalarField2D();

		// over loaded constructor
		zScalarField2D( vector<zVector>(&_positions), int _NR = 1);

		// over loaded constructor
		zScalarField2D(zVector _minBB, zVector _maxBB, int _n_X, int _n_Y , int _NR = 1);

		/*! \brief Overloaded constructor.
		*	\param		[in]	_unit_X		- size of each pixel in x direction.
		*	\param		[in]	_unit_Y		- size of each pixel in y direction.
		*	\param		[in]	_n_X		- number of pixels in x direction.
		*	\param		[in]	_n_Y		- number of pixels in y direction.
		*/
		zScalarField2D(double _unit_X, double _unit_Y, int _n_X, int _n_Y ,  int _NR = 1);

		//---- DESTRUCTOR

		// default destructor
		~zScalarField2D();

		//---- GET SET METHODS

		// get  numScalars
		int getNumScalars();

		// set bounding box
		void setBoundingBox(zVector &_minBB , zVector &_maxBB);

		// get bounding box
		void getBoundingBox(zVector &_minBB, zVector &_maxBB);

		// set position
		void setPosition(zVector &pos, int index);

		// get position
		zVector getPosition(int index);

		// set direction
		void setDirection(zVector &dir, int index);

		// get direction
		zVector getDirection(int index);

		// set weight
		void setWeight(double weight, int index);

		// get weight
		double getWeight(int index);

		// set color
		void setColor(zColor col, int index);

		// get color
		zColor getColor(int index);
		
		// get  index from position
		bool getIndex(zVector &pos, int &outId);

		// get  indices from position
		void getIndices(zVector &pos, int &index_X, int &index_Y);

		// get neighbourhood ring
		vector<int> getNeighbourHoodRing(int index, int numRings = 0);

		vector<int> getNeighbourAdjacents(int index);

	

		
		
};



}


