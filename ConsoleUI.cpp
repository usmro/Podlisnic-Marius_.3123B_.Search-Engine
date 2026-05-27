#include "ConsoleUI.h"
#include "Terminal.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <atomic>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>


namespace fs = std::filesystem;

const std::string ConsoleUI::RESET = "\033[0m";
const std::string ConsoleUI::RED = "\033[31m";
const std::string ConsoleUI::GREEN = "\033[32m";
const std::string ConsoleUI::YELLOW = "\033[33m";
const std::string ConsoleUI::BLUE = "\033[34m";
const std::string ConsoleUI::MAGENTA = "\033[35m";
const std::string ConsoleUI::CYAN = "\033[36m";
const std::string ConsoleUI::WHITE = "\033[37m";
const std::string ConsoleUI::BOLD = "\033[1m";

ConsoleUI::ConsoleUI() : directoryPath("documente"), databaseFile("search_index.db"),historyFile("istoric_operatii.txt"),searchCount(0),accentColor(CYAN) {
    Terminal::setupSignalHandlers();
    loadHistory();
}

void ConsoleUI::clearScreen() { Terminal::clearScreen(); }

void ConsoleUI::printHeader() {
    std::cout << BOLD << accentColor; 
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║           📚 MOTOR DE CĂUTARE DOCUMENTE TEXT 📚          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}
void ConsoleUI::showProgressBar(const std::string& text) {
    std::cout << text << "\n";
    std::cout << "[";

    for (int i = 0; i < 30; i++) {
        std::cout << "#";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    std::cout << "] 100%\n";
}

void ConsoleUI::renderMenu(int selected) {
    std::vector<std::pair<int, std::string>> items = {
        {1, "Încarcă documente din director"},
        {2, "Construiește index"},
        {3, "Caută cuvinte"},
        {4, "Statistici"},
        {5, "Salvează baza de date"},
        {6, "Încarcă baza de date"},
        {7, "Istoric operatii"},
        {8, "Schimba Tema Culori [T]"},
        {9, "Despre aplicatie"},
        {10,"Cuvinte populare"},
        {11, "Cautari salvate"},
        {0, "Iesire"}
    };

    std::cout << BOLD << YELLOW << "┌─────────────────────────────────────────────────────┐\n";
    std::cout << "│                   MENIU PRINCIPAL                     │\n";
    std::cout << "─────────────────────────────────────────────────────┘\n" << RESET;
    
    for (size_t i = 0; i < items.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == selected);
        std::cout << (isSelected ? GREEN + BOLD + " ▶ " : "   ");
        std::cout << "[" << items[i].first << "] " << items[i].second << RESET << "\n";
    }
    std::cout << "\n";
}

void ConsoleUI::printStatusBar() {
    std::cout << CYAN << "─────────────────────────────────────────────────────\n";
    std::cout << " 📊 Doc: " << index.getDocumentCount() 
              << " | 🔤 Cuvinte indexate: " << index.getIndexedWordsCount()
              << " | 💾 DB: " << (databaseFile.empty() ? "N/A" : "Ready") 
              << " | 🔍 Căutări: " << searchCount 
              << RESET << "\n";
}

void ConsoleUI::printSpinner(const std::string& label, std::atomic<bool>& stop) {
    
    const char* frames[] = {"⠋", "⠙", "", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", ""};
    int i = 0;
    while (!stop.load()) {
        std::cout << "\r" << BLUE << label << " " << frames[i] << RESET;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        i = (i + 1) % 10;
    }
    std::cout << "\r" << std::string(label.size() + 5, ' ') << "\r";
}

std::string ConsoleUI::getRelevanceBar(size_t freq, size_t maxFreq) {
    if (maxFreq == 0) return "";
    int len = (freq * 10) / maxFreq;
    std::string bar = "[";
    for (int i = 0; i < 10; ++i) bar += (i < len) ? (GREEN + "█" + RESET) : "░";
    return bar + "] " + std::to_string(freq);
}

void ConsoleUI::highlightWordInText(const std::string& text, const std::string& word) {
    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find(word, pos)) != std::string::npos) {
        result.replace(pos, word.length(), RED + BOLD + word + RESET);
        pos += word.length() + RED.length() + BOLD.length() + RESET.length();
    }
    std::cout << "    Context: \"" << result << "\"\n";
}
void ConsoleUI::addHistory(const std::string& operation) {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::ostringstream entry;
    entry << std::put_time(localTime, "%d.%m.%Y %H:%M:%S")
          << " | " << operation;

    operationHistory.push_back(entry.str());
    if (operationHistory.size() > 100) {
    operationHistory.erase(operationHistory.begin());
    }
    saveHistory();
}

