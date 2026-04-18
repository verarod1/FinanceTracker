#ifndef FINANCEAPP_H
#define FINANCEAPP_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QProgressBar>
#include "FinanceManager.h"

class FinanceApp : public QMainWindow {
    Q_OBJECT

public:
    FinanceApp(QWidget* parent = nullptr);
    ~FinanceApp();

private slots:
    void onAddButtonClicked();
    void onTypeChanged(const QString& text);
    void onDeleteButtonClicked();
    void onPrevMonthClicked();
    void onNextMonthClicked();

private:
    FinanceManager manager;
    int currentMonth;

    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QHBoxLayout* inputLayout;
    QHBoxLayout* navLayout;

    QLabel* monthNameLabel;
    QPushButton* prevMonthButton;
    QPushButton* nextMonthButton;

    QLineEdit* amountEdit;
    QComboBox* typeBox;
    QComboBox* categoryBox;
    QLineEdit* dateEdit;

    QTableWidget* table;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QLabel* summaryLabel;

    QWidget* statsContainer;
    QVBoxLayout* statsLayout;

    void refreshDisplay();
    void updateSummary();
};

#endif