#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <cstring>

// Winsock (Windows)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// Compatibility defines
#define close(s) closesocket(s)
typedef int socklen_t;

#include "MenuItem.h"
#include "MenuLinkedList.h"
#include "Algorithms.h"
#include "Order.h"
#include "JsonHelper.h"

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 4096

// MUTEX untuk thread-safe akses data bersama
// (Bonus: Multithreading)
mutex menuMutex;
mutex orderMutex;

MenuLinkedList menuList;
vector<Order*> orderQueue;

// INISIALISASI DATA MENU
// 3 Tenant: Pak Budi, Bu Sari, Bu Dewi
void initMenu() {
    // === TENANT 1: Warung Pak Budi (Makanan Berat) ===
    menuList.insert(new Food(1,  "Nasi Goreng Spesial",  15000, 20, "Warung Pak Budi", "Porsi Normal"));
    menuList.insert(new Food(2,  "Nasi Ayam Geprek",     18000, 15, "Warung Pak Budi", "Porsi Normal"));
    menuList.insert(new Food(3,  "Nasi Rendang",         20000, 10, "Warung Pak Budi", "Porsi Normal"));
    menuList.insert(new Food(4,  "Mie Goreng Telur",     13000, 25, "Warung Pak Budi", "Porsi Normal"));
    menuList.insert(new Food(5,  "Nasi Uduk Komplit",    16000, 12, "Warung Pak Budi", "Porsi Normal"));

    // === TENANT 2: Warung Bu Sari (Makanan Ringan & Snack) ===
    menuList.insert(new Food(6,  "Gado-Gado",            14000, 18, "Warung Bu Sari",  "Porsi Normal"));
    menuList.insert(new Food(7,  "Ketoprak",             12000, 20, "Warung Bu Sari",  "Porsi Normal"));
    menuList.insert(new Food(8,  "Siomay Bandung",       13000, 30, "Warung Bu Sari",  "Porsi Kecil"));
    menuList.insert(new Food(9,  "Batagor Crispy",       12000, 25, "Warung Bu Sari",  "Porsi Kecil"));
    menuList.insert(new Food(10, "Lontong Sayur",        11000, 15, "Warung Bu Sari",  "Porsi Normal"));

    // === TENANT 3: Warung Bu Dewi (Minuman & Dessert) ===
    menuList.insert(new Drink(11, "Es Teh Manis",         5000, 50, "Warung Bu Dewi", "Large",  false));
    menuList.insert(new Drink(12, "Es Jeruk",             7000, 40, "Warung Bu Dewi", "Medium", false));
    menuList.insert(new Drink(13, "Jus Alpukat",         12000, 20, "Warung Bu Dewi", "Medium", false));
    menuList.insert(new Drink(14, "Kopi Susu Kekinian",  12000, 30, "Warung Bu Dewi", "Large",  false));
    menuList.insert(new Drink(15, "Teh Tarik Panas",      8000, 25, "Warung Bu Dewi", "Medium", true));
    menuList.insert(new Food(16, "Es Pisang Ijo",        10000, 20, "Warung Bu Dewi", "Porsi Normal"));
    menuList.insert(new Food(17, "Puding Coklat",         8000, 30, "Warung Bu Dewi", "Porsi Kecil"));

    cout << "[SERVER] Menu berhasil dimuat: " << menuList.getSize() << " item\n";
}

// KIRIM data ke client
void sendToClient(SOCKET clientSocket, const string& msg) {
    string data = msg + "\n";
    send(clientSocket, data.c_str(), data.size(), 0);
}

// HANDLE request GET_MENU
void handleGetMenu(SOCKET clientSocket) {
    lock_guard<mutex> lock(menuMutex);
    sendToClient(clientSocket, menuList.toJson());
}

