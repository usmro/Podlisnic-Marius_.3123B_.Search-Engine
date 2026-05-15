#ifndef INDEX_H
#define INDEX_H

#include "Document.h"
#include <map>
#include <vector>
#include <string>
#include <set>

struct WordOccurrence {
    std::string filePath;
    size_t frequency;
    std::vector<size_t> positions;
    WordOccurrence(const std::string& path) : filePath(path), frequency(0) {}
};

class Index {
private:
    std::map<std::string, std::vector<Document*>> wordToDocuments;
    std::vector<Document*> documents;
    std::set<std::string> stopwords;
    
    std::string normalizeWord(const std::string& word) const;
    bool isStopword(const std::string& word) const;
    void loadStopwords();
    
public:
    Index();
    ~Index();
    
    void addDocument(Document* doc);
    void buildIndex();
    std::vector<std::pair<std::string, size_t>> search(const std::string& query) const;
    std::vector<Document*> getDocuments() const;
    void clear();
    
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    size_t getDocumentCount() const;
    size_t getIndexedWordsCount() const;
};

#endif