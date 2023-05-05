#include "pendothandler.h"

#include <QThread>

#include "aipenmanager.h"
#include "logHelper.h"

PenDotHandler::PenDotHandler(int id, QObject *parent) : QObject(parent)
  , threadId_(id)
{

}

//void PenDotHandler::run()
//{
//    while(running_) {
//        DotInfo dot;
//        if(DotList::instance()->readDot(dot)) {
//            AiPenManager::getInstance()->handleDot(dot.mac,dot.x,dot.fx,dot.y,dot.fy,dot.noteId,dot.pageId,dot.dotType,dot.force);
//        } else {
//            msleep(100);
//        }
//    }
//}

void PenDotHandler::stopRunning()
{
    running_ = false;
}

void PenDotHandler::onNewDot(QString mac, int x, int fx, int y, int fy, int noteId, int pageId, int type, int force, long long timeStamp)
{
    static int i = 0;
    LOG(INFO) << "PenDotHandler::onNewDot id:" << threadId_ << ",i=" << ++i;
    AiPenManager::getInstance()->handleDot(mac,x,fx,y,fy,noteId,pageId,type,force,timeStamp);
}
