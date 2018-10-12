#pragma once
// DESCRIPTION:
//      color essentials
//

#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <stdexcept>

#include<zSpace\zEnumerators.h>

using namespace std;

namespace zSpace
{

	class zSPACE_API zColor
	{
	public:
		double r;  /* red component				*/
		double g;  /* green component			*/
		double b;  /* blue component			*/
		double a;  /* alpha component			*/

		double h;  /* hue component				*/
		double s;  /* saturation component		*/
		double v;  /* value component			*/
		
		//---- CONSTRUCTOR

		// default constructor
		zColor();

		// overloaded constructor RGB-A
		zColor(double _r, double _g, double _b, double _a);

		// overloaded constructor HSV
		zColor(double _h, double _s, double _v);

		//---- DESTRUCTOR

		// default destructor
		~zColor();

		//---- METHODS

		// convert RGB to HSV
		void toHSV();

		// convert HSV to RGB
		void toRGB();


		//---- OPERATORS

		// operator ==  Check if colors are equal 
		bool operator==(zColor &v1 );

	
	};
}
