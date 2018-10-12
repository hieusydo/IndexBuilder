//
//  IndexOutput.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef IndexOutput_hpp
#define IndexOutput_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

struct Posting {
    Posting(size_t id, size_t f) : docId(id), frequency(f) {}
    
    size_t docId; // also rowLen in UrlTable
    size_t frequency;
};

struct HeapEntry {
    bool operator<(const HeapEntry& rhs) const;
    std::string term;
    size_t docId;
    size_t frequency;
};

class IndexOutput {
    struct LexiconEntry {
        LexiconEntry() {}
        LexiconEntry(size_t p, size_t s) : invListPos(p), invListLen(s) {}
        size_t invListPos;
        size_t invListLen;
    };
    
private:
    std::string indexName;
    std::map<std::string, LexiconEntry> lexicon;

public:
    IndexOutput(const std::string& n) : indexName(n) {}
        
    void addPage(const std::map<std::string, size_t>& freqMap, size_t docId, bool debug);
    
    void addLexiconEntry(const std::string& term, size_t p, size_t s);
    
    void display() const;
    
    std::string getIndexName() const;
    
    HeapEntry getNextTerm(); 
    
    bool isEmpty() const;
};

#endif /* IndexOutput_hpp */
