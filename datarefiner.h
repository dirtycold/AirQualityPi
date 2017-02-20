#ifndef DATAREFINER_H
#define DATAREFINER_H

#include <datasource.h>

class DataRefiner : public QObject
{
    Q_OBJECT
public:
    static DataRefiner &instance ();

    void setSource (DataSource *source);
    void removeSource ();
    const DataSource* source ();

signals:
    void valueChanged (DataSource::DataType type, float value) const;

public slots:

private:
    explicit DataRefiner(QObject *parent = 0);
    ~DataRefiner();

    class Private;
    Private *p;
};

#endif // DATAREFINER_H
