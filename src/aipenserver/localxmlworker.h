#ifndef LOCALXMLINFO_H
#define LOCALXMLINFO_H

#include <QObject>
#include <QSqlDatabase>

class  LocalXmlWorker : public QObject
{
    Q_OBJECT
public:
    enum FileState {
        Unknown = 0,
        NotUpdated,
        Updated,
        Downloading,
        Downloaded
    };
    static LocalXmlWorker* instance();
    ~LocalXmlWorker();
    void installXml();
    FileState getFileState(int wordCode);

public slots:
    void updateWordXml(int wordCode, QString updateTime, QString xmlUrl, bool bookDetail);
    void updateSideXml();
    void onSideXmlResponse(QString updateTime, QString url);
    void onDownloadFinished(int wordCode);

signals:
    void httpRequest(QString,QByteArray,int,int,qulonglong);
private:
    explicit LocalXmlWorker(QObject *parent = nullptr);
    QString getLocalXmlUpdateTime(int wordCode);
    QString getLocalSideXmlUpdateTime();
    bool insertLocalUpdateTime(int wordCode, QString updateTime);
    bool deleteUpdateTime(int wordCode);
    bool updateFileState(int wordCode, FileState state);

private:
    QSqlDatabase xmlDb_;
};

#endif // LOCALXMLINFO_H
