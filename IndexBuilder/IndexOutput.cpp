//
//  IndexOutput.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "IndexOutput.hpp"

IndexOutput::LexiconEntry::LexiconEntry() {} // for std::map
IndexOutput::LexiconEntry::LexiconEntry(unsigned p, unsigned s) : invListPos(p), invListLen(s) {}

IndexOutput::IndexOutput(const std::string& index, const std::string& inter) : indexFn(index), interFn(inter) {
    std::ofstream finalIndex(indexFn, std::ios::binary | std::ios::out);
    std::ifstream interFile(interFn);
    if (!interFile) {
        std::cerr << "Error opening " << interFn << '\n';
        exit(1);
    }

    size_t bufferSize = 100000000;
    char* docBuffer = new char[bufferSize];
    size_t bufferPos = 0; // current position in buffer

    unsigned lastIndexPos = 0;

    std::string term;
    unsigned docId;
    unsigned freq;
    while (interFile >> term >> docId >> freq) {
        Posting newPosting(term, docId, freq);
        readPostingToBuffer(docBuffer, bufferSize, bufferPos, newPosting, lastIndexPos, finalIndex);
    }

    writeBufferToIndex(docBuffer, bufferSize, bufferPos, lastIndexPos, finalIndex);
    delete [] docBuffer;
    finalIndex.close();
    interFile.close();

    writeToDisk("lexicon");
}

void IndexOutput::writeBufferToIndex(char* docBuffer, size_t bufferSize, size_t& currPos, unsigned& lastIndexPos, std::ofstream& finalIndex) {
    std::cout << "Buffer is full.. Writing to index...\n";

    finalIndex.seekp(lastIndexPos);

    size_t i = 0;
    while (i < currPos) {
        std::string term = getStrFromBuffer(docBuffer, bufferSize, i);
        size_t docId = stoi(getStrFromBuffer(docBuffer, bufferSize, i));
        size_t freq = stoi(getStrFromBuffer(docBuffer, bufferSize, i));
        // Update inverted lists
        finalIndex.write(reinterpret_cast<const char *>(&docId), sizeof(docId));
        finalIndex.write(reinterpret_cast<const char *>(&freq), sizeof(freq));
        // Update lexicon
        unsigned entryLen = sizeof(docId) + sizeof(freq);
        lexicon[term] = LexiconEntry(lastIndexPos, entryLen);
        lastIndexPos += entryLen;
    }

    //Flush buffer
    memset(docBuffer, 0, bufferSize);
    currPos = 0;
}

void IndexOutput::readPostingToBuffer(char* docBuffer, size_t bufferSize, size_t& currPos, const Posting& aPosting, unsigned& lastIndexPos, std::ofstream& finalIndex) {
    if (aPosting.size() + currPos > bufferSize) {
        writeBufferToIndex(docBuffer, bufferSize, currPos, lastIndexPos, finalIndex);
    }
    // Else, put to buffer
    else {
        putStrToBuffer(docBuffer, bufferSize, currPos, aPosting.term);
        putStrToBuffer(docBuffer, bufferSize, currPos, std::to_string(aPosting.docId));
        putStrToBuffer(docBuffer, bufferSize, currPos, std::to_string(aPosting.frequency));
    }
}


void IndexOutput::writeToDisk(const std::string& pathname) const {
    std::ofstream ofs(pathname);
    if (!ofs) {
        std::cerr << "Cannot open " << pathname << '\n';
        exit(1);
    }
    for (const auto& e : lexicon)
        ofs << e.first << ' ' << e.second.invListPos << ' ' << e.second.invListLen << '\n';
    ofs.close();
}
