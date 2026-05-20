#include "Index.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

Index::Index() { loadStopwords(); }
Index::~Index() { clear(); }

void Index::loadStopwords() {
    stopwords = {
        "si","sau","dar","in","pe","cu","la","din","printr","pentru",
        "care","ce","cum","unde","cand","de","ca","sa","fi","este",
        "sunt","era","fost","the","and","or","but","on","at","to",
        "for","of","with","by","from","a","an","is","are","was",
        "were","be","been","being","have","has","had","do","does","did"
    };
}

std::string Index::normalizeWord(const std::string& word) const {
    std::string result;
    for (char c : word) if (std::isalnum(c)) result += std::tolower(c);
    return result;
}

bool Index::isStopword(const std::string& word) const {
    return stopwords.find(word) != stopwords.end();
}

void Index::addDocument(Document* doc) { documents.push_back(doc); }

void Index::buildIndex() {
    wordToDocuments.clear();
    for (Document* doc : documents) {
        std::map<std::string, std::vector<size_t>> wordPositions;
        const auto& words = doc->getWords();
        for (size_t i = 0; i < words.size(); ++i) {
            std::string normalized = normalizeWord(words[i]);
            if (!normalized.empty() && !isStopword(normalized)) {
                wordPositions[normalized].push_back(i);
            }
        }
        for (const auto& [word, positions] : wordPositions) {
            bool found = false;
            for (auto* existingDoc : wordToDocuments[word]) {
                if (existingDoc->getFilePath() == doc->getFilePath()) { found = true; break; }
            }
            if (!found) wordToDocuments[word].push_back(doc);
        }
    }
}

std::vector<std::pair<std::string, size_t>> Index::search(const std::string& query) const {
    std::vector<std::pair<std::string, size_t>> results;
    std::string normalized = normalizeWord(query);
    if (isStopword(normalized)) return results;
    auto it = wordToDocuments.find(normalized);
    if (it != wordToDocuments.end()) {
        for (const Document* doc : it->second) {
            size_t freq = 0;
            for (const auto& word : doc->getWords())
                if (normalizeWord(word) == normalized) freq++;
            results.emplace_back(doc->getFilePath(), freq);
        }
        std::sort(results.begin(), results.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
    }
    return results;
}

std::vector<Document*> Index::getDocuments() const { return documents; }

void Index::clear() {
    for (Document* doc : documents) delete doc;
    documents.clear();
    wordToDocuments.clear();
}

bool Index::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    file << documents.size() << "\n";
    for (const Document* doc : documents) file << doc->getFilePath() << "\n";
    file << wordToDocuments.size() << "\n";
    for (const auto& [word, docs] : wordToDocuments) {
        file << word << " " << docs.size();
        for (const Document* doc : docs) file << " " << doc->getFilePath();
        file << "\n";
    }
    file.close();
    return true;
}

bool Index::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    clear();
    size_t docCount; file >> docCount; file.ignore();
    for (size_t i = 0; i < docCount; ++i) {
        std::string path; std::getline(file, path);
        Document* doc = new Document(path);
        if (doc->loadFromFile()) documents.push_back(doc);
        else delete doc;
    }
    size_t indexSize; file >> indexSize; file.ignore();
    for (size_t i = 0; i < indexSize; ++i) {
        std::string word; size_t count;
        file >> word >> count;
        for (size_t j = 0; j < count; ++j) {
            std::string path; file >> path;
            for (Document* doc : documents) {
                if (doc->getFilePath() == path) {
                    wordToDocuments[word].push_back(doc);
                    break;
                }
            }
        }
    }
    file.close();
    return true;
}

size_t Index::getDocumentCount() const { return documents.size(); }
size_t Index::getIndexedWordsCount() const { return wordToDocuments.size(); }

std::vector<std::pair<std::string, size_t>> Index::getTopWords(size_t limit) const {
    std::vector<std::pair<std::string, size_t>> wordFreq;
    wordFreq.reserve(wordToDocuments.size());
    
    for (const auto& [word, docs] : wordToDocuments) {
        wordFreq.emplace_back(word, docs.size());
    }
    
    std::sort(wordFreq.begin(), wordFreq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
        
    if (wordFreq.size() > limit) wordFreq.resize(limit);
    return wordFreq;
}