#ifndef OCRCHARLIST_H
#define OCRCHARLIST_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QList>
#include <QRecursiveMutex>

struct OCRChar
{
    int pageId;
    int xIndex;
    int yIndex;
    int zIndex;
    int ocrContentCode;
    int pos;
    int score;
    bool correct;
    QString imgKey;
    QString imgPath;
    QString ocrContentName;
};

struct OCRWord : public QObject
{
    int groupId;
    QVector<int> contentCodes;
    QMap<int, OCRChar> ocrChars_;
};

class OCRCharHandler : public QObject
{
    Q_OBJECT
public:
    explicit OCRCharHandler(QString macAddr, QObject *parent = nullptr);

    int charPos(OCRChar ocrChar);
    int comparePos(const OCRChar& lhs, const OCRChar &rhs);
    void handleNewDictationWord(int groupId, const QVector<int>& contentCodes);
    void addOCRChar(int pageId, int xIndex, int yIndex, int zIndex,
                    int contentCode, QString contentName, int score,  QString imgKey, QString imgPath);
    bool matchWord(OCRWord* word);
    void matchUnfinishedWords();
    void finishAllWords();
    int findLastOf(int contentCode, const QMap<int, OCRChar>& ocrChars);
    void checkFirstCharForLastWord(bool currentFinished);
    QList<OCRChar> getSortedOCRChar(const QMap<int, OCRChar>& ocrChars);

signals:
    void postOCRChar(int,int,int,OCRChar);

private:
    QString macAddr_;
    QVector<OCRWord*> unfinishedWords_;
    OCRWord* currentWord_ = nullptr;
    OCRWord* lastWord_ = nullptr;
    int firstCharPos_ = -1;
    void processCurrentWordOCRChar();
    void printOCRChars(int groupId, const QMap<int, OCRChar>& ocrChars);
    QRecursiveMutex currentWordLocker_;
};

#endif // OCRCHARLIST_H
