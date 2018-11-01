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
#include <iostream>
#include <fstream>
#include <bitset>

// Variable byte compression implementation
// Based on: https://nlp.stanford.edu/IR-book/html/htmledition/variable-byte-codes-1.html
// TODO: use bit operators to optimize

// Return the encoded bytes of a number n
std::vector<unsigned char> encodeNumVB(size_t n);

// Return a stream of encoded bytes of all the input numbers
std::vector<unsigned char> encodeVB(const std::vector<size_t>& numbers);

// Return all decoded numbers from a stream of encoded bytes
std::vector<size_t> decodeVB(const std::vector<unsigned char>& bytestream);

// Trivial test function to generate files with encoded vs. non-encoded numbers
// Goal is to see that encoded files are smaller
void testVBCompression();

#endif /* VByteCompression_hpp */
