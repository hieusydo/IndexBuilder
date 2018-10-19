//
//  VByteCompression.hpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/18/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#ifndef VByteCompression_hpp
#define VByteCompression_hpp

#include <vector>

// TODO: https://nlp.stanford.edu/IR-book/html/htmledition/variable-byte-codes-1.html

std::vector<char> encodeVbyte(size_t inputNum);

size_t decodeVbyte(std::vector<char> encodedBytes);

#endif /* VByteCompression_hpp */
