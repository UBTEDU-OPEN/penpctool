#ifndef ORIGINDATASAVER_H
#define ORIGINDATASAVER_H

#include <QObject>
#include <QMap>
#include <QFile>

class OriginDataSaver : public QObject
{
    Q_OBJECT
public:
    explicit OriginDataSaver(QObject *parent = nullptr);
    void releaseFile();

public slots:
    void onOriginData(QString macAddr, char* data, int len);

signals:

private:
    QMap<QString,QFile*> originFile;

};

#endif // ORIGINDATASAVER_H
