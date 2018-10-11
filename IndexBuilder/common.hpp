//
//  common.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef common_hpp
#define common_hpp

#include <stdio.h>
#include <string>

// Return true if line has only alphanumeric chars, false otherwise
bool isStrAlnum(const std::string& line);

// Count the number of occurences of a substring in a string
size_t countSubstr(const std::string& aString, const std::string& subStr);

// Split string into words by delimiters
// and also normalize them along the way
inline bool isDelim(char c);
std::vector<std::string> tokenizeDocStream(const std::string& inputString);

#endif /* common_hpp */
