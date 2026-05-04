# Пишем свой трекер финансов на C++ и Qt: от архитектуры до аналитики

Иногда найти удобный трекер расходов сложнее, чем написать свой. Мне нужен был максимально простой, быстрый и контролируемый инструмент без лишних подписок и облачных синхронизаций. В этой статье я расскажу, как реализовала десктопное приложение для учета финансов на C++ с использованием Qt.

Посмотрим на архитектуру, реализацию хранения данных и то, как можно собрать наглядную аналитику из стандартных виджетов, не прибегая к тяжелым библиотекам для графиков.

<img height="400" alt="image" src="https://github.com/user-attachments/assets/78adfb96-0404-42d8-a13a-92009c4e3078" />

## Разделение логики и интерфейса

Проект разбит на две основные части: `FinanceManager` (работа с данными) и `FinanceApp` (графический интерфейс).

Класс `FinanceManager` инкапсулирует всю работу с транзакциями. Структура одной записи максимально проста: сумма, категория, флаг типа (доход/расход) и дата.

```cpp
struct Transaction {
    double amount = 0.0;
    std::string category;
    bool isIncome = false;
    std::string date;
};
```

Хранение данных реализовано простым текстовым файлом `data.txt`. Для локального трекера построчное чтение и запись через `std::ifstream` и `std::ofstream` работают молниеносно и не требуют сторонних зависимостей. Данные сохраняются в формате чисел и строк через пробел. Выглядит это так: `500 Еда 0 01.06`.

## UX-решения: Фильтрация по клику на заголовок

Мне было важно сделать интерфейс `QTableWidget` не просто статичной таблицей, а интерактивным инструментом. Пользователь может кликнуть на заголовок столбца (Тип, Категория или Дата), чтобы открыть диалоговое окно фильтрации.

Логика собирает уникальные значения из выбранного столбца для текущего месяца:

<img height="600" alt="ezgif com-video-to-gif-converter" src="https://github.com/user-attachments/assets/00057109-6207-4e65-b133-9e2a191027b0" />



```cpp
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
    
}
```

Если фильтры применяются, таблица очищается и перерисовывается заново, проверяя каждую транзакцию на соответствие всем активным спискам фильтров (`typeFilters`, `categoryFilters`, `dateFilters`).

## Кастомная аналитика из подручных средств

Самая интересная часть - это окно аналитики за год. Вместо того чтобы подключать сторонние библиотеки отрисовки, я решила собрать диаграммы из базовых виджетов Qt, что дало полный контроль над визуалом и помогло лучше разобраться в Qt.

<img height="600" alt="image" src="https://github.com/user-attachments/assets/0a83c373-a0a9-4067-bac3-7e948636cfdb" />



В главном окне сводка по категориям за месяц реализована через обычные `QProgressBar`. Мы убираем текстовое значение, задаем максимум, равный общим расходам или доходам, и закрашиваем полосу через StyleSheet:

```cpp
QProgressBar* b = new QProgressBar();
b->setMaximum(exp); 
b->setValue(p.second); 
b->setTextVisible(false);
b->setStyleSheet("QProgressBar::chunk { background-color: " + colors[cIdx++ % 8] + "; }");
```

В диалоговом окне годовой аналитики логика сложнее. Вертикальные столбцы графиков строятся с помощью `QVBoxLayout` и вложенных `QWidget`. Относительная высота блоков высчитывается на основе максимального значения за год, а оставшееся пространство сверху заполняется прозрачной пустышкой:

```cpp
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
```

В результате получается легковесный и отзывчивый интерфейс. При наведении на каждый блок всплывает `ToolTip` с названием категории и суммой. Для легенды динамически генерируется сетка из `QHBoxLayout` с цветными квадратиками `■`, которые жестко привязаны к конкретным категориям через палитру цветов.

## Вывод

Qt остается прекрасным инструментом для пет-проектов. Жесткое разделение логики (`FinanceManager`) и графического интерфейса (`FinanceApp`) позволяет легко поддерживать проект. А стандартных компоновочных блоков (`QHBoxLayout`, `QVBoxLayout`, `QWidget`) вполне достаточно для создания нестандартных элементов интерфейса.
