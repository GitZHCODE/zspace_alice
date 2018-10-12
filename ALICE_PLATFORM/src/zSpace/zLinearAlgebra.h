#pragma once
#pragma once


#include "mkl.h"
#include "daal.h"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Eigen>
#include <Eigen/Sparse>
#include <vector>
using namespace Eigen;

using namespace daal;
using namespace daal::algorithms;


#include <zSpace/zGraph.h>
#include <zSpace/zMesh.h>

namespace zSpace
{
	template <class T>
	struct MKL_sparseMatrix
	{
		T *values;
		int *columns;
		int *pointerB;
		int *pointerE;

		int numColumns;
		int numRows;
	};

	/*! \brief This methods create branch - node marix as per http://www.sciencedirect.com/science/article/pii/0045782574900450
	*
	*	\tparam		[in]	T			- Type to work on Eigen matrix type : MatrixXd /SparseMatrix<double>.
	*	\param		[in]	type		- zGraphVertex/ zMeshVertex.
	*	\return				MatrixXd	- edge-node matrix.
	*/
	template <class T, class U>
	void getEdgeNodeMatrix(T &inDatastructure, U *out_edgeNodeMatrix );

	template <class T, class U>
	MKL_sparseMatrix<T> getEdgeNodeSparseMatrix(U &inDatastructure);

	/*!	\brief This method returns the submatrix of the input matrix base
	*
	*	\param		[in]	forceDensities		- zAttributeVector storing the force densiities per edge.
	*	\param		[in]	fixedVertices		- vector of type bool storing information if the vertex is a fixed or free.
	*	\param		[in]	load				- load value.
	*	\param		[in]	type				- zGraphVertex/ zMeshVertex.
	*	\return				bool				- true if solver was successful else false.
	*/
	template <class T>
	bool FDM(T &inDatastructure, zAttributeVector<double> &forceDensities, vector<int> &fixedVertices, double load = 1.0, zHEData type = zGraphVertex);

}

