#include "datarefiner.h"

#include <QString>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

class DataRefiner::Private
{
    friend class DataRefiner;

    DataRefiner *parent {nullptr};

    DataSource *source {nullptr};
    QMetaObject::Connection connection;

    float temperature {NAN};
    float humidity {NAN};
    float airQuality {NAN};

    Private (DataRefiner *parent) : parent (parent) {}

    void refresh ()
    {
        if (source == nullptr)
        {
            return;
        }

        auto setValue = [this] (float *valuePtr, DataSource::DataType type, float newValue)
        {
            static const float A {0.9};
            static const float B {1 - A};

            if (valuePtr == nullptr)
            {
                return;
            }

            if (std::isnan(*valuePtr) && !std::isnan(newValue))
            {
                *valuePtr = newValue;
                emit parent->valueChanged(type, *valuePtr);
            }
            else
            {
                float updatedValue = *valuePtr * A + newValue * B;
                if (! DataSource::isFuzzyEqual(*valuePtr, updatedValue))
                {
                    *valuePtr = updatedValue;
                    emit parent->valueChanged(type, *valuePtr);
                }
            }
        };

        setValue (&temperature, DataSource::Temperature, source->temperature());
        setValue (&humidity, DataSource::Humidity, source->humidity());
        setValue (&airQuality, DataSource::AirQuality, source->airQuality());

        {
            QFile file ("airQualityPi.log");
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            QTextStream s (&file);
            s << QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss.zzz")
              << " "
              << QString::number(temperature)
              << " "
              << QString::number(humidity)
              << " "
              << QString::number(airQuality)
              << endl;
            file.close();
        }
    }
};

DataRefiner &DataRefiner::instance()
{
    static DataRefiner instance;
    return instance;
}

void DataRefiner::setSource(DataSource *source)
{
    if (p->source != nullptr)
    {
        removeSource();
    }

    p->source = source;
    p->connection = connect (p->source, &DataSource::valueUpdated, [this] () { p->refresh();});
}

void DataRefiner::removeSource()
{
    disconnect (p->connection);
    p->source = nullptr;
}

const DataSource *DataRefiner::source()
{
    return p->source;
}

DataRefiner::DataRefiner(QObject *parent)
    : QObject(parent),
      p(new Private (this))
{
}

DataRefiner::~DataRefiner()
{
    delete p;
}
