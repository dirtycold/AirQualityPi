#include "airqualitywidget.h"
#include <QApplication>

#include <QDebug>
#include "datasource.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AirQualityWidget w;
    w.show();

    return a.exec();
}
