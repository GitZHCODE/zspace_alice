// DESCRIPTION:
//      Half-edge mesh 
//

#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif
#include <zSpace/zGraph.h>

namespace zSpace
{

	//!  A mesh class using half-edge data structure.
	/*!
	*/

	class zSPACE_API zMesh : public zGraph
	{
	private:
		
		int n_f;												/*!<stores number of faces - used for zMesh only	 */
		int max_n_f;

		//---- MESH ATTRIBUTES

		// triangles
		vector<int> triangle_Counts;						/*!<list which stores number of triangles per polygon face.	*/
		vector<int> triangle_Connects;						/*!<connection list with vertex ids for each triangle.	*/
		unordered_map<int,vector<int>> polygon_triangleIds;	/*!<triangleIds per polygon	*/



		//---- PRIVATE METHODS
		
	
		/*! \brief This method resizes the array connected with the input type to the specified newSize.
		*
		*	\param		[in]	newSize			- new size of the array.
		*	\param		[in]	type			- zVertexData or zEdgeData or zFaceData.
		*/
		void resizeArray(int newSize, zHEData type = zVertexData);

		/*! \brief This method updates the pointers for boundary Edges.
		*
		*	\param		[in]	numEdges		- number of edges in the mesh.
		*/
		void update_BoundaryEdgePointers(int &numEdges);

		/*! \brief This method resizes and copies information in to the vertex, edge and faces arrays of the current mesh from the coresponding arrays of input mesh.
		*
		*	\param		[in]	other			- input mesh to copy arrays from.
		*/
		void copyArraysfromMesh(zMesh &other);



	public:
			
		zFace *faces;					/*!<face list			*/

			

		// normals	
		vector<zVector> vertexNormals;				/*!<list which stores vertex normals.	*/
		vector<zVector> faceNormals;					/*!<list which stores face normals. Used only for meshes.	*/

		// colors
		vector<zColor> faceColors;					/*!<list which stores face colors. 	*/

		// active
		vector <bool> faceActive;					/*!<list which stores vertex weights.	*/


		// Buffer Attributes
		int VBO_VertexId;
		int VBO_EdgeId;
		int VBO_FaceId;
		int VBO_VertexColorId;
			
		//bounding box
		zVector minBB;
		zVector maxBB;

		vector<int> polyConnects;
		vector<int> polyCounts;

		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zMesh();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_positions		- vector of type zVector containing position information of vertices.
		*	\param		[in]	polyCounts		- vector of type integer with number of vertices per polygon.
		*	\param		[in]	polyConnects	- polygon connection list with vertex ids for each face.
		*/
		zMesh( vector<zVector>(&_positions), vector<int>(&polyCounts), vector<int>(&polyConnects));

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zMesh();


		//---- QUERY METHODS


		/*! \brief This method gets the edges of a zFace.
		*
		*	\param		[in]	index			- index in the face list.
		*	\param		[in]	type			- zFaceData.
		*	\param		[out]	edgeIndicies	- vector of edge indicies.
		*/
		void getEdges(int index, zHEData type , vector<int> &edgeIndicies);

		/*!	\brief This method gets the vertices attached to input zEdge or zFace.
		*
		*	\param		[in]	index			- index in the edge/face list.
		*	\param		[in]	type			- zEdgeData or zFaceData.
		*	\param		[out]	vertexIndicies	- vector of vertex indicies.
		*/
		void getVertices(int index, zHEData type, vector<int> &vertexIndicies);
				
		

		/*! \brief This method gets the faces connected to input zVertex or zFace
		*
		*	\param		[in]	index	- index in the vertex/face list.
		*	\param		[in]	type	- zMeshVertex or zMeshFace.
		*	\param		[out]	faceIndicies	- vector of face indicies.
		*/
		void getConnectedFaces(int index, zHEData type, vector<int> &faceIndicies);

		/*! \brief This method gets the faces attached to input zEdge
		*
		*	\param		[in]	index			- index in the edge list.
		*	\param		[in]	type			- zMeshEdge.
		*	\param		[out]	faceIndicies	- vector of face indicies.
		*/
		void getFaces(int &index, zHEData type, vector<int> &faceIndicies);

		/*!	\brief This method determines if  input zVertex or zEdge or zFace is on the boundary.
		*
		*	\param		[in]	index	- index in the vertex/edge/face list.
		*	\param		[in]	type	- zVertexData or zEdgeData or zFaceData.
		*	\return				bool	- true if on boundary else false.
		*/
		bool onBoundary(int index, zHEData type = zVertexData);

		//--- COMPUTE METHODS 
	
		/*! \brief This method computes the local curvature around a zVertex.
		*
		*	\param		[out]	vertexCurvature	- vector of vertex curvature.
		*/
		void getPrincipalCurvature(vector<zCurvature> &vertexCurvatures);

