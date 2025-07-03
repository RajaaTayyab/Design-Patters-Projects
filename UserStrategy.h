#pragma once
#include <iostream>
// UserStrategy.h
class UserStrategy {
public:
    virtual void accessDashboard() = 0;
    virtual ~UserStrategy() {}
};

class AdminStrategy : public UserStrategy {
public:
    void accessDashboard() override {
        cout << "Admin Dashboard: View all reports and configurations.\n";
    }
};

class GuestStrategy : public UserStrategy {
public:
    void accessDashboard() override {
        cout << "Guest Dashboard: View menu only.\n";
    }
};

// User.h
class User {
    unique_ptr<UserStrategy> strategy;
public:
    void setStrategy(UserStrategy* newStrategy) {
        strategy.reset(newStrategy);
    }
    void accessDashboard() {
        if (strategy) strategy->accessDashboard();
    }
};
