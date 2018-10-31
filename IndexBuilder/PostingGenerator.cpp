//
//  PostingGenerator.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/15/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "PostingGenerator.hpp"

PostingGenerator::PostingGenerator(const std::string& dn, size_t sz) : dirName(dn), bufferSize(sz), docBuffer(new char[sz]), currBufferPos(0) {}

PostingGenerator::~PostingGenerator() {
    delete[] docBuffer;
}

int PostingGenerator::generatePostings() {
    int fileCnt = 0;
    
    UrlTable urlTable;
    
    std::string documentString;
    std::string documentUri;
    int documentLen = 0;
    // Flag to skip all intro data till first "WARC/1.0" with actual page
    bool skipIntro = true;
    
    std::vector<std::string> allFiles = getAllFiles();
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
                if (!documentString.empty()) {
                    // Count frequency
                    std::map<std::string, unsigned> freqMap;
                    tokenizeDocStream(documentString, freqMap);
                    
                    // Move postings to buffer
                    for (const auto& e : freqMap) {
                        Posting newPosting = Posting(e.first, static_cast<unsigned>(urlTable.size() - 1), e.second);
                        putPostingToBuffer(newPosting, fileCnt);
                    }
                    
                    // Clear the document stream for the next page
                    documentString.clear();
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
                documentString += '\n'; // prevent 'adaafasf'
                documentString += line;
            }
        }
        wetFile.close();
        
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Finished parsing " << filename << ' ' << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";
    }
    
    // Write out the rest to disk
    flushBuffer(fileCnt);
    
    // Write urlTable to disk
    urlTable.writeToDisk("urlTable");
    return fileCnt;
}

void PostingGenerator::flushBuffer(int& fileCnt) {
    std::cout << "Buffer is full.. Writing intermediate file...\n";
    std::ofstream interFile(std::to_string(fileCnt++), std::ios::binary | std::ios::out);
    interFile.write((const char*)&docBuffer[0], currBufferPos);
    interFile.close();
    // Reset buffer
    memset(docBuffer, 0, bufferSize);
    currBufferPos = 0;
}

void PostingGenerator::putPostingToBuffer(const Posting& aPosting, int& fileCnt) {
    // If buffer is full, clean it up first
    if (aPosting.size() + currBufferPos > bufferSize) {
        flushBuffer(fileCnt);
    }
    std::string posting = aPosting.term + ' ' + std::to_string(aPosting.docId) + ' ' + std::to_string(aPosting.frequency) + '\n';
    int numWritten = sprintf(docBuffer+currBufferPos, "%s", posting.c_str());
    if (numWritten != posting.size()) {
        std::cerr << "Failed to write\n";
        exit(1);
    }
    currBufferPos += numWritten;
}

void PostingGenerator::parseWetHeader(const std::string& line, std::string& documentUri, int& documentLen) {
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

std::vector<std::string> PostingGenerator::getAllFiles() {
    std::vector<std::string> allFiles;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirName.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string fn = std::string(ent->d_name);
            // Only return WET files
            if (fn.find("warc.wet") != std::string::npos && fn.find(".gz") == std::string::npos) {
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

bool PostingGenerator::isStrAlnum(const std::string& line) {
    for (char c : line)
        if (!isalnum(c)) { return false; }
    return true;
}

size_t PostingGenerator::countSubstr(const std::string& aString, const std::string& subStr) {
    size_t count = 0;
    size_t pos = 0;
    while (aString.find(subStr, pos) != std::string::npos) {
        pos  = aString.find(subStr, pos) + subStr.size();
        count++;
    }
    return count;
}

inline bool PostingGenerator::isDelim(char c) {
    for (int i = 0; i < DELIMITERS.size(); ++i)
        if (DELIMITERS[i] == c)
            return true;
    return false;
}

void PostingGenerator::tokenizeDocStream(const std::string& inputString, std::map<std::string, unsigned>& frequencies) {
    std::stringstream stringStream(inputString);
    char c;
    
    while (stringStream) {
        std::string word;
        
        // Read word
        while (!isDelim((c = stringStream.get())) && c != EOF) {
            word.push_back(c);
        }
        if (c != EOF)
            stringStream.unget();
        
        // Ignore word with non-alphanumeric chars
        if (isStrAlnum(word)) {
            // Normalize word by converting to lowercase
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            
            // Update frequency
            if (word.size() != 0) {
                if (frequencies.find(word) == frequencies.end()) {
                    frequencies[word] = 1;
                } else {
                    frequencies[word] += 1;
                }
            }
        }
        
        // Skip delims
        while (isDelim((c = stringStream.get())));
        if (c != EOF)
            stringStream.unget();
    }
}
