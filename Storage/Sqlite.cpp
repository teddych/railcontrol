/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#include <map>
#include <string>

#include "Languages.h"
#include "Storage/Sqlite.h"

using DataModel::Accessory;
using DataModel::Track;
using DataModel::Feedback;
using DataModel::Loco;
using DataModel::Switch;
using Hardware::HardwareParams;
using std::map;
using std::string;
using std::vector;
using std::to_string;

namespace Storage
{

	// create instance of sqlite
	extern "C" SQLite* create_Sqlite(const StorageParams& params)
	{
		return new SQLite(params);
	}

	// delete instance of sqlite
	extern "C" void destroy_Sqlite(SQLite* sqlite)
	{
		delete (sqlite);
	}

	SQLite::SQLite(const StorageParams& params)
	:	filename(params.filename),
	 	logger(Logger::Logger::GetLogger("SQLite"))
	{
		logger->Info(Languages::TextOpeningSQLite, filename);
		int rc = sqlite3_open(filename.c_str(), &db);
		if (rc)
		{
			logger->Error(Languages::TextUnableToOpenSQLite, sqlite3_errmsg(db));
			sqlite3_close(db);
			db = nullptr;
			return;
		}

		// check if needed tables exist
		map<string, bool> tablenames;
		const char* query = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
		bool ret = Execute(query, CallbackListTables, &tablenames);
		if (ret == false)
		{
			return;
		}

		// create hardware table if needed
		if (tablenames["hardware"] == false)
		{
			bool ret = CreateTableHardware();
			if (ret == false)
			{
				return;
			}
		}

		// create objects table if needed
		if (tablenames["objects"] == false)
		{
			bool ret = CreateTableObjects();
			if (ret == false)
			{
				return;
			}
		}

		// create relations table if needed
		if (tablenames["relations"] == false)
		{
			bool ret = CreateTableRelations();
			if (ret == false)
			{
				return;
			}
		}

		// create settings table if needed
		if (tablenames["settings"] == false)
		{
			bool ret = CreateTableSettings();
			if (ret == false)
			{
				return;
			}
		}
	}

	SQLite::~SQLite()
	{
		if (db == nullptr)
		{
			return;
		}

		logger->Info(Languages::TextClosingSQLite);
		sqlite3_close(db);
		db = nullptr;

		Utils::Utils::CopyFile(logger, filename, filename + "." + std::to_string(time(0)));
	}

	int SQLite::CallbackListTables(void* v, int argc, char **argv, __attribute__((unused)) char **colName)
	{
		map<string, bool>* tablenames = static_cast<map<string, bool>*>(v);
		if (argc != 1)
		{
			return 0;
		}
		(*tablenames)[argv[0]] = true;
		return 0;
	}

	bool SQLite::DropTable(string table)
	{
		logger->Info(Languages::TextDroppingTable, table);
		string query = "DROP TABLE " + table + ";";
		return Execute(query);
	}

	bool SQLite::CreateTableHardware()
	{
		logger->Info(Languages::TextCreatingTable, "hardware");
		const string query = "CREATE TABLE hardware ("
			"controlid UNSIGNED TINYINT PRIMARY KEY,"
			" hardwaretype UNSIGNED TINYINT,"
			" name VARCHAR(50),"
			" arg1 VARCHAR(255),"
			" arg2 VARCHAR(255),"
			" arg3 VARCHAR(255),"
			" arg4 VARCHAR(255),"
			" arg5 VARCHAR(255));";
		return Execute(query);
	}

	bool SQLite::CreateTableObjects()
	{
		logger->Info(Languages::TextCreatingTable, "objects");
		const char* query = "CREATE TABLE objects ("
			"objecttype UNSIGNED TINYINT, "
			"objectid UNSIGNED SHORTINT, "
			"name VARCHAR(50), "
			"object SHORTTEXT,"
			"PRIMARY KEY (objecttype, objectid));";
		return Execute(query);
	}

	bool SQLite::CreateTableRelations()
	{
		logger->Info(Languages::TextCreatingTable, "relations");
		const char* query = "CREATE TABLE relations ("
			"type UNSIGEND TINYINT, "
			"objecttype1 UNSIGNED TINYINT, "
			"objectid1 UNSIGNED SHORTINT, "
			"objecttype2 UNSIGNED TINYINT, "
			"objectid2 UNSIGNED SHORTINT, "
			"priority UNSIGNED TINYINT, "
			"relation SHORTTEXT,"
			"PRIMARY KEY (type, objecttype1, objectid1, objecttype2, objectid2, priority));";
		return Execute(query);
	}

	bool SQLite::CreateTableSettings()
	{
		logger->Info(Languages::TextCreatingTable, "settings");
		const char* query =  "CREATE TABLE settings ("
			"key TINYTEXT, "
			"value SHORTTEXT,"
			"PRIMARY KEY (key));";
		return Execute(query);
	}

	void SQLite::SaveHardwareParams(const Hardware::HardwareParams& hardwareParams)
	{
		string query = "INSERT OR REPLACE INTO hardware VALUES ("
			+ to_string(hardwareParams.controlID) + ", "
			+ to_string(hardwareParams.hardwareType) + ", '"
			+ EscapeString(hardwareParams.name) + "', '"
			+ EscapeString(hardwareParams.arg1) + "', '"
			+ EscapeString(hardwareParams.arg2) + "', '"
			+ EscapeString(hardwareParams.arg3) + "', '"
			+ EscapeString(hardwareParams.arg4) + "', '"
			+ EscapeString(hardwareParams.arg5) + "');";
		Execute(query);
	}

