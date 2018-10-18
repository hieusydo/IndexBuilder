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
        std::cout << i << "  " << urlTable[i].url << "  " << urlTable[i].documentLen << '\n';
}

size_t UrlTable::size() const {
    return urlTable.size();
}

void UrlTable::writeToDisk(const std::string& pathname) const {
    std::ofstream ofs(pathname);
    if (!ofs) {
        std::cerr << "Cannot open " << pathname << '\n';
        exit(1);
    }
    for (size_t i = 0; i < urlTable.size(); ++i)
        ofs << i << ' ' << urlTable[i].url << ' ' << urlTable[i].documentLen << '\n';
    ofs.close();
}

UrlTable::UrlEntry::UrlEntry(const std::string& u, size_t d) : url(u), documentLen(d) {}
