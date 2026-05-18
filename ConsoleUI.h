#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include <string>
#include <vector>
#include <atomic>
#include "Index.h"

class ConsoleUI {
private:
    Index index;
    std::string directoryPath;
    std::string databaseFile;
    
    static const std::string RESET, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, BOLD;
    int searchCount; 
    void clearScreen();
    void printHeader();
    void renderMenu(int selected);
    void printSpinner(const std::string& label, std::atomic<bool>& stop);
    void printStatusBar();
    std::string getRelevanceBar(size_t freq, size_t maxFreq);
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