	void SQLite::AllHardwareParams(std::map<controlID_t, Hardware::HardwareParams*>& hardwareParams)
	{
		const char* query = "SELECT controlid, hardwaretype, name, arg1, arg2, arg3, arg4, arg5 FROM hardware ORDER BY controlid;";
		Execute(query, CallbackAllHardwareParams, &hardwareParams);
	}

	// callback read hardwareparams
	int SQLite::CallbackAllHardwareParams(void* v, int argc, char **argv, __attribute__((unused)) char **colName)
	{
		map<controlID_t,HardwareParams*>* hardwareParams = static_cast<map<controlID_t,HardwareParams*>*>(v);
		if (argc != 8)
		{
			return 0;
		}
		controlID_t controlID = atoi(argv[0]);

		HardwareParams* params = new HardwareParams(controlID, static_cast<hardwareType_t>(atoi(argv[1])), argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
		(*hardwareParams)[controlID] = params;
		return 0;
	}

	// delete control
	void SQLite::DeleteHardwareParams(const controlID_t controlID)
	{
		string query = "DELETE FROM hardware WHERE controlid = " + to_string(controlID) + ";";
		Execute(query);
	}

	// save DataModelobject
	void SQLite::SaveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object)
	{
		string query = "INSERT OR REPLACE INTO objects (objecttype, objectid, name, object) VALUES ("
			+ to_string(objectType) + ", "
			+ to_string(objectID) + ", '"
			+ EscapeString(name) + "', '"
			+ EscapeString(object) + "');";
		Execute(query);
	}

	// delete DataModelobject
	void SQLite::DeleteObject(const objectType_t objectType, const objectID_t objectID)
	{
		string query = "DELETE FROM objects WHERE objecttype = " + to_string(objectType)
			+ " AND objectid = " + to_string(objectID) + ";";
		Execute(query);
	}

	// read DataModelobjects
	void SQLite::ObjectsOfType(const objectType_t objectType, vector<string>& objects)
	{
		string query = "SELECT object FROM objects WHERE objecttype = " + to_string(objectType) + " ORDER BY objectid;";
		Execute(query, CallbackStringVector, &objects);
	}

	// save DataModelrelation
	void SQLite::SaveRelation(const objectType_t objectType1, const objectID_t objectID1, const objectType_t objectType2, const objectID_t objectID2, const priority_t priority, const std::string& relation)
	{
		string query = "INSERT OR REPLACE INTO relations (objecttype1, objectid1, objecttype2, objectid2, priority, relation) VALUES ("
			+ to_string(objectType1) + ", "
			+ to_string(objectID1) + ", "
			+ to_string(objectType2) + ", "
			+ to_string(objectID2) + ", "
			+ to_string(priority) + ", '"
			+ EscapeString(relation) + "');";
		Execute(query);
	}

	// delete DataModelrelaton
	void SQLite::DeleteRelationFrom(const objectType_t objectType, const objectID_t objectID)
	{
		string query = "DELETE FROM relations WHERE objecttype1 = " + to_string(objectType)
			+ " AND objectid1 = " + to_string(objectID) + ";";
		Execute(query);
	}

	// delete DataModelrelaton
	void SQLite::DeleteRelationTo(const objectType_t objectType, const objectID_t objectID)
	{
		string query = "DELETE FROM relations WHERE objecttype2 = " + to_string(objectType)
			+ " AND objectid2 = " + to_string(objectID) + ";";
		Execute(query);
	}

	// read DataModelrelations
	void SQLite::RelationsFrom(const objectType_t objectType, const objectID_t objectID, vector<string>& relations)
	{
		string query = "SELECT relation FROM relations WHERE objecttype1 = " + to_string(objectType)
			+ " AND objectid1 = " + to_string(objectID) + " ORDER BY priority ASC;";
		Execute(query, CallbackStringVector, &relations);
	}

	// read DataModelrelations
	void SQLite::RelationsTo(const objectType_t objectType, const objectID_t objectID, vector<string>& relations)
	{
		string query = "SELECT relation FROM relations WHERE objecttype2 = " + to_string(objectType)
			+ " AND objectid2 = " + to_string(objectID) + ";";
		Execute(query, CallbackStringVector, &relations);
	}

	void SQLite::SaveSetting(const string& key, const string& value)
	{
		Execute("INSERT OR REPLACE INTO settings (key, value) values ('" + EscapeString(key) + "', '" + EscapeString(value) + "');");
	}

	string SQLite::GetSetting(const string& key)
	{
		vector<string> values;
		string query = "SELECT value FROM settings WHERE key = '" + EscapeString(key) + "';";
		bool ret = Execute(query, CallbackStringVector, &values);
		if (ret == false || values.size() == 0)
		{
			return "";
		}

		return values[0];
	}

	// callback read all DataModel objects/relations
	int SQLite::CallbackStringVector(void* v, int argc, char **argv, __attribute__((unused)) char **colName)
	{
		vector<string>* objects = static_cast<vector<string>*>(v);
		if (argc != 1)
		{
			return 0;
		}
		objects->push_back(argv[0]);
		return 0;
	}

	void SQLite::StartTransaction()
	{
		Execute("BEGIN TRANSACTION");
	}

	void SQLite::CommitTransaction()
	{
		Execute("COMMIT");
	}

	bool SQLite::Execute(const char* query, sqlite3_callback callback = nullptr, void* result = nullptr)
	{
		if (db == nullptr)
		{
			return false;
		}

		logger->Debug(Languages::TextQuery, query);

		char* dbError = nullptr;
		int rc = sqlite3_exec(db, query, callback, result, &dbError);
		if (rc == SQLITE_OK)
		{
			return true;
		}

		logger->Error(Languages::TextSQLiteErrorQuery, dbError, query);
		sqlite3_free(dbError);
		return false;
	}

} // namespace Storage
