//
//  InvertedIndex.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/10/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "InvertedIndex.hpp"

InvertedIndex::LexiconEntry::LexiconEntry() {} // for std::map
InvertedIndex::LexiconEntry::LexiconEntry(size_t p, size_t s) : invListPos(p), metadataSize(s) {}

InvertedIndex::InvertedIndex(const std::string& index, const std::string& inter) : indexFn(index), interFn(inter) {
    std::ifstream interFile(interFn);
    if (!interFile) {
        std::cerr << "Error opening " << interFn << '\n';
        exit(1);
    }
    
    // Write in binary
    // Last write position in the final index file
    size_t lastIndexPos = 0;
    
    std::ofstream finalIndex(indexFn, std::ios::binary | std::ios::out);

    std::string currTerm;
    size_t currDocId;
    size_t currFreq;
    interFile >> currTerm >> currDocId >> currFreq;
    std::vector<std::pair<size_t, size_t>> allCurrPostings;
    allCurrPostings.push_back(std::make_pair(currDocId, currFreq));
    
    std::string term;
    size_t docId;
    size_t freq;
    while (interFile >> term >> docId >> freq) {
        if (currTerm == term) {
            allCurrPostings.push_back(std::make_pair(docId, freq));
        } else {
            // write out allCurrPostings
            std::vector<unsigned char> invList;
            std::vector<unsigned char> allChunks;

            // Metadata info
            std::vector<size_t> chunkSize;
            std::vector<size_t> lastDid;

            // Chunk compression
            std::vector<size_t> didChunk;
            std::vector<size_t> freqChunk;
            for(size_t i = 0; i < allCurrPostings.size(); ++i) {
                didChunk.push_back(allCurrPostings[i].first);
                freqChunk.push_back(allCurrPostings[i].second);
                if (didChunk.size() == 128 || i + 1 == allCurrPostings.size()) {
                    
                    // Compress
                    std::vector<unsigned char> didEncoded = encodeVB(didChunk);
                    std::vector<unsigned char> freqEncoded = encodeVB(freqChunk);
                    
                    // Update metadata
                    lastDid.push_back(didChunk.back());
                    chunkSize.push_back(didEncoded.size());
                    chunkSize.push_back(freqEncoded.size());
                    
                    // Extend the chunks
                    allChunks.reserve(allChunks.size() + didEncoded.size() + freqEncoded.size());
                    allChunks.insert(allChunks.end(), didEncoded.begin(), didEncoded.end());
                    allChunks.insert(allChunks.end(), freqEncoded.begin(), freqEncoded.end());
                    
                    // Clear to process next chunk
                    didChunk.clear();
                    freqChunk.clear();
                }
            }
            
            std::vector<unsigned char> chunkSizeLen = encodeNumVB(chunkSize.size());
            std::vector<unsigned char> chunkSizeEncoded = encodeVB(chunkSize);
            
            std::vector<unsigned char> lastDidLen = encodeNumVB(lastDid.size());
            std::vector<unsigned char> lastDidEncoded = encodeVB(lastDid);
            
            
            invList.reserve(chunkSizeLen.size() + chunkSizeEncoded.size() + lastDidLen.size() + lastDidEncoded.size() + allChunks.size());
            invList.insert(invList.end(), chunkSizeLen.begin(), chunkSizeLen.end());
            invList.insert(invList.end(), chunkSizeEncoded.begin(), chunkSizeEncoded.end());
            invList.insert(invList.end(), lastDidLen.begin(), lastDidLen.end());
            invList.insert(invList.end(), lastDidEncoded.begin(), lastDidEncoded.end());
            invList.insert(invList.end(), allChunks.begin(), allChunks.end());
            
            // Update lexicon and write chunk-divided inv list
            size_t metadataLen = chunkSizeLen.size() + chunkSizeEncoded.size() + lastDidLen.size() + lastDidEncoded.size();
            lexicon[currTerm] = LexiconEntry(lastIndexPos, metadataLen);
            
            size_t entryLen = invList.size();
            finalIndex.write((const char*)&invList[0], entryLen);
            lastIndexPos = finalIndex.tellp();
            
            // Reset allCurrPostings with the new term
            allCurrPostings.clear();
            currTerm = term;
            allCurrPostings.push_back(std::make_pair(docId, freq));
        }
    }
    finalIndex.close();
    
    interFile.close();
    writeLexiconToDisk("lexicon");
}

void InvertedIndex::writeLexiconToDisk(const std::string& pathname) const {
    std::ofstream ofs(pathname);
    if (!ofs) {
        std::cerr << "Cannot open " << pathname << '\n';
        exit(1);
    }
    for (const auto& e : lexicon)
        ofs << e.first << ' ' << e.second.invListPos << ' ' << e.second.metadataSize << '\n';
    ofs.close();
}
