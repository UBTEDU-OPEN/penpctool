#include "ocrcharhandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "aipenmanager.h"
#include "logHelper.h"

OCRCharHandler::OCRCharHandler(QString mac, QObject *parent) : QObject(parent)
  , macAddr_(mac)
{

}

//0：同位置 -1：左边相邻 +1：右边相邻 小于-1：左边但非相邻 大于+1：右边非相邻
int OCRCharHandler::comparePos(const OCRChar &lhs, const OCRChar &rhs)
{
    //评审：判定超过5页，认为是换本子，默认认为相邻
    if(rhs.pageId < lhs.pageId && (lhs.pageId - rhs.pageId) >= 5) {
        LOG(INFO) << macAddr_.toStdString() << " comparePos page:" << rhs.pageId
                  << "," << lhs.pageId;
        return 1;
    }
    int diff = rhs.pos - lhs.pos;
    LOG(INFO) << macAddr_.toStdString() << " comparePos diff:" << diff;
    return diff;
}

int OCRCharHandler::charPos(OCRChar ocrChar)
{
    //x0-6 y0-1 z0-3
    return ocrChar.pageId*(7*2*4)+ocrChar.zIndex*14+ocrChar.yIndex*7+ocrChar.xIndex;
}

void OCRCharHandler::processCurrentWordOCRChar()
{
    QMutexLocker locker(&currentWordLocker_);
    if(nullptr != currentWord_) {
        LOG(INFO) << macAddr_.toStdString() << " processOCRChar "
                  << currentWord_ << ",char size=" << currentWord_->ocrChars_.size();
        //1.1 进行当前整词匹配
        bool finished = matchWord(currentWord_);

        //1.3 进行未完成词匹配，整词匹配
        matchUnfinishedWords();

        //1.2 判断第一个字
        checkFirstCharForLastWord(finished);
        if(!finished) {
            LOG(INFO) << macAddr_.toStdString() << "processOCRChar unfinished, char size="
                      << currentWord_->ocrChars_.size();
            printOCRChars(currentWord_->groupId,currentWord_->ocrChars_);
            unfinishedWords_.push_back(currentWord_);
            lastWord_ = currentWord_;
        } else {
            lastWord_ = nullptr; //上一个词已完成
            delete currentWord_;  //不用管了
        }
        currentWord_ = nullptr;
    }
}

void OCRCharHandler::printOCRChars(int groupId, const QMap<int, OCRChar> &ocrChars)
{
    LOG(INFO) << macAddr_.toStdString() << " printOCRChars groupId=" << groupId << ",char size=" << ocrChars.size();
    for(auto cit = ocrChars.cbegin(); cit != ocrChars.cend(); ++cit) {
        const OCRChar& oc = cit.value();
        LOG(INFO) << macAddr_.toStdString() << " printOCRChars pos=" << cit.key()
                  << ",groupId=" << groupId << ",char=" << oc.ocrContentCode << "," << oc.score
                  << ",path=" << oc.imgPath.toStdString();
    }
}

void OCRCharHandler::handleNewDictationWord(int groupId, const QVector<int>& contentCodes)
{
    //1. 先进行字词匹配
    processCurrentWordOCRChar();
    //2. 开始新的词
    currentWord_ = new OCRWord;
    currentWord_->groupId = groupId;
    currentWord_->contentCodes = contentCodes;
}

bool OCRCharHandler::matchWord(OCRWord* word)
{
    LOG(INFO) << macAddr_.toStdString() << " matchWord";
    if(nullptr == word || word->ocrChars_.empty()) {
        return false;
    }

    printOCRChars(word->groupId,word->ocrChars_);

    QVector<int> matchesPos;
    int lastPos = -1;

    auto tempChars = word->ocrChars_;
    for(auto cit = word->contentCodes.crbegin(); cit != word->contentCodes.crend(); ++cit) { //逆序查找
        int contentCode = *cit;
        int pos = findLastOf(contentCode,tempChars);
        if(-1 == pos) {
            LOG(INFO)<< macAddr_.toStdString() << " OCRCharHandler::matchWord not found" << contentCode;
            break;
        }
        tempChars.remove(pos); //相同字不能使用同一個位置
        LOG(INFO) << macAddr_.toStdString() << " matchCurrentWord find pos=" << pos << ",last pos=" << lastPos;
        if(-1 == lastPos || 0 > comparePos(word->ocrChars_[lastPos],word->ocrChars_[pos])) { //这个要比上一个位置靠前，逆序查找的
            matchesPos.push_front(pos);
            lastPos = pos;
        } else { //顺序不匹配（逆序了）,前一个字坐标大于或等于后一个字
            break;
        }
    }

    if(word->contentCodes.size() == matchesPos.size()) {//成功匹配
        LOG(INFO) << macAddr_.toStdString() << "matchWord matched word group id=" << word->groupId;
        int i = 0;
        for(int pos : matchesPos) {
            word->ocrChars_[pos].correct = true;
            emit postOCRChar(word->groupId, i, word->contentCodes[i], word->ocrChars_[pos]);
            word->ocrChars_.remove(pos);
            ++i;
        }
        return true;
    }
    return false;
}

