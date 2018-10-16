//
//  common.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "common.hpp"

const std::string DELIMITERS(";,:. \r\n\t");

bool isStrAlnum(const std::string& line) {
    for (char c : line)
        if (!isalnum(c)) { return false; }
    return true;
}

size_t countSubstr(const std::string& aString, const std::string& subStr) {
    size_t count = 0;
    size_t pos = 0;
    while (aString.find(subStr, pos) != std::string::npos) {
        pos  = aString.find(subStr, pos) + subStr.size();
        count++;
    }
    return count;
}

inline bool isDelim(char c) {
    for (int i = 0; i < DELIMITERS.size(); ++i)
        if (DELIMITERS[i] == c)
            return true;
    return false;
}

// Ref: https://stackoverflow.com/a/43205925
std::vector<std::string> tokenizeDocStream(const std::string& inputString) {
    std::stringstream stringStream(inputString);
    char c;
    std::vector<std::string> wordVector;
    
    while (stringStream) {
        std::string word;
        
        // Read word
        while (!isDelim((c = stringStream.get())) && c != EOF) {
            word.push_back(c);
        }
        if (c != EOF)
            stringStream.unget();
        
        // Filter foreign characters
        if (isStrAlnum(word)) {
            // Normalize word by converting to lowercase
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            wordVector.push_back(word);
        }
        
        // Skip delims
        while (isDelim((c = stringStream.get())));
        if (c != EOF)
            stringStream.unget();
    }
    return wordVector;
}
