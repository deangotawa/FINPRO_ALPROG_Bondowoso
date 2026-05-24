#pragma once
#include <string>
#include <sstream>
using namespace std;

// ABSTRAKSI: Abstract base class MenuItem
class MenuItem {
protected:
    int id;
    string name;
    double price;
    int stock;
    string tenantName;

public:
    MenuItem(int id, const string& name, double price, int stock, const string& tenantName)
        : id(id), name(name), price(price), stock(stock), tenantName(tenantName) {}

    virtual ~MenuItem() {}

    // ABSTRAKSI: Pure virtual method → wajib diimplementasi subclass
    virtual string getCategory() const = 0;
    virtual string getDescription() const = 0;  // POLIMORFISME

    // ENKAPSULASI: Getter & Setter
    int getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    string getTenantName() const { return tenantName; }

    void setPrice(double p) { price = p; }
    void setStock(int s) { stock = s; }

    bool isAvailable() const { return stock > 0; }

    void decreaseStock() {
        if (stock > 0) stock--;
    }

    string toJson() const {
        ostringstream oss;
        oss << "{"
            << "\"id\":" << id << ","
            << "\"name\":\"" << name << "\","
            << "\"price\":" << price << ","
            << "\"stock\":" << stock << ","
            << "\"tenant\":\"" << tenantName << "\","
            << "\"category\":\"" << getCategory() << "\","
            << "\"available\":" << (isAvailable() ? "true" : "false")
            << "}";
        return oss.str();
    }

    string toDisplay() const {
        ostringstream oss;
        oss << "[" << id << "] " << name
            << " - Rp" << (int)price
            << " (" << tenantName << ")"
            << (isAvailable() ? "" : " [HABIS]");
        return oss.str();
    }
};

// PEWARISAN: Food extends MenuItem
class Food : public MenuItem {
private:
    string portion;  // "Porsi Kecil", "Porsi Normal", "Porsi Besar"

public:
    Food(int id, const string& name, double price, int stock,
         const string& tenantName, const string& portion = "Porsi Normal")
        : MenuItem(id, name, price, stock, tenantName), portion(portion) {}

    string getCategory() const override { return "Makanan"; }

    // POLIMORFISME: Override getDescription
    string getDescription() const override {
        return name + " | " + portion + " | Rp" + to_string((int)price);
    }

    string getPortion() const { return portion; }
};

// PEWARISAN: Drink extends MenuItem
class Drink : public MenuItem {
private:
    string size;  // "Small", "Medium", "Large"
    bool isHot;

public:
    Drink(int id, const string& name, double price, int stock,
          const string& tenantName, const string& size = "Medium", bool isHot = false)
        : MenuItem(id, name, price, stock, tenantName), size(size), isHot(isHot) {}

    string getCategory() const override { return "Minuman"; }

    // POLIMORFISME: Override getDescription
    string getDescription() const override {
        string temp = isHot ? "Panas" : "Dingin";
        return name + " | " + size + " | " + temp + " | Rp" + to_string((int)price);
    }

    string getSize() const { return size; }
    bool getIsHot() const { return isHot; }
};
