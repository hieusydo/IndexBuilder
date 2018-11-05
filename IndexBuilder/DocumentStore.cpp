//
//  DocumentStore.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 11/2/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "DocumentStore.hpp"

// callback for sqlite3_exec
static int selectCallback(void* data, int argc, char **argv, char **azColName) {
    // This is how to extract result of the sql statement
    DocumentEntry* res = (DocumentEntry *) data;
    res->doc = argv[0];
    return 0;
}

// callback for sqlite3_exec
static int callback(void *notUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

DocumentStore::DocumentStore(const std::string& database) : databaseType(database) {
    // Connect to the db
    if (database == "redis") {
        try {
            redisClient.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
                if (status == cpp_redis::client::connect_state::dropped) {
                    std::cout << "client disconnected from " << host << ":" << port << std::endl;
                }
            });
        } catch (const cpp_redis::redis_error& e) {
            std::cerr << e.what() << '\n';
            exit(1);
        }
    } else if (database == "sqlite") {
        int rc = sqlite3_open("/Users/hieudosy/Documents/Projects/IndexBuilder/documentstore.db", &sqliteDb);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(sqliteDb) << '\n';
            exit(1);
        } else {
            std::cout << "Opened database successfully\n";
        }
    } else {
        std::cerr << "Database not supported\n";
        exit(1);
    }
}

void DocumentStore::createTable() {
    // Clear and create a new table
    purge();
    std::string createTableStmt = "CREATE TABLE DOCUMENTSTORE(ID INT PRIMARY KEY NOT NULL, DOCUMENT TEXT NOT NULL);";
    char* zErrMsg;
    int rc = sqlite3_exec(sqliteDb, createTableStmt.c_str(), callback, 0, &zErrMsg);
    if(rc != SQLITE_OK) {
        std::cerr << "SQL error: " <<sqlite3_errmsg(sqliteDb) << "\n";
        sqlite3_free(zErrMsg);
    }
}

std::string DocumentStore::getDocument(size_t did) {
    std::string doc;
    if (databaseType == "redis") {
        // TODO: hang when did DNE
        redisClient.get(std::to_string(did), [&doc, &did](cpp_redis::reply& reply) {
            doc = reply.as_string();
        });
        redisClient.sync_commit();
    } else if (databaseType == "sqlite") {
        std::string selectStmt = "SELECT DOCUMENT FROM DOCUMENTSTORE WHERE ID =" + std::to_string(did) + ";";
        char* zErrMsg;
        DocumentEntry data;
        int rc = sqlite3_exec(sqliteDb, selectStmt.c_str(), selectCallback, &data, &zErrMsg);
        if(rc != SQLITE_OK) {
            std::cerr << "SQL error: " <<sqlite3_errmsg(sqliteDb) << "\n";
            sqlite3_free(zErrMsg);
        }
        doc = data.doc;
    }
    return doc;
}

void DocumentStore::putDocument(size_t did, const std::string& doc) {
    if (databaseType == "redis") {
        redisClient.set(std::to_string(did), doc);
        redisClient.sync_commit();
    } else if (databaseType == "sqlite") {
        char* zErrMsg;
        int rc;
        
        // Warning: Turn off sync and change mode for optimization
        // More: https://stackoverflow.com/q/1711631
        rc = sqlite3_exec(sqliteDb, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
        if(rc != SQLITE_OK) {
            std::cerr << "SQL error: " <<sqlite3_errmsg(sqliteDb) << "\n";
            sqlite3_free(zErrMsg);
        }
        rc = sqlite3_exec(sqliteDb, "PRAGMA journal_mode = MEMORY", NULL, NULL, &zErrMsg);
        if(rc != SQLITE_OK) {
            std::cerr << "SQL error: " <<sqlite3_errmsg(sqliteDb) << "\n";
            sqlite3_free(zErrMsg);
        }
        
        // Use prepare statements and binding to avoid escape char errors of sql inserts
        sqlite3_stmt *stmt;
        std::string prepareStr = "INSERT INTO DOCUMENTSTORE(ID,DOCUMENT) VALUES(?, ?)";
        sqlite3_prepare_v2(sqliteDb, prepareStr.c_str(), (int)prepareStr.size(), &stmt, NULL);
        sqlite3_bind_int64(stmt, 1, did);
        sqlite3_bind_text64(stmt, 2, doc.c_str(), doc.size(), SQLITE_TRANSIENT, SQLITE_UTF8);
        sqlite3_step(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_reset(stmt);
    }
}

void DocumentStore::purge() {
    if (databaseType == "redis") {
        redisClient.flushdb();
        redisClient.sync_commit();
    } else if (databaseType == "sqlite") {
        std::string insertStr = "DROP TABLE IF EXISTS DOCUMENTSTORE;";
        char* zErrMsg;
        int rc = sqlite3_exec(sqliteDb, insertStr.c_str(), callback, 0, &zErrMsg);
        if(rc != SQLITE_OK) {
            std::cerr << "SQL error: " <<sqlite3_errmsg(sqliteDb) << "\n";
            sqlite3_free(zErrMsg);
        }
    }
}

void DocumentStore::close() {
    sqlite3_close(sqliteDb);
}
