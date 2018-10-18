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

class InvertedIndex {
    struct LexiconEntry {
        LexiconEntry(); // for std::map
        LexiconEntry(unsigned p, unsigned s);
        unsigned invListPos;
        unsigned invListLen;
    };
    
private:
    std::string indexFn; // filename of the final index
    std::string interFn; // filename of the intermediate file
    std::map<std::string, LexiconEntry> lexicon;
    
    // Buffer fields and methods to process intermediate file
    // TODO: optimize with stringstream? ostringstream??
    size_t bufferSize;
    char* docBuffer;
    size_t currBufferPos;
    
    // Extract a string from i to the next blank space
    std::string getStrFromBuffer(size_t& i);
    
    // Append the string to buffer[currBufferPos]
    void putStrToBuffer(const std::string& aStr);

    // Write everything in buffer to the lastIndexPos byte of the final index
    void writeBufferToIndex(unsigned& lastIndexPos);
    
    // Move posting from intermediate file to the buffer
    // If buffer is full, call writeBufferToIndex
    void readPostingToBuffer(const Posting& aPosting, unsigned& lastIndexPos);

    // Write the lexicon to disk
    void writeLexiconToDisk(const std::string& pathname) const;
    
public:
    // The constructor reads the intermediate file to get each (term docId frequency) into the buffer
    // and when it's full, the data is written to the lexicon and to the final index
    InvertedIndex(const std::string& index, const std::string& inter, size_t bufferSize);

    // Destructor to free docBuffer
    ~InvertedIndex();
};

#endif /* InvertedIndex_hpp */
