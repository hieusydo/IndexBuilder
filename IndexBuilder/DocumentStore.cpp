//
//  DocumentStore.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 11/2/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "DocumentStore.hpp"

DocumentStore::DocumentStore() {
    try {
        client.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
            if (status == cpp_redis::client::connect_state::dropped) {
                std::cout << "client disconnected from " << host << ":" << port << std::endl;
            }
        });
    } catch (const cpp_redis::redis_error& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }
    // Clean out storage each time
//    purge();
}

std::string DocumentStore::getDocument(size_t did) {
    // TODO: hang when did DNE
    std::string doc;
    client.get(std::to_string(did), [&doc, &did](cpp_redis::reply& reply) {
        doc = reply.as_string();
    });
    client.sync_commit();
    return doc;
}

void DocumentStore::putDocument(size_t did, const std::string& doc) {
    client.set(std::to_string(did), doc);
    client.sync_commit();
}

void DocumentStore::purge() {
    client.flushdb();
    client.sync_commit();
}
