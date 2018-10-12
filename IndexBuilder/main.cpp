//
//  main.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/9/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <map>
#include <set>
#include <queue>

#include "common.hpp"
#include "IndexOutput.hpp"
#include "UrlTable.hpp"

using namespace std;

int COUNT = 10;
size_t D_WAY_MERGE = 2;
bool DEBUG_FLAG = false; // write inverted lists in ascii or byte

// Extract documentUri and documentLen IF exists in line parameter
void parseWetHeader(const string& line, string& documentUri, int& documentLen);

// IO efficient merge sort
void mergeUtil(vector<vector<IndexOutput>>& indexLog, vector<IndexOutput>& layerSet, size_t layer) {
    cout << "MERGE UTIL\n";
    priority_queue<pair<HeapEntry, size_t>> mergeHeap; // size_t to know which Index to getNextTerm from

//     Create input stream vector
//    vector<ifstream> buffer(layerSet.size());
    for(size_t i = 0; i < layerSet.size(); ++i) {
//        buffer[i].open(it->getIndexName());
        // Insert the first from each list to heap
        HeapEntry posLex = layerSet[i].getNextTerm();
        mergeHeap.push(make_pair(posLex, i));
    }
    
    string tmpPath = "temp/hex/";
    size_t nextLayer = layer+1;
    string outputName = tmpPath + to_string(nextLayer) + layerSet.begin()->getIndexName().substr(tmpPath.size()+1);
    IndexOutput mergedIndex(outputName);
    
    ofstream mergedFile(outputName, std::ios::binary | std::ios::out);
    if (!mergedFile) {
        cerr << "Cannot open new file " << outputName << endl;
        exit(1);
    }
    cout << outputName << endl;
    size_t pos = 0;
    while (!mergeHeap.empty()) {
        // Do custom pop to handle duplicate key/term
        HeapEntry curr = mergeHeap.top().first;
        size_t idxInLayerSet = mergeHeap.top().second;
        mergeHeap.pop();
//        cout << "Merge heap: " << curr.term << ' ' << curr.docId << ' ' << curr.frequency << endl;
        
        // Merge inverted list in-mem
        // mergedPos
        // mergedLen
        size_t entryLen = sizeof(curr.docId) + sizeof(curr.frequency);
        
        // Update lexicon of new index output
        mergedIndex.addLexiconEntry(curr.term, pos, entryLen);
        
        // Write merged result to file
        mergedFile.write(reinterpret_cast<char*>(&curr.docId), sizeof(curr.docId));
        mergedFile.write(reinterpret_cast<char*>(&curr.frequency), sizeof(curr.frequency));
        pos += entryLen;
        
        // Add next one
        if (!layerSet[idxInLayerSet].isEmpty()) {
            HeapEntry posLex = layerSet[idxInLayerSet].getNextTerm();
            mergeHeap.push(make_pair(posLex, idxInLayerSet));
        }
    }
    
    mergedIndex.display();
    indexLog[nextLayer].push_back(mergedIndex);
    indexLog[layer].clear();
    mergedFile.close();
}

// Merge d files of equal size (logarithmic merge)
void checkAndMerge(vector<vector<IndexOutput>>& indexLog, size_t dWay) {
    for (size_t i = 0; i < indexLog.size(); ++i) {
        // Merge if one layer is full
        if (indexLog[i].size() == dWay) {
            mergeUtil(indexLog, indexLog[i], i);
        }
    }
}

void displayIndexLog(const vector<vector<IndexOutput>>& indexLog) {
    for (size_t i = 0; i < 10; ++i)
        cout << indexLog[i].size() << ' ';
    cout << endl;
}

int main(int argc, const char * argv[]) {
    // TODO: Use Boost to decompress in memory
    // TODO: Use Boost to walk directory to process all WET files
    string filename = "wet_files/00001.warc.wet";
    fstream file(filename);
    if (!file) {
        cerr << "Error opening WET file: " << filename << endl;
        exit(1);
    }
    
    string line;
    // Skip WARC intro info
    getline(file, line);
    if (line.find("WARC/1.0") == string::npos) {
        cout << "No WARC intro info found, might wanna look into this." << endl;
        exit(0);
    }
    
    UrlTable urlTable;
    vector<vector<IndexOutput>> indexLog(256);
    
    string documentStream;
    string documentUri;
    int documentLen = 0;
    // Flag to skip all intro data till first "WARC/1.0" with actual page
    bool skipIntro = true;
    // Main loop: read till end of current WET file
    while (getline(file, line)) {
        // Found the start of a header
        if (line.find("WARC/1.0") != string::npos) {
            // Switch flag off ASA see the first page
            skipIntro = false;
            
            // Generate intermediate file ASA done with one page
            if (!documentStream.empty()) {
                
                // Check for equal size files to merge
//                checkAndMerge(indexLog, D_WAY_MERGE);
                
                vector<string> words = tokenizeDocStream(documentStream);

                // Count frequency
                std::map<string, size_t> freqMap;
                for (const string& w : words) {
                    if (freqMap.find(w) == freqMap.end())
                        freqMap[w] = 1;
                    else
                        freqMap[w] += 1;
                }
                
                // Write to disk, update index
                string indexName = "temp/";
                if (DEBUG_FLAG)
                    indexName += "ascii/";
                else
                    indexName += "hex/";
                
                indexName += ("0_" + to_string(urlTable.size()));
                cout << indexName << endl;
                IndexOutput newIndex(indexName);
                newIndex.addPage(freqMap, urlTable.size(), DEBUG_FLAG);
//                newIndex.display();
                
                indexLog[0].push_back(newIndex);
                
                displayIndexLog(indexLog);
                
                urlTable.addEntry(documentUri, documentLen);
                
                documentStream.clear();
                
                if (COUNT-- == 1)
                    exit(0);
            }
            
            // Parse until end of header
            while (line != "\r\n" && line != "\r") {
                // Try to obtain uri and len
                parseWetHeader(line, documentUri, documentLen);
                // Get next line to be parse
                getline(file, line);
            }
            
            // Print parsed header
            cout << "New: " << documentLen << ' ' << documentUri << endl;
            
        } else if (skipIntro) {
            continue;
        } else {
            // Append actual content of page to stream
            getline(file, line);
            documentStream += line;
        }
    }
    
    urlTable.display();
    
    file.close();
    return 0;
}

void parseWetHeader(const string& line, string& documentUri, int& documentLen) {
    // Split each line in metadata and parse uri & length
    string delimit = ": ";
    size_t delimitLen = delimit.size();
    size_t pos = line.find(delimit);
    
    if (line.substr(0, pos) == "WARC-Target-URI") {
        documentUri = line.substr(pos + delimitLen);
        documentUri.erase(remove(documentUri.begin(), documentUri.end(), '\n'),
                          documentUri.end());
        documentUri.erase(remove(documentUri.begin(), documentUri.end(), '\r'),
                          documentUri.end());
    }
    
    if (line.substr(0, pos) == "Content-Length") {
        documentLen = stoi(line.substr(pos + delimitLen, line.size()));
    }
}
