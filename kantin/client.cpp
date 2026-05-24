#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

// Winsock (Windows)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "JsonHelper.h"
#include "MenuItem.h"

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 65536

SOCKET clientSocket;
string customerName;
string userRole;       // "PEMBELI" atau "KASIR"
int lastOrderId = -1;  // simpan order ID terakhir pembeli

// KIRIM request ke server
void sendRequest(const string& json) {
    string data = json + "\n";
    send(clientSocket, data.c_str(), data.size(), 0);
}

// TERIMA response dari server
string receiveResponse() {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    return string(buffer);
}

// TAMPILKAN MENU dari JSON response
void displayMenuFromJson(const string& json) {
    cout << "\n============================================\n";
    cout << "              DAFTAR MENU KANTIN\n";
    cout << "============================================\n";

    size_t pos = 0;
    int count = 0;
    string lastTenant = "";

    while ((pos = json.find("{\"id\":", pos)) != string::npos) {
        size_t end = json.find("}", pos);
        if (end == string::npos) break;
        string item = json.substr(pos, end - pos + 1);

        int id        = JsonHelper::getInt(item, "id");
        string name   = JsonHelper::getString(item, "name");
        double price  = JsonHelper::getDouble(item, "price");
        string tenant = JsonHelper::getString(item, "tenant");
        string cat    = JsonHelper::getString(item, "category");
        bool avail    = (item.find("\"available\":true") != string::npos);

        if (tenant != lastTenant) {
            cout << "\n--- " << tenant << " ---\n";
            lastTenant = tenant;
        }

        cout << "  [" << id << "] " << name
             << " - Rp" << (int)price
             << " (" << cat << ")"
             << (avail ? "" : " [HABIS]") << "\n";

        pos = end + 1;
        count++;
    }
    cout << "\nTotal: " << count << " item\n";
    cout << "============================================\n";
}

void showMenu() {
    sendRequest(JsonHelper::buildRequestJson("GET_MENU"));
    string resp = receiveResponse();
    displayMenuFromJson(resp);
}

void searchMenu() {
    cout << "Masukkan kata kunci pencarian: ";
    string keyword;
    cin.ignore();
    getline(cin, keyword);

    string json = "{\"type\":\"SEARCH_MENU\",\"keyword\":\"" + keyword + "\"}";
    sendRequest(json);
    string resp = receiveResponse();

    if (resp.find("\"found\":true") != string::npos) {
        cout << "\nHasil pencarian:\n";
        size_t pos = resp.find("\"item\":");
        if (pos != string::npos) {
            string item = resp.substr(pos + 7);
            int id       = JsonHelper::getInt(item, "id");
            string name  = JsonHelper::getString(item, "name");
            double price = JsonHelper::getDouble(item, "price");
            string tenant= JsonHelper::getString(item, "tenant");
            cout << "  [" << id << "] " << name
                 << " - Rp" << (int)price
                 << " (" << tenant << ")\n";
        }
    } else {
        cout << "Menu '" << keyword << "' tidak ditemukan.\n";
    }
}

void sortMenu() {
    cout << "Urutkan berdasarkan:\n";
    cout << "  1. Harga (Quick Sort)\n";
    cout << "  2. Nama  (Merge Sort)\n";
    cout << "Pilih: ";
    int choice;
    cin >> choice;

    string sortBy = (choice == 1) ? "price" : "name";
    string json = "{\"type\":\"SORT_MENU\",\"sortBy\":\"" + sortBy + "\"}";
    sendRequest(json);
    string resp = receiveResponse();
    displayMenuFromJson(resp);
}

