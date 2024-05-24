#ifndef UTILS_H
#define UTILS_H

// Fungsi delay (crossplatform)
#ifdef _WIN32 // Memeriksa apakah sistem operasi yang digunakan adalah Windows
#include <windows.h> 
void delay(int milliseconds) {
    Sleep(milliseconds);
}
#else
#include <unistd.h>
void delay(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

// Fungsi getch (crossplatform)
#ifdef _WIN32 // Memeriksa apakah sistem operasi yang digunakan adalah Windows
#include <conio.h>
char getChar() { // getch overloads function di conio.h soalnya
	 return _getch(); // getch is deprecated, use _getch instead
}
#else
#include <termios.h>
#include <unistd.h>

char getChar() {
    char buf = 0;
    struct termios old;
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}
#endif

// Fungsi ClearScreen (crossplatform)
#ifdef _WIN32 // Memeriksa apakah sistem operasi yang digunakan adalah Windows
#include <cstdlib>
void cls() { // Fungsi untuk membersihkan layar konsol
    system("cls");
}
#else
#include <cstdio>
void cls() {
    system("clear");
}
#endif

// Fungsi menampilkan/menyembunyikan kursor (crossplatform)
void showCursor(bool show) {
	#if defined(_WIN32)
		static const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cci;
		GetConsoleCursorInfo(handle, &cci);
		cci.bVisible = show; // show/hide cursor
		SetConsoleCursorInfo(handle, &cci);
	#elif defined(__linux__)
		cout << (show ? "\033[?25h" : "\033[?25l"); // show/hide cursor
	#endif // Windows/Linux
}

#endif // UTILS_H