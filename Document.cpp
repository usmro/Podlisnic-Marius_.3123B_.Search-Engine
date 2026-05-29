#include "Document.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Document::Document() : filePath(""), content("") {}
Document::Document(const std::string& path) : filePath(path), content("") {}

bool Document::loadFromFile() {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();
    tokenize();
    return true;
}

void Document::tokenize() {
    std::istringstream iss(content);
    std::string word;
    words.clear();
    while (iss >> word) {
        std::string cleanWord;
        for (char c : word) {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r' &&
    c != '.' && c != ',' && c != ';' && c != ':' &&
    c != '!' && c != '?' && c != '"' && c != '\'' &&
    c != '(' && c != ')' && c != '-' && c != '_') {
    cleanWord += std::tolower(c);
}
        }
        if (!cleanWord.empty()) words.push_back(cleanWord);
    }
}

std::string Document::getFilePath() const { return filePath; }
std::string Document::getContent() const { return content; }
const std::vector<std::string>& Document::getWords() const { return words; }

std::string Document::getContext(size_t position, size_t radius) const {
    if (position >= words.size()) return "";
    size_t start = (position > radius) ? position - radius : 0;
    size_t end = std::min(position + radius + 1, words.size());
    std::string context;
    for (size_t i = start; i < end; ++i) {
        if (i > start) context += " ";
        context += words[i];
    }
    return context;
}