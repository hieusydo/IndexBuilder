//
//  IndexOutput.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef IndexOutput_hpp
#define IndexOutput_hpp

#include "common.hpp"
#include "PostingGenerator.hpp"

class IndexOutput {
    struct LexiconEntry {
        LexiconEntry(); // for std::map
        LexiconEntry(unsigned p, unsigned s);
        unsigned invListPos;
        unsigned invListLen;
    };
    
private:
    std::string indexFn;
    std::string interFn;
    std::map<std::string, LexiconEntry> lexicon;

public:
    IndexOutput(const std::string& index, const std::string& inter);
    
    void writeBufferToIndex(char* docBuffer, size_t bufferSize, size_t& currPos, unsigned& lastIndexPos, std::ofstream& finalIndex);
    
    void readPostingToBuffer(char* docBuffer, size_t bufferSize, size_t& currPos, const Posting& aPosting, unsigned& lastIndexPos, std::ofstream& finalIndex);
    
    void writeToDisk(const std::string& pathname) const;
};

#endif /* IndexOutput_hpp */
