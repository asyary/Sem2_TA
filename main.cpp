// IF THE PROGRAM CRASHES, PLEASE RE-RUN THE PROGRAM 3-4 TIMES
// I (we) have absolutely, and I mean, devastatingly mind-blowingly
// no idea why the program SOMETIMES crashes on first-several runs UNPREDICTABLY
#include <iostream>
#include <fstream>
#include <thread>
#include <regex>
#include <cmath>
#include <csignal>
#include <iomanip>

#include "include/utils.h"
#include "include/sha256.h"

using namespace std;
using namespace std::this_thread;

// We tried to make it cross-platform, but it's somehow not working on Linux (but it works on WSL???)

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

struct NodeQueue {
	string nama;
	NodeQueue *next {NULL};
} *headQueue, *tailQueue;

struct ComputerTree {
	int jumlahChild = 0;
	string nama, jenis;
	bool isUsed {0};
	ComputerTree *next {NULL};
	ComputerTree *child {NULL};
};

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;
int totalUser = 0, totalRouter = 0, totalQueue = 0;
const int hashMapSize = 2048;
NodeUser *hashMapUser;
User currentUser;
ComputerTree* server = new ComputerTree; // initiate in init

// Prototypes
void quit();
void menu(Menu dest);
void updateUserDB();
void updatePCDB();
char inputHandler();
string inputHandlerStr(InputType type);

