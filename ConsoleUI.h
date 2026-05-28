#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include <string>
#include <vector>
#include <atomic>
#include "Index.h"
#include <map>

class ConsoleUI {
private:
    Index index;
    std::string directoryPath;
    std::string databaseFile;
    std::string historyFile;
    std::vector<std::string> operationHistory;
    static const std::string RESET, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, BOLD;
    int searchCount; 
    std::string accentColor;
    void clearScreen();
    void printHeader();
    void renderMenu(int selected);
    void printSpinner(const std::string& label, std::atomic<bool>& stop);
    void printStatusBar();
    void showProgressBar(const std::string& text);
    std::string getRelevanceBar(size_t freq, size_t maxFreq);
    void highlightWordInText(const std::string& text, const std::string& word);
    std::map<std::string, int> searchStats;
    void addHistory(const std::string& operation);
    void saveHistory() const;
    void loadHistory();
    std::vector<std::string> savedSearches;
    void addSavedSearch(const std::string& query);
    void executeSearchQuery(const std::string& query);
    bool confirmExit();
public:
    ConsoleUI();
    void run();
    void loadDocuments();
    void buildIndex();
    void searchDocuments();
    void showStatistics();
    void saveDatabase();
    void loadDatabase();
    void showOperationHistory();
    void showAbout();
    void showPopularWords();
    void showSavedSearches();
    void refreshMenu();
};

#endif