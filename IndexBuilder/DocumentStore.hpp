//
//  DocumentStore.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 11/2/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef DocumentStore_hpp
#define DocumentStore_hpp

#include <cpp_redis/cpp_redis>
#include <sqlite3.h>
#include <sstream>
#include <string>

// Struct to store did and the payload
struct DocumentEntry {
    size_t id;
    std::string doc;
};

// Wrapper around the underying database
class DocumentStore {
private:
    // currently support sqlite3 (recommended) and redis
    std::string databaseType;
    
    // Better used as a cache layer
    cpp_redis::client redisClient;
    
    // Main storage layer
    sqlite3* sqliteDb;
    
    // Flush redis db or drop sqlite table
    void purge();

public:
    // Connect to the underlying database
    DocumentStore(const std::string& database);
    
    // Return the payload of the did
    std::string getDocument(size_t did);
    
    // Put a <did, doc> k,v pair to redis /
    // Insert a row (did, doc) to sqlite
    void putDocument(size_t did, const std::string& doc);
    
    // Create a new table in the sqlite db
    void createTable();
    
    // Close connection with sqlite db
    void close();
};

#endif /* DocumentStore_hpp */
