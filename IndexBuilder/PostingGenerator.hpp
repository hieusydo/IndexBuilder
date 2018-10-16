//
//  PostingGenerator.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/15/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef PostingGenerator_hpp
#define PostingGenerator_hpp

#include "common.hpp"
#include "UrlTable.hpp"

struct Posting {
//    Posting(unsigned id, unsigned f)
//        : docId(id), frequency(f) {} // for IndexOutput
    Posting(const std::string& t, unsigned id, unsigned f)
        : term(t), docId(id), frequency(f) {} // for intermediate posting
    
    size_t size() const;
    
    std::string term;
    unsigned docId; // also rowLen in UrlTable
    unsigned frequency;
};

int generatePostings();

// Extract documentUri and documentLen IF exists in line parameter
void parseWetHeader(const std::string& line, std::string& documentUri, int& documentLen);

std::string getStrFromBuffer(char* docBuffer, size_t bufferSize, size_t& i);

void putStrToBuffer(char* docBuffer, size_t bufferSize, size_t& i, const std::string& aStr);

void flushBuffer(char* docBuffer, size_t bufferSize, size_t& currPos, int& fileCnt);

void putPostingToBuffer(char* docBuffer, size_t bufferSize, size_t& currPos, const Posting& aPosting, int& fileCnt);

#endif /* PostingGenerator_hpp */
