#include <iostream>
#include <fstream>
#include <thread>
#include <regex>
#include <cmath>
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
	string nama, username, password, level = "user";
	bool hasBilling {0};
};

struct NodeUser {
	User data;
	NodeUser *next {NULL};
};

struct ComputerTree {
	int jumlahChild = 0;
	string nama, jenis;
	bool isUsed {0};
	ComputerTree *next {NULL};
	ComputerTree *child {NULL};
};

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;
int totalUser = 0, totalRouter = 0;
const int hashMapSize = 2048;
NodeUser *hashMapUser;
User currentUser;
ComputerTree server; // initiate in init

// Prototypes
void quit();
void menu(Menu dest);
void updateUserDB();

void showPCData() {
    cls();
    cout << "==== Daftar PC ====\n\n";
    ComputerTree* temp = &server;
    while (temp != NULL) {
        cout << "Nama: " << temp->nama << "\n";
        cout << "Jenis: " << temp->jenis << "\n";
        cout << "Digunakan: " << (temp->isUsed ? "Ya" : "Tidak") << "\n";
        if (temp->child != NULL) {
            cout << "Anak PC:\n";
            ComputerTree* child = temp->child;
            while (child != NULL) {
                cout << "- " << child->nama << " (" << child->jenis << ") "
                     << (child->isUsed ? "Digunakan" : "Tidak Digunakan") << "\n";
                child = child->next;
            }
        }
        cout << "\n";
        temp = temp->next;
    }
    sysPause();
}

void treatAngka(double saldo, string* saldoStr, int* desimal) {
	int newSaldo = saldo; // reminder to always round by 2 decimal places
	*saldoStr = to_string(newSaldo); // always setw(2) << setfill('0')
	bool isNegatif = false;
	if ((*saldoStr)[0] == '-') {
		isNegatif = true;
		saldoStr->erase(0, 1);
	}
	int saldoLen = saldoStr->length();
	int jarak = 3;
	while (saldoLen > jarak) {
		saldoStr->insert(saldoLen - jarak, 1, '.');
		jarak += 4; saldoLen += 1;
	}
	if (isNegatif) {
		saldoStr->insert(0, "-");
	}
	*desimal = (double)round((saldo - newSaldo)*100);
	return;
}

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

int hashFunction(string key) {
	// let's threw a big word here: "multiplicative fibonacci hashing function"
	// it's just a simple hash function, but it sounds cool
	int sum = 0;
	double inversePhi = 0.6180339887; // fancy schmancy
	for (int i = 0; i < key.length(); i++) {
		sum += key[i]; // sum of all ascii values
	}
	int hashResult = floor(double(sum*inversePhi - floor(sum*inversePhi)) * hashMapSize);
	return hashResult;
	// note that this hashing function is not perfect,
	// but I'll encapsulate part of key in sha256, so all g~
}

string hashPass(string str) {
	string salt = "m8A*w@ok:cK#";
	return picosha2::hash256_hex_string(str+salt);
}

User userValidation(string username) {
	int hashResult = hashFunction(username + hashPass(username));
	NodeUser *temp = &hashMapUser[hashResult];
	while(temp != NULL) {
		if (temp->data.username == username) {
			return temp->data;
		}
		temp = temp->next;
	}
	User emptyUser;
	return emptyUser;
}

void daftar(string nama = "", string username = "") {
	system("cls");
	cout << "==== Daftar ====\n\n";
	cout << "Masukkan nama \t\t\t\t: ";
	if (nama.empty()) {
		nama = inputHandlerStr(NAME);
	} else {
		cout << nama;
	}
	if (nama.empty()) {
		errorHandler("Nama tidak boleh kosong!");
		return daftar();
	}
	cout << "\nMasukkan username (lowercase) \t\t: ";
	if (username.empty()) {
		username = inputHandlerStr(USERNAME);
	} else {
		cout << username;
	}
	if (username.empty()) {
		errorHandler("Username tidak boleh kosong!");
		return daftar(nama);
	}
	// username validation, check if it's already taken
	User user = userValidation(username);
	if(!user.username.empty()) {
		errorHandler("Username sudah terdaftar!");
		return daftar(nama);
	}
	cout << "\nMasukkan password \t\t\t: ";
	string pass = inputHandlerStr(PASSWORD);
	if (pass.empty()) {
		errorHandler("Password tidak boleh kosong!");
		return daftar(nama, username);
	}
	cout << "\nMasukkan ulang password \t\t: ";
	string pass2 = inputHandlerStr(PASSWORD);
	if (pass != pass2) {
		errorHandler("Password tidak sama!");
		return daftar(nama, username);
	}
	User newUser;
	newUser.nama = nama;
	newUser.username = username;
	newUser.password = hashPass(pass);
	int hashResult = hashFunction(username + hashPass(username));
	NodeUser* newNode = new NodeUser;
	newNode->data = newUser;
	if (hashMapUser[hashResult].data.username.empty()) {
		hashMapUser[hashResult] = *newNode;
	} else {
		NodeUser* temp = &hashMapUser[hashResult];
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newNode;
	}
	totalUser++;
	updateUserDB();
	cout << "\n\nPendaftaran berhasil!\n" << newUser.password << "\n";
	system("pause");
	return menu(MAIN_MENU);
}

