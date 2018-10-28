//
//  VByteCompression.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/18/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "VByteCompression.hpp"

std::vector<unsigned char> encodeNumVB(size_t n) {
    std::vector<unsigned char> bytes;
    while (true) {
        uint8_t byte = n % 128;
        // cout << (unsigned)byte << endl;
        byte += 128; // set 1 as continuation bit
        bytes.insert(bytes.begin(), byte);
        if (n < 128) { break; }
        n = n / 128;
    }
    // set 0 as termination bit of the last byte
    bytes.back() = bytes.back() - 128;
    return bytes;
}

std::vector<unsigned char> encodeVB(const std::vector<size_t>& numbers) {
    std::vector<unsigned char> bytestream;
    for (size_t n : numbers) {
        std::vector<unsigned char> bytes = encodeNumVB(n);
        bytestream.insert(bytestream.end(), bytes.begin(), bytes.end());
    }
    return bytestream;
}

std::vector<size_t> decodeVB(const std::vector<char>& bytestream) {
    std::vector<size_t> numbers;
    size_t n = 0;
    for (unsigned char aByte : bytestream) {
        uint8_t intByte = (uint8_t)aByte;
        // cout << (unsigned)thisByte << endl;
        if (intByte > 128) {
            // ignore continuation bit 1 by - 128
            n = 128 * n + (intByte - 128);
        } else {
            n = 128 * n + intByte;
            numbers.push_back(n);
            n = 0;
        }
    }
    return numbers;
}
