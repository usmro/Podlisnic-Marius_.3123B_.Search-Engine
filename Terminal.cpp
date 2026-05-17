#include "Terminal.h"
#include <termios.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <cstdlib>

static struct termios orig_termios;
static bool raw_mode_enabled = false;

void Terminal::disableRawMode() {
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_enabled = false;
        Terminal::showCursor();
    }
}

void Terminal::enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(Terminal::disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_enabled = true;
    Terminal::hideCursor();
}

int Terminal::readKey() {
    char c;
    if (read(STDIN_FILENO, &c, 1) <= 0) return -1;
    if (c == 27) { // ESC
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return 27;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return 27;
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 1000; // UP
                case 'B': return 1001; // DOWN
                case 'C': return 1002; // RIGHT
                case 'D': return 1003; // LEFT
            }
        }
        return 27;
    }
    return c;
}

void Terminal::clearScreen() { std::cout << "\033[2J\033[H"; }
void Terminal::moveCursor(int x, int y) { std::cout << "\033[" << y << ";" << x << "H"; }
void Terminal::hideCursor() { std::cout << "\033[?25l"; }
void Terminal::showCursor() { std::cout << "\033[?25h"; }

void signalHandler(int signum) {
    Terminal::disableRawMode();
    Terminal::clearScreen();
    Terminal::showCursor();
    std::cout << "\n⚡ Exiting gracefully...\n";
    exit(signum);
}

void Terminal::setupSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}