// HANDLE request SEARCH_MENU
void handleSearchMenu(SOCKET clientSocket, const string& json) {
    string keyword = JsonHelper::getString(json, "keyword");
    lock_guard<mutex> lock(menuMutex);
    MenuItem* found = menuList.searchByName(keyword);
    if (found) {
        string resp = "{\"found\":true,\"item\":" + found->toJson() + "}";
        sendToClient(clientSocket, resp);
    } else {
        sendToClient(clientSocket, "{\"found\":false}");
    }
}

// HANDLE request SORT_MENU
void handleSortMenu(SOCKET clientSocket, const string& json) {
    string sortBy = JsonHelper::getString(json, "sortBy");
    lock_guard<mutex> lock(menuMutex);

    vector<MenuItem*> items = menuList.toVector();

    if (sortBy == "price") {
        Algorithms::quickSortByPrice(items, 0, (int)items.size() - 1);
    } else {
        Algorithms::mergeSortByName(items, 0, (int)items.size() - 1);
    }

    string result = "[";
    for (int i = 0; i < (int)items.size(); i++) {
        if (i > 0) result += ",";
        result += items[i]->toJson();
    }
    result += "]";
    sendToClient(clientSocket, result);
}

// HANDLE request ORDER
void handleOrder(SOCKET clientSocket, const string& json, const string& clientAddr) {
    string customer = JsonHelper::getString(json, "customer");
    if (customer.empty()) customer = "Pelanggan";

    Order* order = new Order(customer, clientAddr);

    // Parse items array
    vector<string> itemsJson = JsonHelper::getArray(json, "items");

    bool valid = true;
    {
        lock_guard<mutex> lock(menuMutex);
        for (auto& itemJson : itemsJson) {
            int menuId = JsonHelper::getInt(itemJson, "menuId");
            int qty    = JsonHelper::getInt(itemJson, "quantity");
            if (qty <= 0) qty = 1;

            MenuItem* menu = menuList.searchById(menuId);
            if (!menu) {
                sendToClient(clientSocket,
                    "{\"status\":\"ERROR\",\"message\":\"Menu ID " +
                    to_string(menuId) + " tidak ditemukan\"}");
                valid = false;
                break;
            }
            if (!menu->isAvailable()) {
                sendToClient(clientSocket,
                    "{\"status\":\"ERROR\",\"message\":\"" +
                    menu->getName() + " sudah habis\"}");
                valid = false;
                break;
            }

            OrderItem oi;
            oi.menuId    = menuId;
            oi.menuName  = menu->getName();
            oi.price     = menu->getPrice();
            oi.quantity  = qty;
            oi.tenantName = menu->getTenantName();
            order->addItem(oi);

            for (int i = 0; i < qty; i++) menu->decreaseStock();
        }
    }

    if (!valid) {
        delete order;
        return;
    }

    {
        lock_guard<mutex> lock(orderMutex);
        orderQueue.push_back(order);
    }

    string resp = "{\"status\":\"SUCCESS\","
                  "\"message\":\"Pesanan diterima!\","
                  "\"order\":" + order->toJson() + "}";
    sendToClient(clientSocket, resp);

    cout << "[SERVER] Pesanan baru dari " << clientAddr << ":\n";
    cout << order->toDisplay();
}

// HANDLE request GET_ORDERS (untuk kasir)
void handleGetOrders(SOCKET clientSocket) {
    lock_guard<mutex> lock(orderMutex);
    string result = "[";
    bool first = true;
    for (auto* o : orderQueue) {
        if (o->getStatus() == OrderStatus::PENDING ||
            o->getStatus() == OrderStatus::PROCESSING) {
            if (!first) result += ",";
            result += o->toJson();
            first = false;
        }
    }
    result += "]";
    sendToClient(clientSocket, result);
}

