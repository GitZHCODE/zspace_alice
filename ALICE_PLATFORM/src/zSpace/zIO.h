#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <zSpace/zExchange.h>

namespace zSpace
{
	//EXPORT
	
	/*! \brief This method exports zGraph Attributes of type zAttributeMap  to JSON file format using JSON Modern Library.
	*
	*	\tparam			T					- Type to work with zGraph & zMesh
	*	\param [in]		outfilename			- output file name including the directory path.
	*/
	template <class T>
	void exportJSON_HE(T &HE_Datastructure, string& outfilename, bool vColors = false);

	/*! \brief This method exports zGraph Attributes of type zAttributeMap  to JSON file format using JSON Modern Library.
	*
	*	\tparam			T					- Type to work with zGraph & zMesh
	*	\param [in]		outfilename			- output file name including the directory path.
	*/
	template <class T>
	void exportJSON_SF(T &SF_Datastructure, string& outfilename);


	zSPACE_API void exportJSON_SlimeLines(zSlime &slime, string& outfilename);

	//IMPORT 

	/*! \brief This method imports HE data structure fromo JSON file format using JSON Modern Library.
	*
	*	\tparam			T					- Type to work with zGraph & zMesh
	*	\param [in]		infilename			- input file name including the directory path and json extension.
	*/
	template <class T>
	void importJSON_HE(T &HE_Datastructure, string& infilename, bool vColors = false);


	/*! \brief This method imports zMesh from  OBJ file.
	*
	*	\param [in]		infilename			- input file name including the directory path and json extension.
	*	\return 		zMesh				- import mesh.
	*/	
	zSPACE_API void importOBJ_HE(zMesh &HE_Datastructure, string &infilename);

	zSPACE_API void importJSON_SlimeLines( vector<zGraph> &inGraphs, string& infilename);
}


// Temmplate method Definition

//This method exports HE data structure to JSON file format using JSON Modern Library.
template<class T>
void zSpace::exportJSON_HE(T& HE_Datastructure, string& outfilename, bool vColors)
{
	zHEtransfer out_HE;
	json j;

	out_HE.to_json <T>(j, HE_Datastructure, vColors);

	ofstream myfile;
	myfile.open(outfilename.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << outfilename.c_str() << endl;
		return;
	}
	
	//myfile.precision(16);
	myfile << j.dump();
	myfile.close();
}


template<class T>
void zSpace::exportJSON_SF(T & SF_Datastructure, string& outfilename)
{
	zScalarFieldTransfer out_SF;
	json j;

	out_SF.to_json <T>(j, SF_Datastructure);

	ofstream myfile;
	myfile.open(outfilename.c_str());

	if (myfile.fail())
	{
		cout << " error in opening file  " << outfilename.c_str() << endl;
		return;
	}
	myfile.precision(16);
	myfile << j.dump();
	myfile.close();
}

//This method imports HE data structure fromo JSON file format using JSON Modern Library.
template<class T>
void zSpace::importJSON_HE(T &HE_Datastructure, string& infilename, bool vColors)
{

	json j_in;
	zHEtransfer in_HE;

	ifstream in_myfile;
	in_myfile.open(infilename.c_str());

	int lineCnt = 0;

	if (in_myfile.fail())
	{
		cout << " error in opening file  " << infilename.c_str() << endl;
		return;
	}

	in_myfile >> j_in;
	in_myfile.close();

	in_HE.from_json <T>(j_in, HE_Datastructure, vColors);


}



