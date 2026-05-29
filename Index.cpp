#include "Index.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
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

void Index::addDocument(Document* doc) {
    documents.emplace_back(doc);
}

void Index::buildIndex() {
    invertedIndex.clear();

    for (const auto& doc : documents) {
        std::unordered_map<std::string, std::vector<size_t>> wordPositions;
        const auto& words = doc->getWords();

        for (size_t i = 0; i < words.size(); ++i) {
            std::string normalized = normalizeWord(words[i]);

            if (!normalized.empty() && !isStopword(normalized)) {
                wordPositions[normalized].push_back(i);
            }
        }

        for (const auto& [word, positions] : wordPositions) {
            WordOccurrence occurrence(doc->getFilePath());
            occurrence.frequency = positions.size();
            occurrence.positions = positions;

            invertedIndex[word].push_back(occurrence);
        }
    }
}

std::vector<std::pair<std::string, size_t>> Index::search(const std::string& query) const {
    std::vector<std::pair<std::string, size_t>> results;

    std::string normalized = normalizeWord(query);
    if (normalized.empty() || isStopword(normalized)) {
        return results;
    }

    auto it = invertedIndex.find(normalized);

    if (it != invertedIndex.end()) {
        for (const WordOccurrence& occurrence : it->second) {
            results.emplace_back(occurrence.filePath, occurrence.frequency);
        }

        std::sort(results.begin(), results.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });
    }

    return results;
}

std::vector<Document*> Index::getDocuments() const {
    std::vector<Document*> result;

    for (const auto& doc : documents) {
        result.push_back(doc.get());
    }

    return result;
}

void Index::clear() {
    documents.clear();
    invertedIndex.clear();
}

bool Index::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << documents.size() << "\n";

    for (const auto& doc : documents) {
    file << doc->getFilePath() << "\n";
}

    return true;
}

bool Index::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    clear();

    size_t docCount;
    file >> docCount;
    if (!file) return false;
    file.ignore();

    for (size_t i = 0; i < docCount; ++i) {
        std::string path;
        std::getline(file, path);

        auto doc = std::make_unique<Document>(path);

        if (doc->loadFromFile()) {
        documents.push_back(std::move(doc));
        }
    }

    buildIndex();
    return true;
}

size_t Index::getDocumentCount() const { return documents.size(); }
size_t Index::getIndexedWordsCount() const { return invertedIndex.size(); }

std::vector<std::pair<std::string, size_t>> Index::getTopWords(size_t limit) const {
    std::vector<std::pair<std::string, size_t>> wordFreq;
    wordFreq.reserve(invertedIndex.size());
    
    for (const auto& [word, docs] : invertedIndex) {
        wordFreq.emplace_back(word, docs.size());
    }
    
    std::sort(wordFreq.begin(), wordFreq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
        
    if (wordFreq.size() > limit) wordFreq.resize(limit);
    return wordFreq;
}

int Index::levenshteinDistance(const std::string& a, const std::string& b) const {
    int n = a.size();
    int m = b.size();

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1));

    for (int i = 0; i <= n; i++) {
        dp[i][0] = i;
    }

    for (int j = 0; j <= m; j++) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;

            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }

    return dp[n][m];
}

std::string Index::findClosestWord(const std::string& query) const {
    std::string normalizedQuery = normalizeWord(query);

    if (normalizedQuery.empty()) {
        return "";
    }

    std::string bestWord = "";
    int bestDistance = 3;

    for (const auto& item :invertedIndex ) {
        const std::string& word = item.first;

        if (std::abs(static_cast<int>(word.size()) - static_cast<int>(normalizedQuery.size())) > 2) {
            continue;
        }

        int distance = levenshteinDistance(normalizedQuery, word);

        if (distance < bestDistance) {
            bestDistance = distance;
            bestWord = word;
        }
    }

    return bestWord;
}