void OCRCharHandler::addOCRChar(int pageId, int xIndex, int yIndex, int zIndex,
                         int contentCode, QString contentName, int score, QString imgKey, QString imgPath)
{
    if(nullptr == currentWord_) {
        LOG(INFO) << macAddr_.toStdString() << " addOCRChar currentWord_ is null";
        return;
    }

    OCRChar oc;
    oc.pageId = pageId;
    oc.xIndex = xIndex;
    oc.yIndex = yIndex;
    oc.zIndex = zIndex;
    oc.ocrContentCode = contentCode;
    oc.score = score;
    oc.ocrContentName = contentName;
    oc.imgKey = imgKey;
    oc.imgPath = imgPath;
    oc.pos = charPos(oc);
    int ocrCharSize = 0;
    {
        QMutexLocker locker(&currentWordLocker_);
        currentWord_->ocrChars_.insert(oc.pos,oc);
        ocrCharSize = currentWord_->ocrChars_.size();
    }

    LOG(INFO) << macAddr_.toStdString() << " addOCRChar pos="
              << oc.pos << ",code=" << oc.ocrContentCode << ",key=" << oc.imgKey.toStdString()
              << ",path=" << oc.imgPath.toStdString() << ", insert size="
              << ocrCharSize;
    if(nullptr != lastWord_ && 1 == ocrCharSize) { //上一个词未完成且是第一个字
        firstCharPos_ = oc.pos;
    }
}

int OCRCharHandler::findLastOf(int contentCode, const QMap<int, OCRChar>& ocrChars)
{
    LOG(INFO) << macAddr_.toStdString() << "char size=" << ocrChars.size();
    int lastPos = -1;
    for(auto cit = ocrChars.cbegin(); cit != ocrChars.cend(); ++cit) {
        const OCRChar& ocrChar = cit.value();
        //0分不可信,直到找到最後一個
        if(0 != ocrChar.score && ocrChar.ocrContentCode == contentCode &&
                (-1 == lastPos || 0 < comparePos(ocrChars[lastPos],ocrChar))) {
            lastPos = cit.key();
        }
    }
    return lastPos;
}

void OCRCharHandler::checkFirstCharForLastWord(bool currentFinished)
{
    LOG(INFO) << macAddr_.toStdString() << "checkFirstCharForLastWord "
              << "lastWord_=" << lastWord_;
    //2022-03-17 经讨论，认为规则可以放开一点
    //当前词已处理完，且首字还属于未分配，且上一个词未完成
    if(-1 != firstCharPos_ && currentWord_->ocrChars_.contains(firstCharPos_)
            && nullptr != lastWord_) {
        OCRChar oc = currentWord_->ocrChars_[firstCharPos_];
        int sameWordCount = 0;
        for(const auto& tempOc : currentWord_->ocrChars_) {
            if(tempOc.ocrContentCode == oc.ocrContentCode) {
                ++sameWordCount;
            }
        }
        int needCount = currentWord_->contentCodes.count(oc.ocrContentCode);
        LOG(INFO) << macAddr_.toStdString() << " checkFirstCharForLastWord need="
                  << needCount << ", has=" << sameWordCount;

        if(currentFinished || (sameWordCount > needCount)) {
            currentWord_->ocrChars_.remove(firstCharPos_);
            lastWord_->ocrChars_.insert(oc.pos,oc);
            LOG(INFO) << macAddr_.toStdString() << "checkFirstCharForLastWord matched"
                      << lastWord_->groupId << ","
                      << lastWord_->ocrChars_.size();
            //检查上一个词现在是否可以完成
            bool finished = matchWord(lastWord_);
            LOG(INFO) << macAddr_.toStdString() << "checkFirstCharForLastWord last word finished="
                      << finished;
            if(finished) { //完成则从未完成队列删除
                if(unfinishedWords_.last() == lastWord_) { //double check,应该是一定相等的
                    unfinishedWords_.takeLast();
                } else {
                    LOG(WARNING) << macAddr_.toStdString()
                                 << "checkFirstCharForLastWord last word ptr error";
                }
                delete lastWord_;
            }
        }
    }
    firstCharPos_ = -1; //只判断一次，只用于该函数
    lastWord_ = nullptr; //只判断一次，只用于该函数
}