void ConsoleUI::saveHistory() const {
    std::ofstream file(historyFile);

    if (!file.is_open()) {
        return;
    }

    for (const std::string& entry : operationHistory) {
        file << entry << "\n";
    }
}

void ConsoleUI::loadHistory() {
    operationHistory.clear();

    std::ifstream file(historyFile);

    if (!file.is_open()) {
        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            operationHistory.push_back(line);
        }
    }
}

void ConsoleUI::showOperationHistory() {
    clearScreen();
    printHeader();

    std::cout << BOLD << CYAN << "Istoric operatii\n" << RESET;
    std::cout << YELLOW << "---------------------------------------------\n" << RESET;

    if (operationHistory.empty()) {
        std::cout << BLUE << "Istoricul este gol.\n" << RESET;
    } else {
        for (size_t i = 0; i < operationHistory.size(); i++) {
            std::cout << GREEN << i + 1 << ". " << RESET
                      << operationHistory[i] << "\n";
        }
    }

    std::cout << YELLOW << "---------------------------------------------\n" << RESET;
    std::cout << "\n1. Sterge istoricul";
    std::cout << "\n0. Inapoi";
    std::cout << "\n\nAlege optiunea: ";

    std::string option;
    std::getline(std::cin, option);

    if (option == "1") {
        operationHistory.clear();
        saveHistory();

        std::cout << GREEN << "\nIstoricul a fost sters cu succes!\n" << RESET;
        std::cout << "Apasa ENTER pentru a continua...";
        std::cin.get();
    }
}

void ConsoleUI::loadDocuments() {
    clearScreen();
    printHeader();

    index = Index();

    showProgressBar("Se incarca documentele...");


    
    size_t totalFiles = 0;
    if (fs::exists(directoryPath)) {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") totalFiles++;
        }
    }

    if (totalFiles == 0) {
        std::cout << RED << "✗ Nu am găsit fișiere .txt în director!" << RESET << "\n";
        std::cin.get(); return;
    }

    size_t count = 0;
    
    const char* spinChars[] = {"⠋", "", "⠹", "⠸", "⠼", "⠴", "", "⠧", "⠇", "⠏"};
    int spinIdx = 0;

    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            
            float percent = static_cast<float>(count) / totalFiles * 100.0f;
            
            
            int barWidth = 30;
            int pos = static_cast<int>(barWidth * percent / 100.0f);
            
            std::cout << "\r" << BLUE << "Progres: [";
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << GREEN << "█" << RESET << BLUE;
                else std::cout << "░";
            }
            std::cout << BLUE << "] " << spinChars[spinIdx] << " " 
                      << static_cast<int>(percent) << "%" << RESET;
            std::cout.flush();
            
            spinIdx = (spinIdx + 1) % 10; 

            Document* doc = new Document(entry.path().string());
            if (doc->loadFromFile()) { index.addDocument(doc); }
            else delete doc;
            count++;
        }
    }

    std::cout << "\r" << std::string(70, ' ') << "\r"; 
    std::cout << GREEN << "✓ " << count << " documente încărcate cu succes!" << RESET << "\n";
    addHistory("Au fost incarcate documentele din directorul: " + directoryPath);
    std::cin.get();
}

void ConsoleUI::buildIndex() {
    clearScreen();
    printHeader();
    std::cout << BOLD << CYAN << "🔨 Construire index\n" << RESET;
    
    if (index.getDocuments().empty()) {
        std::cout << RED << "✗ Nu există documente!" << RESET << "\n";
        std::cin.get(); return;
    }

    std::atomic<bool> stopSpinner{false};
    std::thread spinnerThread(&ConsoleUI::printSpinner, this, "Se indexează cuvintele...", std::ref(stopSpinner));
    
    auto start = std::chrono::high_resolution_clock::now();
    index.buildIndex();
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    stopSpinner.store(true);
    spinnerThread.join();
    
    std::cout << GREEN << "✓ Index construit în " << ms << "ms!" << RESET << "\n";
    addHistory("Indexul documentelor a fost construit");
    std::cin.get();
}

void ConsoleUI::addSavedSearch(const std::string& query) {
    if (query.empty()) return;

    for (const auto& q : savedSearches) {
        if (q == query) return;
    }

    savedSearches.push_back(query);

    if (savedSearches.size() > 10) {
        savedSearches.erase(savedSearches.begin());
    }
}

