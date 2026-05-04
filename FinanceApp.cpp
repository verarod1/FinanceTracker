#include "FinanceApp.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QString>
#include <QMessageBox>
#include <QDate>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <set>
#include <algorithm>

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
    analyticsButton = new QPushButton("Аналитика", this);

    navLayout->addWidget(prevMonthButton);
    navLayout->addWidget(monthNameLabel);
    navLayout->addWidget(nextMonthButton);
    navLayout->addWidget(analyticsButton);

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
    table->setHorizontalHeaderLabels({ "Сумма", "Тип ▼", "Категория ▼", "Дата ▼" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionsClickable(true);

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
    connect(analyticsButton, &QPushButton::clicked, this, &FinanceApp::onYearlyAnalyticsClicked);

    connect(amountEdit, &QLineEdit::returnPressed, this, &FinanceApp::onAddButtonClicked);
    connect(dateEdit, &QLineEdit::returnPressed, this, &FinanceApp::onAddButtonClicked);

    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &FinanceApp::onHeaderClicked);

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
            QString tType = history[i].isIncome ? "Доход" : "Расход";
            QString tCat = QString::fromUtf8(history[i].category.c_str());
            QString tDate = QString::fromUtf8(history[i].date.c_str());

            if (!typeFilters.isEmpty() && !typeFilters.contains(tType)) continue;
            if (!categoryFilters.isEmpty() && !categoryFilters.contains(tCat)) continue;
            if (!dateFilters.isEmpty() && !dateFilters.contains(tDate)) continue;

            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(history[i].amount)));
            table->setItem(row, 1, new QTableWidgetItem(tType));
            table->setItem(row, 2, new QTableWidgetItem(tCat));
            table->setItem(row, 3, new QTableWidgetItem(tDate));
        }
    }

    updateSummary();
}

void FinanceApp::onHeaderClicked(int logicalIndex) {
    if (logicalIndex == 0) return;

    std::set<QString> uniqueVals;
    std::vector<Transaction> history = manager.getTransactions();

    for (int i = 0; i < history.size(); i++) {
        if (history[i].date.length() < 5) continue;
        int tMonth = std::stoi(history[i].date.substr(3, 2));

        if (tMonth == currentMonth) {
            QString tType = history[i].isIncome ? "Доход" : "Расход";
            QString tCat = QString::fromUtf8(history[i].category.c_str());
            QString tDate = QString::fromUtf8(history[i].date.c_str());

            if (logicalIndex != 1 && !typeFilters.isEmpty() && !typeFilters.contains(tType)) continue;
            if (logicalIndex != 2 && !categoryFilters.isEmpty() && !categoryFilters.contains(tCat)) continue;
            if (logicalIndex != 3 && !dateFilters.isEmpty() && !dateFilters.contains(tDate)) continue;

            if (logicalIndex == 1) uniqueVals.insert(tType);
            else if (logicalIndex == 2) uniqueVals.insert(tCat);
            else if (logicalIndex == 3) uniqueVals.insert(tDate);
        }
    }

    if (uniqueVals.empty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Фильтр");
    dialog.resize(250, 300);
    QVBoxLayout layout(&dialog);
    QListWidget list;

    QStringList* currentFilter = nullptr;
    if (logicalIndex == 1) currentFilter = &typeFilters;
    else if (logicalIndex == 2) currentFilter = &categoryFilters;
    else if (logicalIndex == 3) currentFilter = &dateFilters;

    for (const QString& val : uniqueVals) {
        QListWidgetItem* item = new QListWidgetItem(val, &list);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (currentFilter->isEmpty() || currentFilter->contains(val)) {
            item->setCheckState(Qt::Checked);
        }
        else {
            item->setCheckState(Qt::Unchecked);
        }
    }

    layout.addWidget(&list);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttons);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        currentFilter->clear();
        bool allChecked = true;
        bool hasChecked = false;

        for (int i = 0; i < list.count(); i++) {
            if (list.item(i)->checkState() == Qt::Checked) {
                currentFilter->append(list.item(i)->text());
                hasChecked = true;
            }
            else {
                allChecked = false;
            }
        }

        if (allChecked) {
            currentFilter->clear();
        }
        else if (!hasChecked) {
            currentFilter->append("___EMPTY___");
        }

        refreshDisplay();
    }
}