// PEMBELI: Buat pesanan
void makeOrder() {
    showMenu();

    vector<pair<int,int>> items;
    cout << "\nMasukkan pesanan Anda (ketik 0 untuk selesai):\n";

    while (true) {
        cout << "ID Menu (0=selesai): ";
        int menuId;
        cin >> menuId;
        if (menuId == 0) break;

        cout << "Jumlah: ";
        int qty;
        cin >> qty;
        if (qty <= 0) qty = 1;

        items.push_back({menuId, qty});
        cout << "Ditambahkan ke keranjang.\n";
    }

    if (items.empty()) {
        cout << "Pesanan kosong.\n";
        return;
    }

    string json = JsonHelper::buildOrderJson(customerName, items);
    sendRequest(json);
    string resp = receiveResponse();

    string status  = JsonHelper::getString(resp, "status");
    string message = JsonHelper::getString(resp, "message");

    if (status == "SUCCESS") {
        // Ambil orderId dari dalam nested "order"
        size_t orderPos = resp.find("\"order\":");
        string orderJson = (orderPos != string::npos) ? resp.substr(orderPos + 8) : resp;

        int orderId   = JsonHelper::getInt(orderJson, "orderId");
        double total  = JsonHelper::getDouble(orderJson, "total");
        string ts     = JsonHelper::getString(orderJson, "timestamp");

        lastOrderId = orderId;

        cout << "\n================================================\n";
        cout << "  PESANAN BERHASIL DITERIMA!\n";
        cout << "================================================\n";
        cout << "  Order ID   : #" << orderId << "\n";
        cout << "  Nama       : " << customerName << "\n";
        cout << "  Waktu      : " << ts << "\n";
        cout << "  Total      : Rp" << (int)total << "\n";
        cout << "  Status     : PENDING\n";
        cout << "------------------------------------------------\n";
        cout << "  Gunakan menu 'Cek Status Pesanan' untuk\n";
        cout << "  memantau pesanan Anda.\n";
        cout << "================================================\n";
    } else {
        cout << "\n[GAGAL] " << message << "\n";
    }
}

// PEMBELI: Cek status pesanan sendiri
void cekStatusPesanan() {
    if (lastOrderId == -1) {
        cout << "\nAnda belum memiliki pesanan aktif.\n";
        return;
    }

    string json = "{\"type\":\"CEK_STATUS\",\"orderId\":" + to_string(lastOrderId) + "}";
    sendRequest(json);
    string resp = receiveResponse();

    string status = JsonHelper::getString(resp, "status");
    if (status == "ERROR") {
        cout << "\n" << JsonHelper::getString(resp, "message") << "\n";
        return;
    }

    string orderStatus = JsonHelper::getString(resp, "orderStatus");
    double total       = JsonHelper::getDouble(resp, "total");
    string customer    = JsonHelper::getString(resp, "customer");
    string ts          = JsonHelper::getString(resp, "timestamp");

    cout << "\n================================================\n";
    cout << "  STATUS PESANAN #" << lastOrderId << "\n";
    cout << "================================================\n";
    cout << "  Nama    : " << customer << "\n";
    cout << "  Waktu   : " << ts << "\n";
    cout << "  Total   : Rp" << (int)total << "\n";
    cout << "  Status  : " << orderStatus << "\n";

    if      (orderStatus == "PENDING")    cout << "  >> Pesanan menunggu diproses kasir\n";
    else if (orderStatus == "PROCESSING") cout << "  >> Pesanan sedang dimasak\n";
    else if (orderStatus == "READY")      cout << "  >> Pesanan SIAP diambil!\n";
    else if (orderStatus == "COMPLETED")  cout << "  >> Pesanan selesai\n";
    else if (orderStatus == "CANCELLED")  cout << "  >> Pesanan dibatalkan\n";

    cout << "================================================\n";
}