void ConsoleUI::searchDocuments() {
    clearScreen();
    printHeader();

    std::cout << BOLD << CYAN << "🔍 Căutare documente\n" << RESET;

    if (index.getIndexedWordsCount() == 0) {
        std::cout << RED << "✗ Index gol! Construiește-l mai întâi." << RESET << "\n";
        std::cin.get();
        return;
    }

    std::string query;
    std::cout << "Cuvânt: " << GREEN;
    std::getline(std::cin, query);
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);
    std::cout << RESET;

    if (query.empty()) {
        std::cin.get();
        return;
    }

    addSavedSearch(query);
    executeSearchQuery(query);

    std::cin.get();
}
void ConsoleUI::executeSearchQuery(const std::string& query) {
    searchStats[query]++;

    auto start = std::chrono::high_resolution_clock::now();
    auto results = index.search(query);
    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (!results.empty()) {
        searchCount++;
    }

    std::cout << "\n✓ " << results.size()
              << " documente găsite în " << ms << "ms\n\n";

    if (results.empty()) {
    std::cout << BLUE << "ℹ Niciun rezultat pentru '" << query << "'" << RESET << "\n";

    std::string suggestion = index.findClosestWord(query);

    if (!suggestion.empty()) {
        std::cout << YELLOW << "Ai vrut sa cauti: "
                  << GREEN << suggestion << RESET << " ? (y/n): ";

        std::string answer;
        std::getline(std::cin, answer);

        if (answer == "y" || answer == "Y") {
            clearScreen();
            printHeader();

            std::cout << BOLD << CYAN << "🔍 Căutare documente\n" << RESET;
            std::cout << "Cautare corectata: " << GREEN << suggestion << RESET << "\n";

            executeSearchQuery(suggestion);
            return;
        }
    }
} else {
        size_t maxFreq = 0;

        for (const auto& r : results) {
            if (r.second > maxFreq) {
                maxFreq = r.second;
            }
        }

        std::cout << GREEN << "✓ " << results.size()
                  << " documente găsite:\n" << RESET;

        for (size_t i = 0; i < results.size(); ++i) {
            const auto& [path, freq] = results[i];

            std::cout << YELLOW << "┌─ #" << (i + 1)
                      << " " << getRelevanceBar(freq, maxFreq)
                      << RESET << "\n";

            std::cout << "│ " << BLUE << path << RESET << "\n";

            for (const auto& doc : index.getDocuments()) {
                if (doc->getFilePath() == path) {
                    const auto& words = doc->getWords();

                    for (size_t j = 0; j < words.size(); ++j) {
                        if (words[j] == query) {
                            std::cout << "│ ";
                            highlightWordInText(doc->getContext(j, 3), query);
                            break;
                        }
                    }

                    break;
                }
            }

            std::cout << YELLOW << "└────────────────────────────────────────\n" << RESET;
        }
    }

    addHistory("Cautare efectuata dupa: " + query);
}

void ConsoleUI::showSavedSearches() {
    clearScreen();
    printHeader();

    std::cout << BOLD << CYAN << "💾 Cautari salvate\n" << RESET;

    if (savedSearches.empty()) {
        std::cout << BLUE << "Nu exista cautari salvate.\n" << RESET;
        std::cin.get();
        return;
    }

    for (size_t i = 0; i < savedSearches.size(); ++i) {
        std::cout << GREEN << i + 1 << ". " << RESET
                  << savedSearches[i] << "\n";
    }

    std::cout << "\nIntrodu numarul cautarii pentru repetare sau 0 pentru inapoi: ";

    std::string option;
    std::getline(std::cin, option);

    if (option == "0" || option.empty()) {
        return;
    }

    int choice = std::stoi(option);

    if (choice < 1 || choice > static_cast<int>(savedSearches.size())) {
        std::cout << RED << "Optiune invalida!\n" << RESET;
        std::cin.get();
        return;
    }

    clearScreen();
    printHeader();

    std::string query = savedSearches[choice - 1];

    std::cout << BOLD << CYAN << "🔍 Repetare cautare: "
              << GREEN << query << RESET << "\n";

    executeSearchQuery(query);

    std::cin.get();
}

void ConsoleUI::showStatistics() {
    clearScreen();
    printHeader();
    std::cout << BOLD << CYAN << "📊 Statistici Sistem\n" << RESET;
    
   
    printStatusBar();
    
    std::cout << "\n🔝 Top 5 Cuvinte Frecvente (după nr. documente):\n";
    auto topWords = index.getTopWords(5); // 🟢 AICI CHEMĂM FUNCȚIA NOUĂ
    
    if (topWords.empty()) {
        std::cout << BLUE << "  ℹ Nu există cuvinte indexate încă." << RESET << "\n";
    } else {
        for (size_t i = 0; i < topWords.size(); ++i) {
            std::cout << YELLOW << "  " << (i+1) << ". " << GREEN << topWords[i].first << RESET 
                      << "  →  prezent în " << CYAN << topWords[i].second << " documente\n";
        }
    }
    
    std::cout << "\n";
    std::cin.get();
}

