
// DESCRIPTION:
//      Open Street map reader
//

#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#include <vector>
#include <algorithm>    // std::sort
#include <ctime>
#include <iostream>
#include <unordered_map>
using namespace std;


#include<zSpace\zVector.h>
#include<zSpace\zDatabase.h>
#include<zSpace\zGraph.h>
#include<zSpace\zMesh.h>
#include <zSpace\zScalarField.h>
#include<zSpace\zExchange.h>

#define MAX_NODES 300000
#define MAX_WAYS 300000
#define MAX_RELATIONS 300000
#define MAX_MEMBERS 300000
#define MAX_TAGS 300000

#define MAX_BUILDINGNODES 200000
#define MAX_BUILDINGOUTLINES 100000

#define MAX_ROADNODES 200000
#define MAX_ROADLINES 100000

#define MAX_LatLon 4

#define MAX_TRIANGLESPERPOLYGON 1000

#define MAX_NODES_PERWAY 50

#define zDefault 0
#define zResidentPopulation 1
#define zWorkPopulation 2
#define zDistance2 3
#define zDistance10 4
#define zDistanceAverage 5
#define zBuildingHeight 6

namespace zSpace
{


	struct zSPACE_API zWays
	{
		int id;

		string OS_wayId;
		vector<int> roadGraph_edgeId;

	};

	struct zSPACE_API zBuildings
	{
		int id;

		string shapeId;
		vector<int> buidlingMesh_vertexId;

		double height;
		double highlightData;
		
	};

	struct zSPACE_API zStations
	{
		string name;
		
		int entries;
		int exits;
	};

	struct zSPACE_API zPID
	{
		
		vector<int> zBuildingID;
	};

	struct zSPACE_API zLSOA
	{
		string lsoa;
		vector<int> postcodeGraph_vertexId;
		vector<string> postcodes;
		vector<int> scalarFieldIds;
	};

	struct zSPACE_API zMSOA
	{
		string msoa;
		vector<string> lsoa;
		
	};

	class zSPACE_API zOpenStreet
	{
	
	protected:

	

	public:
		
	
		int n_zWays;									/*!< number of road Nodes */
		int n_zBuildings;
		int n_zStations;

			
		zGraph roadGraph;								/*!<  graph of roads	*/
		zWays *way;
		
		zMesh buidlingMesh;								/*!<  mesh of buildings	*/
		zBuildings *buidlings;

		zPID *populationIDs;

		zGraph stationGraph;
		zStations *Stations;

		zGraph parkGardenGraph;

		zGraph postcodeGraph;
		zMesh oaMesh;
		vector<zLSOA> lsoa;
		vector<zMSOA> msoa;

		zGraph pedestrianStreetGraph;

		double lat_lon[MAX_LatLon];						/*!< bounding box */

		zVector minBB; 
		zVector maxBB;
		
		zDatabase *zDB;

		zScalarField2D scalarField;

		// attributes

		zAttributeUnorderedMap <string, int> node_roadVertices;
		zAttributeMap <int, string> roadVertices_Node;

		zAttributeUnorderedMap <string, int> wayId_zWays;
		zAttributeMap <int, string> zWays_wayId;

		zAttributeMap <int, string> roadEdges_Way;

		zAttributeUnorderedMap <string, int> buildingShapeId_zBuildings;
		zAttributeMap <int, string> zBuildings_buildingShapeId;

	
		zAttributeVector <string> zBuildings_Postcode;
		zAttributeVector <int> zBuildings_PID;

		zAttributeMap <int, string> vertex_Building;

		zAttributeVector <int> building_postcodeVertex;
		zAttributeVector <int> building_roadVertex;

		zAttributeUnorderedMap <string, int> stationName_zStations;

		vector<zAttributeVector<int>> station_roadVertices;		
		zAttributeVector<double> roadVertex_DistancefromStation;

		vector<zAttributeVector<int>> parkGarden_roadVertices;
		zAttributeVector<double> roadVertex_DistancefromParkGarden;


		zAttributeVector<zVector> building_Centers;

		zAttributeVector<int> postcodes_PID;
		zAttributeVector<string> postcodes;
		zAttributeUnorderedMap <string, int> lsoa_Id; 
		zAttributeUnorderedMap <string, int> msoa_Id;
	


		vector<zAttributeVector<int>> roadGraph_Scalarfield;
		vector<zAttributeVector<int>> buildingCenter_Scalarfield;
		vector<zAttributeVector<int>> stationGraph_Scalarfield;
		vector<zAttributeVector<int>> parkGardenGraph_Scalarfield;
		vector<zAttributeVector<int>> postcodeGraph_Scalarfield;


		
		int postcodeID;

	
		//---- CONSTRUCTOR

		///
		/// <B> default constructor </B> 
		///
		zOpenStreet();

		/// over loaded constructor
		///
		zOpenStreet(char* DatabaseFileName);

		//---- DESTRUCTOR
		
		///
		/// default destructor
		///
		~zOpenStreet();

		//---- METHODS

		///
		/// calculate BoundingBox vectors
		///
		void calculateBoundingBox(double scaleFactor);

		///
		/// calculate distance given latitude and longitudes
		///
		double calculateDistance(double &lat1, double &lon1, double &lat2, double &lon2);
		
		///
		/// mapping to input domain
		///
		double  ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax);

				
		 // NODES
			
		// calculate Node Mapped Coordinates
		zVector calculateFromCoordinates( double &lat, double &lon);

		// SCALAR FIELD

	

		//WAYS

		// highlight
		void highlightWays(string tag);

		
		//RELATION

		// highlight
		void highlightRelations(string tag);

		// PEDESTRIAN

		// create pedestrian graph
		void createPedestrianGraph();