QList<OCRChar> OCRCharHandler::getSortedOCRChar(const QMap<int, OCRChar> &ocrChars)
{
    if(ocrChars.empty()) {
        return QList<OCRChar>();
    }
    //检查是否换过本子，只考虑中途换一本本子吧,换本子的逻辑是<5页以上,目前只做简单考虑，什么又叠加不考虑了
    int originPageId = ocrChars.first().pageId;
    bool changed = false;
    for(const auto& oc : ocrChars) {
        if(qAbs(originPageId-oc.pageId) >= 5) {
            changed = true;
            originPageId = qMax(originPageId,oc.pageId);
            break;
        }
    }
    LOG(INFO) << macAddr_.toStdString() << " getSortedOCRChar changed=" << changed << ",originPageId="
              << originPageId;

    const int kMaxLen = 1000*7*2*4;

    QMap<int,OCRChar> tempChars;
    for(const auto& oc : ocrChars) {
        if(oc.pageId < originPageId && (originPageId - oc.pageId) >= 5) {
            tempChars.insert(oc.pos+kMaxLen,oc);
        } else {
            tempChars.insert(oc.pos,oc);
        }
    }
    LOG(INFO) << macAddr_.toStdString() << " getSortedOCRChar sorted";
    printOCRChars(-1,tempChars);


    return tempChars.values();
}

void OCRCharHandler::finishAllWords()
{
    LOG(INFO) << macAddr_.toStdString() << "payne: finishAllWords";
    processCurrentWordOCRChar();
    LOG(INFO) << macAddr_.toStdString() << "payne: finishAllWords unfinishedWords_ size=" << unfinishedWords_.size();
    for(OCRWord* word : unfinishedWords_) {

        int wordLen = word->contentCodes.size();
        int charLen = word->ocrChars_.size();
        LOG(INFO) << macAddr_.toStdString() << "payne: group id=" << word->groupId << ",word size=" << wordLen
                  << ",char size=" << charLen;

        if(wordLen < 1) {
            continue;
        }
        auto tempList = getSortedOCRChar(word->ocrChars_);
        if(charLen == wordLen) {
            for(int i = 0; i < charLen; ++i) {
                if(tempList[i].score > 0 && tempList[i].ocrContentCode == word->contentCodes[i]) {
                    tempList[i].correct = true;
                } else {
                    tempList[i].correct = false;
                }
                emit postOCRChar(word->groupId,i,word->contentCodes[i], tempList[i]);
            }
        } else {
            bool inOrder = true;
            QMap<int,int> indexToPos;
            int lastPos = -1;
            auto tempChars = word->ocrChars_;
            int index = wordLen;
            for(auto cit = word->contentCodes.crbegin(); cit != word->contentCodes.crend(); ++cit) { //逆序查找
                --index;
                int contentCode = *cit;
                int pos = findLastOf(contentCode,tempChars);
                if(-1 == pos) {
                    LOG(INFO)<< macAddr_.toStdString() << " OCRCharHandler::finishAllWords not found" << contentCode;
                    continue;
                }
                tempChars.remove(pos); //相同字不能使用同一個位置
                LOG(INFO) << macAddr_.toStdString() << " matchCurrentWord find pos=" << pos << ",last pos=" << lastPos;
                if(-1 == lastPos || 0 > comparePos(word->ocrChars_[lastPos],word->ocrChars_[pos])) { //这个要比上一个位置靠前，逆序查找的
                    indexToPos.insert(index,pos);
                    lastPos = pos;
                } else { //顺序不匹配（逆序了）,前一个字坐标大于或等于后一个字
                    inOrder = false;
                    break;
                }
            }
            if(indexToPos.empty()) {
                inOrder = false;
            }
            if(inOrder) {
                QMap<int,int> finalIndexToPos;
                int indexLast = wordLen - 1;
                int tempLast = tempList.size() - 1;
                bool zeroIncluded = indexToPos.contains(0);
                while(!indexToPos.empty()) {
                    int matchesLast = indexToPos.last();
                    int matchesIndex = indexToPos.lastKey();
                    int tempIndex = 0;
                    for(int i = 0; i < charLen; ++i) {
                        if(tempList[i].pos == matchesLast) {
                            tempIndex = i;
                            break;
                        }
                    }
                    finalIndexToPos.insert(matchesIndex,tempIndex);
                    for(int j = indexLast, k = tempLast; j > matchesIndex && k > tempIndex; --j, --k) {
                        finalIndexToPos.insert(j,k);
                    }
                    indexToPos.remove(matchesIndex);
                    indexLast = matchesIndex - 1;
                    tempLast = tempIndex - 1;
                }
                //头部的字错了，没有在匹配列表里，需要补齐
                if(!zeroIncluded) {
                    for(int j = indexLast, k = tempLast; j >= 0 && k >= 0; --j, --k) {
                        finalIndexToPos.insert(j,k);
                    }
                }
                LOG(INFO) << macAddr_.toStdString() << " finishAllWords finalIndexToPos size=" << finalIndexToPos.size()
                          << ",wordLen=" << wordLen;
                for(int i = 0; i < wordLen; ++i) {
                    if(finalIndexToPos.contains(i)) {
                        int tempIdx = finalIndexToPos[i];
                        if(tempList[tempIdx].score > 0 && tempList[tempIdx].ocrContentCode == word->contentCodes[i]) {
                            tempList[tempIdx].correct = true;
                        } else {
                            tempList[tempIdx].correct = false;
                        }
                        emit postOCRChar(word->groupId,i,word->contentCodes[i], tempList[tempIdx]);
                    }
                }
            } else {
                int len = charLen >= wordLen ? wordLen : charLen;
                int pos = 0;
                if(charLen > len) {
                    pos = charLen - len; //取后面的
                }
                QList<OCRChar> targetList = tempList.mid(pos,len);
                for(int i = 0; i < len; ++i) {
                    if(targetList[i].score > 0 && targetList[i].ocrContentCode == word->contentCodes[i]) {
                        targetList[i].correct = true;
                    } else {
                        targetList[i].correct = false;
                    }
                    emit postOCRChar(word->groupId,i,word->contentCodes[i], targetList[i]);
                }
            }
        }
        delete word;
    }
    unfinishedWords_.clear();
    lastWord_ = nullptr;
}

