#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <vector>

class Document {
private:
    std::string filePath;
    std::string content;
    std::vector<std::string> words;
    
public:
    Document();
    Document(const std::string& path);
    
    bool loadFromFile();
    std::string getFilePath() const;
    std::string getContent() const;
    std::vector<std::string> getWords() const;
    void tokenize();
    std::string getContext(size_t position, size_t radius = 2) const;
};

#endif