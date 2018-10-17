//
//  main.cpp
//  IndexBuilder
//
//  Created by Hieu Do on 10/9/18.
//  Copyright © 2018 Hieu Do. All rights reserved.
//

#include "common.hpp"
#include "PostingGenerator.hpp"
#include "IndexOutput.hpp"

std::string invokeUnixUtil(int numPostings);

int main(int argc, const char * argv[]) {
    /*
     Run 1: 1155s - 197779 docs
     */
    std::chrono::steady_clock::time_point beginMain = std::chrono::steady_clock::now();

    // Generate postings from WET files
    int numPostings = generatePostings("wet_files");
//    int numPostings = 9;

    // Sort and merge intermediate postings
    std::string interFn = invokeUnixUtil(numPostings);
//    std::string interFn = "merged";

    // Generate final index
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    IndexOutput("finalIndex", interFn);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Finished generating final index " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";

    std::chrono::steady_clock::time_point endMain = std::chrono::steady_clock::now();
    std::cout << "=====\nFinished IndexBuilder " << std::chrono::duration_cast<std::chrono::seconds>(endMain - beginMain).count() << "s.\n";
    return 0;
}

std::string invokeUnixUtil(int numPostings) {
    std::string res = "merged";
    
    std::string baseCmd = "sort -S 50% -k1,1 -k2n ";
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
        
        // Remember sorted files for merge
        sortedFiles += (sortFile + ' ');
    }
    
    // Delete intermediate files
    
    // Call Unix merge
    std::cout << "Merging all sorted files...\n";
    std::string mergeCmd = baseCmd + " -m " + sortedFiles + " -o " + res;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    system(mergeCmd.c_str());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Finished merging " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s.\n";
    
    return res;
}