void OCRCharHandler::matchUnfinishedWords()
{
    if(nullptr == currentWord_ || currentWord_->ocrChars_.empty() || 0 == unfinishedWords_.size()) {
        return;
    }
    LOG(INFO) << macAddr_.toStdString() << "matchUnfinishedWords unfinishedWords_ size="
              << unfinishedWords_.size();
    printOCRChars(currentWord_->groupId,currentWord_->ocrChars_);

    QSet<int> foundIndexes;

    for(int idx = 0; idx < unfinishedWords_.size(); ++idx) {
        //谁覆盖谁？后写的覆盖前面写的
        OCRWord* word = unfinishedWords_[idx];
        printOCRChars(word->groupId,word->ocrChars_);
        auto tempChars = word->ocrChars_;
        for(auto cit = currentWord_->ocrChars_.cbegin(); cit != currentWord_->ocrChars_.cend(); ++cit) {
            tempChars.insert(cit.key(),cit.value());
        }
        //全词匹配（顺序正确且相邻）
        int wordLen = word->contentCodes.size();
        int charListLen = tempChars.size();
        LOG(INFO) << macAddr_.toStdString() << " matchUnfinishedWords after merge " << word->groupId << ",word len="
                  << wordLen << ",char len=" << charListLen;
        printOCRChars(word->groupId,tempChars);
        QList<OCRChar> ocrCharList = getSortedOCRChar(tempChars);
        QList<OCRChar> targetList;
        if(charListLen >= wordLen) {
            bool found = true;
            for(int i = 0; i <= (charListLen - wordLen); ++i) {
                QList<OCRChar> tempList = ocrCharList.mid(i,wordLen);
                found = true;
                for(int j = 0; j < wordLen; ++j) {
                    if(0 == tempList[j].score || tempList[j].ocrContentCode != word->contentCodes[j]) {
                        found = false;
                        break;
                    }
                    if(j > 0 && 1 != comparePos(tempList[j-1],tempList[j])) {
                        LOG(INFO) << macAddr_.toStdString() << " matchUnfinishedWords 1111";
                        found = false;
                        break;
                    }
                }
                if(found) {
                    LOG(INFO) << macAddr_.toStdString() << " matchUnfinishedWords 2222";
                    targetList = tempList;
                    break;
                }
            }

            if(found) {
                LOG(INFO) << macAddr_.toStdString() << " matchUnfinishedWords found" << word->groupId;
                for(int i = 0; i < wordLen; ++i) {
                    targetList[i].correct = true;
                    emit postOCRChar(word->groupId,i,word->contentCodes[i], targetList[i]);
                    currentWord_->ocrChars_.remove(targetList[i].pos);
                }
                foundIndexes.insert(idx);
            }
        }
    }
    QVector<OCRWord*> tempWords;
    for(int i = 0; i < unfinishedWords_.size(); ++i) {
        if(foundIndexes.contains(i)) {
            auto word = unfinishedWords_[i];
            if(lastWord_ == word) {
                LOG(INFO) << macAddr_.toStdString() << " matchUnfinishedWords remove word:" << word;
                lastWord_ = nullptr;
            }
            delete word;
        } else {
            tempWords.push_back(unfinishedWords_[i]);
        }
    }
    unfinishedWords_.swap(tempWords);
}
