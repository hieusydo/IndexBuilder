//
//  IndexOutput.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "IndexOutput.hpp"

void IndexOutput::addPage(const std::map<std::string, size_t>& freqMap, size_t docId, bool debug) {
    std::ofstream tmpFile(indexName, std::ios::binary | std::ios::out);
    
    
    // Create lexicon structure and posting
    size_t line = 0;
    for (const auto& e : freqMap) {
        Posting newPosting = Posting(docId, e.second);
        
        if (debug) {
            tmpFile << newPosting.docId << ' ' << newPosting.frequency << '\n';
        } else {
            tmpFile.write(reinterpret_cast<const char *>(&newPosting.docId), sizeof(newPosting.docId));
            tmpFile.write(reinterpret_cast<const char *>(&newPosting.frequency), sizeof(newPosting.frequency));
        }
        lexicon[e.first] = LexiconEntry(line, 1);
        ++line;
    }
    
    // Write posting to small file on disk
    tmpFile.close();
}

void IndexOutput::display() const {
    std::cout << "==========\nIndexOutput:\n";
    for (const auto& e : lexicon)
        std::cout << e.first << ' ' << e.second.invListPos << ' ' << e.second.invListLen << '\n';
}

bool IndexOutput::operator<(const IndexOutput& rhs) const {
    return indexName < rhs.indexName;
}

std::string IndexOutput::getIndexName() const {
    return indexName;
}

bool Posting::operator<(const Posting& rhs) const {
    return docId < rhs.docId;
}
