#include "calculator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Calculator calculator;
    calculator.show();
    return app.exec();
}
