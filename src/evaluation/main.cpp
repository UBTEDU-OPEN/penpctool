#include <QCoreApplication>

#include <QDebug>

#include "charevaluateclient.h"
#include "logHelper.h"
#include "projectconst.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    LogHelper::init(argv[0]);
    LOG(INFO) << "word evaluation version:" << Config::localVersion().toStdString();

    WordDataReader client;
    while(true) {
        client.readWordData();
    }
    int ret =  a.exec();
    return ret;
}
