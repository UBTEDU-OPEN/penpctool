#ifndef PENDOTHANDLER_H
#define PENDOTHANDLER_H

#include <QThread>

class PenDotHandler : public QObject
{
    Q_OBJECT
public:
    explicit PenDotHandler(int id, QObject *parent = nullptr);

//    void run() override;

    void stopRunning();

public slots:
    void onNewDot(QString mac, int x, int fx, int y, int fy, int noteId, int pageId, int type, int force, long long timeStamp);

signals:

private:
    volatile bool running_ = true;
    int threadId_;
};

#endif // PENDOTHANDLER_H
