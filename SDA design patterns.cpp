#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <thread>


#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"

using namespace std;

// ===== Logger (Singleton Pattern) =====
class Logger {
private:
    static Logger* instance;
    static mutex mtx;
    ofstream file;
    Logger() { file.open("log.txt", ios::app); }
public:
    static Logger& getInstance() {
        lock_guard<mutex> lock(mtx);
        if (!instance) instance = new Logger();
        return *instance;
    }
    void log(const string& msg) { file << "[LOG] " << msg << endl; }
    ~Logger() { file.close(); }
};
Logger* Logger::instance = nullptr;
mutex Logger::mtx;

// ===== User Type (Strategy Pattern) =====
class UserStrategy {
public:
    virtual void accessDashboard() = 0;
    virtual ~UserStrategy() = default;
};

class AdminStrategy : public UserStrategy {
public:
    void accessDashboard() override { cout << "\n[Admin Dashboard] Full access granted.\n"; }
};

class GuestStrategy : public UserStrategy {
public:
    void accessDashboard() override { cout << "\n[Guest Dashboard] Menu access only.\n"; }
};

class User {
    UserStrategy* strategy = nullptr;
public:
    void setStrategy(UserStrategy* s) { delete strategy; strategy = s; }
    void accessDashboard() { if (strategy) strategy->accessDashboard(); }
    ~User() { delete strategy; }
};

// ===== Observer Pattern =====
class IObserver {
public:
    virtual void onNotify(const string& event) = 0;
    virtual ~IObserver() = default;
};

class AnalyticsObserver : public IObserver {
public:
    void onNotify(const string& event) override {
        cout << "[Analytics] " << event << endl;
    }
};

class NotificationObserver : public IObserver {
public:
    void onNotify(const string& event) override {
        cout << "[Notification] " << event << endl;
    }
};

class EventManager {
    vector<IObserver*> observers;
public:
    void addObserver(IObserver* o) { observers.push_back(o); }
    void notifyAll(const string& event) {
        for (auto o : observers) o->onNotify(event);
    }
};


// ===== Restaurant Core Logic =====
struct Customer {
    string name, itemName;
    int quantity, age;
    double bill;
    Customer() {}
    Customer(string n, string i, int q, double b, int a)
        : name(n), itemName(i), quantity(q), bill(b), age(a) {}
};

struct TakeAwayCustomer {
    Customer customer;
    TakeAwayCustomer* next = nullptr;
    TakeAwayCustomer(string n, string i, int q, double b, int a)
        : customer(n, i, q, b, a) {}
};

struct DineInCustomer {
    Customer customer;
    int table;
    DineInCustomer* next = nullptr;
    DineInCustomer(string n, string i, int q, double b, int a, int t)
        : customer(n, i, q, b, a), table(t) {}
};
struct ServedCustomer {
    string name, itemName, type;
    int quantity, age;
    double bill;
    ServedCustomer* left = nullptr;
    ServedCustomer* right = nullptr;
    ServedCustomer(string n, string i, int q, double b, int a, string t)
        : name(n), itemName(i), quantity(q), bill(b), age(a), type(t) {}
};

class Restaurant {
    vector<string> menu;
    vector<int> prices;
    TakeAwayCustomer* nextTake = nullptr;
    DineInCustomer* nextDine = nullptr;
    ServedCustomer* servedRoot = nullptr;
    double totalEarned = 0;

    // AVL Helpers
    ServedCustomer* insertServed(ServedCustomer* root, const Customer& c, const string& type) {
        if (!root) return new ServedCustomer(c.name, c.itemName, c.quantity, c.bill, c.age, type);
        if (c.name < root->name)
            root->left = insertServed(root->left, c, type);
        else if (c.name > root->name)
            root->right = insertServed(root->right, c, type);
        else throw runtime_error("Duplicate customer");
        return balance(root);
    }
    void deleteTree(ServedCustomer* r) {
        if (!r) return;
        deleteTree(r->left);
        deleteTree(r->right);
        delete r;
    }
    int height(ServedCustomer* r) {
        if (!r) return 0;
        return 1 + max(height(r->left), height(r->right));
    }
    int balanceFactor(ServedCustomer* r) {
        return height(r->left) - height(r->right);
    }
    ServedCustomer* rotateLeft(ServedCustomer* x) {
        ServedCustomer* y = x->right;
        x->right = y->left;
        y->left = x;
        return y;
    }
    ServedCustomer* rotateRight(ServedCustomer* y) {
        ServedCustomer* x = y->left;
        y->left = x->right;
        x->right = y;
        return x;
    }
    ServedCustomer* balance(ServedCustomer* r) {
        int bf = balanceFactor(r);
        if (bf > 1) {
            if (balanceFactor(r->left) < 0)
                r->left = rotateLeft(r->left);
            return rotateRight(r);
        }
        if (bf < -1) {
            if (balanceFactor(r->right) > 0)
                r->right = rotateRight(r->right);
            return rotateLeft(r);
        }
        return r;
    }

public:
    Restaurant() {
        menu = { "", "Burger", "Pasta", "Steak", "Pizza", "Lassi" };
        prices = { 0, 250, 450, 800, 600, 150 };
    }
    ~Restaurant() { deleteTree(servedRoot); }

