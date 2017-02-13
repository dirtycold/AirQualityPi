#include "airqualitypiwidget.h"
#include "textwidget.h"

#include <QVBoxLayout>

AirQualityPiWidget::AirQualityPiWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout (this);
    TextWidget *temperatureWidget = new TextWidget (this);

    layout->addWidget(temperatureWidget);
}
