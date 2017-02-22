#include "datasource.h"

#include <unistd.h>

#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "ads1115.h"

#include <QTimer>
#include <QtMath>
#include <QDebug>

static const int HTU21D_I2C_ADDR {0x40};
static const int HTU21D_TEMP_ID  {0xF3};
static const int HTU21D_HUM_ID   {0xF5};
static const int HTU21D_DELAY    {100};

static const int ADS1115_BASE_ADDR {100};
static const int ADS1115_I2C_ADDR  {0x48};

static const int refreshInterval {500};

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
        fd = wiringPiI2CSetup(HTU21D_I2C_ADDR);
        if ( 0 > fd )
        {
            fprintf (stderr, "Unable to open I2C device: %s\n", strerror (errno));
            exit (-1);
        }

        ads1115Setup(ADS1115_BASE_ADDR, ADS1115_I2C_ADDR);

        QTimer *timer = new QTimer (parent);
        timer->setInterval(refreshInterval);
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

        wiringPiI2CWrite(fd, HTU21D_TEMP_ID);
        delay(HTU21D_DELAY);
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

        wiringPiI2CWrite(fd, HTU21D_HUM_ID);
        delay(HTU21D_DELAY);
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

        rawValue = analogRead(ADS1115_BASE_ADDR + 1);
        rawValueFloat = rawValue * 4.096 / 32768.0;
        newValue = rawValueFloat * 500;

        if (!isFuzzyEqual (newValue, airQuality))
        {
            airQuality = newValue;
            emit parent->valueChanged(DataSource::AirQuality, airQuality);
        }

        emit parent->valueUpdated();

        // qDebug () << ".";
    }
};

DataSource &DataSource::instance()
{
    static DataSource source;
    return source;
}

float DataSource::temperature() const
{
    return p->temperature;
}

float DataSource::humidity() const
{
    return p->humidity;
}

float DataSource::airQuality() const
{
    return p->airQuality;
}

bool DataSource::isFuzzyEqual(float a, float b, float precision)
{
    bool equal = (qAbs (a - b) < qAbs (precision));
    return equal;
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
