// DESCRIPTION:
//      Data base using SQLite (http://www.dreamincode.net/forums/topic/122300-sqlite-in-c/)
//


#pragma once

#define EXPORT_CSV "CSV"

#define TABLE_CREATE "CreateTable"
#define TABLE_INSERT "Insert"
#define TABLE_SELECT "Select"
#define TABLE_SELECTEXISTS "SelectExists"
#define TABLE_SELECTCREATE "Selection and Creation"
#define TABLE_SELECTCOUNT "Select Number of Rows"
#define TABLE_UPDATE "Update"
#define TABLE_DROP "Delete"

#define Max_Latlon 4
#define Max_ExpresionTree 900

#include <vector>
#include <algorithm>    // std::sort
#include <SQLite/sqlite3.h>
#include<string>
#include <zSpace\zVector.h>
using namespace std;



namespace zSpace
{
	// define structs


	// define class
	class zDatabase
	{
	private:
		sqlite3 *database;
	public:

		//---- CONSTRUCTOR

		// default constructor
		zDatabase();

		//  constructor
		zDatabase(char* filename);

		//---- DESTRUCTOR

		//  destructor
		~zDatabase();

		//---- METHODS

		//  Open database
		bool open(char* filename);

		//  close database
		void close();

		// sqlCommand -  CREATE TABLE , INSERT ,SELECT
		bool sqlCommand(vector<string> &sqlStatment, string sqlCommandType, bool displayError, vector<string> &outStatment, bool colType = true);

		//create Table
		void tableCreate(vector<string> &sqlStatment, string &tableName, vector<string> &columnNames, vector<string> &columnTypes, bool checkPrimaryKey, vector<string> &primarykey);

		//insert Table
		void tableInsert(vector<string> &sqlStatment, string &tableName, vector<string>& columnNames, vector<string> &values);

	


		//Select
		//void tableSelect(vector<string> &sqlStatment, string &tableName, bool specificColumnData, string &columnNames, bool checkCondition, string &ConditionStmt);

		//Select
		void tableSelect(vector<string> &sqlStatment, string &tableName, bool checkRowId, bool specificColumnData, vector<string> &columnNames, bool checkCondition, string &ConditionStmt, bool checkOrderBy, string &OrderBy);

		// Select row Count
		void tableSelectCount(vector<string> &sqlStatment, string &tableName);

		//Select Exists
		void tableSelectExists(vector<string> &sqlStatment, string &tableName, bool specificColumnData, string &columnNames, bool checkCondition, string &ConditionStmt);

		//  sub table Insert 
		void subTableInsert(vector<string> &sqlStatment, string &subTableName, string &tableName, bool specificColumnData, string &columnNames, bool checkCondition, string &ConditionStmt);

		//Update
		void tableUpdate(vector<string> &sqlStatment, string &tableName, string &columnNames_Values, string &ConditionStmt);

		// Export -  CSV
		void exportCSV(vector<string> &sqlStatment, string &tableName, string &directoryPath, string &fileName);

		
		
		// -- NODE TABLE

		// statement
		void nodeTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists );

		//Insert
		void nodeTableInsert(vector<string> &sqlStatment, string &tableName, string &nodeID, double &lat, double &lon);

		// NODE Selection Statment given bounding box
		void nodeSubTableInsertfromBB(vector<string> &sqlStatment, string &subtableName, string &tableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);

		// -- NODE-WAY TABLE
		// statement
		void nodeWayTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		//Insert
		void nodeWayTableInsert(vector<string> &sqlStatment, string &tableName, string &nodeID, string &wayId, string &next, string &prev);

		// -- WAY TABLE

		// statement
		
