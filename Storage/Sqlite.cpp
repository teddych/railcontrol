/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2025 by Teddy / Dominik Mahrer - www.railcontrol.org

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

#include <ctime>
#include <map>
#include <string>

#include "Languages.h"
#include "Storage/Sqlite.h"
#include "Utils/Integer.h"

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
	SQLite::SQLite(const StorageParams* params)
	:	filename(params->filename),
	 	logger(Logger::Logger::GetLogger("SQLite")),
	 	keepBackups(params->keepBackups)
	{
		Utils::Utils::RemoveOldBackupFiles (logger, filename, keepBackups);
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
		string tableNameRelations = "relations";
		if (tablenames[tableNameRelations] == true)
		{
			ret = CheckTableRelations();
		}
		else {
			ret = CreateTableRelations(tableNameRelations);
		}
		if (ret == false)
		{
			return;
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

		if (keepBackups == 0)
		{
			return;
		}
		Utils::Utils::CopyFile(logger, filename, filename + "." + std::to_string(std::time(nullptr)));
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

	bool SQLite::DropTable(const string table)
	{
		logger->Info(Languages::TextDroppingTable, table);
		string query = "DROP TABLE " + table + ";";
		return Execute(query);
	}

	bool SQLite::CreateTableHardware()
	{
		logger->Info(Languages::TextCreatingTable, "hardware");
		const string query = "CREATE TABLE hardware ("
			"controlid UNSIGNED TINYINT,"
			" hardwaretype UNSIGNED TINYINT,"
			" name VARCHAR(50),"
			" arg1 VARCHAR(255),"
			" arg2 VARCHAR(255),"
			" arg3 VARCHAR(255),"
			" arg4 VARCHAR(255),"
			" arg5 VARCHAR(255),"
			"PRIMARY KEY (controlid));";
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

	bool SQLite::CreateTableRelations(const string& name)
	{
		logger->Info(Languages::TextCreatingTable, name);
		const string query = "CREATE TABLE " + name + " ("
			"type UNSIGNED TINYINT, "
			"objectid1 UNSIGNED SHORTINT, "
			"objecttype2 UNSIGNED TINYINT, "
			"objectid2 UNSIGNED SHORTINT, "
			"priority UNSIGNED TINYINT, "
			"relation SHORTTEXT,"
			"PRIMARY KEY (type, objectid1, objecttype2, objectid2, priority));";
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

	struct TableInfo
	{
		public:
			int cid;
			string name;
			string type;
			bool notNull;
			bool defaultNull;
			string defaultValue;
			int primaryKey;
	};

	bool SQLite::CheckTableRelations()
	{
		vector<TableInfo> tableInfos;
		string query = "PRAGMA table_info('relations');";
		bool ret = Execute(query, CallbackTableInfo, &tableInfos);
		if (ret == false)
		{
			return false;
		}

		string name = "relations";
		if (tableInfos.size() != 6)
		{
			return DropTable(name) && CreateTableRelations(name);
		}

		if (tableInfos[0].name.compare("type") != 0)
		{
			return UpdateTableRelations1();
		}

		// FIXME: simple check, extend with check if each object really exists
		query = "DELETE FROM relations WHERE objectid1 like 0;";
		ret = Execute(query);
		if (ret == false)
		{
			return false;
		}

		logger->Info(Languages::TextIsUpToDate, name);
		return true;
	}

	bool SQLite::UpdateTableRelations1()
	{
		const string tableName = "relations";
		const string tempTableName = "relations_temp";
		bool ret = CreateTableRelations(tempTableName);
		if (ret == false)
		{
			return false;
		}
		logger->Info(Languages::TextCopyingFromTo, tableName, tempTableName);
		const string query = "INSERT INTO " + tempTableName + " SELECT objecttype1 * 8, objectid1, objecttype2, objectid2, priority, relation FROM " + tableName + ";";
		ret = Execute(query);
		if (ret == false)
		{
			return false;
		}
		ret = DropTable(tableName);
		if (ret == false)
		{
			return false;
		}
		logger->Info(Languages::TextRenamingFromTo, tempTableName, tableName);
		return RenameTable(tempTableName, tableName);
	}

	bool SQLite::RenameTable(const string& oldName, const string& newName)
	{
		const string query = "ALTER TABLE " + oldName + " RENAME TO " + newName + ";";
		return Execute(query);
	}

	int SQLite::CallbackTableInfo(void* v, int argc, char **argv, __attribute__((unused)) char **colName)
	{
		vector<TableInfo>* tableInfos = static_cast<vector<TableInfo>*>(v);
		if (argc != 6)
		{
			return 0;
		}
		TableInfo tableInfo;
		tableInfo.cid = Utils::Integer::StringToInteger(argv[0]);
		tableInfo.name = argv[1];
		tableInfo.type = argv[2];
		tableInfo.notNull = Utils::Integer::StringToInteger(argv[3]) > 0;
		tableInfo.defaultNull = argv[4] == nullptr;
		tableInfo.defaultValue = argv[4] != nullptr ? argv[4] : "";
		tableInfo.primaryKey = Utils::Integer::StringToInteger(argv[5]);
		tableInfos->push_back(tableInfo);
		return 0;
	}

	void SQLite::SaveHardwareParams(const Hardware::HardwareParams& hardwareParams)
	{
		string query = "INSERT OR REPLACE INTO hardware VALUES ("
			+ to_string(hardwareParams.GetControlID()) + ", "
			+ to_string(hardwareParams.GetHardwareType()) + ", '"
			+ EscapeString(hardwareParams.GetName()) + "', '"
			+ EscapeString(hardwareParams.GetArg1()) + "', '"
			+ EscapeString(hardwareParams.GetArg2()) + "', '"
			+ EscapeString(hardwareParams.GetArg3()) + "', '"
			+ EscapeString(hardwareParams.GetArg4()) + "', '"
			+ EscapeString(hardwareParams.GetArg5()) + "');";
		Execute(query);
	}

	void SQLite::AllHardwareParams(std::map<ControlID, Hardware::HardwareParams*>& hardwareParams)
	{
		const char* query = "SELECT controlid, hardwaretype, name, arg1, arg2, arg3, arg4, arg5 FROM hardware ORDER BY controlid;";
		Execute(query, CallbackAllHardwareParams, &hardwareParams);
	}

	// callback read hardwareparams
	int SQLite::CallbackAllHardwareParams(void* v, int argc, char **argv, __attribute__((unused)) char **colName)
	{
		map<ControlID,HardwareParams*>* hardwareParams = static_cast<map<ControlID,HardwareParams*>*>(v);
		if (argc != 8)
		{
			return 0;
		}
		ControlID controlID = Utils::Integer::StringToInteger(argv[0]);

		HardwareParams* params = new HardwareParams(controlID, static_cast<HardwareType>(Utils::Integer::StringToInteger(argv[1])), argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
		(*hardwareParams)[controlID] = params;
		return 0;
	}

	// delete control
	void SQLite::DeleteHardwareParams(const ControlID controlID)
	{
		string query = "DELETE FROM hardware WHERE controlid = " + to_string(controlID) + ";";
		Execute(query);
	}

	// save DataModel object
	void SQLite::SaveObject(const ObjectType objectType, const ObjectID objectID, const std::string& name, const std::string& object)
	{
		string query = "INSERT OR REPLACE INTO objects (objecttype, objectid, name, object) VALUES ("
			+ to_string(objectType) + ", "
			+ to_string(objectID) + ", '"
			+ EscapeString(name) + "', '"
			+ EscapeString(object) + "');";
		Execute(query);
	}

	// delete DataModel object
	void SQLite::DeleteObject(const ObjectType objectType, const ObjectID objectID)
	{
		string query = "DELETE FROM objects WHERE objecttype = " + to_string(objectType)
			+ " AND objectid = " + to_string(objectID) + ";";
		Execute(query);
	}

	// read DataModel objects
	void SQLite::ObjectsOfType(const ObjectType objectType, vector<string>& objects)
	{
		string query = "SELECT object FROM objects WHERE objecttype = " + to_string(objectType) + " ORDER BY objectid;";
		Execute(query, CallbackStringVector, &objects);
	}

	// save DataModel relation
	void SQLite::SaveRelation(const DataModel::Relation::RelationType type, const ObjectID objectID1, const ObjectType objectType2, const ObjectID objectID2, const Priority priority, const std::string& relation)
	{
		string query = "INSERT OR REPLACE INTO relations (type, objectid1, objecttype2, objectid2, priority, relation) VALUES ("
			+ to_string(type) + ", "
			+ to_string(objectID1) + ", "
			+ to_string(objectType2) + ", "
			+ to_string(objectID2) + ", "
			+ to_string(priority) + ", '"
			+ EscapeString(relation) + "');";
		Execute(query);
	}

	// delete DataModel relation
	void SQLite::DeleteRelationsFrom(const DataModel::Relation::RelationType type, const ObjectID objectID)
	{
		string query = "DELETE FROM relations WHERE type = " + to_string(type)
			+ " AND objectid1 = " + to_string(objectID) + ";";
		Execute(query);
	}

	// delete DataModel relation
	void SQLite::DeleteRelationsTo(const ObjectType objectType, const ObjectID objectID)
	{
		string query = "DELETE FROM relations WHERE objecttype2 = " + to_string(objectType)
			+ " AND objectid2 = " + to_string(objectID) + ";";
		Execute(query);
	}

	// read DataModel relations
	void SQLite::RelationsFrom(const DataModel::Relation::RelationType type, const ObjectID objectID, vector<string>& relations)
	{
		string query = "SELECT relation FROM relations WHERE type = " + to_string(type)
			+ " AND objectid1 = " + to_string(objectID) + " ORDER BY priority ASC;";
		Execute(query, CallbackStringVector, &relations);
	}

	// read DataModel relations
	void SQLite::RelationsTo(const ObjectType objectType, const ObjectID objectID, vector<string>& relations)
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
		Execute("BEGIN TRANSACTION;");
	}

	void SQLite::CommitTransaction()
	{
		Execute("COMMIT;");
	}

	bool SQLite::Execute(const char* query, sqlite3_callback callback = nullptr, void* result = nullptr)
	{
		if (db == nullptr)
		{
			return false;
		}

		char* dbError = nullptr;
		int rc = sqlite3_exec(db, query, callback, result, &dbError);
		if (rc == SQLITE_OK)
		{
			int affected = sqlite3_changes(db);

			if (affected)
			{
				logger->Debug(Languages::TextQueryAffected, query, affected);
			}
			else
			{
				logger->Debug(Languages::TextQuery, query);
			}
			return true;
		}

		logger->Error(Languages::TextSQLiteErrorQuery, dbError, query);
		sqlite3_free(dbError);
		return false;
	}

	string SQLite::EscapeString(const string& input)
	{
		string output;
		for (size_t i = 0; i < input.size(); ++i)
		{
			if (input[i] == '\'')
			{
				output += "'";
			}
			output += input[i];
		}
		return output;
	}
} // namespace Storage
