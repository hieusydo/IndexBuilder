//
//  PostingGenerator.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/15/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "PostingGenerator.hpp"

int generatePostings(const std::string& dirName) {
    size_t bufferSize = 100000000;
    char* docBuffer = new char[bufferSize];
    size_t bufferPos = 0; // current position in buffer
    int fileCnt = 0;
    
    UrlTable urlTable;
    
    std::string documentStream;
    std::string documentUri;
    int documentLen = 0;
    // Flag to skip all intro data till first "WARC/1.0" with actual page
    bool skipIntro = true;
    
    std::vector<std::string> allFiles = getAllFiles(dirName);
    for (const std::string& filename : allFiles) {
        
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
        std::fstream wetFile(dirName + '/' + filename);
        if (!wetFile) {
            std::cerr << "Error opening WET file: " << filename << std::endl;
            exit(1);
        }
        
        std::string line;
        // Skip WARC intro info
        getline(wetFile, line);
        if (line.find("WARC/1.0") == std::string::npos) {
            std::cout << "No WARC intro info found, might wanna look into this." << std::endl;
            exit(0);
        }
        
        // Main loop: read till end of current WET file
        while (getline(wetFile, line)) {
            // Found the start of a header
            if (line.find("WARC/1.0") != std::string::npos) {
                // Switch flag off ASA see the first page
                skipIntro = false;
                
                // Generate intermediate posting once a page is parsed
                if (!documentStream.empty()) {
                    std::vector<std::string> words = tokenizeDocStream(documentStream);
                    
                    // Count frequency
                    std::map<std::string, unsigned> freqMap;
                    for (const std::string& w : words) {
                        if (w.size() == 0) { continue; }
                        if (freqMap.find(w) == freqMap.end()) {
                            freqMap[w] = 1;
                        } else {
                            freqMap[w] += 1;
                        }
                    }
                    
                    // Move postings to buffer
                    for (const auto& e : freqMap) {
                        Posting newPosting = Posting(e.first, static_cast<unsigned>(urlTable.size()), e.second);
                        putPostingToBuffer(docBuffer, bufferSize, bufferPos, newPosting, fileCnt);
                    }
                    
                    // Clear the document stream for the next page
                    documentStream.clear();
                }
                
                // Parse until end of header
                while (line != "\r\n" && line != "\r") {
                    // Try to obtain uri and len
                    parseWetHeader(line, documentUri, documentLen);
                    // Get next line to be parse
                    getline(wetFile, line);
                }
                
                urlTable.addEntry(documentUri, documentLen);
                
            } else if (skipIntro) {
                continue;
            } else {
                // Append actual content of page to stream
                getline(wetFile, line);
                documentStream += '\n'; // prevent 'adaafasf'
                documentStream += line;
            }
        }
        wetFile.close();
        
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Finished parsing " << filename << ' ' << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";
    }
    
    // Flush out the rest to disk
    flushBuffer(docBuffer, bufferSize, bufferPos, fileCnt);

    delete [] docBuffer;
    
    // Write urlTable to disk
    urlTable.writeToDisk("urlTable");
    return fileCnt;
}

std::string getStrFromBuffer(char* docBuffer, size_t bufferSize, size_t& i) {
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

void putStrToBuffer(char* docBuffer, size_t bufferSize, size_t& i, const std::string& aStr) {
    if (i > bufferSize) {
        std::cerr << "Bad here putStrToBuffer\n";
        exit(1);
    }
    for (char c : aStr)
        docBuffer[i++] = c;
    docBuffer[i++] = ' ';
}

void flushBuffer(char* docBuffer, size_t bufferSize, size_t& currPos, int& fileCnt) {
    std::cout << "Buffer is full.. Writing to file...\n";
    std::ofstream interFile(std::to_string(fileCnt++));
    size_t i = 0;
    while (i < currPos) {
        std::string term = getStrFromBuffer(docBuffer, bufferSize, i);
        size_t docId = stoi(getStrFromBuffer(docBuffer, bufferSize, i));
        size_t freq = stoi(getStrFromBuffer(docBuffer, bufferSize, i));
        interFile << term << ' ' << docId << ' ' << freq << '\n';
    }
    interFile.close();
    //Flush buffer
    memset(docBuffer, 0, bufferSize);
    currPos = 0;
}

void putPostingToBuffer(char* docBuffer, size_t bufferSize, size_t& currPos, const Posting& aPosting, int& fileCnt) {
    // TODO: write in binary
    // If buffer is full, write to file and flush buffer
    if (aPosting.size() + currPos > bufferSize) {
        flushBuffer(docBuffer, bufferSize, currPos, fileCnt);
    }
    // Else, put to buffer
    else {
        putStrToBuffer(docBuffer, bufferSize, currPos, aPosting.term);
        putStrToBuffer(docBuffer, bufferSize, currPos, std::to_string(aPosting.docId));
        putStrToBuffer(docBuffer, bufferSize, currPos, std::to_string(aPosting.frequency));
    }
}

void parseWetHeader(const std::string& line, std::string& documentUri, int& documentLen) {
    // Split each line in metadata and parse uri & length
    std::string delimit = ": ";
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

size_t Posting::size() const {
    return term.size() + std::to_string(docId).size() + std::to_string(frequency).size() + 2; // 2 for 2 space char
}

std::vector<std::string> getAllFiles(const std::string& dirName) {
    std::vector<std::string> allFiles;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirName.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string fn = std::string(ent->d_name);
            // Only return WET files
            if (fn.find("warc.wet") != std::string::npos) {
                allFiles.push_back(fn);
            }
        }
        closedir (dir);
    } else {
        std::cerr << "Cannot open directory " << dirName << '\n';
        exit(1);
    }
    return allFiles;
}
