//
//  InvertedIndex.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "InvertedIndex.hpp"

InvertedIndex::LexiconEntry::LexiconEntry() {} // for std::map
InvertedIndex::LexiconEntry::LexiconEntry(unsigned p, unsigned s) : invListPos(p), invListLen(s) {}

InvertedIndex::InvertedIndex(const std::string& index, const std::string& inter, size_t sz) : indexFn(index), interFn(inter), bufferSize(sz), docBuffer(new char[sz]), currBufferPos(0) {
//    std::ofstream finalIndex(indexFn, std::ios::binary | std::ios::out);
    std::ifstream interFile(interFn);
    if (!interFile) {
        std::cerr << "Error opening " << interFn << '\n';
        exit(1);
    }
    
    // Last write position in the final index file
    unsigned lastIndexPos = 0;

    std::string term;
    unsigned docId;
    unsigned freq;
    while (interFile >> term >> docId >> freq) {
        Posting newPosting(term, docId, freq);
        // TODO (optional): format postings for query processing
        // TODO (optional): varbyte compression
        readPostingToBuffer(newPosting, lastIndexPos);
    }
    interFile.close();

    writeBufferToIndex(lastIndexPos);
    writeLexiconToDisk("lexicon");
}

InvertedIndex::~InvertedIndex() {
    delete [] docBuffer;
}

void InvertedIndex::writeBufferToIndex(unsigned& lastIndexPos) {
    std::cout << "Buffer is full.. Writing final index...\n";
    
    std::ofstream finalIndex(indexFn, std::ios::binary | std::ios::out);
    finalIndex.seekp(lastIndexPos);

    size_t i = 0;
    while (i < currBufferPos) {
        std::string term = getStrFromBuffer(i);
        size_t docId = stoi(getStrFromBuffer(i));
        size_t freq = stoi(getStrFromBuffer(i));
        // Update inverted lists
        finalIndex.write(reinterpret_cast<const char *>(&docId), sizeof(docId));
        finalIndex.write(reinterpret_cast<const char *>(&freq), sizeof(freq));
        // Update lexicon
        unsigned entryLen = sizeof(docId) + sizeof(freq);
        if (lexicon.find(term)!= lexicon.end()) {
            lexicon[term].invListLen += entryLen;
        } else {
            lexicon[term] = LexiconEntry(lastIndexPos, entryLen);
        }
        lastIndexPos += entryLen;
    }
    finalIndex.close();

    //Reset buffer
    memset(docBuffer, 0, bufferSize);
    currBufferPos = 0;
}

void InvertedIndex::readPostingToBuffer(const Posting& aPosting, unsigned& lastIndexPos) {
    // If buffer is full, clear it out first
    if (aPosting.size() + currBufferPos > bufferSize) {
        writeBufferToIndex(lastIndexPos);
    }
    putStrToBuffer(aPosting.term);
    putStrToBuffer(std::to_string(aPosting.docId));
    putStrToBuffer(std::to_string(aPosting.frequency));
}


void InvertedIndex::writeLexiconToDisk(const std::string& pathname) const {
    std::ofstream ofs(pathname);
    if (!ofs) {
        std::cerr << "Cannot open " << pathname << '\n';
        exit(1);
    }
    for (const auto& e : lexicon)
        ofs << e.first << ' ' << e.second.invListPos << ' ' << e.second.invListLen << '\n';
    ofs.close();
}

std::string InvertedIndex::getStrFromBuffer(size_t& i) {
    if (i > bufferSize) {
        std::cerr << "Bad access getStrFromBuffer\n";
        exit(1);
    }
    std::string res = "";
    while (docBuffer[i] != ' ') {
        res += docBuffer[i++];
    }
    i++;
    return res;
}

void InvertedIndex::putStrToBuffer(const std::string& aStr) {
    if (currBufferPos > bufferSize) {
        std::cerr << "Bad here putStrToBuffer\n";
        exit(1);
    }
    for (char c : aStr)
        docBuffer[currBufferPos++] = c;
    docBuffer[currBufferPos++] = ' ';
}
