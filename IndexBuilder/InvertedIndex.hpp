//
//  InvertedIndex.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef InvertedIndex_hpp
#define InvertedIndex_hpp

#include <string>
#include <map>
#include "PostingGenerator.hpp"
#include "VByteCompression.hpp"

class InvertedIndex {
    struct LexiconEntry {
        LexiconEntry(); // for std::map
        LexiconEntry(size_t p, size_t s);
        size_t invListPos;
        size_t metadataSize;
    };
    
private:
    std::string indexFn; // filename of the final index
    std::string interFn; // filename of the intermediate file
    std::map<std::string, LexiconEntry> lexicon;

    // Write the lexicon to disk
    void writeLexiconToDisk(const std::string& pathname) const;
    
public:
    // The constructor reads the intermediate file to get each (term docId frequency) into the buffer
    // and when it's full, the data is written to the lexicon and to the final index
    InvertedIndex(const std::string& index, const std::string& inter);
    
};

#endif /* InvertedIndex_hpp */
