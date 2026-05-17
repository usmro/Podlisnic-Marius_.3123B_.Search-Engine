#ifndef TERMINAL_H
#define TERMINAL_H

namespace Terminal {
    void enableRawMode();
    void disableRawMode();
    int readKey(); // Returnează: 1000=UP, 1001=DOWN, 1002=RIGHT, 1003=LEFT, altfel char
    void clearScreen();
    void moveCursor(int x, int y);
    void hideCursor();
    void showCursor();
    void setupSignalHandlers();
}

#endif