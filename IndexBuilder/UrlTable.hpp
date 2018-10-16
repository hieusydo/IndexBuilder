//
//  UrlTable.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef UrlTable_hpp
#define UrlTable_hpp

#include "common.hpp"

class UrlTable {
    struct UrlEntry {
        UrlEntry(const std::string& u, size_t d) : url(u), documentLen(d) {}
        std::string url;
        size_t documentLen;
    };
private:
    std::vector<UrlEntry> urlTable;
    
public:
    void addEntry(const std::string& u, size_t d);
    void display() const;
    size_t size() const;
    void writeToDisk(const std::string& pathname) const;
};

#endif /* UrlTable_hpp */
