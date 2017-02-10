#include "airqualitypiwidget.h"
#include "temperaturewidget.h"

#include <QVBoxLayout>

AirQualityPiWidget::AirQualityPiWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout (this);
    TemperatureWidget *temperatureWidget = new TemperatureWidget (this);

    layout->addWidget(temperatureWidget);
}
