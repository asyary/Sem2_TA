#include <iostream>
#include <fstream>

#include "include/utils.h"
#include "include/sha256.h"

using namespace std;

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;

// Prototypes
void quit();

void quit() {
	isQuit = true;
	system("cls");
	cout << "Terima kasih sudah menggunakan layanan xGate Warnet.\n\n";
	exit(0);
}

void init() {
	cls();
}

int main() {
	atexit(quit);
	signal(SIGINT, exit);
	init();

	return 0;
}