void FinanceApp::onPrevMonthClicked() {
    currentMonth--;
    if (currentMonth < 1) currentMonth = 12;
    typeFilters.clear();
    categoryFilters.clear();
    dateFilters.clear();
    refreshDisplay();
}

void FinanceApp::onNextMonthClicked() {
    currentMonth++;
    if (currentMonth > 12) currentMonth = 1;
    typeFilters.clear();
    categoryFilters.clear();
    dateFilters.clear();
    refreshDisplay();
}

void FinanceApp::updateSummary() {
    double inc = 0.0;
    double exp = 0.0;
    std::map<std::string, double> incData;
    std::map<std::string, double> expData;

    std::vector<Transaction> history = manager.getTransactions();

    for (int i = 0; i < history.size(); i++) {
        if (history[i].date.length() < 5) continue;
        int tMonth = std::stoi(history[i].date.substr(3, 2));

        if (tMonth == currentMonth) {
            QString tType = history[i].isIncome ? "Доход" : "Расход";
            QString tCat = QString::fromUtf8(history[i].category.c_str());
            QString tDate = QString::fromUtf8(history[i].date.c_str());

            if (!typeFilters.isEmpty() && !typeFilters.contains(tType)) continue;
            if (!categoryFilters.isEmpty() && !categoryFilters.contains(tCat)) continue;
            if (!dateFilters.isEmpty() && !dateFilters.contains(tDate)) continue;

            if (history[i].isIncome) {
                inc += history[i].amount;
                incData[history[i].category] += history[i].amount;
            }
            else {
                exp += history[i].amount;
                expData[history[i].category] += history[i].amount;
            }
        }
    }

    double balance = inc - exp;

    summaryLabel->setText("Доходы: " + QString::number(inc) + " | Расходы: " + QString::number(exp) + " | Баланс: " + QString::number(balance));

    delete statsContainer;
    statsContainer = new QWidget(this);
    statsLayout = new QVBoxLayout(statsContainer);
    mainLayout->addWidget(statsContainer);

    QStringList colors = { "#55aaff", "#55ff7f", "#ff557f", "#ffff7f", "#aaffff", "#ffaa7f", "#aa55ff", "#e1e1e1" };
    int cIdx = 0;

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

void FinanceApp::onYearlyAnalyticsClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Аналитика за год");
    dialog.resize(900, 600);
    dialog.setStyleSheet("QToolTip { color: #000000; background-color: #ffffff; border: 1px solid #767676; }");

    QVBoxLayout mainLayout(&dialog);
    QHBoxLayout chartLayout;

    std::vector<Transaction> history = manager.getTransactions();
    double maxVal = 0;
    double incMonth[13] = { 0 };
    double expMonth[13] = { 0 };
    std::map<std::string, double> incMonthCat[13];
    std::map<std::string, double> expMonthCat[13];
    std::set<std::string> incCats;
    std::set<std::string> expCats;

    for (int i = 0; i < history.size(); i++) {
        if (history[i].date.length() >= 5) {
            int m = std::stoi(history[i].date.substr(3, 2));
            if (m >= 1 && m <= 12) {
                if (history[i].isIncome) {
                    incMonth[m] += history[i].amount;
                    incMonthCat[m][history[i].category] += history[i].amount;
                    incCats.insert(history[i].category);
                }
                else {
                    expMonth[m] += history[i].amount;
                    expMonthCat[m][history[i].category] += history[i].amount;
                    expCats.insert(history[i].category);
                }
            }
        }
    }

    for (int i = 1; i <= 12; i++) {
        if (incMonth[i] > maxVal) maxVal = incMonth[i];
        if (expMonth[i] > maxVal) maxVal = expMonth[i];
    }
    if (maxVal == 0) maxVal = 1;

    QStringList mNames = { "", "Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек" };

    std::map<std::string, QString> catColors;
    QStringList palette = { "#55aaff", "#55ff7f", "#ff557f", "#ffff7f", "#aaffff", "#ffaa7f", "#aa55ff", "#e1e1e1", "#ffa500", "#ff00ff", "#00ffff", "#f08080" };
    int colorIdx = 0;

    for (const std::string& cat : incCats) {
        catColors[cat] = palette[colorIdx % palette.size()];
        colorIdx++;
    }
    for (const std::string& cat : expCats) {
        catColors[cat] = palette[colorIdx % palette.size()];
        colorIdx++;
    }

    for (int i = 1; i <= 12; i++) {
        QVBoxLayout* monthLayout = new QVBoxLayout();
        QHBoxLayout* barsLayout = new QHBoxLayout();

        auto makeBar = [&](double total, std::map<std::string, double>& cats) {
            QWidget* container = new QWidget();
            container->setFixedWidth(25);
            container->setStyleSheet("background: #2b2b2b; border: 1px solid #1e1e1e;");
            QVBoxLayout* layout = new QVBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            double emptySpace = maxVal - total;
            if (emptySpace > 0) {
                QWidget* spacer = new QWidget();
                spacer->setStyleSheet("background: transparent; border: none;");
                layout->addWidget(spacer, static_cast<int>(emptySpace * 100));
            }

            for (auto const& p : cats) {
                QWidget* chunk = new QWidget();
                chunk->setStyleSheet("background-color: " + catColors[p.first] + "; border: none;");
                chunk->setToolTip(QString::fromUtf8(p.first.c_str()) + ": " + QString::number(p.second));
                layout->addWidget(chunk, static_cast<int>(p.second * 100));
            }
            return container;
            };

        barsLayout->addWidget(makeBar(incMonth[i], incMonthCat[i]));
        barsLayout->addWidget(makeBar(expMonth[i], expMonthCat[i]));

        QLabel* lbl = new QLabel(mNames[i]);
        lbl->setAlignment(Qt::AlignCenter);

        monthLayout->addLayout(barsLayout);
        monthLayout->addWidget(lbl);
        chartLayout.addLayout(monthLayout);
    }

    mainLayout.addLayout(&chartLayout);

    QHBoxLayout* legendMainLayout = new QHBoxLayout();
    legendMainLayout->addStretch();

    if (!incCats.empty()) {
        QVBoxLayout* incBox = new QVBoxLayout();
        incBox->setSpacing(2);
        QLabel* incLbl = new QLabel("<b>Доходы</b>");
        incLbl->setAlignment(Qt::AlignLeft);
        incBox->addWidget(incLbl);

        for (const std::string& cat : incCats) {
            QHBoxLayout* itemL = new QHBoxLayout();
            itemL->setContentsMargins(0, 0, 0, 0);
            itemL->setSpacing(5);
            QLabel* clr = new QLabel("■");
            clr->setStyleSheet("color: " + catColors[cat] + "; font-size: 16px;");
            QLabel* txt = new QLabel(QString::fromUtf8(cat.c_str()));
            itemL->addWidget(clr);
            itemL->addWidget(txt);
            itemL->addStretch();
            incBox->addLayout(itemL);
        }
        incBox->addStretch();
        legendMainLayout->addLayout(incBox);
    }

    if (!incCats.empty() && !expCats.empty()) {
        legendMainLayout->addSpacing(50);
    }

    if (!expCats.empty()) {
        QVBoxLayout* expBox = new QVBoxLayout();
        expBox->setSpacing(2);
        QLabel* expLbl = new QLabel("<b>Расходы</b>");
        expLbl->setAlignment(Qt::AlignLeft);
        expBox->addWidget(expLbl);

        for (const std::string& cat : expCats) {
            QHBoxLayout* itemL = new QHBoxLayout();
            itemL->setContentsMargins(0, 0, 0, 0);
            itemL->setSpacing(5);
            QLabel* clr = new QLabel("■");
            clr->setStyleSheet("color: " + catColors[cat] + "; font-size: 16px;");
            QLabel* txt = new QLabel(QString::fromUtf8(cat.c_str()));
            itemL->addWidget(clr);
            itemL->addWidget(txt);
            itemL->addStretch();
            expBox->addLayout(itemL);
        }
        expBox->addStretch();
        legendMainLayout->addLayout(expBox);
    }

    legendMainLayout->addStretch();
    mainLayout.addLayout(legendMainLayout);

    dialog.exec();
}