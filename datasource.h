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

    float temperature () const;
    float humidity () const;
    float airQuality () const;

signals:
    void valueChanged (DataType type, float value) const;

private:
    DataSource (QObject *parent = nullptr);
    ~DataSource ();

    class Private;
    Private *p;
};

#endif // DATASOURCE_H
