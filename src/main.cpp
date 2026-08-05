// main.cpp
// Author: Alena Voronchikhina
// Description: Entry point for the Mini Search Engine project.
//              Builds the index from local text files and allows
//              querying using AND search (CLI args or interactive).
// Created: October 2025

#include "Indexer.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace {

void print_hits(const Indexer& ix, const std::vector<int>& hits) {
	if (hits.empty()) {
		std::cout << "No results\n";
		return;
	}
	for (int id : hits)
		std::cout << ix.documents()[id] << "\n";
}

}  // namespace

int main(int argc, char* argv[]) {
	Indexer ix;
	ix.build({"data/doc1.txt", "data/doc2.txt"});

	// Non-interactive: ./search cats milk
	if (argc > 1) {
		std::string q = argv[1];
		for (int i = 2; i < argc; ++i) {
			q += ' ';
			q += argv[i];
		}
		print_hits(ix, ix.query_and(q));
		return 0;
	}

	std::cout << "Type query (AND). 'exit' to quit.\n";
	std::string q;
	while (true) {
		std::cout << "query> ";
		if (!std::getline(std::cin, q) || q == "exit") break;
		print_hits(ix, ix.query_and(q));
	}
}
