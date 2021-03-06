//
//  main.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/9/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include <string>
#include <iostream>
#include "PostingGenerator.hpp"
#include "InvertedIndex.hpp"
#include "DocumentStore.hpp"

// Calling Unix sort util to sort and merge intermediate postings
std::string invokeUnixUtil(int numPostings, size_t bufferSize);

int main(int argc, const char * argv[]) {
    std::chrono::steady_clock::time_point beginMain = std::chrono::steady_clock::now();

    // Setting buffer size
    size_t bufferSize = 500000000;
    if (argc < 2) {
        std::cout << "Default buffer size " << bufferSize << " is used.\n";
    } else if (argc == 2) {
        bufferSize = atoi(argv[1]);
        std::cout << "Buffer size is set to " << bufferSize << '\n';
    } else {
        std::cerr << "Arguments not supported.\n";
        exit(1);
    }

    // 1. Generate postings from WET files
//    int numPostings = PostingGenerator("/Volumes/BACKUP/", bufferSize).generatePostings();
    int numPostings = PostingGenerator("wet_files/", bufferSize).generatePostings();


    // 2. Sort and merge intermediate postings
    std::string interFn = invokeUnixUtil(numPostings, bufferSize);

    // 3. Generate final index
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    InvertedIndex("finalIndex", interFn);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Finished generating final index " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";

    // Clean up merged file
    system("rm -f merged");

    std::chrono::steady_clock::time_point endMain = std::chrono::steady_clock::now();
    std::cout << "=====\nFinished IndexBuilder " << std::chrono::duration_cast<std::chrono::seconds>(endMain - beginMain).count() << "s.\n";
    
    return 0;
}

std::string invokeUnixUtil(int numPostings, size_t bufferSize) {    
    std::string res = "merged";
    
//    std::string baseCmd = "sort -S " + std::to_string(bufferSize) + "b -k1,1 -k2n ";
    std::string baseCmd = "sort -k1,1 -k2n ";
    
    std::string sortedFiles;
    while (--numPostings >= 0) {
        std::string file = std::to_string(numPostings);
        std::string sortFile = 's' + file;
        std::cout << "Calling Unix sort for " << file << '\n';
        std::string sortCmd = baseCmd + file + " -o " + sortFile; // s to denote sorted file
        
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        system(sortCmd.c_str());
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Finished sorting an intermediate file " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";
        
        // Remove unsorted file
        std::string rmCmd = "rm -f " + std::to_string(numPostings);
        system(rmCmd.c_str());
        
        // Remember sorted files for merge
        sortedFiles += (sortFile + ' ');
    }
    
    // Call Unix merge
    std::cout << "Merging all sorted files...\n";
    std::string mergeCmd = baseCmd + " -m " + sortedFiles + " -o " + res;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    system(mergeCmd.c_str());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Finished merging " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";
    
    // Remove sorted files
    std::string rmCmd = "rm -f " + sortedFiles;
    system(rmCmd.c_str());
    
    return res;
}
