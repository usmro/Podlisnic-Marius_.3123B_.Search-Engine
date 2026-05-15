#include "ConsoleUI.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

// Inițializare culori
const std::string ConsoleUI::RESET = "\033[0m";
const std::string ConsoleUI::RED = "\033[31m";
const std::string ConsoleUI::GREEN = "\033[32m";
const std::string ConsoleUI::YELLOW = "\033[33m";
const std::string ConsoleUI::BLUE = "\033[34m";
const std::string ConsoleUI::MAGENTA = "\033[35m";
const std::string ConsoleUI::CYAN = "\033[36m";
const std::string ConsoleUI::WHITE = "\033[37m";
const std::string ConsoleUI::BOLD = "\033[1m";

ConsoleUI::ConsoleUI() : directoryPath("documente"), databaseFile("search_index.db") {}

void ConsoleUI::clearScreen() { std::cout << "\033[2J\033[1;1H"; }

void ConsoleUI::printHeader() {
    clearScreen();
    std::cout << BOLD << CYAN;
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║           MOTOR DE CĂUTARE DOCUMENTE TEXT            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}

void ConsoleUI::printMenu() {
    std::cout << BOLD << YELLOW << "┌─────────────────────────────────────────────────────┐\n";
    std::cout << "│                   MENIU PRINCIPAL                     │\n";
    std::cout << "└─────────────────────────────────────────────────────┘\n" << RESET;
    std::cout << GREEN << "  [1]" << RESET << " Încarcă documente din director\n";
    std::cout << GREEN << "  [2]" << RESET << " Construiește index\n";
    std::cout << GREEN << "  [3]" << RESET << " Caută cuvinte\n";
    std::cout << GREEN << "  [4]" << RESET << " Statistici\n";
    std::cout << GREEN << "  [5]" << RESET << " Salvează baza de date\n";
    std::cout << GREEN << "  [6]" << RESET << " Încarcă baza de date\n";
    std::cout << GREEN << "  [0]" << RESET << " Ieșire\n\n";
}

void ConsoleUI::printSeparator() { std::cout << CYAN << "─────────────────────────────────────────────────────\n" << RESET; }
void ConsoleUI::printSuccess(const std::string& msg) { std::cout << GREEN << "✓ " << msg << RESET << "\n"; }
void ConsoleUI::printError(const std::string& msg) { std::cout << RED << "✗ " << msg << RESET << "\n"; }
void ConsoleUI::printInfo(const std::string& msg) { std::cout << BLUE << " " << msg << RESET << "\n"; }

void ConsoleUI::printProgressBar(size_t current, size_t total, const std::string& label) {
    int barWidth = 40;
    float progress = static_cast<float>(current) / total;
    int pos = barWidth * progress;
    std::cout << "\r" << BLUE << label << " [" << RESET;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << GREEN << "█" << RESET;
        else std::cout << "░";
    }
    std::cout << BLUE << "] " << static_cast<int>(progress * 100) << "%" << RESET << "  ";
    std::cout.flush();
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
    printHeader();
    std::cout << BOLD << CYAN << "📂 Încărcare documente\n" << RESET;
    printSeparator();

    std::cout << "Director curent: " << GREEN << directoryPath << RESET << "\n\n";

    if (!fs::exists(directoryPath)) {
        fs::create_directories(directoryPath);
        printInfo("Directorul a fost creat. Adaugă fișiere .txt în el.");
        std::cout << "\nApasă Enter pentru a continua...";
        std::cin.ignore(); std::cin.get();
        return;
    }

    size_t count = 0;
    size_t total = std::distance(fs::directory_iterator(directoryPath), fs::directory_iterator());

    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            printProgressBar(count, total, "Progres");
            Document* doc = new Document(entry.path().string());
            if (doc->loadFromFile()) {
                index.addDocument(doc);
                count++;
            } else { delete doc; }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::cout << "\n\n";
    printSuccess("Au fost încărcate " + std::to_string(count) + " documente!");
    std::cout << "\nApasă Enter pentru a continua...";
    std::cin.ignore(); std::cin.get();
}

void ConsoleUI::buildIndex() {
    printHeader();
    std::cout << BOLD << CYAN << "🔨 Construire index\n" << RESET;
    printSeparator();

    if (index.getDocuments().empty()) {
        printError("Nu există documente încărcate! Încarcă documente mai întâi.");
    } else {
        std::cout << "Se construiește indexul...\n\n";
        auto start = std::chrono::high_resolution_clock::now();
        index.buildIndex();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        printSuccess("Index construit cu succes!");
        std::cout << "\n";
        printInfo("Documente: " + std::to_string(index.getDocumentCount()));
        printInfo("Cuvinte indexate: " + std::to_string(index.getIndexedWordsCount()));
        printInfo("Timp: " + std::to_string(duration.count()) + "ms");
    }
    std::cout << "\n\nApasă Enter pentru a continua...";
    std::cin.ignore(); std::cin.get();
}