    const vector<string>& getMenu() const { return menu; }
    const vector<int>& getPrices() const { return prices; }

    void placeTakeAwayOrder(const string& name, int index, int qty, int age) {
        double bill = prices[index] * qty;
        auto* newNode = new TakeAwayCustomer(name, menu[index], qty, bill, age);
        if (!nextTake || nextTake->customer.age < age) {
            newNode->next = nextTake;
            nextTake = newNode;
        }
        else {
            auto* cur = nextTake;
            while (cur->next && cur->next->customer.age >= age)
                cur = cur->next;
            newNode->next = cur->next;
            cur->next = newNode;
        }

        cout << "TakeAway order placed successfully for " << name << "!\n";
        Logger::getInstance().log("TakeAway order placed for " + name);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }


    void placeDineInOrder(const string& name, int index, int qty, int age, int table) {
        double bill = prices[index] * qty;
        auto* newNode = new DineInCustomer(name, menu[index], qty, bill, age, table);
        if (!nextDine) nextDine = newNode;
        else {
            auto* cur = nextDine;
            while (cur->next) cur = cur->next;
            cur->next = newNode;
        }

        cout << "DineIn order placed successfully for " << name << " at table " << table << "!\n";
        Logger::getInstance().log("DineIn order placed for " + name);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }


    Customer serveTakeAwayOrder() {
        if (!nextTake) throw runtime_error("No TakeAway orders");
        auto* temp = nextTake;
        nextTake = nextTake->next;
        Customer served = temp->customer;
        servedRoot = insertServed(servedRoot, served, "TakeAway");
        totalEarned += served.bill;
        delete temp;
        return served;
    }

    Customer serveDineInOrder() {
        if (!nextDine) throw runtime_error("No DineIn orders");
        auto* temp = nextDine;
        nextDine = nextDine->next;
        Customer served = temp->customer;
        servedRoot = insertServed(servedRoot, served, "DineIn");
        totalEarned += served.bill;
        delete temp;
        return served;
    }

    void serveAllOrders(const function<void(const Customer&)>& cb) {
        while (nextTake) cb(serveTakeAwayOrder());
        while (nextDine) cb(serveDineInOrder());
    }

    vector<Customer> getServedOrders() const {
        vector<Customer> list;
        vector<ServedCustomer*> stack;
        ServedCustomer* curr = servedRoot;
        while (curr || !stack.empty()) {
            while (curr) { stack.push_back(curr); curr = curr->left; }
            curr = stack.back(); stack.pop_back();
            list.push_back({ curr->name, curr->itemName, curr->quantity, curr->bill, curr->age });
            curr = curr->right;
        }
        return list;
    }

    double calculatePendingBill() const {
        double total = 0;
        for (auto* c = nextTake; c; c = c->next) total += c->customer.bill;
        for (auto* c = nextDine; c; c = c->next) total += c->customer.bill;
        return total;
    }

    double getTotalEarnings() const { return totalEarned; }
    TakeAwayCustomer* getNextTakeAway() const { return nextTake; }
    DineInCustomer* getNextDineIn() const { return nextDine; }

    bool searchServedOrderByName(const string& name) const {
        ServedCustomer* curr = servedRoot;
        while (curr) {
            if (curr->name == name) {
                cout << " Found: " << curr->name << " ordered " << curr->itemName
                    << " (" << curr->quantity << ") - Rs. " << curr->bill
                    << " [" << curr->type << "]\n";
                return true;
            }
            if (name < curr->name)
                curr = curr->left;
            else
                curr = curr->right;
        }
        cout << " No served order found for: " << name << endl;
        return false;
    }
};

// ===== Console UI =====
void showMenu(const vector<string>& menu, const vector<int>& prices) {
    cout << "\n===== MENU =====\n";
    for (size_t i = 1; i < menu.size(); ++i)
        cout << i << ". " << menu[i] << " - Rs. " << prices[i] << endl;
}


