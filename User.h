#pragma once
#ifndef USER_H
#define USER_H

#include <iostream>
using namespace std;

class UserStrategy {
public:
    virtual void accessDashboard() = 0;
    virtual ~UserStrategy() = default;
};

class AdminStrategy : public UserStrategy {
public:
    void accessDashboard() override {
        cout << "\nWelcome, Admin. You have full access." << endl;
    }
};

class GuestStrategy : public UserStrategy {
public:
    void accessDashboard() override {
        cout << "\nWelcome, Guest. You can only view the menu" << endl;
    }
};

class User {
    UserStrategy* strategy = nullptr;
public:
    void setStrategy(UserStrategy* s) {
        delete strategy;
        strategy = s;
    }
    void accessDashboard() {
        if (strategy)
            strategy->accessDashboard();
    }
    ~User() {
        delete strategy;
    }
};

#endif
