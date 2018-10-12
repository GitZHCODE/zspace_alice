// DESCRIPTION:
//      Half-edge graph 
//


#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <zSpace/zUtilities.h>


#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Eigen>
#include <Eigen/Sparse>




using namespace Eigen;
using namespace std;

typedef Eigen::SparseMatrix<double> SpMat; // declares a column-major sparse matrix type of double
typedef Eigen::Triplet<double> T;
typedef DiagonalMatrix<double, Eigen::Dynamic, Eigen::Dynamic> Diag;

#define zMatrixColumns 0
#define zMatrixRows 1


#define zTXT 0
#define zJSON 1
#define zOBJ 2


namespace zSpace
{

	//!  A graph class using half-edge data structure  
	/*!
	 */
	class zSPACE_API zGraph
	{

	protected:

		//---- PROTCTED ATTRIBUTES

		int n_v;												/*!<stores number of vertices */
		int n_e;												/*!<stores number of edges */

		int max_n_v;
		int max_n_e;


		//---- PROTECTED METHODS

		/*! \brief This method resizes the array connected with the input type to the specified newSize.
		*
		*	\param		[in]	newSize			- new size of the array.
		*	\param		[in]	type			- zGraphVertex/zMeshVertex or zGraphEdge/zMeshEdge or zMeshFace.
		*/
		void resizeArray(int newSize, zHEData type = zVertexData);
	

	public:

		//----  ATTRIBUTES
	

		zVertex *vertices;										/*!<vertex list			*/
		zEdge *edges;											/*!<edge list			*/

		vector<zVector> vertexPositions;				/*!<list which stores vertex positions.			*/
		
		unordered_map <string, int> verticesEdge;		/*!<vertices to edgeId map. Used to check if edge exists with the haskey being the vertex sequence.	 */
		unordered_map <string, int> positionVertex;		/*!<position to vertexId map. Used to check if vertex exists with the haskey being the vertex position.	 */


		// colors
		vector<zColor> vertexColors;					/*!<list which stores vertex colors.	*/
		vector<zColor> edgeColors;						/*!<list which stores edge colors.	*/

		// weights
		vector <double> vertexWeights;					/*!<list which stores vertex weights.	*/
		vector	<double> edgeWeights;					/*!<list which stores edge weights.	*/

		// active
		vector <bool> vertexActive;					/*!<list which stores vertex weights.	*/
		vector<bool> edgeActive;					/*!<list which stores edge weights.	*/


		//bounding box
		zVector minBB;
		zVector maxBB;


		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		 */
		zGraph();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_positions		- vector of type zVector containing position information of vertices.
		*	\param		[in]	edgeConnects	- edge connection list with vertex ids for each edge
		*/
		zGraph(vector<zVector>(&_positions), vector<int>(&edgeConnects));

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zGraph();

		//---- GET-SET METHODS

		/*! \brief This method gets the edges connected to input zVertex or zEdge.
		*
		*	\param		[in]	index			- index in the vertex/edge list.
		*	\param		[in]	type			- zVertexData or zEdgeData.
		*	\param		[out]	edgeIndicies	- vector of edge indicies.
		*/
		void getConnectedEdges(int index, zHEData type, vector<int>& edgeIndicies);

		/*! \brief This method gets the vertices connected to input zGraphVertex or zMeshVertex.
		*
		*	\param		[in]	index			- index in the vertex list.
		*	\param		[in]	type			- zGraphVertex or zMeshVertex.
		*	\param		[out]	vertexIndicies	- vector of vertex indicies.
		*/
		void getConnectedVertices(int index, zHEData type, vector<int>& vertexIndicies);


		/*!	\brief This method gets the vertices attached to input zEdge.
		*
		*	\param		[in]	index			- index in the edge list.
		*	\param		[in]	type			- zEdgeData.
		*	\param		[out]	vertexIndicies	- vector of vertex indicies.
		*/
		void getVertices(int index, zHEData type, vector<int>& vertexIndicies);

		/*!	\brief This method calculate the valency of the input zVertex.
		*
		*	\param		[in]	index	- index in the vertex list.
		*	\return				int		- valency of input vertex input valence.
		*/
		int getVertexValence(int index);

		/*!	\brief This method determines if input zVertex is on the boundary ie vertex with valency one.
		*
		*	\param		[in]	index	- index in the vertex list.
		*	\param		[in]	valence	- input valence value.
		*	\return				bool	- true if valency is equal to input valence.
		*/
		bool checkVertexValency(int index, int valence = 1);

		//--- ATTRIBUTES 
		
		void computeEdgeColorfromVertexColor();


		//--- COMPUTE METHODS 