void showPCData() {
    cls();
    cout << "==== Daftar PC ====\n\n";
    ComputerTree* temp = server->child;
    while (temp != NULL) {
        cout << "Nama: " << temp->nama << "\n";
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
	 return menu(ADMIN_MENU);
}

void deleteQueue() {
	ofstream tulisQueue("./data/queue.txt", ios::trunc);
	NodeQueue* delQ = headQueue;
	headQueue = headQueue->next;
	delete delQ;
	totalQueue--;
	tulisQueue << totalQueue << "\n";
	NodeQueue* temp = headQueue;
	while (temp != NULL) {
		tulisQueue << temp->nama << "\n";
		temp = temp->next;
	}
	tulisQueue.close();
}

void konfirmasiBilling() {
	cls();
	cout << "==== Konfirmasi Billing ====\n\n";
	if (headQueue == NULL || tailQueue == NULL) {
		cout << "Tidak ada antrian!\n";
		sysPause();
		return menu(ADMIN_MENU);
	}
	cout << "Antrian:\n";
	NodeQueue* temp = headQueue;
	cout << temp->nama + "\n";
	cout << "Konfirmasi pembayaran? (y/n) : ";
	char konfirmasi = inputHandler();
	if (konfirmasi != 'y' && konfirmasi != 'Y') {
		return menu(ADMIN_MENU);
	}
	cout << "\n\nPembayaran " + temp->nama + " berhasil dikonfirmasi!\n";
	deleteQueue();
	sysPause();
	return konfirmasiBilling();
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

void addQueue(ComputerTree* pc) {
	ofstream tulisQueue("./data/queue.txt", ios::trunc);
	totalQueue++;
	tulisQueue << totalQueue << "\n";
	if (headQueue == NULL || tailQueue == NULL) {
		headQueue = new NodeQueue;
		headQueue->nama = pc->nama;
		tulisQueue << headQueue->nama << "\n";
		tailQueue = headQueue;
	} else {
		NodeQueue* newNode = new NodeQueue;
		newNode->nama = pc->nama;
		NodeQueue* temp = headQueue;
		while (temp->next != tailQueue) {
			tulisQueue << temp->nama << "\n";
			temp = temp->next;
		}
		tulisQueue << temp->nama << "\n";
		tailQueue->next = newNode;
		tailQueue = newNode;
		tulisQueue << newNode->nama << "\n";
	}
}

void pesanPC() {
	cls();
	cout << "==== Pesan PC ====\n\n";
	if (currentUser.hasBilling) {
		cout << "Anda sudah memesan PC!\n";
		sysPause();
		return menu(USER_MENU);
	}
	ComputerTree* temp = server->child;
	int i = 1;
	while (temp != NULL) {
		ComputerTree* pcTemp = temp->child;
		while (pcTemp != NULL) {
			if (!pcTemp->isUsed) {
				cout << to_string(i) + ". " + pcTemp->nama + "\n";
				i++;
			}
			pcTemp = pcTemp->next;
		}
		temp = temp->next;
	}
	cout << "0. Kembali\n\nMasukkan pilihan : ";
	int pil = 0;
	cin >> pil;
	ComputerTree* PCdipesan = new ComputerTree;
	if (pil == 0) {
		return menu(USER_MENU);
	} else if (pil <= i || pil > 0) {
		// find the PC
		ComputerTree* currentRouter = server->child;
		ComputerTree* temp = server->child;
		while (temp != NULL) {
			ComputerTree* pcTemp = temp->child;
			while (pcTemp != NULL) {
				if (!pcTemp->isUsed) {
					pil--;
					if (pil == 0) {
						PCdipesan = pcTemp;
						break;
					}
				}
				pcTemp = pcTemp->next;
			}
			if (pil <= 0) {
				break;
			}
			temp = temp->next;
		}
	}
	cls();
	cout << "==== Pesan PC ====\n\n";
	cout << "PC yang dipilih: " + PCdipesan->nama + "\n\n";
	cout << "Berapa jam? : ";
	int jam;
	cin >> jam;
	double harga = 3000 * jam;
	string hargaStr;
	int desimal;
	treatAngka(harga, &hargaStr, &desimal);
	cout << "\nTotal harga: Rp " + hargaStr + "," << setw(2) << setfill('0') << desimal << "\n\n";
	cout << "Konfirmasi pembayaran? (y/n) : ";
	char konfirmasi = inputHandler();
	if (konfirmasi != 'y' && konfirmasi != 'Y') {
		return menu(USER_MENU);
	}
	PCdipesan->isUsed = true;
	addQueue(PCdipesan);
	currentUser.hasBilling = true;
	updatePCDB();
	updateUserDB();
	cout << "\n\nPC " + PCdipesan->nama + " berhasil dipesan!\nSilakan membayar di kasir.\n\n";
	sysPause();
	return menu(USER_MENU);
}

void addPC() {
	cls();
	cout << "==== Tambah PC ====\n\n";
	cout << "Masukkan jumlah PC yang diinginkan: ";
	int jumlah, jumlahTmp, jumlahRouter, maxPC, jumlahPC;
	cin >> jumlah;
	jumlahTmp = jumlah;
	jumlahRouter = server->jumlahChild;
	maxPC = 10 * jumlahRouter;
	ComputerTree* tmp = server->child;
	while (tmp != NULL) {
		jumlahPC += tmp->jumlahChild;
		tmp = tmp->next;
	}
	if ((jumlahPC + jumlah) > maxPC) {
		errorHandler("PC terlalu banyak!");
		return menu(ADMIN_MENU);
	} else {
		ComputerTree* currentRouter = server->child;
		int currentRouterInt = 1;
		while (jumlah > 0) {
			if (currentRouter->jumlahChild == 10) {
				currentRouter = currentRouter->next;
				currentRouterInt++;
				continue;
			}
			ComputerTree* newPC = new ComputerTree;
			newPC->nama = "PC" + to_string(currentRouterInt) + "_" + to_string(currentRouter->jumlahChild + 1);
			newPC->jenis = "PC";
			newPC->isUsed = false;
			if (currentRouter->child == NULL) {
				currentRouter->child = newPC;
			} else {
				ComputerTree* temp = currentRouter->child;
				while (temp->next != NULL) {
					temp = temp->next;
				}
				temp->next = newPC;
			}
			currentRouter->jumlahChild++;
			jumlah--;
		}
	}
	updatePCDB();
	cout << "\n" + to_string(jumlahTmp) + " PC berhasil ditambahkan!\n";
	sysPause();
	return menu(ADMIN_MENU);
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
	cls();
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
	cout << "\n\nPendaftaran berhasil!\n";
	sysPause();
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
			cout << "1. Pesan PC\n2. Tampilkan PC\n0. Keluar\n\nMasukkan pilihan : ";
			char pil = '\0';
			pil = inputHandler();
			switch (pil) {
				case '1':
					pesanPC();
					break;
				
				case '2':
					showPCData();
					break;

				case '0':
					quit();
					break;
				
				default:
					cout << "Pilihan invalid!\n";
					sysPause();
					menu(USER_MENU);
					break;
			}
		}
		break;

		case ADMIN_MENU: {
			if (currentUser.level != "admin") {
				return menu(USER_MENU);
			}
			cout << "==== Selamat datang di xGate, " + currentUser.nama + " ====\n\n";
			cout << "1. Lihat daftar PC\n2. Tambah PC\n3. Konfirmasi billing\n0. Keluar\n\nMasukkan pilihan : ";
			char pil = '\0';
			pil = inputHandler();
			switch (pil) {
				case '1':
					showPCData();
					break;

				case '2':
					addPC();
					break;

				case '3':
					konfirmasiBilling();
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

void readDB() {
	ifstream bacaUser("./data/user.txt");
	if (bacaUser.fail()) {
		errorHandler("Gagal membaca data user!");
		return quit();
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
	int i = 1, jumlahRouter = 0;
	while (true) {
		ifstream bacaPC("./data/server/router" + to_string(i) + "/pc.txt");
		if (bacaPC.fail()) {
			break;
		}
		jumlahRouter++;
		ComputerTree* newRouter = new ComputerTree;
		bacaPC >> newRouter->jumlahChild;
		newRouter->nama = "Router" + to_string(i);
		newRouter->jenis = "Router";
		for (int j = 0; j < newRouter->jumlahChild; j++) {
			ComputerTree* newPC = new ComputerTree;
			newPC->nama = "PC" + to_string(i) + "_" + to_string(j+1);
			newPC->jenis = "PC";
			bacaPC >> newPC->isUsed;
			if (newRouter->child == NULL) {
				newRouter->child = newPC;
			} else {
				ComputerTree* temp = newRouter->child;
				while (temp->next != NULL) {
					temp = temp->next;
				}
				temp->next = newPC;
			}
		}
		if (server->child == NULL) {
			server->child = newRouter;
		} else {
			ComputerTree* temp = server->child;
			while (temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = newRouter;
		}
		server->jumlahChild = jumlahRouter;
		bacaPC.close();
		i++;
	}
	// read queue
	ifstream bacaQueue("./data/queue.txt");
	if (bacaQueue.fail()) {
		errorHandler("Gagal membaca data antrian!");
		return quit();
	}
	bacaQueue >> totalQueue;
	string nama;
	for (int i = 0; i < totalQueue; i++) {
		bacaQueue >> nama;
		NodeQueue* newNode = new NodeQueue;
		newNode->nama = nama;
		if (headQueue == NULL || tailQueue == NULL) {
			headQueue = newNode;
			tailQueue = headQueue;
		} else {
			tailQueue->next = newNode;
			tailQueue = newNode;
		}
	}
	doneReading = true;
	return;
}

void updatePCDB() {
	ComputerTree* temp = server->child;
	int i = 1;
	while (temp != NULL) {
		ofstream tulisPC("./data/server/router" + to_string(i) + "/pc.txt", ios::trunc);
		tulisPC << temp->jumlahChild << "\n";
		ComputerTree* child = temp->child;
		while (child != NULL) {
			tulisPC << child->isUsed << "\n";
			child = child->next;
		}
		tulisPC.close();
		temp = temp->next;
		i++;
	}
}

void updateUserDB() {
	ofstream tulisUser("./data/user.txt", ios::trunc);
	tulisUser << totalUser << "\n";
	for (int i = 0; i < hashMapSize; i++) {
		NodeUser* temp = &hashMapUser[i];
		while (temp != NULL) {
			if (!temp->data.username.empty()) {
				tulisUser << temp->data.nama << "\n" << temp->data.username << "\n" << temp->data.password << "\n" << temp->data.level << "\n" << temp->data.hasBilling << "\n";
			}
			temp = temp->next;
		}
	}
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
		server->nama = "Main Server";
		server->jenis = "Main Server";
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
