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

std::string invokeUnixUtil(int numPostings) {
    std::string res = "merged";
    
    std::string baseCmd = "sort -S 50% -k1,1 -k2n ";
    std::string sortedFiles;
    while (--numPostings >= 0) {
        std::string file = 's' + std::to_string(numPostings);
        std::cout << "Calling Unix sort for " << file << '\n';
        std::string sortCmd = baseCmd + file + " -o " + file; // _ to denote sorted file
        system(sortCmd.c_str());
        
        // Remember sorted files for merge
        sortedFiles += (file + ' ');
    }
    
    // Delete intermediate files
    
    // Call Unix merge
    std::cout << "Merging all sorted files...\n";
    std::string mergeCmd = baseCmd + " -m " + sortedFiles + " -o " + res;
    system(mergeCmd.c_str());
    
    return res;
}

int main(int argc, const char * argv[]) {
    // TODO: add timer
    // Generate postings from WET files
    int numPostings = generatePostings();
//    int numPostings = 8;
    
    // Sort and merge intermediate postings
    std::string interFn = invokeUnixUtil(numPostings);
//    std::string interFn = "merged";
    
    // Generate final index
    IndexOutput("finalIndex", interFn);
    
    return 0;
}
