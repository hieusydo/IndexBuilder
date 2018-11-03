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

class DocumentStore {
private:
    cpp_redis::client client;
    
    void purge();

public:
    DocumentStore();
    
    std::string getDocument(size_t did);
    
    void putDocument(size_t did, const std::string& doc);
};

#endif /* DocumentStore_hpp */
