#pragma once
#include "MenuItem.h"
#include <stdexcept>

// LINKED LIST MANUAL (Bonus Point)
// Kompleksitas:
//   - insert: O(1) di head, O(n) di tail
//   - search by id: O(n)
//   - delete: O(n)
class MenuLinkedList {
private:
    struct Node {
        MenuItem* item;
        Node* next;
        Node(MenuItem* item) : item(item), next(nullptr) {}
    };

    Node* head;
    int size;

public:
    MenuLinkedList() : head(nullptr), size(0) {}

    ~MenuLinkedList() {
        Node* curr = head;
        while (curr) {
            Node* next = curr->next;
            delete curr->item;
            delete curr;
            curr = next;
        }
    }

    // Insert di tail - O(n)
    void insert(MenuItem* item) {
        Node* newNode = new Node(item);
        if (!head) {
            head = newNode;
        } else {
            Node* curr = head;
            while (curr->next) curr = curr->next;
            curr->next = newNode;
        }
        size++;
    }

    // Linear Search by ID - O(n)
    // Big O: O(n) karena worst case harus cek semua node
    MenuItem* searchById(int id) const {
        Node* curr = head;
        while (curr) {
            if (curr->item->getId() == id) return curr->item;
            curr = curr->next;
        }
        return nullptr;
    }

    // Linear Search by Name (case-insensitive partial match) - O(n*m)
    // Big O: O(n*m) n=jumlah item, m=panjang string
    MenuItem* searchByName(const string& keyword) const {
        string lowerKey = keyword;
        for (char& c : lowerKey) c = tolower(c);

        Node* curr = head;
        while (curr) {
            string itemName = curr->item->getName();
            for (char& c : itemName) c = tolower(c);
            if (itemName.find(lowerKey) != string::npos) return curr->item;
            curr = curr->next;
        }
        return nullptr;
    }

    // Konversi ke array untuk sorting
    vector<MenuItem*> toVector() const {
        vector<MenuItem*> result;
        Node* curr = head;
        while (curr) {
            result.push_back(curr->item);
            curr = curr->next;
        }
        return result;
    }

    // Get all items as JSON array - O(n)
    string toJson() const {
        string result = "[";
        Node* curr = head;
        bool first = true;
        while (curr) {
            if (!first) result += ",";
            result += curr->item->toJson();
            first = false;
            curr = curr->next;
        }
        result += "]";
        return result;
    }

    int getSize() const { return size; }
    Node* getHead() const { return head; }
};
