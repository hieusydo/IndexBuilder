//
//  VByteCompression.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/18/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "VByteCompression.hpp"

std::vector<char> encodeNumVB(size_t n) {
    std::vector<char> bytes;
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

std::vector<char> encodeVB(const std::vector<size_t>& numbers) {
    std::vector<char> bytestream;
    for (size_t n : numbers) {
        std::vector<char> bytes = encodeNumVB(n);
        bytestream.insert(bytestream.end(), bytes.begin(), bytes.end());
    }
    return bytestream;
}

std::vector<size_t> decodeVB(const std::vector<char>& bytestream) {
    std::vector<size_t> numbers;
    size_t n = 0;
    for (char aByte : bytestream) {
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

void testVBCompression() {
    std::vector<size_t> numbers = {34, 144, 113, 162};
    std::vector<char> encoded = encodeVB(numbers);
    std::ios_base::sync_with_stdio(false);
    
    // Write encoded numbers
    // Use xxd -b testwrite to view in bits
    std::ofstream ofs("testwrite", std::ios::binary | std::ios::out);
    for (const char e : encoded) {
        std::cout << std::bitset<8>(e) << '\n';
        ofs.write(&e, sizeof(e));
    }
    ofs.close();
    
    // Write non-encoded numbers
    // xxd testwrite-novb to view in hex
    std::ofstream ofsa("testwrite-novb", std::ios::binary | std::ios::out);
    for (size_t n : numbers) {
        ofsa.write(reinterpret_cast<const char *>(&n), sizeof(n));
    }
    ofsa.close();
}
