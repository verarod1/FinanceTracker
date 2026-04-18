#include "FinanceApp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FinanceApp window;
    window.show();
    return app.exec();
}
