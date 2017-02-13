#include "textwidget.h"
#include "datasource.h"

#include <QLabel>
#include <QFormLayout>

TextWidget::TextWidget(QWidget *parent) : QWidget(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *temperatureTextLabel = new QLabel ("Temperature", this);
    QLabel *temperatureValueLabel = new QLabel (this);

    QLabel *humidityTextLabel = new QLabel ("Humidity", this);
    QLabel *humidityValueLabel = new QLabel (this);

    QLabel *airQualityTextLabel = new QLabel ("Air Quality", this);
    QLabel *airQualityValueLabel = new QLabel (this);

    layout->addRow(temperatureTextLabel, temperatureValueLabel);
    layout->addRow(humidityTextLabel, humidityValueLabel);
    layout->addRow(airQualityTextLabel, airQualityValueLabel);

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