void ConsoleUI::searchDocuments() {
    printHeader();
    std::cout << BOLD << CYAN << "🔍 Căutare documente\n" << RESET;
    printSeparator();

    if (index.getIndexedWordsCount() == 0) {
        printError("Indexul este gol! Construiește indexul mai întâi.");
        std::cout << "\nApasă Enter pentru a continua...";
        std::cin.ignore(); std::cin.get();
        return;
    }

    std::string query;
    std::cout << "Introdu cuvântul de căutat: " << GREEN;
    std::getline(std::cin, query);
    std::cout << RESET;

    if (query.empty()) {
        printError("Căutarea nu poate fi goală!");
        std::cout << "\nApasă Enter pentru a continua...";
        std::cin.ignore(); std::cin.get();
        return;
    }

    auto results = index.search(query);
    std::cout << "\n";
    if (results.empty()) {
        printInfo("Nu s-au găsit rezultate pentru '" + query + "'");
    } else {
        printSuccess("S-au găsit " + std::to_string(results.size()) + " documente!\n");
        int rank = 1;
        for (const auto& [filePath, frequency] : results) {
            std::cout << YELLOW << "┌─ Document #" << rank++ << RESET << "\n";
            std::cout << "│ " << BLUE << "Fișier: " << RESET << filePath << "\n";
            std::cout << "│ " << BLUE << "Frecvență: " << RESET << GREEN << frequency << " apariții" << RESET << "\n";

            for (const auto& doc : index.getDocuments()) {
                if (doc->getFilePath() == filePath) {
                    const auto& words = doc->getWords();
                    bool found = false;
                    for (size_t i = 0; i < words.size() && !found; ++i) {
                        if (words[i] == query) {
                            std::cout << "│ ";
                            highlightWordInText(doc->getContext(i, 3), query);
                            found = true;
                        }
                    }
                    break;
                }
            }
            std::cout << YELLOW << "└────────────────────────────────────────\n\n" << RESET;
        }
    }
    std::cout << "Apasă Enter pentru a continua...";
    std::cin.ignore(); std::cin.get();
}

void ConsoleUI::showStatistics() {
    printHeader();
    std::cout << BOLD << CYAN << " Statistici\n" << RESET;
    printSeparator();
    std::cout << "\n";
    printInfo("Total documente: " + std::to_string(index.getDocumentCount()));
    printInfo("Cuvinte indexate: " + std::to_string(index.getIndexedWordsCount()));
    if (!index.getDocuments().empty()) {
        size_t totalWords = 0, totalChars = 0;
        for (const auto& doc : index.getDocuments()) {
            totalWords += doc->getWords().size();
            totalChars += doc->getContent().size();
        }
        printInfo("Total cuvinte: " + std::to_string(totalWords));
        printInfo("Total caractere: " + std::to_string(totalChars));
    }
    std::cout << "\nApasă Enter pentru a continua...";
    std::cin.ignore(); std::cin.get();
}

void ConsoleUI::saveDatabase() {
    printHeader();
    std::cout << BOLD << CYAN << "💾 Salvare bază de date\n" << RESET;
    printSeparator();
    if (index.saveToFile(databaseFile)) printSuccess("Baza de date a fost salvată în: " + databaseFile);
    else printError("Eroare la salvarea bazei de date!");
    std::cout << "\nApasă Enter pentru a continua...";
    std::cin.ignore(); std::cin.get();
}

void ConsoleUI::loadDatabase() {
    printHeader();
    std::cout << BOLD << CYAN << "📥 Încărcare bază de date\n" << RESET;
    printSeparator();
    if (index.loadFromFile(databaseFile)) {
        printSuccess("Baza de date a fost încărcată!");
        printInfo("Documente: " + std::to_string(index.getDocumentCount()));
        printInfo("Cuvinte indexate: " + std::to_string(index.getIndexedWordsCount()));
    } else {
        printError("Nu s-a putut încărca baza de date sau fișierul nu există.");
    }
    std::cout << "\nApasă Enter pentru a continua...";
    std::cin.ignore(); std::cin.get();
}

void ConsoleUI::run() {
    int choice;
    do {
        printHeader();
        printMenu();
        std::cout << "Alege o opțiune: " << GREEN << BOLD;
        std::cin >> choice;
        std::cin.ignore();
        std::cout << RESET;

        switch (choice) {
            case 1: loadDocuments(); break;
            case 2: buildIndex(); break;
            case 3: searchDocuments(); break;
            case 4: showStatistics(); break;
            case 5: saveDatabase(); break;
            case 6: loadDatabase(); break;
            case 0: printInfo("La revedere! 👋\n"); break;
            default: printError("Opțiune invalidă!"); std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (choice != 0);
}