#pragma once
// DESCRIPTION:
//      Vector essentials
//

#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <stdexcept>
#include <zSpace/zColor.h>
#include <vector>
using namespace std;

#define PI 3.14159265

namespace zSpace
{
	class zSPACE_API zVector
	{
	public:
		double x;			/*!< x component				*/
		double y;			/*!< y component				*/
		double z;			/*!< z component				*/

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zVector();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_x		- x component of the zVector.
		*	\param		[in]	_z		- y component of the zVector.
		*	\param		[in]	_z		- z component of the zVector.
		*/
		zVector(double _x, double _y, double _z);

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zVector();

		//---- OPERATORS

		/*! \brief This operator checks for equality of two zVectors.
		*
		*	\param		[in]	v1		- zVector against which the equality is checked.
		*	\return				bool	- true if valency is equal to input valence.
		*/
		bool operator==(zVector v1);

		/*! \brief This operator is used for vector addition.
		*
		*	\param		[in]	v1		- zVector which is added to the current vector.
		*	\return				zVector	- resultant vector after the addition.
		*/
		zVector operator+(zVector v1);

		/*! \brief This operator is used for vector subtraction.
		*
		*	\param		[in]	v1		- zVector which is subtracted from the current vector.
		*	\return				zVector	- resultant vector after the subtraction.
		*/
		zVector operator -(zVector v1);

		/*! \brief This operator is used for vector dot product.
		*
		*	\param		[in]	v1		- zVector which is used for the dot product with the current vector.
		*	\return				double	- resultant value after the dot product.
		*/
		double operator *(zVector v1);

		/*! \brief This operator is used for vector cross procduct.
		*
		*	\param		[in]	v1		- zVector which is used for the cross product with the current vector.
		*	\return				zVector	- resultant vector after the cross product.
		*/
		zVector operator ^(zVector v1);

		/*! \brief This operator is used for scalar addition of a vector.
		*
		*	\param		[in]	val		- scalar value to be added to the current vector.
		*	\return				zVector	- resultant vector after the scalar addition.
		*/
		zVector operator +(double val);

		/*! \brief This operator is used for scalar subtraction of a vector.
		*
		*	\param		[in]	val		- scalar value to be subtracted from the current vector.
		*	\return				zVector	- resultant vector after the scalar subtraction.
		*/
		zVector operator -(double val);

		/*! \brief This operator is used for scalar muliplication of a vector.
		*
		*	\param		[in]	val		- scalar value to be multiplied with the current vector.
		*	\return				zVector	- resultant vector after the scalar multiplication.
		*/
		zVector operator *(double val);

		/*! \brief This operator is used for scalar division of a vector.
		*
		*	\param		[in]	val		- scalar value used to divide from the current vector.
		*	\return				zVector	- resultant vector after the scalar division.
		*/
		zVector operator /(double val);

		//---- OVERLOADED OPERATORS

		/*! \brief This overloaded operator is used for vector addition and assigment of the result to the current vector.
		*
		*	\param		[in]	v1		- zVector which is added to the current vector.
		*/
		void operator +=(zVector v1);

		/*! \brief This overloaded operator is used for vector subtraction and assigment of the result to the current vector.
		*
		*	\param		[in]	v1		- zVector which is subtacted from the current vector.
		*/
		void operator -=(zVector v1);

		
		/*! \brief This overloaded operator is used for scalar addition and assigment of the result to the current vector.
		*
		*	\param		[in]	val		- scalar value to be added to the current vector.
		*/
		void operator +=(double val);

		/*! \brief This overloaded operator is used for scalar subtraction and assigment of the result to the current vector.
		*
		*	\param		[in]	val		- scalar value to be sbtracted from the current vector.
		*/
		void operator -=(double val);

		/*! \brief This overloaded operator is used for scalar multiplication and assigment of the result to the current vector.
		*
		*	\param		[in]	val		- scalar value to be multiplied to the current vector.
		*/
		void operator *=(double val);

		/*! \brief This overloaded operator is used for scalar division and assigment of the result to the current vector.
		*
		*	\param		[in]	val		- scalar value used to divide from the current vector.
		*/
		void operator /=(double val);

		//---- METHODS


		/*! \brief This method returns the magnitude/length of the zVector.
		*
		*	\return				double		- value of the maginute of the vector.
		*/
		double length();

		/*! \brief This method normalizes the vector to unit length.
		*
		*/
		void normalize();

		/*! \brief This method returns the distance between the current zVector and input zVector.
		*
		*	\return				double		- value of the distance between the vectors.
		*/
		double distanceTo(zVector v1);

		/*! \brief This method returns the angle between the current zVector and input zVector.
		*
		*	\return				double		- value of the angle between the vectors.
		*/
		double angle(zVector v1);

		/*! \brief This method returns the dihedral angle between the two input zVectors using current zVector as edge reference.
		*
		*	\return				double		- value of the dihedral angle between the vectors.
		*/
		double dihedralAngle(zVector v1, zVector v2);	

	
		/*! \brief This method returns the component value of the current zVector.
		*
		*	\param		[in]	i			- index. ( 0 - x component, 0 - y component, 2 - z component).
		*	\return				double		- value of the dihedral angle between the vectors.
		*/
		double  getComponent(int i);


	
	};
}

