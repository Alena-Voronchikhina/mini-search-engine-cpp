// Indexer.hpp
// Author: Alena Voronchikhina
// Description: Declares the class Indexer for building and querying
//              an inverted index from a list of text documents.
// Created: October 2025

#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Indexer {
	public:
		// Builds an inverted index from the given list of document paths.
		void build(const std::vector<std::string> &docs);

		// Returns IDs of documents that contain all words in the query (AND).
		[[nodiscard]] std::vector<int> query_and(const std::string &raw) const;

		// Exposes the stored document paths (for printing results in main()).
		[[nodiscard]] const std::vector<std::string> &documents() const { return docs_; }

	private:
		// Normalizes text: lowercase; non-letters become spaces.
		static std::string normalize(std::string s);

		std::vector<std::string> docs_;							// paths to indexed documents
		std::unordered_map<std::string, std::vector<int>> inv_; // word -> doc IDs
};
