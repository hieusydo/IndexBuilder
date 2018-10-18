//
//  UrlTable.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef UrlTable_hpp
#define UrlTable_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class UrlTable {
    struct UrlEntry {
        UrlEntry(const std::string& u, size_t d);
        std::string url;
        size_t documentLen;
    };
private:
    // The index of the entry is the docId
    std::vector<UrlEntry> urlTable;
    
public:
    // Add url u with content length d to the table
    void addEntry(const std::string& u, size_t d);
    
    // Print table row by row
    void display() const;
    
    // Returns size of urlTable vector
    size_t size() const;
    
    // Write the rows of urlTable to disk
    void writeToDisk(const std::string& pathname) const;
};

#endif /* UrlTable_hpp */
