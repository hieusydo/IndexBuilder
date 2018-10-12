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
    for (size_t i = 0; i < urlTable.size(); ++i)
        std::cout << i << urlTable[i].documentLen << ' ' << urlTable[i].url << '\n';
}

size_t UrlTable::size() const {
    return urlTable.size();
}
