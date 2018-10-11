//
//  UrlTable.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "UrlTable.hpp"

void UrlTable::addEntry(const std::string& u, size_t d) {
    urlTable.push_back(UrlEntry(u, d));
}

void UrlTable::display() const {
    std::cout << "==========\nUrlTable:\n";
    for (const UrlEntry& e : urlTable)
        std::cout << e.documentLen << ' ' << e.url << '\n';
}

size_t UrlTable::size() const {
    return urlTable.size();
}
