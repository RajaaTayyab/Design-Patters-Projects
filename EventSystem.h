#pragma once

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <vector>
#include <string>
#include <iostream>


class IObserver {
public:
    virtual void onNotify(const std::string& event) = 0;
    virtual ~IObserver() = default;
};

class AnalyticsObserver : public IObserver {
public:
    void onNotify(const std::string& event) override {
        std::cout << "[Analytics] Processing event: " << event << std::endl;
    }
};

class NotificationObserver : public IObserver {
public:
    void onNotify(const std::string& event) override {
        std::cout << "[Notification] Sending notification for: " << event << std::endl;
    }
};

class EventManager {
        std::vector<IObserver*> observers;
public:
    void addObserver(IObserver* observer) {
        observers.push_back(observer);
    }

    void notifyAll(const std::string& event) {
        for (auto& observer : observers)
            observer->onNotify(event);
    }
};

#endif
