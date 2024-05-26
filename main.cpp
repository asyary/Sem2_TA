#include <iostream>
#include <fstream>
#include <thread>
#include <regex>
#include <csignal>

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

enum InputType {
	NAME, // name can contain anything, A-Za-z0-9 and \s whitespace
	USERNAME, // username can only contain A-Za-z0-9
	PASSWORD, // password can contain anything, ^[\x20-\x7E]+$
};

// Structs
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
const int hashMapSize = 2048;
NodeUser *hashMapUser;
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

string inputHandlerStr(InputType type) {
	char ch = '\0';
	string str = "";
	while(ch != 13) {
		ch = inputHandler();
		string tempStr;
		tempStr.append(1, ch); // what?
		if (ch == 13) {
			continue; // some failsafe redundancy
		}
		if (ch == 8) { // backspace
			if (str.length() == 0) {
				continue;
			}
			cout << "\b \b"; // ooga booga way to delete a char on screen
			if (!str.empty()) {
				str.pop_back();
			}
			continue;
		} else if (type == USERNAME) {
			if (!regex_match(tempStr, regex("^[a-z0-9]{1}$"))) {
				continue;
			}
			if (str.length() < 20) {
				cout << ch;
				str += ch;
			}
		} else if (type == PASSWORD) {
			if (!regex_match(tempStr, regex("^[\x20-\x7E]{1}$"))) {
				continue;
			}
			if (str.length() < 20) {
				cout << "*";
				str += ch;
			}
		} else if (type == NAME) {
			if (!regex_match(tempStr, regex("^[A-Za-z\\s']{1}$"))) {
				continue;
			}
			if (str.length() < 50) { // idk, 50 seems right
				cout << ch;
				str += ch;
			}
		}
	}
	return str;
}

void errorHandler(string err) {
	if (isQuit) {
		return;
	}
	cls();
	cout << "==== ERROR ====\n\n" + err + "\n\n";
	sysPause();
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
					sysPause();
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
	sysPause();
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
		// init hashmap
		hashMapUser = new NodeUser[hashMapSize];
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