		void wayTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		void wayNodesTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		void wayTagsTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);


		//Insert
		void wayTableInsert(vector<string> &sqlStatment, string &tableName, string &wayId);

		void wayNodesTableInsert(vector<string> &sqlStatment, string &tableName, string &wayID, string &wayNodeId);

		void wayTagsTableInsert(vector<string> &sqlStatment, string &tableName, string &wayID, string &wayTagKey, string &wayTagValue);


	
		// WAYNODES SUB TABLE INSERT from BB
		void wayNodesSubTableInsertfromBB(vector<string> &sqlStatment, string &wayNodeSubTableName, string &wayNodeTableName, string &nodetableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);

		// WAYTAGS SUB TABLE INSERT from BB
		void wayTagsSubTableInsertfromBB(vector<string> &sqlStatment, string &wayTagSubTableName, string &wayTagTableName, string &wayNodeTableName, string &nodetableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);


		// -- RELATION TABLE


		// relation members statement
		void relationMembersTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		// relation tags statement
		void relationTagsTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);


		//Insert to RelationMembers
		void relationMembersTableInsert(vector<string> &sqlStatment, string &tableName, string &relationID, string &relationMemberType, string &relationMemberRef, string &relationMemberRole);

		//Insert to Relationags
		void relationTagsTableInsert(vector<string> &sqlStatment, string &tableName, string &relationID, string &relationTagKey, string &relationTagValue);

		
		//  RELATION MEMBER subtable insert given bounding box : Member Type Node and ways
		void relationMemberSubTableInsertfromBB(vector<string> &sqlStatment, string &relationMemberSubTableName, string &relationMemberTableName, string &nodeSubTableName, string &wayNodesSubTableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);

		//  RELATION MEMBER subtable insert given bounding box : Member Type Relation
		void relationMemberSubTableInsertfromBB(vector<string> &sqlStatment, string &relationMemberSubTableName, string &relationMemberTableName, bool specificColumnData, string &columnNames);


		//  RELATION TAG subtable insert given bounding box : Member Type Node and ways
		void relationTagSubTableInsertfromBB(vector<string> &sqlStatment, string &relationTagSubTableName, string &relationTagTableName, string &relationMemberTableName, string &nodeSubTableName, string &wayNodesSubTableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);

		//  RELATION Tag subtable insert given bounding box : Member Type Relation
		void relationTagSubTableInsertfromBB(vector<string> &sqlStatment, string &relationTagSubTableName, string &relationTagTableName, string &relationMemberTableName, bool specificColumnData, string &columnNames);

		// -- POSTCODE_LATLON TABLE

		// statement
		void postCodeLatLonTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		//Insert
		void postCodeLatLonTableInsert(vector<string> &sqlStatment, string &tableName, string &postCode, double &lat, double &lon);

		// -- POSTCODE_PROPERTY TABLE

		// statement
		void postCodePropertyTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		//Insert
		void postCodePropertyTableInsert(vector<string> &sqlStatment, string &tableName, string &propertyId, string &postCode, double &propertyPrice, string &propertyType, string &propertyBuild, string &estateType);


		// --- BUILDING_OUTLINE TABLE
		// statement
		void buildingOutlineTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		//Insert
		void buildingOutlineTableInsert(vector<string> &sqlStatment, string &tableName, string &buildingId, string &shapeId, double &buildingHeight, double &buildingArea, double &buildingPerimeter);

		// Insert given bounding box
		void buildingOutlineSubTableInsertfromBB(vector<string> &sqlStatment, string &buildingOutlineSubTableName, string &buildingOutlineTableName, string &buildingNodeTableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);


		// --- BUILDING_NODES TABLE
		// statement
		void buildingNodeTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		//Insert
		void buildingNodeTableInsert(vector<string> &sqlStatment, string &tableName, string &shapeId, double &lat, double &lon);

		// Insert given bounding box
		void buildingNodeSubTableInsertfromBB(vector<string> &sqlStatment, string &subtableName, string &tableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);


		// --- ROAD_LINES TABLE
		// statement
		void roadLineTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		// Insert
		void roadLineTableInsert(vector<string> &sqlStatment, string &tableName, string &shapeId, string &roadId, string &roadClass, string &roadNumber, string &roadName);

		// Insert given bounding box
		void roadLineSubTableInsertfromBB(vector<string> &sqlStatment, string &roadLineSubTableName, string &roadLineTableName, string &roadNodeTableName, string &roadLine_NodeTableName, bool specificColumnData, vector<string> &columnNames, double(&latLon)[Max_Latlon]);

		// --- ROADLINES_NODES TABLE
		// statement
		void roadLine_NodesTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		// Insert
		void roadLine_NodesTableInsert(vector<string> &sqlStatment, string &tableName, string &shapeId, string &nodeId);

		// Insert given bounding box
		void roadLine_NodesInsertfromBB(vector<string> &sqlStatment, string &roadLineNodesSubTableName, string &roadLineNodeTableName, string &roadNodeTableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);


		// --- ROAD_NODES TABLE
		// statement
		void roadNodeTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

		// Insert
		void roadNodeTableInsert(vector<string> &sqlStatment, string &tableName, string &nodeId, double &lat, double &lon);

		// Insert given bounding box
		void roadNodeSubTableInsertfromBB(vector<string> &sqlStatment, string &subtableName, string &tableName, bool specificColumnData, string &columnNames, double(&latLon)[Max_Latlon]);


		// --- TRAFFIC TABLE
		
		// statement
		void trafficEdgeTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);
		void trafficNodeTableStatement(vector<string> &sqlStatment, string &tableName, bool checkExists);

	};
}


