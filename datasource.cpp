#include "datasource.h"

#include <QTimer>

DataSource &DataSource::instance()
{
    static DataSource source;
    return source;
}

float DataSource::temperature() const
{
    return 0;
}

float DataSource::humidity() const
{
    return 0;
}

int DataSource::airQuality() const
{
    return 0;
}

DataSource::DataSource(QObject *parent)
    : QObject (parent)
{

}

DataSource::~DataSource()
{

}
