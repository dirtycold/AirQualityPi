#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>

class DataSource final : public QObject
{
    Q_OBJECT

public:
    static DataSource &instance ();

    enum DataType
    {
        Temperature,
        Humidity,
        AirQuality,
    };

    float value (DataType type) const;

signals:
    void valueChanged (DataType type, float value) const;

public:
    DataSource (QObject *parent = nullptr);
    ~DataSource ();
};

#endif // DATASOURCE_H
