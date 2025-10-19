// main.cpp
// Author: Alena Voronchikhina
// Description: Entry point for the Mini Search Engine project.
//              Builds the index from local text files and allows
//              interactive querying using AND search.
// Created: October 2025

#include "Indexer.hpp"
#include <iostream>

int main() {
	// Create an Indexer instance and build the inverted index
	// from two sample documents located in the data/ folder.
	Indexer ix;
	ix.build({"data/doc1.txt", "data/doc2.txt"});   // local corpus
	std::cout << "Type query (AND). 'exit' to quit.\n";
	std::string q;
	while (true) {
		std::cout << "query> ";
		if (!std::getline(std::cin, q) || q == "exit") break;

		// Perform an AND query over the indexed documents.
		auto hits = ix.query_and(q);

		// Display results or indicate no matches.
		if (hits.empty()) {
			std::cout << "No results\n";
			continue;
		}

		// Print the file paths of all documents that matched.
		for (int id : hits)
			std::cout << ix.documents()[id] << "\n";
	}
}
