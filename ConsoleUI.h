#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include "Index.h"
#include <string>

class ConsoleUI {
private:
    Index index;
    std::string directoryPath;
    std::string databaseFile;
    
    static const std::string RESET, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, BOLD;
    
    void clearScreen();
    void printHeader();
    void printMenu();
    void printSeparator();
    void printSuccess(const std::string& msg);
    void printError(const std::string& msg);
    void printInfo(const std::string& msg);
    void printProgressBar(size_t current, size_t total, const std::string& label);
    void highlightWordInText(const std::string& text, const std::string& word);
    
public:
    ConsoleUI();
    void run();
    void loadDocuments();
    void buildIndex();
    void searchDocuments();
    void showStatistics();
    void saveDatabase();
    void loadDatabase();
};

#endif