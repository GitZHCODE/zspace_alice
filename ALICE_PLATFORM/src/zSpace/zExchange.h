#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <zSpace/zMesh.h>
#include <zSpace/zScalarField.h>
#include <zSpace/zSlime.h>

#include "modernJSON\json.hpp"

#include <iostream>
using namespace std;

using json = nlohmann::json;;

namespace zSpace
{

	class zSPACE_API zHEtransfer
	{
		private:
			vector<int> vertices;
			vector<vector<int>> halfedges;
			vector<int> faces;

			vector<vector<double>> vertexAttributes;
			vector<vector<double>> halfedgeAttributes;
			vector<vector<double>> faceAttributes;

		public:
			zHEtransfer();
			~zHEtransfer();

			/*! \brief This method creates the JSON file from the HE data structure using JSON Modern Library.
			*
			*	\tparam			T					- Type to work with zGraph & zMesh
			*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
			*	\param	[out]	j					- json file.
			*/
			template<class T>
			void to_json(json &j, T &HE_Datastructure, bool vColors = false);

			/*! \brief This method creates the HE data structure from JSON file using JSON Modern Library.
			*
			*	\tparam			T					- Type to work with zGraph & zMesh
			*	\param	[in]	j					- json file.
			*	\param	[out]	HE_Datastructure	- output datastructure (zGraph or zMesh).
			*/
			template<class T>
			void from_json(json &j, T &HE_Datastructure, bool vColors = false);
	};

	class zSPACE_API zLINEtransfer
	{
	private:
		
		vector<vector<double>> lPositions;
		vector<double> lQuaternions;
		vector<double> lColors;
		double lWeights;

		

	public:
		zLINEtransfer();
		~zLINEtransfer();

		/*! \brief This method creates the JSON file from the SLIME agent trail using JSON Modern Library.
		*
		*	\param	[in]	slime				- input zSlime.
		*	\param	[out]	j					- json file.
		*/
		void to_json(json &j, vector<zVector> &positions , Matrix3d &rotMat , zColor &color , double &weight);

		/*! \brief This method creates the SLIME agent trail from JSON file using JSON Modern Library.
		*
		*	\tparam			T					- Type to work with zGraph & zMesh
		*	\param	[in]	j					- json file.
		*	\param	[out]	HE_Datastructure	- output datastructure (zGraph or zMesh).
		*/
		zGraph from_json(json &j);
	};

	class zSPACE_API zScalarFieldTransfer
	{
	private:
		vector<int> resolution;
		vector<double> scalars;

		vector<vector<double>> bounds;

	public:
		zScalarFieldTransfer();
		~zScalarFieldTransfer();

		/*! \brief This method creates the JSON file from the HE data structure using JSON Modern Library.
		*
		*	\tparam			T					- Type to work with zGraph & zMesh
		*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
		*	\param	[out]	j					- json file.
		*/
		template<class T>
		void to_json(json &j, T &SF_Datastructure);

		/*! \brief This method creates the HE data structure from JSON file using JSON Modern Library.
		*
		*	\tparam			T					- Type to work with zGraph & zMesh
		*	\param	[in]	j					- json file.
		*	\param	[out]	HE_Datastructure	- output datastructure (zGraph or zMesh).
		*/
		template<class T>
		void from_json(json &j, T &SF_Datastructure);
	};
	
	struct zSPACE_API zbuildingData
	{
		int id;
		time_t updated;
		int userId;
		double x;
		double y;
		int floorId;
		int zoneId;
		int roomId;
	};
	
	class zSPACE_API zBuildingDataTransfer
	{
	
		public:

			vector<zbuildingData> occupancyData;

			zBuildingDataTransfer();
			~zBuildingDataTransfer();

			/*! \brief This method creates the JSON file from the HE data structure using JSON Modern Library.
			*
			*	\param	[in]	HE_Datastructure	- input datastructure (zGraph or zMesh).
			*	\param	[out]	j					- json file.
			*/
			void to_json(json &j);

