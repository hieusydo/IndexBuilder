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
    size_t pos = 0;
    for (const auto& e : freqMap) {
        Posting newPosting = Posting(docId, e.second);
        
        if (debug) {
            tmpFile << newPosting.docId << ' ' << newPosting.frequency << '\n';
        } else {
            tmpFile.write(reinterpret_cast<const char *>(&newPosting.docId), sizeof(newPosting.docId));
            tmpFile.write(reinterpret_cast<const char *>(&newPosting.frequency), sizeof(newPosting.frequency));
        }
        size_t entryLen = sizeof(newPosting.docId) + sizeof(newPosting.frequency);
        lexicon[e.first] = LexiconEntry(pos, entryLen);
        pos += entryLen;
    }
    
    // Write posting to small file on disk
    tmpFile.close();
}

void IndexOutput::display() const {
    std::cout << "==========\nIndexOutput:\n";
    for (const auto& e : lexicon)
        std::cout << e.first << ' ' << e.second.invListPos << ' ' << e.second.invListLen << '\n';
}

std::string IndexOutput::getIndexName() const {
    return indexName;
}

HeapEntry IndexOutput::getNextTerm() {
    HeapEntry res;
    res.term = lexicon.begin()->first;
//    res.invListPos = lexicon.begin()->second.invListPos;
//    res.invListLen = lexicon.begin()->second.invListLen;
    std::ifstream ifs(indexName, std::ios::binary | std::ios::out);
    if (!ifs) {
        std::cerr << "Failure to open " << indexName << '\n';
        exit(1);
    }
    ifs.seekg(lexicon.begin()->second.invListPos);
    ifs.read(reinterpret_cast<char *>(&res.docId), sizeof(res.docId));
    ifs.read(reinterpret_cast<char *>(&res.frequency), sizeof(res.frequency));
        
    lexicon.erase(res.term);
    if (lexicon.empty()) {
        std::cout << "Lexicon empty... Deleting file\n";
        if (remove(indexName.c_str()) != 0)
            std::cout << "Error removing " << indexName << '\n';
    }
    
    return res;
}

bool HeapEntry::operator<(const HeapEntry& rhs) const {
    return term < rhs.term;
}

void IndexOutput::addLexiconEntry(const std::string& term, size_t p, size_t s) {
    lexicon[term] = LexiconEntry(p, s);
}

bool IndexOutput::isEmpty() const {
    return lexicon.empty();
}
