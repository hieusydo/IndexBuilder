//
//  PostingGenerator.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/15/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef PostingGenerator_hpp
#define PostingGenerator_hpp

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <dirent.h>
#include "UrlTable.hpp"

struct Posting {
    Posting(const std::string& t, unsigned id, unsigned f)
        : term(t), docId(id), frequency(f) {} // for intermediate posting
    
    size_t size() const;
    
    std::string term;
    unsigned docId; // also rowLen in UrlTable
    unsigned frequency;
};

class PostingGenerator {
private:
    // Directory with all WET files
    std::string dirName;
    
    // Delimiters to tokenize string
    const std::string DELIMITERS = ";,:. \r\n\t";
    
    // Buffer fields and methods to process WET files
    size_t bufferSize;
    char* docBuffer;
    size_t currBufferPos;

    // Extract a string from i to the next blank space
    std::string getStrFromBuffer(size_t& i);
    
    // Append the string to buffer[currBufferPos]
    void putStrToBuffer(const std::string& aStr);
    
    // Write everything in buffer to the lastIndexPos byte of the final index
    void flushBuffer(int& fileCnt);
    
    // Put posting data to buffer, call flushBuffer first if full
    void putPostingToBuffer(const Posting& aPosting, int& fileCnt);
    
    // Extract documentUri and documentLen IF exists in line parameter
    void parseWetHeader(const std::string& line, std::string& documentUri, int& documentLen);
    
    // Get a vector of all WET files from the directory
    std::vector<std::string> getAllFiles();
    
    // Return true if line has only alphanumeric chars, false otherwise
    bool isStrAlnum(const std::string& line);
    
    // Count the number of occurences of a substring in a string
    size_t countSubstr(const std::string& aString, const std::string& subStr);
    
    // Split string into words by delimiters
    // and also normalize them along the way
    inline bool isDelim(char c);
    void tokenizeDocStream(const std::string& inputString, std::map<std::string, unsigned>& frequencies);
    
public:
    // The constructor initialize the directory name and the buffer
    PostingGenerator(const std::string& dn, size_t sz);
    
    // Destructor to free docBuffer
    ~PostingGenerator();
    
    // Parse the WET files to generate intermediate postings
    int generatePostings();
};

#endif /* PostingGenerator_hpp */
