#pragma once

#include <vector>
#include <algorithm>    // std::sort
#include <map>
#include <unordered_map>
#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string
#include <fstream>
using namespace std;

namespace zSpace
{
	//!   zAttribute template class defined to hold attribute information. 
	/*!
	*/
	template <class T>
	class zAttribute
	{
	public:

		string attributeName;	/*!< stores attribute name of the zAttribute. */
		T value;				/*!< stores attribute value of the zAttribute. */

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zAttribute();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zAttribute();

		//---- GET-SET METHODS
	
		/*! \brief This method returns the attribute name of the zAttribute.
		*	\return				attributeName.
		*/
		string getAttributeName();

		/*! \brief This method sets the attribute name of the zAttribute.
		*	\param		[in]	attributeName.
		*/
		void setAttributeName(string _name);

		/*! \brief This method returns the attribute value associated with the zAttribute.
		*	\return				attributeValue.
		*/
		T getValue();

		/*! \brief This method sets the attribute value associated with the zAttribute.
		*	\param		[in]	attributeValue.
		*/
		void setValue(T _value);		
		
	};

	//!   zAttributeVector template class defined to hold attribute information in a vector. 
	/*!
	*/
	template <class T>
	class zAttributeVector
	{
	public:

		string attributeName;	/*!< stores attribute name of the zAttributeVector.	*/
		vector<T> values;		/*!< stores attribute values of the zAttributeVector.	*/

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zAttributeVector();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zAttributeVector();

		//---- GET-SET METHODS

		/*! \brief This method returns the attribute name of the zAttributeVector.
		*	\return				attributeName.
		*/
		string getAttributeName();

		/*! \brief This method sets the attribute name of the zAttributeVector.
		*	\param		[in]	attributeName.
		*/
		void setAttributeName(string _name);

		/*! \brief This method returns the attribute value associated with the zAttribute at the input index.
		*	\param		[in]	index.
		*	\return				attributeValue.
		*/
		T getValue(int index);

		/*! \brief This method sets the attribute value associated with the zAttribute at the input index.
		*	\param		[in]	attributeValue.
		*	\param		[in]	index.
		*/
		void setValue(T _value, int index);

		/*! \brief This method returns the attribute values associated with the zAttributeVector.
		*	\return				attributeValues.
		*/
		vector<T> getValues();

		/*! \brief This method sets the attribute values associated with the zAttributeVector.
		*	\param		[in]	attributeValues.
		*/
		void setValues(vector<T> _values);

		/*! \brief This method clears the attribute values associated with the zAttributeVector.
		*/
		void clearValues();

		/*! \brief This method returns the size of attribute values associated with the zAttributeVector.
		*	\return				size of attributeValues.
		*/
		int getValuesSize();

		void addAttributeValue(T _value);

		
	};

	//!   zAttributeUnorderedMap template struct defined to hold attribute information as an unordered map. 
	/*!
	*/
	template <class T, class U>
	struct zAttributeUnorderedMap
	{
		string attributeName;
		unordered_map<T, U> map;
	};

	//!   zAttributeMap template struct defined to hold attribute information in as a map. 
	/*!
	*/
	template <class T, class U>
	struct zAttributeMap
	{
		string attributeName;
		map<T, U> map;
	};

	////!   zAttributeMap template struct defined to hold attribute information in as a map. 
	///*!
	//*/
	//template <class T>
	//struct zAttributeScalarField
	//{
	//	vector<T> values;
	//};

}



// INLINE definition

template<class T>
inline zSpace::zAttribute<T>::zAttribute()
{
	this->attributeName = "NoName";
}

template<class T>
inline zSpace::zAttribute<T>::~zAttribute()
{
}

template<class T>
inline string zSpace::zAttribute<T>::getAttributeName()
{
	return this->attributeName;
}

template<class T>
inline void zSpace::zAttribute<T>::setAttributeName(string _name)
{
	this->attributeName = _name;
}

template<class T>
inline T zSpace::zAttribute<T>::getValue()
{
	return this->value;
}

template<class T>
inline void zSpace::zAttribute<T>::setValue(T _value)
{
	this->value = _value;
}

// ---- zATTRIBUTEVECTOR

template<class T>
inline zSpace::zAttributeVector<T>::zAttributeVector()
{
	this->attributeName = "NoName";
	this->values.clear();
}

template<class T>
inline zSpace::zAttributeVector<T>::~zAttributeVector()
{
}

template<class T>
inline string zSpace::zAttributeVector<T>::getAttributeName()
{
	return this->attributeName;
}

template<class T>
inline void zSpace::zAttributeVector<T>::setAttributeName(string _name)
{
	this->attributeName = _name;

}

template<class T>
inline T zSpace::zAttributeVector<T>::getValue(int index)
{
	return values[index];
}

template<class T>
inline void zSpace::zAttributeVector<T>::setValue(T _value, int index)
{
	values[index] = _value;
}

template<class T>
inline vector<T> zSpace::zAttributeVector<T>::getValues()
{
	return values;
}

template<class T>
inline void zSpace::zAttributeVector<T>::setValues(vector<T> _values)
{
	values = _values;
}

template<class T>
inline void zSpace::zAttributeVector<T>::clearValues()
{
	this->values.clear();
}

template<class T>
inline int zSpace::zAttributeVector<T>::getValuesSize()
{
	return values.size();
}

template<class T>
inline void zSpace::zAttributeVector<T>::addAttributeValue(T _value)
{
	values.push_back(_value);
}
