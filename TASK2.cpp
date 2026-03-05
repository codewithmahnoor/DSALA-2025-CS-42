#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;

const unsigned int PERM_WITHDRAW = 1;
const unsigned int PERM_DEPOSIT  = 2;
const unsigned int PERM_TRANSFER = 4;
const unsigned int PERM_VIP      = 8;

const unsigned int TYPE_DEPOSIT  = 1;
const unsigned int TYPE_WITHDRAW = 2;
const unsigned int TYPE_TRANSFER = 3;

class Account {
protected:
    int accountId;
    string name;
    double balance;
    unsigned int permissions;
    vector<double> transactions;
    vector<unsigned int> compressedTransactions;

    void addCompressedTransaction(unsigned int type, double amount) {
        unsigned int intAmount = static_cast<unsigned int>(amount);
        unsigned int compressed = (type << 28) | (intAmount & 0x0FFFFFFF);
        compressedTransactions.push_back(compressed);
    }

public:
    Account(int id, string n, double bal, unsigned int perm) 
        : accountId(id), name(n), balance(bal), permissions(perm) {}

    virtual ~Account() {}

    int getId() const { return accountId; }
    string getName() const { return name; }
    double getBalance() const { return balance; }
    virtual string getAccountType() const = 0;

    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;

    virtual void saveToFile(ofstream& outFile) {
        outFile << "ACCOUNT " << getAccountType() << "\n";
        outFile << accountId << " " << name << " " << balance << " " << permissions << "\n";
        outFile << "TRANSACTIONS\n";
        for (double t : transactions) {
            outFile << t << "\n";
        }
        outFile << "END_TRANSACTIONS\n";
    }

    void displayAccount() const {
        cout << "\n--- Account Details ---\n";
        cout << "ID: " << accountId << " | Name: " << name << " | Type: " << getAccountType() << "\n";
        cout << "Balance: $" << balance << "\n";
        cout << "Permissions Flag: " << permissions << "\n";
        cout << "Transaction History:\n";
        if (transactions.empty()) cout << "  None\n";
        for (double t : transactions) {
            cout << "  " << (t > 0 ? "+" : "") << t << "\n";
        }
    }

    void loadTransactionDirectly(double amount) {
        transactions.push_back(amount);
    }
};

class SavingsAccount : public Account {
public:
    SavingsAccount(int id, string n, double bal, unsigned int perm) 
        : Account(id, n, bal, perm) {}

    string getAccountType() const override { return "Savings"; }

    void deposit(double amount) override {
        if (!(permissions & PERM_DEPOSIT)) {
            cout << "[Error] Deposit permission denied for this account.\n";
            return;
        }
        balance += amount;
        transactions.push_back(amount);
        addCompressedTransaction(TYPE_DEPOSIT, amount);
        cout << "[Success] Deposited $" << amount << "\n";
    }

    void withdraw(double amount) override {
        if (!(permissions & PERM_WITHDRAW)) {
            cout << "[Error] Withdraw permission denied for this account.\n";
            return;
        }
        if (balance - amount < 0) {
            cout << "[Error] Insufficient funds.\n";
            return;
        }
        balance -= amount;
        transactions.push_back(-amount);
        addCompressedTransaction(TYPE_WITHDRAW, amount);
        cout << "[Success] Withdrew $" << amount << "\n";
    }
};

class CurrentAccount : public Account {
public:
    CurrentAccount(int id, string n, double bal, unsigned int perm) 
        : Account(id, n, bal, perm) {}

    string getAccountType() const override { return "Current"; }

    void deposit(double amount) override {
        if (!(permissions & PERM_DEPOSIT)) {
            cout << "[Error] Deposit permission denied.\n";
            return;
        }
        balance += amount;
        transactions.push_back(amount);
        addCompressedTransaction(TYPE_DEPOSIT, amount);
        cout << "[Success] Deposited $" << amount << "\n";
    }

    void withdraw(double amount) override {
        if (!(permissions & PERM_WITHDRAW)) {
            cout << "[Error] Withdraw permission denied.\n";
            return;
        }
        if (balance - amount < 0) {
            cout << "[Error] Insufficient funds.\n";
            return;
        }
        balance -= amount;
        transactions.push_back(-amount);
        addCompressedTransaction(TYPE_WITHDRAW, amount);
        cout << "[Success] Withdrew $" << amount << "\n";
    }
};

class BankSystem {
private:
    vector<Account*> accounts;
    string filename = "bank_data.txt";