// HANDLE request UPDATE_STATUS
void handleUpdateStatus(SOCKET clientSocket, const string& json) {
    int orderId    = JsonHelper::getInt(json, "orderId");
    string status  = JsonHelper::getString(json, "status");

    lock_guard<mutex> lock(orderMutex);
    for (auto* o : orderQueue) {
        if (o->getOrderId() == orderId) {
            if      (status == "PROCESSING") o->setStatus(OrderStatus::PROCESSING);
            else if (status == "READY")      o->setStatus(OrderStatus::READY);
            else if (status == "COMPLETED")  o->setStatus(OrderStatus::COMPLETED);
            else if (status == "CANCELLED")  o->setStatus(OrderStatus::CANCELLED);

            sendToClient(clientSocket,
                "{\"status\":\"SUCCESS\",\"message\":\"Order #" +
                to_string(orderId) + " diupdate ke " + status + "\"}");
            return;
        }
    }
    sendToClient(clientSocket, "{\"status\":\"ERROR\",\"message\":\"Order tidak ditemukan\"}");
}

// HANDLE request CEK_STATUS (untuk pembeli)
void handleCekStatus(SOCKET clientSocket, const string& json) {
    int orderId = JsonHelper::getInt(json, "orderId");

    lock_guard<mutex> lock(orderMutex);
    for (auto* o : orderQueue) {
        if (o->getOrderId() == orderId) {
            ostringstream oss;
            oss << "{"
                << "\"status\":\"SUCCESS\","
                << "\"orderId\":" << orderId << ","
                << "\"orderStatus\":\"" << statusToString(o->getStatus()) << "\","
                << "\"customer\":\"" << o->getCustomerName() << "\","
                << "\"total\":" << o->getTotalPrice() << ","
                << "\"timestamp\":\"" << o->getTimestamp() << "\""
                << "}";
            sendToClient(clientSocket, oss.str());
            return;
        }
    }
    sendToClient(clientSocket, "{\"status\":\"ERROR\",\"message\":\"Order tidak ditemukan\"}");
}

// HANDLE CLIENT (per-thread)
// Bonus: Multithreading — setiap client dapat thread sendiri
void handleClient(SOCKET clientSocket, string clientAddr) {
    cout << "[SERVER] Client terhubung: " << clientAddr << "\n";

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) break;

        string json(buffer);
        // Trim newline
        while (!json.empty() && (json.back() == '\n' || json.back() == '\r'))
            json.pop_back();

        cout << "[SERVER] Terima dari " << clientAddr << ": " << json.substr(0, 80) << "\n";

        string type = JsonHelper::getType(json);

        if      (type == "GET_MENU")      handleGetMenu(clientSocket);
        else if (type == "SEARCH_MENU")   handleSearchMenu(clientSocket, json);
        else if (type == "SORT_MENU")     handleSortMenu(clientSocket, json);
        else if (type == "ORDER")         handleOrder(clientSocket, json, clientAddr);
        else if (type == "GET_ORDERS")    handleGetOrders(clientSocket);
        else if (type == "UPDATE_STATUS") handleUpdateStatus(clientSocket, json);
        else if (type == "CEK_STATUS")    handleCekStatus(clientSocket, json);
        else if (type == "DISCONNECT")    break;
        else {
            sendToClient(clientSocket, "{\"status\":\"ERROR\",\"message\":\"Unknown command\"}");
        }
    }

    cout << "[SERVER] Client disconnected: " << clientAddr << "\n";
    closesocket(clientSocket);
}

// MAIN SERVER
int main() {
    // Inisialisasi Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] WSAStartup gagal\n";
        return 1;
    }

    initMenu();

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "[ERROR] Gagal membuat socket\n";
        return 1;
    }

    // Reuse address
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "[ERROR] Bind gagal\n";
        return 1;
    }

    listen(serverSocket, 10);
    cout << "==============================================\n";
    cout << " SERVER KANTIN UI - Port " << PORT << "\n";
    cout << " Multithreaded | JSON | OOP | Linked List\n";
    cout << "==============================================\n";
    cout << "[SERVER] Menunggu koneksi client...\n\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) continue;

        string clientIP = string(inet_ntoa(clientAddr.sin_addr)) +
                          ":" + to_string(ntohs(clientAddr.sin_port));

        // Spawn thread baru untuk setiap client (Multithreading Bonus)
        thread(handleClient, clientSocket, clientIP).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
