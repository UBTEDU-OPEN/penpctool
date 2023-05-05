#ifndef UPLOADORIGINDATAWORKER_H
#define UPLOADORIGINDATAWORKER_H

#include <QObject>

class UploadOriginDataWorker : public QObject
{
    Q_OBJECT
public:
    explicit UploadOriginDataWorker(QObject *parent = nullptr);

public slots:
    void uploadOriginData(int workClassId, QString classTimeStamp);

signals:

};

#endif // UPLOADORIGINDATAWORKER_H