int main() 
{
    Logger::getInstance().log("System Started");
    Restaurant restaurant;
    EventManager manager;
    manager.addObserver(new AnalyticsObserver());
    manager.addObserver(new NotificationObserver());

    User user;
    cout << CYAN << "User Type (1 = Admin, 2 = Guest): " << RESET;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    int type; cin >> type;
    if (type == 1) user.setStrategy(new AdminStrategy());
    else user.setStrategy(new GuestStrategy());
    user.accessDashboard();

    bool run = true;
    while (run) {
        cout << CYAN << "\n==== Welcome! What would you like to do? ====\n" << RESET;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        cout << YELLOW << "1. Show Menu\n" << RESET;
        cout << YELLOW << "2. Place TakeAway Order\n" << RESET;
        cout << YELLOW << "3. Place DineIn Order\n" << RESET;
        cout << YELLOW << "4. Serve TakeAway Order\n" << RESET;
        cout << YELLOW << "5. Serve DineIn Order\n" << RESET;
        cout << YELLOW << "6. Serve All Orders\n" << RESET;
        cout << YELLOW << "7. View Served Orders\n" << RESET;
        cout << YELLOW << "8. View Pending Orders\n" << RESET;
        cout << YELLOW << "9. View Pending Bill\n" << RESET;
        cout << YELLOW << "10. View Total Earnings\n" << RESET;
        cout << YELLOW << "11. Search Served Orders by Name\n" << RESET;
        cout << RED << "12. Exit\n" << RESET;
        cout << CYAN << "Choice: " << RESET;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        int ch; cin >> ch;

        if (ch == 1) showMenu(restaurant.getMenu(), restaurant.getPrices());

        else if (ch == 2) {
            string n; int i, q, a;
            cout << CYAN << "Name: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> n;
            cout << CYAN << "Menu Index: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> i;
            cout << CYAN << "Qty: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> q;
            cout << CYAN << "Age: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> a;
            restaurant.placeTakeAwayOrder(n, i, q, a);
        }

        else if (ch == 3) {
            string n; int i, q, a, t;
            cout << CYAN << "Name: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> n;
            cout << CYAN << "Menu Index: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> i;
            cout << CYAN << "Qty: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> q;
            cout << CYAN << "Age: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> a;
            cout << CYAN << "Table: " << RESET; std::this_thread::sleep_for(std::chrono::milliseconds(1000)); cin >> t;
            restaurant.placeDineInOrder(n, i, q, a, t);
        }

        else if (ch == 4) {
            try {
                Customer c = restaurant.serveTakeAwayOrder();
                manager.notifyAll("TakeAway order served: " + c.name);
            }
            catch (const exception& e) {
                cout << RED << e.what() << RESET << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        else if (ch == 5) {
            try {
                Customer c = restaurant.serveDineInOrder();
                manager.notifyAll("DineIn order served: " + c.name);
            }
            catch (const exception& e) {
                cout << RED << e.what() << RESET << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        else if (ch == 6) {
            restaurant.serveAllOrders([&](const Customer& c) {
                manager.notifyAll("Order served: " + c.name);
                });
        }

        else if (ch == 7) {
            for (auto& c : restaurant.getServedOrders()) {
                cout << GREEN << c.name << " ordered " << c.itemName << " (" << c.quantity << ") - Rs. " << c.bill << RESET << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        else if (ch == 8) {
            cout << CYAN << "\nPending Orders:\n" << RESET;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            TakeAwayCustomer* t = restaurant.getNextTakeAway();
            while (t) {
                cout << "[TakeAway] " << t->customer.name << " - " << t->customer.itemName
                    << " (" << t->customer.quantity << ") - Rs. " << t->customer.bill << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                t = t->next;
            }

            DineInCustomer* d = restaurant.getNextDineIn();
            while (d) {
                cout << "[DineIn] " << d->customer.name << " - " << d->customer.itemName
                    << " (" << d->customer.quantity << ") - Rs. " << d->customer.bill
                    << " [Table " << d->table << "]" << endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                d = d->next;
            }
        }

        else if (ch == 9) {
            cout << YELLOW << "Pending: Rs. " << restaurant.calculatePendingBill() << RESET << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        else if (ch == 10) {
            cout << YELLOW << "Earnings: Rs. " << restaurant.getTotalEarnings() << RESET << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        else if (ch == 11) {
            string name;
            cout << CYAN << "Enter customer name to search: " << RESET;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            cin >> name;
            restaurant.searchServedOrderByName(name);
            Logger::getInstance().log("Searched for served order: " + name);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        else if (ch == 12) {
            run = false;
        }

        else {
            cout << RED << "Invalid option" << RESET << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    Logger::getInstance().log("System Closed");
    return 0;
}