#ifndef FINANCEMANAGER_H
#define FINANCEMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <map>

struct Transaction {
    double amount = 0.0;
    std::string category;
    bool isIncome = false;
    std::string date;
};

class FinanceManager {
private:
    std::vector<Transaction> transactions;

public:
    void addTransaction(double amount, std::string category, bool isIncome, std::string date);
    std::vector<Transaction> getTransactions();

    void saveToFile();
    void loadFromFile();

    void removeTransaction(int index);

    double getTotalIncome(int month);
    double getTotalExpense(int month);

    std::map<std::string, double> getExpensesByCategory(int month);
    std::map<std::string, double> getIncomeByCategory(int month);
};

#endif