void login(string username = "") {
	cls();
	cout << "==== Login ====\n\nUsername : ";
	if (username.empty()) {
		username = inputHandlerStr(USERNAME);
	} else {
		cout << username;
	}
	if (username.empty()) {
		errorHandler("Username tidak boleh kosong!");
		return login();
	} // whoa, what?
	// username validation
	User user = userValidation(username);
	if(user.username.empty()) {
		errorHandler("Username tidak ditemukan!");
		return login();
	}
	cout << "\nPassword : ";
	string pass = inputHandlerStr(PASSWORD);
	int userHash = hashFunction(username + hashPass(pass));
	if (user.password != hashPass(pass)) {
		errorHandler("Password salah!");
		return login(username);
	}
	// all checks out, set currentUser and proceed to menu
	currentUser = user;
	if (currentUser.level == "admin") {
		return menu(ADMIN_MENU);
	} else {
		return menu(USER_MENU);
	}
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

				case '0':
					quit();
					break;
					
				default:
					cout << "Pilihan invalid!\n";
					sysPause();
					menu(MAIN_MENU);
					break;
			}
		}
		break;

		case USER_MENU: {
			cout << "==== Selamat datang di xGate, " + currentUser.nama + " ====\n\n";
			cout << "1. Pesan PC\n2.Tambah billing\n3. Cek billing\n0. Keluar\n\nMasukkan pilihan : ";
			char pil = '\0';
			pil = inputHandler();
			switch (pil) {
				
				
				default:
					cout << "Pilihan invalid!\n";
					sysPause();
					menu(USER_MENU);
					break;
			}
		}
		break;

		case ADMIN_MENU: {
			cout << "==== Selamat datang di xGate, " + currentUser.nama + " ====\n\n";
			cout << "1. Lihat daftar PC\n0. Keluar\n\nMasukkan pilihan : ";
			char pil = '\0';
			pil = inputHandler();
			switch (pil) {
				case '1':
					showPCData();
					menu(ADMIN_MENU);
					break;

				case '0':
					quit();
					break;

				default:
					cout << "Pilihan invalid!\n";
					sysPause();
					menu(ADMIN_MENU);
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
	tulisUser << 1;
	// ADD ADMIN USER
	tulisUser << "Admin\nadmin\n" << hashPass("admin") << "\nadmin\n0";
	tulisUser.close();
	ofstream tulisPC("./data/pc.txt");
	tulisPC << 2;
	tulisPC << "\n5\n";
	for (int i = 0; i < 5; i++) {
		tulisPC << "PC1_" << i+1 << "\n";
	
	}
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
		User newUser;
		newUser.nama = nama;
		newUser.username = username;
		newUser.password = password;
		newUser.level = level;
		newUser.hasBilling = hasBilling;
		int hashResult = hashFunction(username + hashPass(username));
		NodeUser* newNode = new NodeUser;
		newNode->data = newUser;
		if (hashMapUser[hashResult].data.username.empty()) {
			hashMapUser[hashResult] = *newNode;
		} else {
			NodeUser* temp = &hashMapUser[hashResult];
			while (temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = newNode;
		}
	}
	bacaUser.close();
	ifstream bacaPC("./data/pc.txt");
	if (bacaPC.fail()) { // n-, nah, th-, this can't be
		errorHandler("Error: DB PC tidak ditemukan!");
	}
	// jumlah child -> nama -> jenis -> isUsed -> child
	bacaPC >> totalRouter;
	for (int i = 0; i < totalRouter; i++) {
		int jumlahChild;
		string nama, jenis;
		bool isUsed;
		bacaPC >> jumlahChild;
		ComputerTree* newNode = new ComputerTree;
		bacaPC.ignore(); // pesky newline
		getline(bacaPC, nama);
		bacaPC >> jenis >> isUsed;
		newNode->nama = nama;
		newNode->jenis = jenis;
		newNode->isUsed = isUsed;
		ComputerTree* temp = newNode;
		for (int j = 0; j < jumlahChild; j++) {
			ComputerTree* childNode = new ComputerTree;
			bacaPC.ignore(); // pesky newline
			getline(bacaPC, childNode->nama);
			bacaPC >> childNode->jenis >> childNode->isUsed;
			if (temp->child == NULL) {
				temp->child = childNode;
			} else {
				ComputerTree* tempChild = temp->child;
				while (tempChild->next != NULL) {
					tempChild = tempChild->next;
				}
				tempChild->next = childNode;
			}
		}
		while (server.child != NULL) {
			server.child = server.child->next;
		}
		server.child = newNode;
	}
	bacaPC.close();
	doneReading = true;
	return;
}

void updateUserDB() {

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
		// init server
		server.nama = "Main Server";
		server.jenis = "Main Server";
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
