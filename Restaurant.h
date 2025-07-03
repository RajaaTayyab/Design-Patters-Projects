#pragma once
#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <vector>
#include <string>
#include <functional>
using namespace std;

struct Customer {
    string name;
    string itemName;
    int quantity;
    double bill;
    int age;

    Customer() {}
    Customer(string n, string item, int q, double b, int a)
        : name(n), itemName(item), quantity(q), bill(b), age(a) {}
};

struct TakeAwayCustomer {
    Customer customer;
    TakeAwayCustomer* next = nullptr;

    TakeAwayCustomer(string n, string item, int q, double b, int a)
        : customer(n, item, q, b, a) {}
};

struct DineInCustomer {
    Customer customer;
    int tableNumber;
    DineInCustomer* next = nullptr;

    DineInCustomer(string n, string item, int q, double b, int a, int table)
        : customer(n, item, q, b, a), tableNumber(table) {}
};

struct ServedCustomer {
    string name;
    string itemName;
    int quantity;
    double bill;
    int age;
    string type;
    ServedCustomer* left = nullptr;
    ServedCustomer* right = nullptr;

    ServedCustomer(string n, string item, int q, double b, int a, string t)
        : name(n), itemName(item), quantity(q), bill(b), age(a), type(t) {}
};

class Restaurant {
private:
    vector<string> menu;
    vector<int> prices;
    TakeAwayCustomer* nextTakeAway = nullptr;
    DineInCustomer* nextDineIn = nullptr;
    ServedCustomer* servedRoot = nullptr;
    double totalEarnings = 0.0;

    ServedCustomer* insertServed(ServedCustomer* root, const Customer& c, const string& type);
    void deleteTree(ServedCustomer* root);
    int height(ServedCustomer* root);
    int balanceFactor(ServedCustomer* root);
    ServedCustomer* rotateLeft(ServedCustomer* root);
    ServedCustomer* rotateRight(ServedCustomer* root);
    ServedCustomer* balance(ServedCustomer* root);

public:
    Restaurant();
    ~Restaurant();

    void initializeMenu();
    const vector<string>& getMenu() const;
    const vector<int>& getPrices() const;

    void placeTakeAwayOrder(const string& name, int menuIndex, int quantity, int age);
    void placeDineInOrder(const string& name, int menuIndex, int quantity, int age, int tableNumber);
    Customer serveTakeAwayOrder();
    Customer serveDineInOrder();
    void serveAllOrders(const function<void(const Customer&)>& callback);

    vector<Customer> getServedOrders() const;
    double calculatePendingBill() const;
    double getTotalEarnings() const;
    TakeAwayCustomer* getNextTakeAway() const { return nextTakeAway; }
    DineInCustomer* getNextDineIn() const { return nextDineIn; }
    bool searchServedOrderByName(const string& name) const;

};

#endif