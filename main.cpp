#include <iostream>
#include <fstream>
#include <thread>

#include "include/utils.h"
#include "include/sha256.h"

using namespace std;
using namespace std::this_thread;

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;

// Prototypes
void quit();

void menu(int num) {

}

void readDB() {
	doneReading = true;
}

void loadingScr() {
	char spinner[4] = {'|', '/', '-', '\\'};
	int counter  = 0;
	cout << "Loading...  ";
	while(!doneLoading || !doneReading) {
		cout << '\b' << spinner[counter];
		counter = (counter+1) % 4;
		delay(200);
	}
}

void greet() {
	cls();
	string banner = R"(
              .----.
  .---------. | == |   ___    ___ ________  ________  _________  _______
  |.-"""""-.| |----|  |\  \  /  /|\   ____\|\   __  \|\___   ___\\  ___ \
  ||       || | == |  \ \  \/  / | \  \___|\ \  \|\  \|___ \  \_\ \   __/|
  ||       || |----|   \ \    / / \ \  \  __\ \   __  \   \ \  \ \ \  \_|/__
  |'-.....-'| |::::|    /     \/   \ \  \|\  \ \  \ \  \   \ \  \ \ \  \_|\ \
  `"")---(""` |___.|   /  /\   \    \ \_______\ \__\ \__\   \ \__\ \ \_______\
 /:::::::::::\" _  "  /__/ /\ __\    \|_______|\|__|\|__|    \|__|  \|_______|
/:::=======:::\`\`\   |__|/ \|__|
`"""""""""""""`  '-'
)";
	cout << banner << "\nSelamat datang di aplikasi xGate Warnet!\n\n";
	pause();
	menu(1);
}

void quit() {
	isQuit = true;
	cls();
	cout << "Terima kasih sudah menggunakan layanan xGate Warnet.\n\n";
	exit(0);
}

void init() {
	cls();
	showCursor(false);
	thread t1(loadingScr);
	thread t2(readDB);
	thread t3([]() { // lambda function uhuy
		delay(2500); // minimum loading time 2.5 detik
		doneLoading = true;
	});
	t1.join();
	t2.join();
	t3.join();
	showCursor(true);
	doneLoading = false; doneReading = false;
	return greet();
}

int main() {
	atexit(quit);
	signal(SIGINT, exit);
	init();

	return 0;
}
