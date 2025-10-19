// Indexer.cpp
// Author: Alena Voronchikhina
// Description: Implements the Indexer class that builds and queries
//              an inverted index mapping words to document IDs.
// Created: October 2025

#include "Indexer.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>

// Converts a string to lowercase and replaces non-letters with spaces.
// Ensures consistent word normalization across documents and queries.
std::string Indexer::normalize(std::string s) {
	for (char& c : s)
		c = std::isalpha(static_cast<unsigned char>(c)) ? std::tolower(c) : ' ';
	return s;
}

// Builds an inverted index from a list of document file paths.
// Each unique word is mapped to the IDs of documents where it appears.
void Indexer::build(const std::vector<std::string>& docs) {
	docs_ = docs;
	inv_.clear();

	// Process each document by ID.
	for (int id = 0; id < static_cast<int>(docs.size()); ++id) {
		std::ifstream f(docs[id]);
		if (!f) continue;	// Skip if file cannot be opened.

		// Read the entire file into a string.
		std::string all((std::istreambuf_iterator<char>(f)), {});
		std::istringstream in(normalize(all));

		std::string w;
		std::set<std::string> seen;	// Track unique words per document.

		// Add each unique word once per document.
		while (in >> w)
			if (seen.insert(w).second)
				inv_[w].push_back(id);
	}

	// Sort posting lists for efficient set intersection during queries.
	for (auto& [_, vec] : inv_)
		std::sort(vec.begin(), vec.end());
}

// Executes an AND query: returns document IDs that contain all query words.
std::vector<int> Indexer::query_and(const std::string& raw) const {
	std::istringstream in(normalize(raw));
	std::string w;
	bool first = true;
	std::vector<int> hits, tmp;

	// For each word, intersect its doc list with current results.
	while (in >> w) {
		auto it = inv_.find(w);
		if (it == inv_.end()) return {};	// Missing term -> no match.

		if (first) {
			hits = it->second;	// Initialize with first word’s docs.
			first = false;
		} else {
			tmp.clear();
			std::set_intersection(
				hits.begin(), hits.end(),
				it->second.begin(), it->second.end(),
				std::back_inserter(tmp)
			);
			hits.swap(tmp);
		}
	}
	return hits;
}
