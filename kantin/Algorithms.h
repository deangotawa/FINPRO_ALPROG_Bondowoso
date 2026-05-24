#pragma once
#include "MenuItem.h"
#include <vector>

// ALGORITMA SORTING & SEARCHING MANUAL

class Algorithms {
public:

    // QUICK SORT by Price
    // Big O:
    //   Best/Average: O(n log n)
    //   Worst case:   O(n²) jika pivot selalu min/max
    static void quickSortByPrice(vector<MenuItem*>& arr, int low, int high) {
        if (low < high) {
            int pi = partitionByPrice(arr, low, high);
            quickSortByPrice(arr, low, pi - 1);
            quickSortByPrice(arr, pi + 1, high);
        }
    }

private:
    static int partitionByPrice(vector<MenuItem*>& arr, int low, int high) {
        double pivot = arr[high]->getPrice();
        int i = low - 1;

        for (int j = low; j < high; j++) {
            if (arr[j]->getPrice() <= pivot) {
                i++;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[high]);
        return i + 1;
    }

public:
    // MERGE SORT by Name (alphabetical)
    // Big O:
    //   All cases: O(n log n) — konsisten
    //   Space:     O(n) auxiliary
    static void mergeSortByName(vector<MenuItem*>& arr, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSortByName(arr, left, mid);
            mergeSortByName(arr, mid + 1, right);
            mergeByName(arr, left, mid, right);
        }
    }

private:
    static void mergeByName(vector<MenuItem*>& arr, int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        vector<MenuItem*> L(n1), R(n2);
        for (int i = 0; i < n1; i++) L[i] = arr[left + i];
        for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            if (L[i]->getName() <= R[j]->getName()) {
                arr[k++] = L[i++];
            } else {
                arr[k++] = R[j++];
            }
        }
        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];
    }

public:
    // BINARY SEARCH by ID (array harus sorted by ID dulu)
    // Big O: O(log n)
    static int binarySearchById(vector<MenuItem*>& arr, int targetId) {
        int low = 0, high = (int)arr.size() - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (arr[mid]->getId() == targetId) return mid;
            else if (arr[mid]->getId() < targetId) low = mid + 1;
            else high = mid - 1;
        }
        return -1;  // tidak ditemukan
    }

    // LINEAR SEARCH by Name
    // Big O: O(n)
    static int linearSearchByName(vector<MenuItem*>& arr, const string& keyword) {
        string lowerKey = keyword;
        for (char& c : lowerKey) c = tolower(c);

        for (int i = 0; i < (int)arr.size(); i++) {
            string itemName = arr[i]->getName();
            for (char& c : itemName) c = tolower(c);
            if (itemName.find(lowerKey) != string::npos) return i;
        }
        return -1;
    }

    // Quick sort by ID (untuk keperluan binary search)
    static void quickSortById(vector<MenuItem*>& arr, int low, int high) {
        if (low < high) {
            int pi = partitionById(arr, low, high);
            quickSortById(arr, low, pi - 1);
            quickSortById(arr, pi + 1, high);
        }
    }

private:
    static int partitionById(vector<MenuItem*>& arr, int low, int high) {
        int pivot = arr[high]->getId();
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (arr[j]->getId() <= pivot) {
                i++;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[high]);
        return i + 1;
    }
};
