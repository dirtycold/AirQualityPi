#include "datarefiner.h"

#include <QString>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QContiguousCache>
#include <QTimer>

#include <QDebug>

static const int cacheSize {10};
static const int refreshInterval {5000};

class DataRefiner::Private
{
    friend class DataRefiner;

    DataRefiner *parent {nullptr};

    DataSource *source {nullptr};
    QMetaObject::Connection connection;

    float temperature {NAN};
    float humidity {NAN};
    float airQuality {NAN};

    typedef QContiguousCache<float> ValueCache;
    ValueCache temperatureCache;
    ValueCache humidityCache;
    ValueCache airQualityCache;

    Private (DataRefiner *parent) : parent (parent)
    {
        temperatureCache.setCapacity(cacheSize);
        humidityCache.setCapacity(cacheSize);
        airQualityCache.setCapacity(cacheSize);

        QTimer *timer = new QTimer (parent);
        timer->setSingleShot(false);
        timer->setInterval(refreshInterval);

        parent->connect(timer, &QTimer::timeout, [this] () { refreshValue();});
        timer->start();

    }

    void processUpdate ()
    {
        if (source == nullptr)
        {
            return;
        }

        temperatureCache.append(source->temperature());
        humidityCache.append(source->humidity());
        airQualityCache.append(source->airQuality());
    }

    void refreshValue ()
    {
        if (source == nullptr)
        {
            return;
        }

        auto setValue = [this] (float *valuePtr, DataSource::DataType type, float newValue)
        {
            static const float A {0.9};
            static const float B {1 - A};
            // static const float PREC {0.1};

            if (valuePtr == nullptr)
            {
                return;
            }

            if (!std::isnormal(newValue))
            {
                return;
            }

            if (std::isnan(*valuePtr))
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
            }
        };

        auto cacheAverage = [] (const ValueCache &cache)
        {
            // cache.normalizeIndexes();
            int count = cache.count();
            float sum {0};
            for (int i = cache.firstIndex(); i < cache.lastIndex(); ++i)
            {
                sum += cache.at(i);
            }
            sum /= count;
            return sum;
        };

        setValue (&temperature, DataSource::Temperature, cacheAverage (temperatureCache));
        setValue (&humidity, DataSource::Humidity, cacheAverage (humidityCache));
        setValue (&airQuality, DataSource::AirQuality, cacheAverage (airQualityCache));
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
    p->connection = connect (p->source, &DataSource::valueUpdated, [this] () { p->processUpdate();});
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
