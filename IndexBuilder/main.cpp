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

int COUNT = 0;
size_t D_WAY_MERGE = 2;
bool DEBUG_FLAG = false; // write inverted lists in ascii or byte

// Extract documentUri and documentLen IF exists in line parameter
void parseWetHeader(const string& line, string& documentUri, int& documentLen);

// IO efficient merge sort
void mergeUtil(vector<set<IndexOutput>>& indexLog, set<IndexOutput>& layer) {
    cout << "MERGE UTIL\n";
    priority_queue<Posting> mergeHeap;
    
    // Create input stream vector
    vector<ifstream> buffer(layer.size());
    size_t i = 0;
    for(auto it = layer.begin(); it != layer.end(); it++) {
        buffer[i].open(it->getIndexName());
        size_t docId;
        size_t freq;
        buffer[i] >> docId >> freq;
        mergeHeap.push(Posting(docId, freq));
        i++;
    }
    
    while (!mergeHeap.empty()) {
        Posting ap = mergeHeap.top();
        cout << "Merge heap: " << ap.docId << ' ' << ap.frequency << endl;
        mergeHeap.pop();
    }
}

// Merge d files of equal size (logarithmic merge)
void checkAndMerge(vector<set<IndexOutput>>& indexLog, size_t dWay) {
    for (set<IndexOutput>& layer : indexLog) {
        // Merge if one layer is full
        if (layer.size() == dWay) {
            mergeUtil(indexLog, layer);
        }
    }
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
    vector<set<IndexOutput>> indexLog(256);
    
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
                
                // TODO: Check for equal size files to merge
                checkAndMerge(indexLog, D_WAY_MERGE);
                                
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
                IndexOutput newIndex(indexName);
                newIndex.addPage(freqMap, urlTable.size(), DEBUG_FLAG);
                newIndex.display();
                
                indexLog[0].insert(newIndex);
                
                urlTable.addEntry(documentUri, documentLen);
                urlTable.display();
                
                documentStream.clear();
                
                if (COUNT++ == 3)
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
            cout << documentUri << " " << documentLen << endl;
            
        } else if (skipIntro) {
            continue;
        } else {
            // Append actual content of page to stream
            getline(file, line);
            documentStream += line;
        }
    }
    
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