// KASIR: Lihat & update antrian pesanan
void kasirMode() {
    while (true) {
        cout << "\n=== PANEL KASIR ===\n";
        cout << "1. Lihat antrian pesanan\n";
        cout << "2. Update status pesanan\n";
        cout << "3. Kembali\n";
        cout << "Pilih: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            sendRequest(JsonHelper::buildRequestJson("GET_ORDERS"));
            string resp = receiveResponse();

            cout << "\n=== ANTRIAN PESANAN AKTIF ===\n";
            if (resp == "[]" || resp.find("{") == string::npos) {
                cout << "Tidak ada pesanan aktif.\n";
                continue;
            }

            size_t pos = 0;
            bool found = false;
            while ((pos = resp.find("\"orderId\":", pos)) != string::npos) {
                size_t end = resp.find("\"items\"", pos);
                if (end == string::npos) break;
                int depth = 0;
                size_t e2 = end;
                while (e2 < resp.size()) {
                    if (resp[e2] == '{' || resp[e2] == '[') depth++;
                    else if (resp[e2] == '}' || resp[e2] == ']') {
                        depth--;
                        if (depth < 0) break;
                    }
                    e2++;
                }
                string order  = resp.substr(pos - 1, e2 - pos + 2);
                int oid       = JsonHelper::getInt(order, "orderId");
                string cust   = JsonHelper::getString(order, "customer");
                string status = JsonHelper::getString(order, "status");
                double total  = JsonHelper::getDouble(order, "total");
                string time   = JsonHelper::getString(order, "timestamp");

                cout << "  #" << oid
                     << " | " << cust
                     << " | Rp" << (int)total
                     << " | " << status
                     << " | " << time << "\n";
                found = true;
                pos = e2;
            }
            if (!found) cout << "Tidak ada pesanan aktif.\n";

        } else if (choice == 2) {
            cout << "Order ID yang ingin diupdate: #";
            int orderId;
            cin >> orderId;
            cout << "Status baru:\n";
            cout << "  1. PROCESSING (sedang dimasak)\n";
            cout << "  2. READY      (siap diambil)\n";
            cout << "  3. COMPLETED  (selesai)\n";
            cout << "  4. CANCELLED  (dibatalkan)\n";
            cout << "Pilih: ";
            int s; cin >> s;
            string newStatus;
            if      (s == 1) newStatus = "PROCESSING";
            else if (s == 2) newStatus = "READY";
            else if (s == 3) newStatus = "COMPLETED";
            else if (s == 4) newStatus = "CANCELLED";
            else { cout << "Pilihan tidak valid.\n"; continue; }

            string json = "{\"type\":\"UPDATE_STATUS\","
                          "\"orderId\":" + to_string(orderId) + ","
                          "\"status\":\"" + newStatus + "\"}";
            sendRequest(json);
            string resp = receiveResponse();
            cout << JsonHelper::getString(resp, "message") << "\n";

        } else break;
    }
}

// MAIN
int main(int argc, char* argv[]) {
    string serverIP = "127.0.0.1";
    if (argc > 1) serverIP = argv[1];

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] WSAStartup gagal\n";
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "[ERROR] Gagal membuat socket\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(PORT);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "[ERROR] Gagal terhubung ke server " << serverIP << ":" << PORT << "\n";
        return 1;
    }

    cout << "==============================================\n";
    cout << "      APLIKASI PEMESANAN KANTIN UI\n";
    cout << "      Server: " << serverIP << ":" << PORT << "\n";
    cout << "==============================================\n";

    // Pilih role
    cout << "Login sebagai:\n";
    cout << "  1. Pembeli\n";
    cout << "  2. Kasir\n";
    cout << "Pilih: ";
    int roleChoice; cin >> roleChoice;
    userRole = (roleChoice == 2) ? "KASIR" : "PEMBELI";

    if (userRole == "KASIR") {
        // Kasir pakai password sederhana
        cout << "Password kasir: ";
        string pass; cin >> pass;
        if (pass != "kasir123") {
            cout << "Password salah! Akses ditolak.\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        customerName = "Kasir";
        cout << "\nSelamat datang, Kasir!\n";
    } else {
        cout << "Masukkan nama Anda: ";
        cin.ignore();
        getline(cin, customerName);
        if (customerName.empty()) customerName = "Mahasiswa";
        cout << "\nSelamat datang, " << customerName << "!\n";
    }

    // Loop menu sesuai role
    while (true) {
        if (userRole == "PEMBELI") {
            cout << "\n=== MENU PEMBELI ===\n";
            cout << "1. Lihat semua menu\n";
            cout << "2. Cari menu\n";
            cout << "3. Urutkan menu\n";
            cout << "4. Buat pesanan\n";
            cout << "5. Cek status pesanan saya\n";
            cout << "6. Keluar\n";
            cout << "Pilih: ";
            int choice; cin >> choice;
            switch (choice) {
                case 1: showMenu();          break;
                case 2: searchMenu();        break;
                case 3: sortMenu();          break;
                case 4: makeOrder();         break;
                case 5: cekStatusPesanan();  break;
                case 6:
                    sendRequest(JsonHelper::buildRequestJson("DISCONNECT"));
                    closesocket(clientSocket);
                    WSACleanup();
                    cout << "Terima kasih! Sampai jumpa.\n";
                    return 0;
                default: cout << "Pilihan tidak valid.\n";
            }
        } else {
            // KASIR langsung masuk panel kasir
            kasirMode();
            sendRequest(JsonHelper::buildRequestJson("DISCONNECT"));
            closesocket(clientSocket);
            WSACleanup();
            cout << "Sesi kasir selesai.\n";
            return 0;
        }
    }
}
