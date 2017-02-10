#include "temperaturewidget.h"
#include "datasource.h"

#include <QLabel>
#include <QFormLayout>

TemperatureWidget::TemperatureWidget(QWidget *parent) : QWidget(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *temperatureTextLabel = new QLabel ("Temperature", this);
    QLabel *temperatureValueLabel = new QLabel (this);

    QLabel *humidityTextLabel = new QLabel ("Humidity", this);
    QLabel *humidityValueLabel = new QLabel (this);

    layout->addRow(temperatureTextLabel, temperatureValueLabel);
    layout->addRow(humidityTextLabel, humidityValueLabel);

    DataSource &dataSource = DataSource::instance();

    connect(&dataSource, &DataSource::valueChanged, [=] (DataSource::DataType type, float value) {
        switch (type)
        {
        case DataSource::Temperature:
            temperatureValueLabel->setText(QString::number(value));
            break;
        case DataSource::Humidity:
            humidityValueLabel->setText(QString::number(value));
            break;
        default:
            break;
        }
    });
}