			/*! \brief This method creates the HE data structure from JSON file using JSON Modern Library.
			*
			*	\param	[in]	j					- json file.
			*	\param	[out]	HE_Datastructure	- output datastructure (zGraph or zMesh).
			*/
			void from_json(json &j);
	};
		

	

}


//TEMPLATE METHODS defined in header

template<>
inline void zSpace::zHEtransfer::to_json(json & j, zMesh & HE_Datastructure, bool vColors)
{
	// Vertices
	for (int i = 0; i < HE_Datastructure.numVertices(); i++)
	{
		if (HE_Datastructure.vertices[i].getEdge()) vertices.push_back(HE_Datastructure.vertices[i].getEdge()->getEdgeId());
		else vertices.push_back(-1);

	}

	//Edges
	for (int i = 0; i < HE_Datastructure.numEdges(); i++)
	{
		vector<int> HE_edges;

		if (HE_Datastructure.edges[i].getPrev()) HE_edges.push_back(HE_Datastructure.edges[i].getPrev()->getEdgeId());
		else HE_edges.push_back(-1);

		if (HE_Datastructure.edges[i].getNext()) HE_edges.push_back(HE_Datastructure.edges[i].getNext()->getEdgeId());
		else HE_edges.push_back(-1);

		if (HE_Datastructure.edges[i].getVertex()) HE_edges.push_back(HE_Datastructure.edges[i].getVertex()->getVertexId());
		else HE_edges.push_back(-1);

		if (HE_Datastructure.edges[i].getFace()) HE_edges.push_back(HE_Datastructure.edges[i].getFace()->getFaceId());
		else HE_edges.push_back(-1);

		halfedges.push_back(HE_edges);
	}

	// Faces
	for (int i = 0; i < HE_Datastructure.numPolygons(); i++)
	{
		if (HE_Datastructure.faces[i].getEdge()) faces.push_back(HE_Datastructure.faces[i].getEdge()->getEdgeId());
		else faces.push_back(-1);
	}

	// vertex Attributes
	for (int i = 0; i < HE_Datastructure.vertexPositions.size(); i++)
	{
		vector<double> v_attrib;

		v_attrib.push_back(HE_Datastructure.vertexPositions[i].x);
		v_attrib.push_back(HE_Datastructure.vertexPositions[i].y);
		v_attrib.push_back(HE_Datastructure.vertexPositions[i].z);

		if (vColors)
		{
			v_attrib.push_back(HE_Datastructure.vertexColors[i].r);
			v_attrib.push_back(HE_Datastructure.vertexColors[i].g);
			v_attrib.push_back(HE_Datastructure.vertexColors[i].b);
		}
		

		vertexAttributes.push_back(v_attrib);
	}


	// Json file 
	j["Vertices"] = vertices;
	j["Halfedges"] = halfedges;
	j["Faces"] = faces;
	j["VertexAttributes"] = vertexAttributes;
	j["HalfedgeAttributes"] = halfedgeAttributes;
	j["FaceAttributes"] = faceAttributes;
}

template<>
inline void zSpace::zHEtransfer::to_json(json & j, zGraph & HE_Datastructure, bool vColors)
{
	// Vertices
	for (int i = 0; i < HE_Datastructure.numVertices(); i++)
	{
		if (HE_Datastructure.vertices[i].getEdge()) vertices.push_back(HE_Datastructure.vertices[i].getEdge()->getEdgeId());
		else vertices.push_back(-1);

	}

	//Edges
	for (int i = 0; i < HE_Datastructure.numEdges(); i++)
	{
		vector<int> HE_edges;

		if (HE_Datastructure.edges[i].getPrev()) HE_edges.push_back(HE_Datastructure.edges[i].getPrev()->getEdgeId());
		else HE_edges.push_back(-1);

		if (HE_Datastructure.edges[i].getNext()) HE_edges.push_back(HE_Datastructure.edges[i].getNext()->getEdgeId());
		else HE_edges.push_back(-1);

		if (HE_Datastructure.edges[i].getVertex()) HE_edges.push_back(HE_Datastructure.edges[i].getVertex()->getVertexId());
		else HE_edges.push_back(-1);

		halfedges.push_back(HE_edges);
	}



	// vertex Attributes
	for (int i = 0; i < HE_Datastructure.vertexPositions.size(); i++)
	{
		vector<double> v_attrib;

		v_attrib.push_back(HE_Datastructure.vertexPositions[i].x);
		v_attrib.push_back(HE_Datastructure.vertexPositions[i].y);
		v_attrib.push_back(HE_Datastructure.vertexPositions[i].z);

		vertexAttributes.push_back(v_attrib);
	}


	// Json file 
	j["Vertices"] = vertices;
	j["Halfedges"] = halfedges;

	j["VertexAttributes"] = vertexAttributes;
	j["HalfedgeAttributes"] = halfedgeAttributes;

}


