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

int main() {
	atexit(quit);
	signal(SIGINT, exit);
	init();

	return 0;
}
