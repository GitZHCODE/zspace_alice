#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include<memory>
using namespace std;

#include <zSpace/zVector.h>

namespace zSpace
{
	// Class Vertex , Edge and Face
	class zSPACE_API zEdge;
	class zSPACE_API zVertex;
	class zSPACE_API zFace;

	//!   zEdge class defined to hold edge information of a half-edge data structure. 
	/*!
	*/
	class zSPACE_API zEdge
	{
	public:
		//---- CONSTRUCTOR

		/*! \brief Default constructor.
		*/
		zEdge();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zEdge();
		
		//---- GET-SET METHODS

		/*! \brief This method returns the edgeId of current zEdge.
		*	\return				edgeId.
		*/
		int getEdgeId();

		/*! \brief This method sets the edgeId of current zEdge to the the input value.
		*	\param		[in]	edgeId.
		*/
		void setEdgeId(int EdgeId);

		/*! \brief This method returns the symmetry edge of current zEdge.
		*	\return				symmetry edge of type zEdge.
		*/
		zEdge* getSym();
		
		/*! \brief This method sets the symmetry edge of current zEdge to the the input edge
		*	\param		[in]	symmetry edge of type zEdge.
		*/
		void setSym(zEdge* _sym);

		/*! \brief This method returns the previous edge of current zEdge.
		*	\return				previous edge of type zEdge.
		*/
		zEdge* getPrev();

		/*! \brief This method sets the previous edge of current zEdge to the the input edge
		*	\param		[in]	previous edge of type zEdge.
		*/
		void setPrev(zEdge* _prev);

		/*! \brief This method returns the next edge of current zEdge.
		*	\return				next edge of type zEdge.
		*/
		zEdge* getNext();

		/*! \brief This method sets the next edge of current zEdge to the the input edge
		*	\param		[in]	next edge of type zEdge.
		*/
		void setNext(zEdge* _next);

		/*! \brief This method returns the vertex pointed to by the current zEdge.
		*	\return				vertex of type zVertex.
		*/
		zVertex* getVertex();

		/*! \brief This method sets the vertex pointed to by the current zEdge to the the input vertex.
		*	\param		[in]	vertex of type zVertex.
		*/
		void setVertex(zVertex* _v);

		/*! \brief This method returns the face pointed to by the current zEdge.
		*	\return				face of type zface.
		*/
		zFace* getFace();

		/*! \brief This method sets the vertex pointed to by the current zEdge to the the input vertex.
		*	\param		[in]	face of type zface.
		*/
		void setFace(zFace* _v);
		

	private:
		int edgeId;		/*!< stores edgeId. It is unique per edge. 	*/

		zVertex *v;		/*!< pointer to zVertex	in vertex list.		*/
		zFace *f;		/*!< pointer to zFace in face list.		*/

		zEdge *prev;	/*!< pointer to previous zEdge in a polygon.			*/
		zEdge *next;	/*!< pointer to next zEdge in a polygon.			*/
		zEdge *sym;		/*!< pointer to symmerty/twin zEdge  in edge list.			*/

	};

	//!  zVertex class defined to hold vertex information of a half-edge data structure.  
	/*!
	*/
	class zSPACE_API zVertex
	{

	public:
		//---- DESTRUCTOR

		/*! \brief Default constructor.
		*/
		zVertex();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zVertex();

		//---- GET-SET METHODS

		/*! \brief This method returns the vertexId of current zVertex.
		*	\return				vertexId.
		*/
		int getVertexId();

		/*! \brief This method sets the vertexId of current zVertex to the the input value.
		*	\param		[in]	vertexId.
		*/
		void setVertexId(int _vertexId);

		/*! \brief This method returns the associated edge of current zVertex.
		*	\return				associated edge of type zEdge.
		*/
		zEdge* getEdge();

		/*! \brief This method sets the associated edge of current zVertex to the the input edge
		*	\param		[in]	edge of type zEdge.
		*/
		void setEdge(zEdge* _e);



	private:
		int vertexId;	/*!< stores vertexId. It is unique per vertex.		*/
		zEdge *e;		/*!< pointer to zEdge starting at the current zVertex.		*/

	};

	//!  zFace class defined to hold polygon information of a half-edge datastructure. 
	/*!
	*/
	class zSPACE_API zFace
	{
	public:
		/*! \brief Default constructor.
		*/
		zFace();

		//---- DESTRUCTOR

		/*! \brief Default destructor.
		*/
		~zFace();

		//---- GET-SET METHODS

		/*! \brief This method returns the faceId of current zFace.
		*	\return				faceId.
		*/
		int getFaceId();

		/*! \brief This method sets the faceId of current zFace to the the input value.
		*	\param		[in]	faceId.
		*/
		void setFaceId(int _faceId);

		/*! \brief This method returns the associated edge of current zFace.
		*	\return				associated edge of type zEdge.
		*/
		zEdge* getEdge();

		/*! \brief This method sets the associated edge of current zFace to the the input edge
		*	\param		[in]	associated edge of type zEdge.
		*/
		void setEdge(zEdge* _e);

	private:
		int faceId;		/*!< stores faceId. It is unique per polygon.		*/
		zEdge *e;		/*!< pointer to one of the zEdge contained in the polygon.		*/
	};

	//!  zCurvature struct defined to hold curvature information of a half-edge zGraph or zMesh. 
	/*!
	*/
	struct zSPACE_API zCurvature
	{
		double k1;		/*!< stores principal curvature 1		*/
		double k2;		/*!< stores principal curvature 2		*/

	};



	//!  zPlane struct hold information of a plane defined by the 3 axis vectors (X,Y,Z) and origin. 
	/*!
	*/
	struct zSPACE_API zPlane
	{
		zVector X;
		zVector Y;
		zVector Z;

		zVector O;
	};

	//!  zPlane struct hold information of a plane defined by the 3 axis vectors (X,Y,Z) and origin. 
	/*!
	*/
	struct zSPACE_API zCluster
	{
		zVector clusterCenter;
		vector<int> ids;		
	};


	struct zSPACE_API zPolyhedraDual
	{
		/*vector<int> */


	};

}