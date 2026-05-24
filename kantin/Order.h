#pragma once
#include "MenuItem.h"
#include <vector>
#include <sstream>
#include <ctime>

enum class OrderStatus {
    PENDING,
    PROCESSING,
    READY,
    COMPLETED,
    CANCELLED
};

string statusToString(OrderStatus s) {
    switch(s) {
        case OrderStatus::PENDING:    return "PENDING";
        case OrderStatus::PROCESSING: return "PROCESSING";
        case OrderStatus::READY:      return "READY";
        case OrderStatus::COMPLETED:  return "COMPLETED";
        case OrderStatus::CANCELLED:  return "CANCELLED";
        default: return "UNKNOWN";
    }
}

// ORDER: Satu item dalam pesanan
struct OrderItem {
    int menuId;
    string menuName;
    double price;
    int quantity;
    string tenantName;

    string toJson() const {
        ostringstream oss;
        oss << "{"
            << "\"menuId\":" << menuId << ","
            << "\"menuName\":\"" << menuName << "\","
            << "\"price\":" << price << ","
            << "\"quantity\":" << quantity << ","
            << "\"tenant\":\"" << tenantName << "\","
            << "\"subtotal\":" << (price * quantity)
            << "}";
        return oss.str();
    }
};

// ORDER: Satu transaksi pesanan lengkap
// ENKAPSULASI: semua field private
class Order {
private:
    int orderId;
    string customerName;
    vector<OrderItem> items;
    OrderStatus status;
    double totalPrice;
    string timestamp;
    string clientAddr;

    static int nextId;

    string getCurrentTime() {
        time_t now = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
        return string(buf);
    }

public:
    Order(const string& customerName, const string& clientAddr)
        : orderId(nextId++), customerName(customerName),
          status(OrderStatus::PENDING), totalPrice(0), clientAddr(clientAddr) {
        timestamp = getCurrentTime();
    }

    void addItem(const OrderItem& item) {
        items.push_back(item);
        totalPrice += item.price * item.quantity;
    }

    // Getter
    int getOrderId() const { return orderId; }
    string getCustomerName() const { return customerName; }
    OrderStatus getStatus() const { return status; }
    double getTotalPrice() const { return totalPrice; }
    string getTimestamp() const { return timestamp; }
    string getClientAddr() const { return clientAddr; }
    const vector<OrderItem>& getItems() const { return items; }

    // Setter
    void setStatus(OrderStatus s) { status = s; }

    string toJson() const {
        ostringstream oss;
        oss << "{"
            << "\"orderId\":" << orderId << ","
            << "\"customer\":\"" << customerName << "\","
            << "\"status\":\"" << statusToString(status) << "\","
            << "\"total\":" << totalPrice << ","
            << "\"timestamp\":\"" << timestamp << "\","
            << "\"items\":[";
        for (int i = 0; i < (int)items.size(); i++) {
            if (i > 0) oss << ",";
            oss << items[i].toJson();
        }
        oss << "]}";
        return oss.str();
    }

    string toDisplay() const {
        ostringstream oss;
        oss << "\n=== ORDER #" << orderId << " ===\n";
        oss << "Pelanggan : " << customerName << "\n";
        oss << "Waktu     : " << timestamp << "\n";
        oss << "Status    : " << statusToString(status) << "\n";
        oss << "Items:\n";
        for (auto& item : items) {
            oss << "  - " << item.menuName
                << " x" << item.quantity
                << " = Rp" << (int)(item.price * item.quantity)
                << " (" << item.tenantName << ")\n";
        }
        oss << "TOTAL: Rp" << (int)totalPrice << "\n";
        return oss.str();
    }
};

int Order::nextId = 1001;
