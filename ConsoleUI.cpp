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

ConsoleUI::ConsoleUI() : directoryPath("documente"), databaseFile("search_index.db") {
    Terminal::setupSignalHandlers();
}

void ConsoleUI::clearScreen() { Terminal::clearScreen(); }

void ConsoleUI::printHeader() {
    std::cout << BOLD << CYAN;
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║           📚 MOTOR DE CĂUTARE DOCUMENTE TEXT 📚          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}

void ConsoleUI::renderMenu(int selected) {
    std::vector<std::pair<int, std::string>> items = {
        {1, "Încarcă documente din director"},
        {2, "Construiește index"},
        {3, "Caută cuvinte"},
        {4, "Statistici"},
        {5, "Salvează baza de date"},
        {6, "Încarcă baza de date"},
        {0, "Ieșire"}
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
              << " | 💾 DB: " << (databaseFile.empty() ? "N/A" : "Ready") << RESET << "\n";
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

void ConsoleUI::loadDocuments() {
    clearScreen();
    printHeader();
    std::cout << BOLD << CYAN << "📂 Încărcare documente\n" << RESET;

    
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
    std::cin.get();
}

void ConsoleUI::searchDocuments() {
    clearScreen();
    printHeader();
    std::cout << BOLD << CYAN << "🔍 Căutare documente\n" << RESET;
    
    if (index.getIndexedWordsCount() == 0) {
        std::cout << RED << "✗ Index gol! Construiește-l mai întâi." << RESET << "\n";
        std::cin.get(); return;
    }

    std::string query;
    std::cout << "Cuvânt: " << GREEN;
    std::getline(std::cin, query);
    std::cout << RESET;
    if (query.empty()) { std::cin.get(); return; }

    auto results = index.search(query);
    std::cout << "\n";
    if (results.empty()) {
        std::cout << BLUE << "ℹ Niciun rezultat pentru '" << query << "'" << RESET << "\n";
    } else {
        size_t maxFreq = 0;
        for (const auto& r : results) if (r.second > maxFreq) maxFreq = r.second;
        
        std::cout << GREEN << "✓ " << results.size() << " documente găsite:\n" << RESET;
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& [path, freq] = results[i];
            std::cout << YELLOW << "┌─ #" << (i+1) << " " << getRelevanceBar(freq, maxFreq) << RESET << "\n";
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
    std::cin.get();
}

void ConsoleUI::showStatistics() {
    clearScreen();
    printHeader();
    std::cout << BOLD << CYAN << "📊 Statistici\n" << RESET;
    printStatusBar();
    std::cin.get();
}

void ConsoleUI::saveDatabase() {
    clearScreen();
    printHeader();
    if (index.saveToFile(databaseFile)) 
        std::cout << GREEN << "✓ Salvat în " << databaseFile << RESET << "\n";
    else 
        std::cout << RED << "✗ Eroare la salvare!" << RESET << "\n";
    std::cin.get();
}

void ConsoleUI::loadDatabase() {
    clearScreen();
    printHeader();
    if (index.loadFromFile(databaseFile)) {
        std::cout << GREEN << "✓ Bază de date încărcată!" << RESET << "\n";
        printStatusBar();
    } else {
        std::cout << RED << " Nu s-a putut încărca." << RESET << "\n";
    }
    std::cin.get();
}

void ConsoleUI::run() {
    Terminal::enableRawMode();
    clearScreen();
    
    int selected = 0;
    int menuSize = 7; 
    int key;

    while (true) {
        clearScreen();
        printHeader();
        renderMenu(selected);
        printStatusBar();
        std::cout << "\n   Folosește ↑/↓ pentru navigare, ENTER pentru selectare\n";

        key = Terminal::readKey();
        if (key == 1000) selected = (selected - 1 + menuSize) % menuSize;     
        else if (key == 1001) selected = (selected + 1) % menuSize;             
        else if (key == '\n' || key == '\r') {                                  
            int choice = (selected == 6) ? 0 : (selected + 1);
            Terminal::disableRawMode(); 
            switch (choice) {
                case 1: loadDocuments(); break;
                case 2: buildIndex(); break;
                case 3: searchDocuments(); break;
                case 4: showStatistics(); break;
                case 5: saveDatabase(); break;
                case 6: loadDatabase(); break;
                case 0: 
                    clearScreen(); 
                    std::cout << GREEN << "👋 La revedere!\n" << RESET; 
                    return;
            }
            Terminal::enableRawMode(); 
            selected = 0; 
        }
    }
}