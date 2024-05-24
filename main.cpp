#include <iostream>
using namespace std;

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

// Fungsi ClearScreen (crossplatform)
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

// Global variables
bool isQuit = false, doneLoading = false, doneReading = false;

// Konstanta password admin
const string ADMIN_PASSWORD = "Admin123";

// Prototypes
void quit();

void quit() {
	isQuit = true;
	clearScreen();
	cout << "Terima kasih sudah menggunakan layanan xGate Warnet.\n\n"; // @keboooooo ingpokan nama warnet yang terkenal+pendek (edit dikit biar ga sama)
	exit(0);
}

void init() {

}

// Fungsi untuk menampilkan menu utama
void showMainMenu() {
    clearScreen();
    cout << "Selamat datang di xGate Warnet" << endl;
    cout << "1. Admin" << endl;
    cout << "2. User" << endl;
    cout << "0. Keluar" << endl;
    cout << "Pilihan: ";
}

// Fungsi untuk input password admin
bool authenticateAdmin() {
    string password;
    int attempts = 0;
    
    while (attempts < 3) {
        cout << "Masukkan password admin: ";
        cin >> password;
        
        if (password == ADMIN_PASSWORD) {
            return true;
        } else {
            attempts++;
            cout << "Password salah. Sisa percobaan: " << 3 - attempts << endl;
        }
    }
    
    cout << "NOT AUTHENTICATED. PLS GTFO!!" << endl;
    exit(0);
}

// Fungsi untuk menampilkan menu admin
void showAdminMenu() {
    clearScreen();
    cout << "Menu Admin" << endl;
    // serah apa aja :v
}

// Fungsi untuk menampilkan menu user
void showUserMenu() {
    clearScreen();
    cout << "Menu User" << endl;
    // serah apa aja :v
}









int main() {
	//atexit(quit);
	//signal(SIGINT, exit);
	//init();
    int choice;

    while (!isQuit) {
        showMainMenu();
        choice=getch();

        if (choice == '0') { // Periksa apakah input adalah '0'
            quit();
            continue; // Lanjutkan ke iterasi loop berikutnya
        }

        if (choice >= '1' && choice <= '8') { // Memeriksa apakah input berupa angka 1-8
            choice = choice - '0'; // Konversi dari nilai ASCII ke nilai numerik
        }
        cout << choice << endl;
        
        switch (choice) {
            case 1:
                if (authenticateAdmin()) {
                    showAdminMenu();
                }
                break;
            case 2:
                showUserMenu();
                break;
            case 0:
                quit();
                break;
            default:
                cout << "Pilihan tidak valid." << endl;
                break;
        }
    }
    
    return 0;
}