		/*! \brief This method averages the positions of vertex except for the ones on the boundary.
		*
		*	\param		[in]	numSteps	- number of times the averaging is carried out.
		*/
		void averageVertices(int numSteps);

		/*! \brief This method calculates the bounding box of the zGraph.
		*
		*	\param  	[out]	minBB	- stores zVector of bounding box minimum.
		*	\param		[out]	maxBB	- stores zVector of bounding box maximum.
		*/
		void computeBoundingBox(zVector &minBB, zVector &maxBB);


		//---- VERTEX 

		/*! \brief This method adds a vertex to the vertices array.
		*
		*	\param		[in]	pos			- zVector holding the position information of the vertex.
		*/
		void addVertex(zVector &pos);

		/*! \brief This method deletes the zGraph vertices given in the input vertex list.
		*
		*	\param		[in]	pos			- zVector holding the position information of the vertex.
		*/
		void deleteVertex(vector<int> &vertexList);

		/*! \brief This method detemines if a vertex already exists at the input position
		*
		*	\param		[in]		pos			- position to be checked.
		*	\param		[out]		outVertexId	- stores vertexId if the vertex exists else it is -1.
		*	\return		[out]		bool		- true if vertex exists else false.
		*/
		bool vertexExists(zVector pos,  int &outVertexId);

		/*! \brief This method calculates the number of vertices in zGraph or zMesh
		*	\return				number of vertices.
		*/
		int numVertices();

		/*! \brief This method sets the number of vertices in zGraph  the input value.
		*	\param				number of vertices.
		*/
		void setNumVertices(int _n_v);


		//---- EDGE 

		/*! \brief This method adds an edge and its symmetry edge to the edges array.
		*
		*	\param		[in]	v1			- start zVertex of the edge.
		*	\param		[in]	v2			- end zVertex of the edge.
		*/
		void addEdges(int &v1, int &v2);


		/*! \brief This method calculates the number of edges in zGraph or zMesh.
		*	\return				number of edges.
		*/
		int numEdges();

		/*! \brief This method sets the number of edges in zMesh  the input value.
		*	\param				number of edges.
		*/
		void setNumEdges(int _n_e);

		/*! \brief This method detemines if an edge already exists between input vertices.
		*
		*	\param		[in]	v1			- vertexId 1.
		*	\param		[in]	v2			- vertexId 2.
		*	\param		[out]	outEdgeId	- stores edgeId if the edge exists else it is -1.
		*	\return		[out]	bool		- true if edge exists else false.
		*/
		bool edgeExists(int v1, int v2, int &outEdgeId);

		/*! \brief This method collapses an edge into vertex.
		*
		*	\param		[in]	edgeList	- indicies of the edges to be collapsed.
		*/
		void collapseEdges(vector<int> &edgeList);

		/*! \brief This method sorts edges cyclically around a given vertex.
		*
		*	\param		[in]	unsortedEdges		- vector of type zEdge holding edges to be sorted
		*	\param		[in]	center				- zVertex to which all the above edges are connected.
		*	\param		[in]	sortReferenceId		- local index in the unsorted edge which is the start reference for sorting.
		*	\param		[out]	sortedEdges			- vector of zVertex holding the sorted edges.
		*/
		void cyclic_sortEdges(vector<int> &unSortedEdges, zVector &center, int sortReferenceIndex, vector<int> &sortedEdges);

		/*! \brief This method sorts edges cyclically around a given vertex using a bestfit plane.
		*
		*	\param		[in]	unsortedEdges		- vector of type zEdge holding edges to be sorted
		*	\param		[in]	center				- zVertex to which all the above edges are connected.
		*	\param		[in]	sortReferenceId		- local index in the unsorted edge which is the start reference for sorting.
		*	\param		[out]	sortedEdges			- vector of zVertex holding the sorted edges.
		*/
		void cyclic_sortEdges_bestFitPlane(vector<int> &unSortedEdges, zVector &center, int sortReferenceIndex, vector<int> &sortedEdges);


		/*! \brief This method splits a set of edges of a graph in a continuous manner.
		*
		*	\param		[in]	edgeList		- indicies of the edges to be split.
		*	\param		[in]	edgeFactor		- array of factors in the range [0,1] that represent how far along each edge must the split be done. This array must have the same number of elements as the edgeList array.
		*	\param		[out]	splitVertexId	- stores indices of the new vertex per edge in the given input edgelist.
		*/
		void splitEdges(vector<int> &edgeList, vector<double> &edgeFactor, vector<int> &splitVertexId);


		//---- UTILITY METHODS

		/*! \brief This method returns the total edge length of the graph
		*/
		double totalEdgeLength();


		/*! \brief This method prints graph information
		*/
		void printGraphInfo();

	
	
	

	
	};


}