template<>
inline void zSpace::zHEtransfer::from_json(json & j, zMesh & HE_Datastructure, bool vColors)
{
	// Vertices
	vertices.clear();
	vertices = (j["Vertices"].get<vector<int>>());

	//Edges
	halfedges.clear();
	halfedges = (j["Halfedges"].get<vector<vector<int>>>());

	// Faces
	faces.clear();
	faces = (j["Faces"].get<vector<int>>());


	// update  mesh

	HE_Datastructure.vertices = new zVertex[vertices.size() * 2];
	HE_Datastructure.edges = new zEdge[halfedges.size() * 2];
	HE_Datastructure.faces = new zFace[faces.size() * 2];

	HE_Datastructure.setNumVertices(vertices.size());
	HE_Datastructure.setNumEdges(floor(halfedges.size()));
	HE_Datastructure.setNumPolygons(faces.size());

	HE_Datastructure.vertexActive.clear();
	for (int i = 0; i < vertices.size(); i++)
	{
		HE_Datastructure.vertices[i].setVertexId(i);
		if (vertices[i] != -1) HE_Datastructure.vertices[i].setEdge(&HE_Datastructure.edges[vertices[i]]);

		HE_Datastructure.vertexActive.push_back(true);
	}

	int k = 0;
	HE_Datastructure.edgeActive.clear();
	for (int i = 0; i < halfedges.size(); i++)
	{
		HE_Datastructure.edges[i].setEdgeId(i);

		if (halfedges[i][k] != -1) 	HE_Datastructure.edges[i].setPrev(&HE_Datastructure.edges[halfedges[i][k]]);
		if (halfedges[i][k + 1] != -1) HE_Datastructure.edges[i].setNext(&HE_Datastructure.edges[halfedges[i][k + 1]]);
		if (halfedges[i][k + 2] != -1) HE_Datastructure.edges[i].setVertex(&HE_Datastructure.vertices[halfedges[i][k + 2]]);
		if (halfedges[i][k + 3] != -1) HE_Datastructure.edges[i].setFace(&HE_Datastructure.faces[halfedges[i][k + 3]]);


		if (i % 2 == 0) HE_Datastructure.edges[i].setSym(&HE_Datastructure.edges[i]);
		else  HE_Datastructure.edges[i].setSym(&HE_Datastructure.edges[i - 1]);

		HE_Datastructure.edgeActive.push_back(true);
	}

	HE_Datastructure.faceActive.clear();
	for (int i = 0; i < faces.size(); i++)
	{
		HE_Datastructure.faces[i].setFaceId(i);
		if (faces[i] != -1) HE_Datastructure.faces[i].setEdge(&HE_Datastructure.edges[faces[i]]);

		HE_Datastructure.faceActive.push_back(true);
	}

	// Vertex Attributes
	vertexAttributes = j["VertexAttributes"].get<vector<vector<double>>>();
	//printf("\n vertexAttributes: %zi %zi", vertexAttributes.size(), vertexAttributes[0].size());

	HE_Datastructure.vertexPositions.clear();
	for (int i = 0; i < vertexAttributes.size(); i++)
	{
		for (int k = 0; k < vertexAttributes[i].size(); k++)
		{
			// position
			if (vertexAttributes[i].size() == 3)
			{
				zVector pos(vertexAttributes[i][k], vertexAttributes[i][k + 1], vertexAttributes[i][k + 2]);

				HE_Datastructure.vertexPositions.push_back(pos);
				k += 2;
			}
		}
	}


	// Edge Attributes
	halfedgeAttributes = j["HalfedgeAttributes"].get<vector<vector<double>>>();


	// Edge Attributes
	faceAttributes = j["FaceAttributes"].get<vector<vector<double>>>();



}