void ConsoleUI::saveDatabase() {
    clearScreen();
    printHeader();

    if (index.saveToFile(databaseFile)) {
        std::cout << GREEN << "✓ Salvat în " << databaseFile << RESET << "\n";
        addHistory("Baza de date a fost salvata");
    } else {
        std::cout << RED << "✗ Eroare la salvare!" << RESET << "\n";
        addHistory("Eroare la salvarea bazei de date");
    }

    std::cin.get();
}

void ConsoleUI::loadDatabase() {
    clearScreen();
    printHeader();

    if (index.loadFromFile(databaseFile)) {
        std::cout << GREEN << "✓ Baza de date incarcata!" << RESET << "\n";
        printStatusBar();
        addHistory("Baza de date a fost incarcata");
    } else {
        std::cout << RED << "✗ Nu s-a putut incarca baza de date." << RESET << "\n";
        addHistory("Eroare la incarcarea bazei de date");
    }

    std::cin.get();
}
void ConsoleUI::showAbout() {
    clearScreen();
    printHeader();

    std::cout << BOLD << CYAN << "Despre aplicatie\n" << RESET;
    std::cout << "----------------------------------\n";
    std::cout << "Denumire: Motor de cautare documente\n";
    std::cout << "Autor: Podlisnic Marius\n";
    std::cout << "Limbaj: C++\n";
    std::cout << "Paradigma: Programare orientata pe obiecte\n";
    std::cout << "Functionalitati:\n";
    std::cout << "- incarcare documente\n";
    std::cout << "- construire index\n";
    std::cout << "- cautare documente\n";
    std::cout << "- salvare/incarcare baza de date\n";
    std::cout << "- istoric operatii\n";
    std::cout << "\nApasa ENTER pentru a reveni la meniu...";
    std::cin.get();
}
void ConsoleUI::showPopularWords()
{
    clearScreen();
    printHeader();

    if(searchStats.empty())
    {
        std::cout<<"Nu exista cautari.\n";
        std::cin.get();
        return;
    }

    std::string topWord;
    int maxCount=0;

    for(auto item : searchStats)
    {
        if(item.second>maxCount)
        {
            maxCount=item.second;
            topWord=item.first;
        }
    }

    std::cout<<"Cuvant cel mai cautat: "<<topWord<<"\n";
    std::cout<<"Numar cautari: "<<maxCount<<"\n";

    std::cout<<"\nENTER pentru revenire...";
    std::cin.get();
}

bool ConsoleUI::confirmExit() {
    clearScreen();
    printHeader();

    std::cout << YELLOW << "Sigur doresti sa iesi din aplicatie? (y/n): " << RESET;

    std::string answer;
    std::getline(std::cin, answer);

    return answer == "y" || answer == "Y";
}

void ConsoleUI::run() {
    Terminal::enableRawMode();
    clearScreen();
    if (fs::exists(databaseFile)) {
        if (index.loadFromFile(databaseFile)) {
            std::cout << GREEN << "✅ Index încărcat automat din " << databaseFile << RESET << "\n";
            printStatusBar();
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }
    }
    int selected = 0;
    int menuSize = 12;
    int key;

    while (true) {
        clearScreen();
        printHeader();
        renderMenu(selected);
        printStatusBar();
        std::cout << "\n   Folosește ↑/↓ pentru navigare, ENTER pentru selectare\n";

        key = Terminal::readKey();
        if (key == 't' || key == 'T') {
            if (accentColor == CYAN) accentColor = BLUE;
            else if (accentColor == BLUE) accentColor = GREEN;
            else accentColor = CYAN;
            
            clearScreen();
            continue; 
        }
        if (key == 1000) selected = (selected - 1 + menuSize) % menuSize;     
        else if (key == 1001) selected = (selected + 1) % menuSize;             
        else if (key == '\n' || key == '\r') {                                  
            int choice = (selected == 11) ? 0 : (selected + 1);
            Terminal::disableRawMode(); 
            switch (choice) {
                case 1: loadDocuments(); break;
                case 2: buildIndex(); break;
                case 3: searchDocuments(); break;
                case 4: showStatistics(); break;
                case 5: saveDatabase(); break;
                case 6: loadDatabase(); break;
                case 7: showOperationHistory(); break;
                case 8: 
                    if (accentColor == CYAN) accentColor = BLUE;
                    else if (accentColor == BLUE) accentColor = GREEN;
                    else accentColor = CYAN;
                    break;
                case 9:
                    showAbout();
                    break;
                case 10:
                    showPopularWords();
                    break;
                case 11:
                    showSavedSearches();
                    break;
                case 0: 
                    if (confirmExit()) {
                        clearScreen();
                        std::cout << GREEN << "👋 La revedere!\n" << RESET;
                        addHistory("Aplicatia a fost inchisa");
                        return;
                    }               
                    break;
            }
            Terminal::enableRawMode(); 
            selected = 0; 
        }
    }
}