		/*! \brief This method computes the centers of a zEdge or zFace.
		*
		*	\param		[in]	type			- zEdgeData or zFaceData.
		*	\param		[out]	centers			- vector of centers of type zVector.
		*/
		void getCenters(zHEData type, vector<zVector> &centers);

		/*! \brief This method computes the lengths of the edges of a zMesh.
		*
		*	\param		[out]	edgeLengths			- vector of edge lengths.
		*/
		void getEdgeLengths(vector<double> &edgeLengths);
		

		/*! \brief This method computes the dihedral angle per edge of zMesh.
		*
		*	\param		[out]	dihedralAngles		- vector of edge dihedralAngles.
		*/
		void getEdgeDihedralAngles(vector<double> &dihedralAngles);

		/*! \brief This method computes the area around every vertex of a zMesh.
		*
		*	\param		[out]	vertexAreas		- vector of vertex Areas.
		*/
		void getVertexArea(vector<double> &vertexAreas);

		/*! \brief This method computes the area around every vertex of a zMesh based on face centers.
		*
		*	\param		[in]	faceCenters		- vector of face centers of type zVector.
		*	\param		[in]	edgeCenters		- vector of edge centers of type zVector.
		*	\param		[out]	vertexAreas		- vector of vertex Areas.
		*/
		void getVertexArea(vector<zVector> &faceCenters, vector<zVector> &edgeCenters, vector<double> &vertexAreas);

		//---- METHODS

		/*! \brief This method creats a copy of the current mesh.
		*/
		zMesh meshCopy();
		

		/*! \brief This method prints mesh information
	    */
		void printMeshInfo();

		/*! \brief This method stores mesh connectivity information in the input lists
		*
		*	\param		[out]	polyConnects	- stores list of polygon connection with vertex ids for each face.
		*	\param		[out]	polyCounts		- stores number of vertices per polygon.
		*/
		void computePolyConnects_PolyCount(vector<int> (&polyConnects), vector<int> (&polyCounts));

		/*! \brief This method computes the triangles associated with each polygon face of the mesh.
		*/
		void computePolygonTriangles();

		/*! \brief This method computes the normals assoicated with vertices and polygon faces .
		*/
		void computeMeshNormals();

		void computeTriMeshNormals();

		void setFaceNormals(vector<zVector> &fNormals);

		
	
		/*! \brief This method copies the triangle lists of the mesh into the given input lists.
		*
		*	\param		[out]	polyConnects_tris	- stores list of polygon connection with vertex ids for each triangle.
		*	\param		[out]	tris_perPolygon		- stores number of triangles per polygon.
		*/
		void getTriangles(vector<int>(&polyConnects_tris), vector<int>(&tris_perPolygon));
		
		/*! \brief This method returns a triangulated mesh of the input mesh.
		*
		*	\param		[out]	polyConnects_tris	- stores list of polygon connection with vertex ids for each triangle.
		*	\param		[out]	tris_perPolygon		- stores number of triangles per polygon.
		*	\return		[out]	zMesh				- triangulated mesh.
		*/
		zMesh meshTriangulate();

		/*! \brief This method triangulates the input mesh.
		*/
		void triangulate(bool computeNormal = false);

		/*! \brief This method triangulates the input polygon using ear clipping algorithm based on : https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf 
		*
		*	\param		[in]	polyIndex			- polygon index  of the face to be triangulated in the face array.
		*	\param		[out]	numTris				- number of triangles in the input polygon.
		*	\param		[out]	tris				- index array of each triangle associated with the face.
		*/
		void polyTriangulate(int &polyIndex, int &numTris, vector<int> &tris, bool computeNormal = true);

		/*! \brief This method splits a set of edges and faces of a mesh in a continuous manner.
		*
		*	\param		[in]	edgeList	- indicies of the edges to be split.
		*	\param		[in]	edgeFactor	- array of factors in the range [0,1] that represent how far along each edge must the split be done. This array must have the same number of elements as the edgeList array.
		*/
		void splitFaces(vector<int> &edgeList, vector<double> &edgeFactor);

		/*! \brief This method subdivides all the faces and edges of the mesh.
		*
		*	\param		[in]	numDivisions	- number of subdivision to be done on the mesh.
		*/
		void subDivideMesh(int numDivisions);
		 
		/*! \brief This method subdivides the face and contained edges of the mesh at the given input index.
		*
		*	\param		[in]	numDivisions	- number of subdivision to be done on the mesh.
		*/
		void subDivideFace(int &index, int numDivisions);

		//---- VERTEX 

		/*! \brief This method deletes the zMesh vertices given in the input vertex list.
		*
		*	\param		[in]	pos			- zVector holding the position information of the vertex.
		*/
		void deleteVertex(vector<int> &vertexList);

		//---- EDGE 
		
		/*! \brief This method collapses all the edges in the input edge list.
		*
		*	\param		[in]	edgeList	- indicies of the edges to be collapsed.
		*/
		void collapseEdges(vector<int> &edgeList);

