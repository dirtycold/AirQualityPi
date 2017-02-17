#include "datasource.h"

#include <unistd.h>

#include "wiringPi.h"
#include "wiringPiI2C.h"

#include "ads1115.h"

#include <QTimer>
#include <QtMath>
#include <QDebug>

#include <QString>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

static const int htu21d_i2c_addr {0x40};
static const int htu21d_temp_id  {0xF3};
static const int htu21d_hum_id   {0xF5};
static const int htu21d_delay    {50};

static const int ads1115_base_addr {100};
static const int ads1115_i2c_addr  {0x48};

class DataSource::Private
{
    friend class DataSource;
    float temperature   {NAN};
    float humidity      {NAN};
    float airQuality    {NAN};

    QTimer refreshTimer;

    int fd;

    DataSource *parent { nullptr};

    Private (DataSource *parent)
        : parent (parent)
    {}

    // HTU21D reading derived from
    // rpi-examples
    // https://github.com/leon-anavi/rpi-examples

    void initSensors ()
    {
        setenv("WIRINGPI_GPIOMEM", "1", 1);
        wiringPiSetup();
        fd = wiringPiI2CSetup(htu21d_i2c_addr);
        if ( 0 > fd )
        {
            fprintf (stderr, "Unable to open I2C device: %s\n", strerror (errno));
            exit (-1);
        }

        ads1115Setup(ads1115_base_addr, ads1115_i2c_addr);

        QTimer *timer = new QTimer;
        timer->setInterval(500);
        timer->setSingleShot(false);

        parent->connect(timer, &QTimer::timeout, [this] () {
            // refresh
            refreshValues();
        });

        timer->start();
    }

    void refreshValues ()
    {
        unsigned char buf [4];
        unsigned int rawValue;
        float rawValueFloat;
        float newValue;

        auto isFuzzyEqual = [](float a, float b, float precision = 1e-2)
        {
            bool equal = (qAbs (a - b) < qAbs (precision));
            return equal;
        };

        wiringPiI2CWrite(fd, htu21d_temp_id);
        delay(htu21d_delay);
        read(fd, buf, 3);
        rawValue = (buf [0] << 8 | buf [1]) & 0xFFFC;
        // Convert sensor reading into temperature.
        // See page 14 of the datasheet
        rawValueFloat = rawValue / 65536.0;
        newValue = -46.85 + (175.72 * rawValueFloat);

        if (!isFuzzyEqual (newValue, temperature))
        {
            temperature = newValue;
            emit parent->valueChanged(DataSource::Temperature, temperature);
        }

        wiringPiI2CWrite(fd, htu21d_hum_id);
        delay(htu21d_delay);
        read(fd, buf, 3);
        rawValue = (buf [0] << 8 | buf [1]) & 0xFFFC;
        // Convert sensor reading into humidity.
        // See page 15 of the datasheet
        rawValueFloat = rawValue / 65536.0;
        newValue = -6.0 + (125.0 * rawValueFloat);

        if (!isFuzzyEqual (newValue, humidity))
        {
            humidity = newValue;
            emit parent->valueChanged(DataSource::Humidity, humidity);
        }

        rawValue = analogRead(ads1115_base_addr + 1);
        rawValueFloat = rawValue * 4.096 / 32768.0;
        newValue = rawValueFloat * 500;

        if (!isFuzzyEqual (newValue, airQuality))
        {
            airQuality = newValue;
            emit parent->valueChanged(DataSource::AirQuality, airQuality);
        }

        // qDebug () << ".";

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

DataSource &DataSource::instance()
{
    static DataSource source;
    return source;
}

float DataSource::value(DataSource::DataType type) const
{
    float value (NAN);
    switch (type)
    {
    case Temperature:
        value = p->temperature;
        break;
    case Humidity:
        value = p->humidity;
        break;
    case AirQuality:
        value = p->airQuality;
        break;
    default:
        break;
    }
    return value;
}

DataSource::DataSource(QObject *parent)
    : QObject (parent),
      p (new DataSource::Private (this))
{
    p->initSensors();
}

DataSource::~DataSource()
{
    delete p;
}
