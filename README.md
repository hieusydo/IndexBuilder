# IndexBuilder

CS 6913 (Web Search Engine) Assignment - NYU Tandon School of Engineering

## Goal
Create an inverted index structure from CommonCrawl data

## What It Does
- Use merge sort indexing
- Compress final index with variable byte encoding and chunk-wise compression
- End-to-end indexing rate of 430 documents per second
- Store original documents to database (SQLite3 or Redis)