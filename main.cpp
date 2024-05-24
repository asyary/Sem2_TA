#include <iostream>
using namespace std;

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;

// Prototypes
void quit();

void quit() {
	isQuit = true;
	system("cls");
	cout << "Terima kasih sudah menggunakan layanan xGate Warnet.\n\n"; // @keboooooo ingpokan nama warnet yang terkenal+pendek (edit dikit biar ga sama)
	exit(0);
}

void init() {

}

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
#else
#include <termios.h>
#include <unistd.h>

char getch() {
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

// Fungsi clearscreen (crossplatform)
#ifdef _WIN32 // Memeriksa apakah sistem operasi yang digunakan adalah Windows
#include <cstdlib>
void clearScreen() { // Fungsi untuk membersihkan layar konsol
    system("cls");
}
#else
#include <cstdio>
void clearScreen() {
    system("clear");
}
#endif


int main() {
	atexit(quit);
	signal(SIGINT, exit);
	init();

	return 0;
}
