#pragma once
#ifndef RESTAURANT_SYSTEM_H
#define RESTAURANT_SYSTEM_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <algorithm>
using namespace std;

// ===== Logger (Singleton Pattern) =====
class Logger {
private:
    static Logger* instance;
    static mutex mtx;
    ofstream file;
    Logger();
public:
    static Logger& getInstance();
    void log(const string& msg);
    ~Logger();
};

// ===== User Type (Strategy Pattern) =====
class UserStrategy {
public:
    virtual void accessDashboard() = 0;
    virtual ~UserStrategy() = default;
};

class AdminStrategy : public UserStrategy {
public:
    void accessDashboard() override;
};

class GuestStrategy : public UserStrategy {
public:
    void accessDashboard() override;
};

class User {
    UserStrategy* strategy = nullptr;
public:
    void setStrategy(UserStrategy* s);
    void accessDashboard();
    ~User();
};

// ===== Observer Pattern =====
class IObserver {
public:
    virtual void onNotify(const string& event) = 0;
    virtual ~IObserver() = default;
};

class AnalyticsObserver : public IObserver {
public:
    void onNotify(const string& event) override;
};

class NotificationObserver : public IObserver {
public:
    void onNotify(const string& event) override;
};

class EventManager {
    vector<IObserver*> observers;
public:
    void addObserver(IObserver* o);
    void notifyAll(const string& event);
};

// ===== Restaurant Core Logic =====
struct Customer {
    string name, itemName;
    int quantity, age;
    double bill;
    Customer();
    Customer(string n, string i, int q, double b, int a);
};

class Restaurant {
    struct TakeAwayCustomer;
    struct DineInCustomer;
    struct ServedCustomer;

    vector<string> menu;
    vector<int> prices;
    TakeAwayCustomer* nextTake = nullptr;
    DineInCustomer* nextDine = nullptr;
    ServedCustomer* servedRoot = nullptr;
    double totalEarned = 0;

    ServedCustomer* insertServed(ServedCustomer* root, const Customer& c, const string& type);
    void deleteTree(ServedCustomer* r);
    int height(ServedCustomer* r);
    int balanceFactor(ServedCustomer* r);
    ServedCustomer* rotateLeft(ServedCustomer* x);
    ServedCustomer* rotateRight(ServedCustomer* y);
    ServedCustomer* balance(ServedCustomer* r);

public:
    Restaurant();
    ~Restaurant();

    const vector<string>& getMenu() const;
    const vector<int>& getPrices() const;

    void placeTakeAwayOrder(const string& name, int index, int qty, int age);
    void placeDineInOrder(const string& name, int index, int qty, int age, int table);
    Customer serveTakeAwayOrder();
    Customer serveDineInOrder();
    void serveAllOrders(const function<void(const Customer&)>& cb);
    vector<Customer> getServedOrders() const;
    double calculatePendingBill() const;
    double getTotalEarnings() const;
};

void showMenu(const vector<string>& menu, const vector<int>& prices);

#endif
