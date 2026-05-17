#include "ConsoleUI.h"
#include <clocale> 
#include <iostream>
int main() {
    std::setlocale(LC_ALL, "");
    ConsoleUI ui;
    ui.run();
    return 0;
}