template<>
inline void zSpace::zHEtransfer::from_json(json & j, zGraph & HE_Datastructure, bool vColors)
{
	// Vertices
	vertices.clear();
	vertices = (j["Vertices"].get<vector<int>>());

	//Edges
	halfedges.clear();
	halfedges = (j["Halfedges"].get<vector<vector<int>>>());

	//printf("\n halfedges: %i halfedges 0 : %i  ", halfedges.size(), halfedges[0].size());

	// update  graph

	HE_Datastructure.vertices = new zVertex[vertices.size() * 2];;
	HE_Datastructure.edges = new zEdge[halfedges.size() * 2];;

	HE_Datastructure.setNumVertices(vertices.size());
	HE_Datastructure.setNumEdges(halfedges.size());


	for (int i = 0; i < vertices.size(); i++)
	{
		HE_Datastructure.vertices[i].setVertexId(i);
		HE_Datastructure.vertices[i].setEdge(&HE_Datastructure.edges[vertices[i]]);
	}

	for (int i = 0; i < halfedges.size(); i++)
	{
		HE_Datastructure.edges[i].setEdgeId(i);

		if (halfedges[i][0] != -1) 	HE_Datastructure.edges[i].setPrev(&HE_Datastructure.edges[halfedges[i][0]]);
		if (halfedges[i][1] != -1) HE_Datastructure.edges[i].setNext(&HE_Datastructure.edges[halfedges[i][1]]);
		if (halfedges[i][2] != -1) HE_Datastructure.edges[i].setVertex(&HE_Datastructure.vertices[halfedges[i][2]]);


		if (i % 2 == 0) HE_Datastructure.edges[i].setSym(&HE_Datastructure.edges[i]);
		else  HE_Datastructure.edges[i].setSym(&HE_Datastructure.edges[i - 1]);

	}

	// Vertex Attributes
	vertexAttributes = j["VertexAttributes"].get<vector<vector<double>>>();
	//printf("\n vertexAttributes: %i %i", vertexAttributes.size(), vertexAttributes[0].size());

	HE_Datastructure.vertexPositions.clear();
	for (int i = 0; i < vertexAttributes.size(); i++)
	{
		for (int j = 0; j < vertexAttributes[i].size(); j++)
		{
			// position
			if (vertexAttributes[i].size() == 3)
			{
				zVector pos(vertexAttributes[i][j], vertexAttributes[i][j + 1], vertexAttributes[i][j + 2]);

				HE_Datastructure.vertexPositions.push_back(pos);
				j += 2;
			}
		}
	}

	// Edge Attributes
	halfedgeAttributes = j["HalfedgeAttributes"].get<vector<vector<double>>>();

}


// SCALAR Field

template<>
inline void zSpace::zScalarFieldTransfer::to_json(json & j, zScalarField2D & SF_Datastructure)
{
	// Resolution

	resolution.push_back(SF_Datastructure.n_X);
	resolution.push_back(SF_Datastructure.n_Y);
	resolution.push_back(0);

	//Scalars
	for (int i = 0; i < SF_Datastructure.scalars.size(); i++)
	{
		scalars.push_back(SF_Datastructure.scalars[i].weight);
	}

	// bounds
	vector<double> min_BB;
	min_BB.push_back(SF_Datastructure.minBB.x);
	min_BB.push_back(SF_Datastructure.minBB.y);
	min_BB.push_back(SF_Datastructure.minBB.z);
	
	vector<double> max_BB;
	max_BB.push_back(SF_Datastructure.maxBB.x);
	max_BB.push_back(SF_Datastructure.maxBB.y);
	max_BB.push_back(SF_Datastructure.maxBB.z);
	
	bounds.push_back(min_BB);
	bounds.push_back(max_BB);
	
	// Json file 
	j["Resolution"] = resolution;
	j["Bounds"] = bounds;
	j["Scalars"] = scalars;


}