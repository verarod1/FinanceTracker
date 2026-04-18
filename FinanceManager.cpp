#include "FinanceManager.h"

void FinanceManager::addTransaction(double amount, std::string category, bool isIncome, std::string date) {
    Transaction t;
    t.amount = amount;
    t.category = category;
    t.isIncome = isIncome;
    t.date = date;
    transactions.push_back(t);
}

std::vector<Transaction> FinanceManager::getTransactions() {
    return transactions;
}

void FinanceManager::saveToFile() {
    std::ofstream file("data.txt");
    for (int i = 0; i < transactions.size(); i++) {
        file << transactions[i].amount << " "
            << transactions[i].category << " "
            << transactions[i].isIncome << " "
            << transactions[i].date << "\n";
    }
    file.close();
}

void FinanceManager::loadFromFile() {
    std::ifstream file("data.txt");
    if (!file.is_open()) {
        return;
    }
    transactions.clear();
    Transaction t;
    while (file >> t.amount >> t.category >> t.isIncome >> t.date) {
        transactions.push_back(t);
    }
    file.close();
}

void FinanceManager::removeTransaction(int index) {
    if (index >= 0 && index < transactions.size()) {
        transactions.erase(transactions.begin() + index);
        saveToFile();
    }
}

double FinanceManager::getTotalIncome(int month) {
    double total = 0.0;
    for (int i = 0; i < transactions.size(); i++) {
        int tMonth = std::stoi(transactions[i].date.substr(3, 2));
        if (transactions[i].isIncome == true && tMonth == month) {
            total = total + transactions[i].amount;
        }
    }
    return total;
}

double FinanceManager::getTotalExpense(int month) {
    double total = 0.0;
    for (int i = 0; i < transactions.size(); i++) {
        int tMonth = std::stoi(transactions[i].date.substr(3, 2));
        if (transactions[i].isIncome == false && tMonth == month) {
            total = total + transactions[i].amount;
        }
    }
    return total;
}

std::map<std::string, double> FinanceManager::getExpensesByCategory(int month) {
    std::map<std::string, double> summary;
    for (int i = 0; i < transactions.size(); i++) {
        int tMonth = std::stoi(transactions[i].date.substr(3, 2));
        if (transactions[i].isIncome == false && tMonth == month) {
            summary[transactions[i].category] += transactions[i].amount;
        }
    }
    return summary;
}

std::map<std::string, double> FinanceManager::getIncomeByCategory(int month) {
    std::map<std::string, double> summary;
    for (int i = 0; i < transactions.size(); i++) {
        int tMonth = std::stoi(transactions[i].date.substr(3, 2));
        if (transactions[i].isIncome == true && tMonth == month) {
            summary[transactions[i].category] += transactions[i].amount;
        }
    }
    return summary;
}