    Account* findAccount(int id) {
        for (Account* acc : accounts) {
            if (acc->getId() == id) return acc;
        }
        return nullptr;
    }

public:
    ~BankSystem() {
        for (Account* acc : accounts) {
            delete acc;
        }
        accounts.clear();
    }

    void createAccount() {
        int id, typeChoice;
        string name;
        double bal;
        unsigned int perms = 0;

        cout << "Enter Account ID: "; cin >> id;
        if (findAccount(id) != nullptr) {
            cout << "Account ID already exists!\n";
            return;
        }

        cout << "Enter Name: "; cin >> name;
        cout << "Enter Initial Balance: "; cin >> bal;
        
        cout << "Account Type (1 for Savings, 2 for Current): "; cin >> typeChoice;

        perms = PERM_DEPOSIT | PERM_WITHDRAW | PERM_TRANSFER;

        Account* newAcc = nullptr;
        if (typeChoice == 1) {
            newAcc = new SavingsAccount(id, name, bal, perms);
        } else {
            newAcc = new CurrentAccount(id, name, bal, perms);
        }

        accounts.push_back(newAcc);
        cout << "[Success] Account created.\n";
    }

    void performTransaction(bool isDeposit) {
        int id;
        double amount;
        cout << "Enter Account ID: "; cin >> id;
        
        Account* acc = findAccount(id);
        if (!acc) {
            cout << "[Error] Account not found.\n";
            return;
        }

        cout << "Enter Amount: "; cin >> amount;
        
        if (isDeposit) acc->deposit(amount);
        else acc->withdraw(amount);
    }

    void showAccount() {
        int id;
        cout << "Enter Account ID: "; cin >> id;
        Account* acc = findAccount(id);
        if (acc) acc->displayAccount();
        else cout << "[Error] Account not found.\n";
    }

    void saveToFile() {
        ofstream outFile(filename);
        if (!outFile) {
            cout << "[Error] Could not open file for saving.\n";
            return;
        }
        for (Account* acc : accounts) {
            acc->saveToFile(outFile);
        }
        outFile.close();
        cout << "[Success] All accounts saved to " << filename << "\n";
    }

    void loadFromFile() {
        ifstream inFile(filename);
        if (!inFile) {
            cout << "[Error] Could not open file for reading.\n";
            return;
        }

        for (Account* acc : accounts) delete acc;
        accounts.clear();

        string marker, accType, name;
        int id;
        double balance, transAmt;
        unsigned int perms;

        while (inFile >> marker >> accType) {
            if (marker != "ACCOUNT") break;

            inFile >> id >> name >> balance >> perms;

            Account* newAcc = nullptr;
            if (accType == "Savings") newAcc = new SavingsAccount(id, name, balance, perms);
            else if (accType == "Current") newAcc = new CurrentAccount(id, name, balance, perms);

            inFile >> marker;
            while (inFile >> marker && marker != "END_TRANSACTIONS") {
                transAmt = stod(marker);
                newAcc->loadTransactionDirectly(transAmt);
            }
            accounts.push_back(newAcc);
        }
        inFile.close();
        cout << "[Success] Data loaded from " << filename << "\n";
    }

    void showMonthlySummary() {
        double monthlyTotals[12] = {0}; 
        
        for (Account* acc : accounts) {
            monthlyTotals[0] += acc->getBalance(); 
        }

        cout << "\n--- Monthly Summary (Simulated for Month 1) ---\n";
        cout << "Total Bank Holdings for Month 1: $" << monthlyTotals[0] << "\n";
    }
};

int main() {
    BankSystem bank;
    int choice;

    while (true) {
        cout << "\n===============================\n";
        cout << "   SECURE BANKING SYSTEM\n";
        cout << "===============================\n";
        cout << "1. Create Account\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Show Account\n";
        cout << "5. Save to File\n";
        cout << "6. Load from File\n";
        cout << "7. Show Monthly Summary (Arrays)\n";
        cout << "8. Exit\n";
        cout << "Select an option: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Exiting.\n";
            break;
        }

        switch (choice) {
            case 1: bank.createAccount(); break;
            case 2: bank.performTransaction(true); break;
            case 3: bank.performTransaction(false); break;
            case 4: bank.showAccount(); break;
            case 5: bank.saveToFile(); break;
            case 6: bank.loadFromFile(); break;
            case 7: bank.showMonthlySummary(); break;
            case 8: 
                cout << "Exiting system safely. Destructors will clean up memory.\n"; 
                return 0;
            default: cout << "Invalid choice. Try again.\n";
        }
    }
    return 0;
}