		// Output Area - LSOA/ MSOA
		void createOAMesh();

		void computeOA_Scalarfield(bool exportFile = false , string outfilename = "C:/Users/vishu.b/Desktop/lsoa_Scalarfield_10.json");

		void importOA_Scalarfield(string infilename = "C:/Users/vishu.b/Desktop/lsoa_Scalarfield_10.json" );

	
		
		// POSTCODES

		// create postcode graph
		void createPostcodeGraph();

		// import SQL data per postcode
		void import_postcodeGraph_SQL_vertexData(string tag, zColor &col1, zColor &col2, double domainMin = 0.1, double domainMax = 0.9, double heightMin = 0, double heightMax = 100);

		// SCALAR FIELD

		// get scalar Data
		template < class T>
		vector<T> getScalarFieldData(string tag = "density_residential", zAttributeData attributeDataType = zDoubleAttribute, zDataLevel dataLevel = zLsoaData);

		// PARKS AND GARDENS
	
		// ceate graph ofparks and gardens
		void createParkGardenGraph();

		// compute computeParkGarden_RoadVertices
		void computeParkGarden_RoadVertices(double distance, double &scaleFactor);

		// distance from roadVertex_DistancefromParkGarden
		void roadVertex_DistancefromParks(double &scaleFactor);

		//ROADS
			
		// create road graph
		void createRoadGraph();

		// export road graph to SQL - edge classification
		void export_roadGraph_SQL_edgeType();

		// export road graph to SQL - edge traffic
		void export_roadGraph_SQL_vertexTraffic();

		// export road graph to SQL - vertex distance to station
		void export_roadGraph_SQL_vertexDistanceData(string tag);

		// import road graph to SQL - edge traffic
		void import_roadGraph_SQL_edgeTraffic();



		// EXPORT GRAPH

		// export Map Attributes of a graph
		void export_MapAttributes(zAttributeMap<int,string> &inAttributeMap, string  outfilename = "C:/Users/vishu.b/Desktop/out.json");

		// export Map Attributes of a graph to SQL
		void export_MapAttributes_Graph_SQL(zGraph &graph, zAttributeMap<int, string> &inAttributeMap, string  tableName = "roadGraph_vertexToNode", vector<string> columnNames = { "vertexId", "nodeId" }, vector<string> columnTypes = { "INTEGER","INTEGER" }, bool primaryKeyCheck = false, vector<string> primaryKey = {"vertexId"});

		// export graph edges to SQL
		void exportGraph_SQL_edges(zGraph &graph, string tableName = "roadGraph_edges");

		// export road graph to SQL - only from to vertex per edge
		void exportGraph_SQL_edgeVertices(zGraph &graph, string tableName = "roadGraph_edgeVertices");

		

		// IMPORT GRAPH

		// import Map Attributes of a graph/mesh
		template <class T, class U>
		void import_MapAttributes(zAttributeMap<T, U> &inAttributeMap, string  outfilename = "C:/Users/vishu.b/Desktop/out.json" );

		// import Map Attributes and coreponding unorderedmap of a graph
		void import_MapAttributes(zAttributeMap<int, string> &inAttributeMap, zAttributeUnorderedMap<string, int> &correponding_inAttributeUnorderedMap, string  outfilename = "C:/Users/vishu.b/Desktop/out.json");


		// import road graph to SQL - vertex Data
		void importGraph_SQL_vertexData(string tag, zColor &col1, zColor &col2, zColor &col3, double domainMin = 0.1, double domainMid = 0.5, double domainMax = 0.9);


		
		// BUILDINGS

		// create building Mesh
		void createBuildingMesh();

		// compute station_roadEdge
		void computeBuilding_RoadVertex();

		// import building mesh attributes
		void import_BuildingMesh_Attributes(string  infilename = "C:/Users/vishu.b/Desktop/out_mesh.json");

		// compute building center
		void computeBuildingCenter();

		// compute building postcode
		void computeBuildingPostcode();

		// import mesh face Data from SQL table
		void import_BuildingMesh_SQL_faceData(string tag, zColor &col1, zColor &col2, zColor &col3, double domainMin = 0.1, double domainMid = 0.5, double domainMax = 0.9);

		// import property prices from SQL table
		void import_BuildingMesh_SQL_propertyData(string tag, zColor &col1, zColor &col2, zColor &col3, double domainMin = 0.1, double domainMid = 0.5, double domainMax = 0.9);

		// export building  to SQL - postcodes
		void exportBuilding_SQL_Postcodes();

		// export building  to SQL - RoadVertex
		void exportBuilding_SQL_RoadVertex(double &scaleFactor);



		// STATIONS

		// create stations
		void createStations(string tag, bool exportFile =false);

		// compute station_roadEdge
		void computeStation_RoadVertices(double distance, double &scaleFactor);

		// distance from stations
		void roadVertex_DistancefromStations(double &scaleFactor);

		// import station visitor data
		void import_stationGraph_SQL_visitorData(string tag, zColor &col1, zColor &col2, zColor &col3, double domainMin = 0.1, double domainMid = 0.5, double domainMax = 0.9);

				
	
		
		// DISPLAY METHODS
		
		// animate Mesh
		void animateMeshFace(zMesh &inMesh, zAttributeVector<zColor> &fromAttribute, zAttributeVector<zColor> &toAttribute, double timerValue);
		
		// get facecolor
		zAttributeVector<zColor> getFaceColorData(string tag, zDataLevel dataLevel, zColor & val1, zColor & val2, double domainMin, double domainMax);

		// UTILITIES


		// shortest Distance between two points
		void shortestDistance(double &lat1, double &lon1, double &lat2, double &lon2, string &nodeSubTableName, string &wayNodesSubTableName,string &wayTagsSubTableName);
		
	};
}


