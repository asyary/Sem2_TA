#include <iostream>
#include <fstream>
#include <thread>

#include "include/utils.h"
#include "include/sha256.h"

using namespace std;
using namespace std::this_thread;

// Enums
enum Menu {
	MAIN_MENU,
	ADMIN_MENU,
	USER_MENU
};

struct User {
	string nama, username, password, level;
	bool hasBilling;
};

struct NodeUser {
	User data;
	NodeUser *next {NULL};
};

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;
int totalUser = 0;
User currentUser;

// Prototypes
void quit();

char inputHandler() {
	char pil = getChar();
	if (pil == 3 || pil == 4 || pil == 24 || pil == 26) {
		quit(); // Handles CTRL+C, CTRL+D, CTRL+X, CTRL+Z
	}
	return pil;
}

void errorHandler(string err) {
	if (isQuit) {
		return;
	}
	cls();
	cout << "==== ERROR ====\n\n" + err + "\n\n";
	pause();
}

string hashPass(string str) {
	string salt = "m8A*w@ok:cK#";
	return picosha2::hash256_hex_string(str+salt);
}

void daftar() {
	cls();
	cout << "==== Daftar ====\n\n";
	cout << "Masukkan nama \t\t\t\t: ";
}

void login() {
	cls();
}

void menu(Menu dest) {
	cls();
	switch (dest) {
		case MAIN_MENU: {
			string mainMenu = "==== Selamat Datang ====\n\n1. Login\n2. Daftar\n0. Keluar\n\n";
			cout << mainMenu << "Masukkan pilihan : ";
			char pil = inputHandler();
			switch (pil) {
				case '1':
					login();
					break;

				case '2':
					daftar();
					break;
					
				default:
					cout << "Pilihan invalid!\n";
					pause();
					menu(MAIN_MENU);
					break;
			}
		}
		break;
	}
}

void createDB() {
	// This function should ONLY be callled once
	mkdir("data");
	ofstream tulisUser("./data/user.txt");
	tulisUser << 0;
	tulisUser.close();
	doneReading = true;
	return;
}

void readDB() {
	ifstream bacaUser("./data/user.txt");
	if (bacaUser.fail()) {
		createDB();
		return;
	}
	bacaUser >> totalUser;
	// nama -> username -> password -> level -> hasBilling
	for (int i = 0; i < totalUser; i++) {
		string nama, username, password, level;
		bool hasBilling;
		bacaUser.ignore(); // pesky newline
		getline(bacaUser, nama);
		bacaUser >> username >> password >> level >> hasBilling;
	}
	bacaUser.close();
	// ADD MORE STUFF
	doneReading = true;
	return;
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
	menu(MAIN_MENU);
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
