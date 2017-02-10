#include "airqualitypiwidget.h"
#include <QApplication>
#include "datasource.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AirQualityPiWidget w;
    w.show();

    return a.exec();
}
