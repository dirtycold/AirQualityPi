#include "airqualitypiwidget.h"
#include <QApplication>
#include "datasource.h"
#include "datarefiner.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DataSource &dataSource = DataSource::instance();
    DataRefiner &dataRefiner = DataRefiner::instance();
    dataRefiner.setSource(&dataSource);

    AirQualityPiWidget w;
    w.show();

    return a.exec();
}
