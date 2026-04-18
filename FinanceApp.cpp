#include "FinanceApp.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QString>
#include <QMessageBox>
#include <QDate>

FinanceApp::FinanceApp(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Finance Tracker");
    resize(600, 850);

    currentMonth = QDate::currentDate().month();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    navLayout = new QHBoxLayout();
    prevMonthButton = new QPushButton("<", this);
    nextMonthButton = new QPushButton(">", this);
    monthNameLabel = new QLabel("", this);
    monthNameLabel->setAlignment(Qt::AlignCenter);
    monthNameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");

    navLayout->addWidget(prevMonthButton);
    navLayout->addWidget(monthNameLabel);
    navLayout->addWidget(nextMonthButton);

    inputLayout = new QHBoxLayout();
    amountEdit = new QLineEdit(this);
    amountEdit->setPlaceholderText("Сумма");
    typeBox = new QComboBox(this);
    typeBox->addItems({ "Расход", "Доход" });
    categoryBox = new QComboBox(this);
    categoryBox->addItems({ "Еда", "Транспорт", "Жилье", "Обязательства", "Развлечения", "Бытовые", "Здоровье", "Другое" });
    dateEdit = new QLineEdit(this);
    dateEdit->setPlaceholderText("Дата (ДД.ММ)");

    inputLayout->addWidget(amountEdit);
    inputLayout->addWidget(typeBox);
    inputLayout->addWidget(categoryBox);
    inputLayout->addWidget(dateEdit);

    table = new QTableWidget(0, 4, this);
    table->setHorizontalHeaderLabels({ "Сумма", "Тип", "Категория", "Дата" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    addButton = new QPushButton("Добавить запись", this);
    deleteButton = new QPushButton("Удалить выбранное", this);

    summaryLabel = new QLabel("", this);
    statsContainer = new QWidget(this);
    statsLayout = new QVBoxLayout(statsContainer);

    mainLayout->addLayout(navLayout);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(addButton);
    mainLayout->addWidget(deleteButton);
    mainLayout->addWidget(table);
    mainLayout->addWidget(summaryLabel);
    mainLayout->addWidget(statsContainer);

    connect(addButton, &QPushButton::clicked, this, &FinanceApp::onAddButtonClicked);
    connect(deleteButton, &QPushButton::clicked, this, &FinanceApp::onDeleteButtonClicked);
    connect(typeBox, &QComboBox::currentTextChanged, this, &FinanceApp::onTypeChanged);
    connect(prevMonthButton, &QPushButton::clicked, this, &FinanceApp::onPrevMonthClicked);
    connect(nextMonthButton, &QPushButton::clicked, this, &FinanceApp::onNextMonthClicked);

    connect(amountEdit, &QLineEdit::returnPressed, this, &FinanceApp::onAddButtonClicked);
    connect(dateEdit, &QLineEdit::returnPressed, this, &FinanceApp::onAddButtonClicked);

    manager.loadFromFile();
    refreshDisplay();
}

FinanceApp::~FinanceApp() {
}

void FinanceApp::refreshDisplay() {
    QStringList months = { "", "Январь", "Февраль", "Март", "Апрель", "Май", "Июнь", "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь" };
    monthNameLabel->setText(months[currentMonth]);

    table->setRowCount(0);
    std::vector<Transaction> history = manager.getTransactions();

    for (int i = 0; i < history.size(); i++) {
        if (history[i].date.length() < 5) continue;

        int tMonth = std::stoi(history[i].date.substr(3, 2));

        if (tMonth == currentMonth) {
            int row = table->rowCount();
            table->insertRow(row);

            table->setItem(row, 0, new QTableWidgetItem(QString::number(history[i].amount)));
            table->setItem(row, 1, new QTableWidgetItem(history[i].isIncome ? "Доход" : "Расход"));
            table->setItem(row, 2, new QTableWidgetItem(QString::fromUtf8(history[i].category.c_str())));
            table->setItem(row, 3, new QTableWidgetItem(QString::fromUtf8(history[i].date.c_str())));
        }
    }

    updateSummary();
}

void FinanceApp::onPrevMonthClicked() {
    currentMonth--;
    if (currentMonth < 1) currentMonth = 12;
    refreshDisplay();
}

void FinanceApp::onNextMonthClicked() {
    currentMonth++;
    if (currentMonth > 12) currentMonth = 1;
    refreshDisplay();
}

void FinanceApp::updateSummary() {
    double inc = manager.getTotalIncome(currentMonth);
    double exp = manager.getTotalExpense(currentMonth);
    double balance = inc - exp;

    summaryLabel->setText("Доходы: " + QString::number(inc) + " | Расходы: " + QString::number(exp) + " | Баланс: " + QString::number(balance));

    delete statsContainer;
    statsContainer = new QWidget(this);
    statsLayout = new QVBoxLayout(statsContainer);
    mainLayout->addWidget(statsContainer);

    QStringList colors = { "#55aaff", "#55ff7f", "#ff557f", "#ffff7f", "#aaffff", "#ffaa7f", "#aa55ff", "#e1e1e1" };
    int cIdx = 0;

    std::map<std::string, double> incData = manager.getIncomeByCategory(currentMonth);
    if (!incData.empty()) {
        statsLayout->addWidget(new QLabel("<b>Доходы за месяц:</b>"));
        for (auto const& p : incData) {
            QHBoxLayout* r = new QHBoxLayout();
            QLabel* l = new QLabel(QString::fromUtf8(p.first.c_str()) + " (" + QString::number(p.second) + ")");
            l->setFixedWidth(150);
            QProgressBar* b = new QProgressBar();
            b->setMaximum(inc); b->setValue(p.second); b->setTextVisible(false);
            b->setStyleSheet("QProgressBar::chunk { background-color: " + colors[cIdx++ % 8] + "; }");
            r->addWidget(l); r->addWidget(b); statsLayout->addLayout(r);
        }
    }

    std::map<std::string, double> expData = manager.getExpensesByCategory(currentMonth);
    if (!expData.empty()) {
        statsLayout->addWidget(new QLabel("<b>Расходы за месяц:</b>"));
        for (auto const& p : expData) {
            QHBoxLayout* r = new QHBoxLayout();
            QLabel* l = new QLabel(QString::fromUtf8(p.first.c_str()) + " (" + QString::number(p.second) + ")");
            l->setFixedWidth(150);
            QProgressBar* b = new QProgressBar();
            b->setMaximum(exp); b->setValue(p.second); b->setTextVisible(false);
            b->setStyleSheet("QProgressBar::chunk { background-color: " + colors[cIdx++ % 8] + "; }");
            r->addWidget(l); r->addWidget(b); statsLayout->addLayout(r);
        }
    }
}

void FinanceApp::onAddButtonClicked() {
    QString amountStr = amountEdit->text();
    QString typeStr = typeBox->currentText();
    QString categoryStr = categoryBox->currentText();
    QString dateStr = dateEdit->text();

    if (amountStr.isEmpty() || dateStr.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }

    bool ok;
    double amount = amountStr.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Ошибка", "Неверная сумма!");
        return;
    }

    if (dateStr.length() != 5 || dateStr[2] != '.') {
        QMessageBox::warning(this, "Ошибка", "Введите дату строго в формате ДД.ММ!");
        return;
    }

    bool isIncome = (typeStr == "Доход");

    manager.addTransaction(amount, categoryStr.toUtf8().constData(), isIncome, dateStr.toUtf8().constData());
    manager.saveToFile();

    amountEdit->clear();
    dateEdit->clear();
    refreshDisplay();
}

void FinanceApp::onDeleteButtonClicked() {
    int row = table->currentRow();
    if (row == -1) return;

    QString date = table->item(row, 3)->text();
    double amount = table->item(row, 0)->text().toDouble();

    std::vector<Transaction> history = manager.getTransactions();
    for (int i = 0; i < history.size(); i++) {
        if (history[i].date == date.toStdString() && history[i].amount == amount) {
            manager.removeTransaction(i);
            break;
        }
    }

    refreshDisplay();
}

void FinanceApp::onTypeChanged(const QString& text) {
    categoryBox->clear();
    if (text == "Доход") categoryBox->addItems({ "Зарплата", "Стипендия", "Подарок", "Другое" });
    else categoryBox->addItems({ "Еда", "Транспорт", "Жилье", "Обязательства", "Развлечения", "Бытовые", "Здоровье", "Другое" });
}