		/*! \brief This method collapses an edge into a vertex.
		*
		*	\param		[in]	index	- index of the edge to be collapsed.
		*/
		void collapseEdge(int index);


		/*! \brief This method splits a set of edges of a mesh in a continuous manner.
		*
		*	\param		[in]	edgeList		- indicies of the edges to be split.
		*	\param		[in]	edgeFactor		- array of factors in the range [0,1] that represent how far along each edge must the split be done. This array must have the same number of elements as the edgeList array.
		*	\param		[out]	splitVertexId	- stores indices of the new vertex per edge in the given input edgelist.
		*/
		void splitEdges(vector<int> &edgeList, vector<double> &edgeFactor, vector<int> &splitVertexId);

		/*! \brief This method splits an edge and inserts a vertex along the edge at the input factor.
		*
		*	\param		[in]	index			- index of the edge to be split.
		*	\param		[in]	edgeFactor		- factor in the range [0,1] that represent how far along each edge must the split be done. 
		*	\return				int				- index of the new vertex added after splitinng the edge.
		*/
		int splitEdge(int index, double edgeFactor = 0.5);


		/*! \brief This method flips the edge shared bettwen two rainglua faces.
		*
		*	\param		[in]	edgeList	- indicies of the edges to be collapsed.
		*/
		void flipTriangleEdges(vector<int> &edgeList);

		/*! \brief This method deletes the zMesh vertices given in the input vertex list.
		*
		*	\param		[in]	pos			- zVector holding the position information of the vertex.
		*/
		void deleteEdge(vector<int> &edgeList);

		//---- FACE 

		/*! \brief This method adds a face with null edge pointer to the faces array.
		*
		*/
		void addPolygon();

		/*! \brief This method adds a face to the faces array and updates the pointers of vertices, edges and polygons of the mesh based on face vertices. 
		*
		*	\param		[in]	fVertices	- array of ordered vertices that make up the polygon.
		*/
		void addPolygon(vector<int> &fVertices);

		
		/*! \brief This method returns the number of polygons in the mesh
		*
		*	\return		[out]	number of polygons 
		*/
		int numPolygons();

		/*! \brief This method sets the number of faces in zMesh  the input value.
		*	\param				number of faces.
		*/
		void setNumPolygons(int _n_f);

		//---- UTILITY METHODS

		/*! \brief This method checks if the given input points liess within the input triangle.
		*	\param		[in]	pt			- zVector holding the position information of the point to be checked.
		*	\param		[in]	t0,t1,t2	- zVector holding the position information for the 3 points of the triangle.
		*	\return				bool		- true if point is inside the input triangle.
		*/
		bool pointInTriangle(zVector &pt, zVector &t0, zVector &t1, zVector &t2);

		/*! \brief This method splits an edge longer than the given input value at its midpoint and  triangulates the mesh. the adjacent triangles are split into 2-4 triangles.
		* ( Based on http://lgg.epfl.ch/publications/2006/botsch_2006_GMT_eg.pdf)
		*/
		void splitLongEdges(double maxEdgeLength = 0.5);

		/*! \brief This method collapses an edge shorter than the given minimum edge length value if the collapsing doesnt produce adjacent edges longer than the maximum edge length.
		* ( Based on http://lgg.epfl.ch/publications/2006/botsch_2006_GMT_eg.pdf)
		*/
		void collapseShortEdges(double minEdgeLength = 0.1, double maxEdgeLength = 0.5);

		/*! \brief This method equalizes the vertex valences by flipping edges of the input triangulated mesh. Target valence for interior vertex is 4 and boundary vertex is 6.
		* ( Based on http://lgg.epfl.ch/publications/2006/botsch_2006_GMT_eg.pdf)
		*/
		void equalizeValences();

		/*! \brief This method applies an iterative smoothing to the mesh by  moving the vertex but constrained to its tangent plane. 
		* ( Based on http://lgg.epfl.ch/publications/2006/botsch_2006_GMT_eg.pdf)
		*/
		void tangentialRelaxation();

		/*! \brief This method returns the minimum and maximum edge lengths in the mesh.
		*	\param		[out]	minVal		- minimum edge length in the mesh.
		*	\param		[out]	maxVal		- maximum edge length in the mesh.
		*/
		void minMax_edgeLengths(double &minVal, double &maxVal);

		//---- COLOR METHODS

		/*! \brief This method computes the face colors based on the vertex colors.
		*/
		void computeFaceColorfromVertexColor();

		/*! \brief This method computes the vertex colors based on the face colors.
		*/
		void computeVertexColorfromFaceColor();

		/*! \brief This method smoothens the color attributes.
		*	\param		[in]	smoothVal		- number of iterations to run the smooth operation.
		*	\param		[in]	type			- zVertexData or zEdgeData or zFaceData.
		*/
		void smoothColors(int smoothVal = 1, zHEData type = zVertexData);

		
	};
}
